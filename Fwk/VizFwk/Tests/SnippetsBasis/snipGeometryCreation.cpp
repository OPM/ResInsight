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

#include "snipGeometryCreation.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool GeometryCreation::onInitialize()
{
    ref<ModelBasicList> myModel = new ModelBasicList;

    // Simple patch
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createPatch(Vec3f(10, 0, 0), Vec3f::X_AXIS, Vec3f::Y_AXIS, 6, 4, &builder);
        myModel->addPart(createPartFromBuilder(builder, Color3f::GREEN).p());
    }

    // Patch(es) using generator
    {
        PatchGenerator gen;
        gen.setOrigin(Vec3d(10, 5, 0));
        gen.setExtent(6, 4);
        gen.setSubdivisions(6, 4);
        {
            GeometryBuilderFaceList builder;
            gen.generate(&builder);
            myModel->addPart(createPartFromBuilder(builder, Color3f::GREEN).p());
        }
    
        gen.setOrigin(Vec3d(10, 5, -1));
        gen.setQuads(false);
        gen.setWindingCCW(false);
        {
            GeometryBuilderFaceList builder;
            gen.generate(&builder);
            myModel->addPart(createPartFromBuilder(builder, Color3f::GREEN).p());
        }
    }

    // Patch with non perpendicular axes
    {
        PatchGenerator gen;
        gen.setAxes(Vec3d(1, 0, 0).getNormalized(), Vec3d(-1, 1, 0).getNormalized());
        gen.setOrigin(Vec3d(10, 10, 0));
        gen.setExtent(6, 4);
        gen.setSubdivisions(6, 4);
        {
            GeometryBuilderFaceList builder;
            gen.generate(&builder);
            myModel->addPart(createPartFromBuilder(builder, Color3f::GREEN).p());
        }
    }


    // Simple boxes
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createBox(Vec3f(4, -1, 0), Vec3f(8, 3, 4), &builder);
        myModel->addPart(createPartFromBuilder(builder, Color3f::BLUE).p());
    }

    // Box with subdivisions
    {
        BoxGenerator gen;
        gen.setMinMax(Vec3d(4, 4, 0), Vec3d(8, 8, 4));
        gen.setSubdivisions(2, 4, 6);

        GeometryBuilderFaceList builder;
        gen.generate(&builder);
        myModel->addPart(createPartFromBuilder(builder, Color3f::BLUE).p());
    }

    // Sphere
    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(2, 20, 20, &builder);
        myModel->addPart(createPartFromBuilder(builder, Color3f::DARK_RED).p());
    }


    // Sphere without shared nodes (flat shading)
    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createSphere(2, 20, 20, &builder);
        builder.transformVertexRange(0, builder.vertexCount() - 1, Mat4f::fromTranslation(Vec3f(0, 5, 0)));

        ref<DrawableGeo> geo = createGeoWitoutSharedNodes(builder.drawableGeo().p());
        geo->computeNormals();

        ref<Effect> eff = new Effect;
        eff->setRenderState(new RenderStateMaterial_FF(Color3f::DARK_RED));

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());

        myModel->addPart(part.p());
    }

    
    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->rendering(0)->scene()->addModel(myModel.p());

    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    m_renderSequence->firstRendering()->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    addEdgesRendering();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryCreation::onKeyPressEvent(KeyEvent* keyEvent)
{
    Key key = keyEvent->key();
    char character = keyEvent->character();

    if (key == Key_M)
    {
        bool edgesOn = (character == 'm') ? true : false;
        if (edgesOn)
        {
            if (m_renderSequence->renderingCount() == 1)
            {
                addEdgesRendering();
            }
        }
        else
        {
            if (m_renderSequence->renderingCount() == 2)
            {
                Rendering* renderingToRemove = m_renderSequence->rendering(1);
                m_renderSequence->removeRendering(renderingToRemove);
            }
        }
    }

    keyEvent->setRequestedAction(REDRAW);
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void GeometryCreation::addEdgesRendering()
{
    ref<RenderStatePolygonOffset> polyOffset = new RenderStatePolygonOffset;
    polyOffset->configureLineNegativeOffset();

    ref<Effect> lineEffect = new Effect;
    lineEffect->setRenderState(new RenderStatePolygonMode(RenderStatePolygonMode::LINE));
    lineEffect->setRenderState(polyOffset.p());
    lineEffect->setRenderState(new RenderStateDepth(true, RenderStateDepth::LEQUAL));

    ref<RenderStateMaterial_FF> lineMat = new RenderStateMaterial_FF;
    lineMat->setAmbientAndDiffuse(Color3f::WHITE);
    lineEffect->setRenderState(lineMat.p());
    lineEffect->setRenderState(new RenderStateLighting_FF(false));

    ref<Rendering> newRendering = new Rendering;
    newRendering->setCamera(m_camera.p());
    newRendering->setEffectOverride(lineEffect.p());
    newRendering->setScene(m_renderSequence->rendering(0)->scene());
    newRendering->setClearMode(Viewport::DO_NOT_CLEAR);

    m_renderSequence->addRendering(newRendering.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Part> GeometryCreation::createPartFromBuilder(const GeometryBuilderFaceList& builder, const Color3f& color)
{
    ref<DrawableGeo> geo = new DrawableGeo;

    ref<Vec3fArray> vertices = builder.vertices();
    ref<UIntArray> faceList = builder.faceList();

    geo->setVertexArray(vertices.p());
    geo->setFromFaceList(*faceList);
    geo->computeNormals();

    ref<Part> part = new Part;
    part->setDrawable(geo.p());

    ref<Effect> eff = new Effect;
    eff->setRenderState(new RenderStateMaterial_FF(color));
    part->setEffect(eff.p());

    return part;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> GeometryCreation::createGeoWitoutSharedNodes(const DrawableGeo* srcGeo)
{
    size_t numFaces = srcGeo->faceCount();
    const Vec3fArray* srcVertices = srcGeo->vertexArray();

    GeometryBuilderDrawableGeo builder;

    UIntArray faceIndices;
    size_t i;
    for (i = 0; i < numFaces; i++)
    {
        srcGeo->getFaceIndices(i, &faceIndices);

        if (faceIndices.size() == 3)
        {
            builder.addTriangleByVertices(srcVertices->get(faceIndices[0]), srcVertices->get(faceIndices[1]), srcVertices->get(faceIndices[2]));
        }
        else if (faceIndices.size() == 4)
        {
            builder.addQuadByVertices(srcVertices->get(faceIndices[0]), srcVertices->get(faceIndices[1]), srcVertices->get(faceIndices[2]), srcVertices->get(faceIndices[3]));
        }
    }

    return builder.drawableGeo();
}


} // namespace snip

