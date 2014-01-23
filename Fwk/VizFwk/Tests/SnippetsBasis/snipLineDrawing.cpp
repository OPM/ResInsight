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

#include "snipLineDrawing.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool LineDrawing::onInitialize()
{
    ref<ModelBasicList> myModel = new ModelBasicList;

    {
        // Use a high number to test effect of BOing the vertex attribute
        ref<Vec3fArray> spinePoints = generatePointsOnCircle(4, Math::toRadians(340.0f), 200);
        //ref<Vec3fArray> spinePoints = generatePointsOnCircle(4, Math::toRadians(340.0f), 2000000);
        //ref<Vec3fArray> spinePoints = generateDummySpine();
        ref<DrawableGeo> geo = createGeoFromSpine(*spinePoints, true);

        ShaderProgramGenerator gen("snipLineDrawing", ShaderSourceProvider::instance());
        gen.addVertexCodeFromFile("WideLines_Vert");
        gen.addFragmentCode(ShaderSourceRepository::src_Color);
        gen.addFragmentCodeFromFile("WideLines_Frag");

        ref<Effect> eff = new Effect;
        eff->setShaderProgram(gen.generate().p());
        eff->setUniform(new UniformFloat("u_color", Color4f(Color3::YELLOW)));
        eff->setUniform(new UniformFloat("u_lineRadius", 0.2f));

        ref<RenderStateBlending> blending = new RenderStateBlending;
        blending->configureTransparencyBlending();
        eff->setRenderState(blending.p());

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());
        myModel->addPart(part.p());
    }

    addPartsComparingThickLineToCylinder(myModel.p());
    addPartWithThickLineAndPixelWidth(myModel.p());

    addPartsWithNormalLines(myModel.p());

    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());

    m_renderSequence->firstRendering()->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

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
void LineDrawing::addPartsComparingThickLineToCylinder(ModelBasicList* model)
{
    const float radius = 1.0f;
    const float lineLength = 5.0f;

    // The thick line
    {
        ref<Vec3fArray> spinePoints = new Vec3fArray;
        spinePoints->reserve(2);
        spinePoints->add(Vec3f(0, 0, 0));
        spinePoints->add(Vec3f(0, 0, lineLength));

        ref<DrawableGeo> geo = createGeoFromSpine(*spinePoints, false);

        ShaderProgramGenerator gen("thickLineVsCylinder", ShaderSourceProvider::instance());
        gen.addVertexCodeFromFile("WideLines_Vert");
        gen.addFragmentCode(ShaderSourceRepository::src_Color);
        gen.addFragmentCodeFromFile("WideLines_Frag");

        ref<Effect> eff = new Effect;
        eff->setShaderProgram(gen.generate().p());
        eff->setUniform(new UniformFloat("u_color", Color4f(Color3::GREEN)));
        eff->setUniform(new UniformFloat("u_lineRadius", radius));

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    // A true cylinder
    {
        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createObliqueCylinder(radius, radius, lineLength, 0, 0, 10, true, false, false, 1, &builder);

        ref<DrawableGeo> geo = builder.drawableGeo();
        geo->computeNormals();

        ref<Effect> eff = new Effect;
        eff->setRenderState(new RenderStateMaterial_FF(Color3f::RED));

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LineDrawing::addPartWithThickLineAndPixelWidth(ModelBasicList* model)
{
    const float lineWidthPixels = 10.0f;

    ref<Vec3fArray> spinePoints = new Vec3fArray;
    spinePoints->reserve(2);
    spinePoints->add(Vec3f(-5, 2, 0));
    spinePoints->add(Vec3f(5, 2, 0));

    ref<DrawableGeo> geo = createGeoFromSpine(*spinePoints, false);

    ShaderProgramGenerator gen("thickLineVsCylinder", ShaderSourceProvider::instance());
    gen.addVertexCode("#define PIXEL_WIDTH");
    gen.addVertexCodeFromFile("WideLines_Vert");
    gen.addFragmentCode(ShaderSourceRepository::src_Color);
    gen.addFragmentCodeFromFile("WideLines_Frag");

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(gen.generate().p());
    eff->setUniform(new UniformFloat("u_color", Color4f(Color3::BLUE)));
    eff->setUniform(new UniformFloat("u_lineWidthPixels", lineWidthPixels));

    ref<Part> part = new Part;
    part->setDrawable(geo.p());
    part->setEffect(eff.p());

    model->addPart(part.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LineDrawing::addPartsWithNormalLines(ModelBasicList* model)
{
    ShaderProgramGenerator gen("normalLine", ShaderSourceProvider::instance());
    gen.addVertexCode(ShaderSourceRepository::vs_Minimal);
    gen.addFragmentCode(ShaderSourceRepository::src_Color);
    gen.addFragmentCode(ShaderSourceRepository::fs_Unlit);
    ref<ShaderProgram> prog = gen.generate();

    // Make sure these lines are drawn last
    const int visualPriority = 99;

    float lineWidths[] = { 0.5f, 1.0f, 1.3f, 2.0f, 3.0f, 4.0f, 5.0f, 10.0f, 20.0f };
    size_t numLines = sizeof(lineWidths)/sizeof(float);

    // Normal lines
    for (size_t i = 0; i < numLines; i++)
    {
        ref<DrawableGeo> geo = createLineGeo(Vec3f(2 + i*0.5f, 0, 1), Vec3f(0, 4, 0));

        ref<Effect> eff = new Effect;
        eff->setShaderProgram(prog.p());
        eff->setUniform(new UniformFloat("u_color", Color4f(Color3::WHITE)));
        eff->setRenderState(new RenderStateLine(lineWidths[i]));

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());
        part->setPriority(visualPriority);
        model->addPart(part.p());
    }

    // Lines with smoothing antialiasing using LINE_SMOOTH and blending
    for (size_t i = 0; i < numLines; i++)
    {
        ref<DrawableGeo> geo = createLineGeo(Vec3f(2 + i*0.5f, 0, 1), Vec3f(0, -4, 0));

        ref<Effect> eff = new Effect;
        eff->setShaderProgram(prog.p());
        eff->setUniform(new UniformFloat("u_color", Color4f(Color3::WHITE)));

        ref<RenderStateLine> line = new RenderStateLine(lineWidths[i]);
        line->enableSmooth(true);
        eff->setRenderState(line.p());

        ref<RenderStateBlending> blending = new RenderStateBlending;
        blending->enableBlending(true);
        blending->setFunction(RenderStateBlending::SRC_ALPHA, RenderStateBlending::ONE_MINUS_SRC_ALPHA);
        eff->setRenderState(blending.p());

        ref<Part> part = new Part;
        part->setDrawable(geo.p());
        part->setEffect(eff.p());
        part->setPriority(visualPriority);
        model->addPart(part.p());
    }
}


//--------------------------------------------------------------------------------------------------
/// Create a geo with line starting at startPoint and ending at startPoint + vec
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> LineDrawing::createLineGeo(const Vec3f& startPoint, const Vec3f& vec)
{
    ref<Vec3fArray> vertices = new Vec3fArray;
    vertices->reserve(2);
    vertices->add(startPoint);
    vertices->add(startPoint + vec);

    ref<PrimitiveSetDirect> primSet = new PrimitiveSetDirect(PT_LINES);
    primSet->setStartIndex(0);
    primSet->setIndexCount(vertices->size());

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    geo->addPrimitiveSet(primSet.p());

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<DrawableGeo> LineDrawing::createGeoFromSpine(const Vec3fArray& spinePoints, bool roundedEnds)
{
    size_t numSpinePoints = spinePoints.size();
    CVF_ASSERT(numSpinePoints >= 2);

    size_t numVertices = 2*numSpinePoints;

    ref<Vec3fArray> vertices = new Vec3fArray;
    ref<Vec3fArray> fwdVectors = new Vec3fArray;
    ref<FloatArray> alphaFactors = new FloatArray;
    ref<Vec2fArray> circleFactors = new Vec2fArray;
    //ref<Vec2fArray> texCoords = new Vec2fArray;
    ref<UIntArray> indices = new UIntArray;
    vertices->reserve(numVertices);
    fwdVectors->reserve(numVertices);
    alphaFactors->reserve(numVertices);
    circleFactors->reserve(numVertices);
    //texCoords->reserve(numVertices);
    indices->reserve(numVertices);

    size_t i;
    for (i = 0; i < numSpinePoints; i++)
    {
        const Vec3f& pt = spinePoints[i];
        const Vec3f& prevPt = spinePoints[i > 0 ? i - 1 : i];
        const Vec3f& nextPt = spinePoints[i < numSpinePoints - 1 ? i + 1 : i];
        Vec3f fwdVec = (nextPt - prevPt).getNormalized();

        vertices->add(pt);
        vertices->add(pt);
        fwdVectors->add(fwdVec);
        fwdVectors->add(fwdVec);

        float alpha = CVF_MAX(0.2f, 1.0f - (static_cast<float>(i)/numSpinePoints));
        alphaFactors->add(alpha);
        alphaFactors->add(alpha);

        if (roundedEnds && (i == 0 || i == numSpinePoints - 1))
        {
            if (i == 0)
            {
                circleFactors->add(Vec2f(-1, -1));
                circleFactors->add(Vec2f(1, -1));
            }
            else
            {
                circleFactors->add(Vec2f(-1, 1));
                circleFactors->add(Vec2f(1, 1));
            }
        }
        else
        {
            circleFactors->add(Vec2f(-1, 0));
            circleFactors->add(Vec2f(1, 0));
        }

        //texCoords->add(Vec2f(-1, alpha));
        //texCoords->add(Vec2f( 1, alpha));

        indices->add(static_cast<cvf::uint>(2*i));
        indices->add(static_cast<cvf::uint>(2*i + 1));
    }

    ref<PrimitiveSetIndexedUInt> primSet = new PrimitiveSetIndexedUInt(PT_TRIANGLE_STRIP);
    primSet->setIndices(indices.p());

    ref<DrawableGeo> geo = new DrawableGeo;
    geo->setVertexArray(vertices.p());
    //geo->setTextureCoordArray(texCoords.p());
    
    ref<Vec3fVertexAttribute> fwdVectorAttrib = new Vec3fVertexAttribute("a_fwdVector", fwdVectors.p());
    ref<FloatVertexAttribute> alphaFactorAttrib = new FloatVertexAttribute("a_alpha", alphaFactors.p());
    ref<Vec2fVertexAttribute> circleFactorsAttrib = new Vec2fVertexAttribute("a_circleFactors", circleFactors.p());
    geo->setVertexAttribute(fwdVectorAttrib.p());
    geo->setVertexAttribute(alphaFactorAttrib.p());
    geo->setVertexAttribute(circleFactorsAttrib.p());
    
    geo->addPrimitiveSet(primSet.p());

    return geo;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> LineDrawing::generatePointsOnCircle(float radius, float sweepAngle, cvf::uint numPoints)
{
    ref<Vec3fArray> points = new Vec3fArray;
    points->reserve(numPoints);

    float da = static_cast<float>(sweepAngle/numPoints);
    Vec3f point = Vec3f::ZERO;

    size_t i;
    for (i = 0; i < numPoints; i++) 
    {
        float sinA = Math::sin(i*da);
        float cosA = Math::cos(i*da);

        point.x() = -sinA*radius;
        point.y() = cosA*radius;

        points->add(point);
    }

    return points;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Vec3fArray> LineDrawing::generateDummySpine()
{
    ref<Vec3fArray> spinePoints = new Vec3fArray;
    spinePoints->reserve(10);
    spinePoints->add(Vec3f(0, 0, 0));
    spinePoints->add(Vec3f(1, 0, 0));
    spinePoints->add(Vec3f(2, 1, 0));
    spinePoints->add(Vec3f(3, 1.1f, 0));

    return spinePoints;
}




} // namespace snip

