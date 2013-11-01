//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################


#include "cvfLibCore.h"
#include "cvfLibGeometry.h"
#include "cvfLibRender.h"
#include "cvfLibViewing.h"

#include "cvfuInputEvents.h"
#include "cvfuSnippetPropertyPublisher.h"

#include "snipDrawableAnimation.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DrawableAnimation::DrawableAnimation()
{
    m_lastAnimUpdateTimeStamp = 0;
    m_geometryCompletionRatio = 0.5;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool DrawableAnimation::onInitialize()
{
    m_propUpdateType = new PropertyEnum("Update type");
    m_propUpdateType->addItem("ScopeOnly",          "Set scope only");
    m_propUpdateType->addItem("RespecifyIndices",   "Respecify indices");
    m_propUpdateType->addItem("NewIndices",         "New indices");
    m_propertyPublisher->publishProperty(m_propUpdateType.p());

    std::vector<Color4f> colors;

    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createSphere(2, 500, 500, &builder);
        ref<DrawableGeo> tempGeo = builder.drawableGeo();

        SrcData srcData = extractSrcDataFromGeo(tempGeo.p());
        m_srcData.push_back(srcData);
        colors.push_back(Color4f(Color3::ORANGE));
    }

    {
        const size_t cellCount = 1000;
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createPatch(Vec3f(3, 0, 0), Vec3f::X_AXIS*(4.0f/cellCount), Vec3f::Y_AXIS*(4.0f/cellCount), cellCount, cellCount, &builder);
        ref<DrawableGeo> tempGeo = builder.drawableGeo();

        SrcData srcData = extractSrcDataFromGeo(tempGeo.p());
        m_srcData.push_back(srcData);
        colors.push_back(Color4f(Color3::DARK_CYAN));
    }

    {
        BoxGenerator gen;
        gen.setCenterAndExtent(Vec3d(0, 5, 0), Vec3d(2, 2, 2));
        gen.setSubdivisions(800, 400, 200);

        GeometryBuilderDrawableGeo builder;
        gen.generate(&builder);
        ref<DrawableGeo> tempGeo = builder.drawableGeo();
        SrcData srcData = extractSrcDataFromGeo(tempGeo.p());
        m_srcData.push_back(srcData);
        colors.push_back(Color4f(Color3::DARK_GREEN));
    }


    cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    gen.configureStandardHeadlightColor();
    ref<ShaderProgram> prog = gen.generate();

    m_model = new ModelBasicList;

    size_t numParts = m_srcData.size();
    size_t ip;
    for (ip = 0; ip < numParts; ip++)
    {
        SrcData srcData = m_srcData[ip];
        ref<DrawableGeo> geo = createGeoFromSrcData(srcData, 1.0);

        ref<Effect> eff = new Effect;
        eff->setShaderProgram(prog.p());
        eff->setUniform(new UniformFloat("u_color", colors[ip]));

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());

        m_model->addPart(part.p());
    }

    m_model->updateBoundingBoxesRecursive();
    m_renderSequence->firstRendering()->scene()->addModel(m_model.p());

    BoundingBox bb = m_model->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
DrawableAnimation::SrcData DrawableAnimation::extractSrcDataFromGeo(DrawableGeo* geo)
{
    CVF_ASSERT(geo);
    CVF_ASSERT(geo->primitiveSetCount() > 0);

    if (!geo->normalArray())
    {
        geo->computeNormals();
    }

    SrcData srcData;
    srcData.vertices = new Vec3fArray(*(geo->vertexArray()));
    srcData.normals = new Vec3fArray(*(geo->normalArray()));

    PrimitiveSet* primSet = geo->primitiveSet(0);
    size_t numIndices = primSet->indexCount();
    srcData.indices = new UIntArray(numIndices);
    size_t i;
    for (i = 0; i < numIndices; i++)
    {
        srcData.indices->set(i, primSet->index((i)));
    }

    return srcData;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> DrawableAnimation::createGeoFromSrcData(SrcData srcData, double geometryCompletionRatio)
{
    const size_t maxNumTris = srcData.indices->size()/3;
    const size_t numTris = CVF_MIN(maxNumTris, static_cast<size_t>(geometryCompletionRatio*static_cast<double>(maxNumTris) + 0.5));

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(srcData.vertices.p());
    geo->setNormalArray(srcData.normals.p());

    PrimitiveSetIndexedUIntScoped* primSet = new PrimitiveSetIndexedUIntScoped(PT_TRIANGLES);
    primSet->setIndices(srcData.indices.p(), 0, 3*numTris);
    geo->addPrimitiveSet(primSet);

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableAnimation::updateGeo_ScopeOnly(SrcData srcData, double geometryCompletionRatio, DrawableGeo* geo)
{
    CVF_ASSERT(geo);

    CVF_ASSERT(geo->vertexArray() == srcData.vertices);
    CVF_ASSERT(geo->normalArray() == srcData.normals);

    const size_t maxNumTris = srcData.indices->size()/3;
    const size_t numTris = CVF_MIN(maxNumTris, static_cast<size_t>(geometryCompletionRatio*static_cast<double>(maxNumTris) + 0.5));

    CVF_ASSERT(geo->primitiveSetCount() >= 1);
    PrimitiveSetIndexedUIntScoped* primSet = dynamic_cast<PrimitiveSetIndexedUIntScoped*>(geo->primitiveSet(0));
    CVF_ASSERT(primSet);

    primSet->setScope(0, 3*numTris);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableAnimation::updateGeo_RespecifyIndices(SrcData srcData, double geometryCompletionRatio, DrawableGeo* geo)
{
    CVF_ASSERT(geo);

    CVF_ASSERT(geo->vertexArray() == srcData.vertices);
    CVF_ASSERT(geo->normalArray() == srcData.normals);

    const size_t maxNumTris = srcData.indices->size()/3;
    const size_t numTris = CVF_MIN(maxNumTris, static_cast<size_t>(geometryCompletionRatio*static_cast<double>(maxNumTris) + 0.5));

    CVF_ASSERT(geo->primitiveSetCount() >= 1);
    PrimitiveSetIndexedUIntScoped* primSet = dynamic_cast<PrimitiveSetIndexedUIntScoped*>(geo->primitiveSet(0));
    CVF_ASSERT(primSet);

    primSet->setIndices(srcData.indices.p(), 0, 3*numTris);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableAnimation::updateGeo_NewIndices(SrcData srcData, double geometryCompletionRatio, DrawableGeo* geo)
{
    CVF_ASSERT(geo);

    CVF_ASSERT(geo->vertexArray() == srcData.vertices);
    CVF_ASSERT(geo->normalArray() == srcData.normals);

    const size_t maxNumTris = srcData.indices->size()/3;
    const size_t numTris = CVF_MIN(maxNumTris, static_cast<size_t>(geometryCompletionRatio*static_cast<double>(maxNumTris) + 0.5));

    CVF_ASSERT(geo->primitiveSetCount() >= 1);
    PrimitiveSetIndexedUIntScoped* primSet = dynamic_cast<PrimitiveSetIndexedUIntScoped*>(geo->primitiveSet(0));
    CVF_ASSERT(primSet);

    if (numTris > 0)
    {
        ref<UIntArray> indices = new UIntArray(3*numTris);
        indices->copyData(*srcData.indices.p(), 3*numTris, 0, 0);
        primSet->setIndices(indices.p(), 0, indices->size());
    }
    else
    {
        primSet->setIndices(NULL, 0, 0);
    }
    
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableAnimation::onUpdateAnimation(double animationTime, PostEventAction* postEventAction)
{
    *postEventAction = NONE;

    const int numSteps = 1000;
    const double maxFps = 50.0;
    double frameChangeDelta = 1.0/maxFps;
    if (Math::abs(animationTime - m_lastAnimUpdateTimeStamp) > frameChangeDelta)
    {
        m_geometryCompletionRatio += (1.0/numSteps);
        if (m_geometryCompletionRatio > 1.0) m_geometryCompletionRatio = 0;

        size_t numParts = m_model->partCount();
        CVF_ASSERT(numParts == m_srcData.size());
        size_t ip;
        for (ip = 0; ip < numParts; ip++)
        {
            SrcData srcData = m_srcData[ip];
            Part* part = m_model->part(ip);
            DrawableGeo* geo = dynamic_cast<DrawableGeo*>(part->drawable());
            CVF_ASSERT(geo);

            String currIdent = m_propUpdateType->currentIdent();
            if (currIdent == "ScopeOnly")
            {
                updateGeo_ScopeOnly(srcData, m_geometryCompletionRatio, geo);
            }
            else if (currIdent == "RespecifyIndices")
            {
                updateGeo_RespecifyIndices(srcData, m_geometryCompletionRatio, geo);
            }
            else if (currIdent == "NewIndices")
            {
                updateGeo_NewIndices(srcData, m_geometryCompletionRatio, geo);
            }
        }

        *postEventAction = REDRAW;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void DrawableAnimation::onPropertyChanged(Property* property, PostEventAction* postEventAction)
{
    if (property == m_propUpdateType)
    {
        m_geometryCompletionRatio = 0;

        size_t numParts = m_model->partCount();
        CVF_ASSERT(numParts == m_srcData.size());
        size_t ip;
        for (ip = 0; ip < numParts; ip++)
        {
            Part* part = m_model->part(ip);
            DrawableGeo* oldGeo = dynamic_cast<DrawableGeo*>(part->drawable());
            CVF_ASSERT(oldGeo);
            DrawableGeo::RenderMode renderMode = oldGeo->renderMode();

            SrcData srcData = m_srcData[ip];
            ref<DrawableGeo> geo = createGeoFromSrcData(srcData, 1.0);
            geo->setRenderMode(renderMode);

            part->setDrawable(0, geo.p());
        }
    }

    *postEventAction = REDRAW;
}





} // namespace snip

