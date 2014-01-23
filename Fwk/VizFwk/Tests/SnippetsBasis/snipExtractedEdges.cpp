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
#include "cvfLibRender.h"
#include "cvfLibGeometry.h"
#include "cvfLibViewing.h"

#include "cvfuInputEvents.h"

#include "snipExtractedEdges.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ExtractedEdges::ExtractedEdges()
{
    m_useShaders = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ExtractedEdges::onInitialize()
{
    // Must set up model and rendering first
    ref<ModelBasicList> myModel = new ModelBasicList;

    // Disable all sorting (except priority) so we have control over draw order
    ref<Rendering> rendering = m_renderSequence->firstRendering();
    rendering->setRenderQueueSorter(new cvf::RenderQueueSorterBasic(cvf::RenderQueueSorterBasic::MINIMAL));
    rendering->scene()->addModel(myModel.p());
    rendering->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    // Box
    {
        BoxGenerator gen;
        gen.setCenterAndExtent(Vec3d(0, 0, 0), Vec3d(4, 4, 4));
        gen.setSubdivisions(12, 8, 4);
        GeometryBuilderFaceList builder;
        gen.generate(&builder);
        ref<Vec3fArray> vertexArray = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        //Experiment by splitting vertices at sharp edges
        //In order to try it, we must first weld the vertices
        //weldVertices(vertexArray.p(), faceList.p());
        //splitVerticesAtSharpEdges(vertexArray.p(), faceList.p());

        {
            // Box surface
            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->setFromFaceList(*faceList);
            geo->computeNormals();
            addAsSurfacePart(geo.p(), Color3::YELLOW);
        }
        {
            // Box outline
            OutlineEdgeExtractor ee(Math::toRadians(60.0), *vertexArray);
            ee.addFaceList(*faceList);

            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->addPrimitiveSet(new PrimitiveSetIndexedUInt(PT_LINES, ee.lineIndices().p()));
            addAsLinesPart(geo.p(), Color3::RED);
        }
        {
            // Box mesh
            MeshEdgeExtractor ee;
            ee.addFaceList(*faceList);

            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->addPrimitiveSet(new PrimitiveSetIndexedUInt(PT_LINES, ee.lineIndices().p()));
            addAsLinesPart(geo.p(), Color3::DARK_CYAN);
        }
    }

    myModel->updateBoundingBoxesRecursive();
    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ExtractedEdges::addAsSurfacePart(DrawableGeo* geo, const Color3f& color)
{
    ModelBasicList* model = dynamic_cast<ModelBasicList*>(m_renderSequence->firstRendering()->scene()->model(0));

    ref<Effect> eff = new Effect;

    cvf::RenderStatePolygonOffset* polyOffset = new cvf::RenderStatePolygonOffset;
    polyOffset->configurePolygonPositiveOffset();
    eff->setRenderState(polyOffset);

    if (m_useShaders)
    {
        ShaderProgramGenerator gen("surfaceProgram", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_Standard);
        gen.addFragmentCode(ShaderSourceRepository::src_Color);
        gen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
        gen.addFragmentCode(ShaderSourceRepository::fs_Standard);

        eff->setShaderProgram(gen.generate().p());
        eff->setUniform(new UniformFloat("u_color", Color4f(color)));
    }
    else
    {
        eff->setRenderState(new cvf::RenderStateMaterial_FF(color));
    }

    ref<Part> part = new Part;
    part->setDrawable(geo);
    part->setEffect(eff.p());

    model->addPart(part.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ExtractedEdges::addAsLinesPart(DrawableGeo* geo, const Color3f& color)
{
    ModelBasicList* model = dynamic_cast<ModelBasicList*>(m_renderSequence->firstRendering()->scene()->model(0));

    ref<Effect> eff = new Effect;
    //eff->setRenderState(new cvf::RenderStateDepth(true, cvf::RenderStateDepth::LEQUAL));

    if (m_useShaders)
    {
        ShaderProgramGenerator gen("lineProgram", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_Standard);
        gen.addFragmentCode(ShaderSourceRepository::src_Color);
        gen.addFragmentCode(ShaderSourceRepository::fs_Unlit);

        eff->setShaderProgram(gen.generate().p());
        eff->setUniform(new UniformFloat("u_color", Color4f(color)));
    }
    else
    {
        eff->setRenderState(new cvf::RenderStateMaterial_FF(color));
        eff->setRenderState(new cvf::RenderStateLighting_FF(false));
    }

    ref<Part> part = new Part;
    part->setDrawable(geo);
    part->setEffect(eff.p());

    model->addPart(part.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ExtractedEdges::weldVertices(Vec3fArray* vertexArray, UIntArray* faceList)
{
    VertexWelder welder;
    welder.initialize(0.0001, 0.1, 100);

    size_t i = 0;
    while (i < faceList->size())
    {
        cvf::uint numVerts = faceList->get(i++);
        size_t j;
        for (j = 0; j < numVerts; j++)
        {
            cvf::uint idx = faceList->get(i);
            idx = welder.weldVertex(vertexArray->get(idx), NULL);
            faceList->set(i, idx);
            i++;
        }
    }

    ref<Vec3fArray> newVertexArray = welder.createVertexArray();
    vertexArray->resize(newVertexArray->size());
    vertexArray->copyData(*newVertexArray, newVertexArray->size(), 0, 0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ExtractedEdges::splitVerticesAtSharpEdges(Vec3fArray* vertexArray, UIntArray* faceList)
{
    std::vector<cvf::uint> triConn;

    size_t i = 0;
    while (i < faceList->size())
    {
        cvf::uint numVerts = faceList->get(i++);
        size_t j;
        for (j = 0; j < numVerts - 2; j++)
        {
            triConn.push_back(faceList->get(i));
            triConn.push_back(faceList->get(i + j + 1));
            triConn.push_back(faceList->get(i + j + 2));
        }

        i += numVerts;
    }

    // Note! Does not work
    //TriangleVertexSplitter tvs(Math::toRadians(60.0), UIntArray (triConn), *vertexArray);

    UIntArray triConn2(triConn);
    TriangleVertexSplitter tvs(Math::toRadians(60.0), triConn2, *vertexArray);

    ref<UIntArray> newTriConn = tvs.triangleIndices();
    ref<Vec3fArray> newVertexArr = tvs.vertexArray();

    size_t t = 0;
    i = 0;
    while (i < faceList->size())
    {
        cvf::uint numVerts = faceList->get(i++);

        faceList->set(i++, newTriConn->get(t));
        faceList->set(i++, newTriConn->get(t + 1));

        size_t j;
        for (j = 0; j < numVerts - 2; j++)
        {
            faceList->set(i++, newTriConn->get(t + 2));
            t += 3;
        }
    }

    vertexArray->resize(newVertexArr->size());
    vertexArray->copyData(*newVertexArr, newVertexArr->size(), 0, 0);
}


} // namespace snip

