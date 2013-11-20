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
#include "cvfuSnippetPropertyPublisher.h"

#include "snipVisualPriority.h"

namespace snip {

static const double DEF_NEAR_PLANE = 0.1;
static const double DEF_FAR_PLANE = 10000;


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool VisualPriority::onInitialize()
{
    m_propUseShaders = new PropertyBool("Use shaders", m_openGLContext->capabilities()->supportsOpenGL2());
    m_propAutomaticClipping = new PropertyBool("Automatic clipping", true);
    m_propUsePolygonOffset = new PropertyBool("Use polygon offset", true);
    m_propUseEyeLift = new PropertyBool("Use eye lift", true);
    m_propertyPublisher->publishProperty(m_propUseShaders.p());
    m_propertyPublisher->publishProperty(m_propAutomaticClipping.p());
    m_propertyPublisher->publishProperty(m_propUsePolygonOffset.p());
    m_propertyPublisher->publishProperty(m_propUseEyeLift.p());

    // Must set up model first
    m_theModel = new ModelBasicList;

    buildModel();

    // Set up the rendering
    // Disable all sorting (except priority) so that draw order doesn't change between when rebuilding the model
    ref<Rendering> rendering = m_renderSequence->firstRendering();
    rendering->setRenderQueueSorter(new cvf::RenderQueueSorterBasic(cvf::RenderQueueSorterBasic::MINIMAL));
    rendering->scene()->addModel(m_theModel.p());
    rendering->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    m_camera->setProjectionAsPerspective(40, DEF_NEAR_PLANE, DEF_FAR_PLANE);
    BoundingBox bb = m_theModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VisualPriority::onPaintEvent(PostEventAction* postEventAction)
{
    if (m_propAutomaticClipping->value())
    {
        if (m_renderSequence.notNull() && m_camera.notNull())
        {
            BoundingBox bb = m_renderSequence->boundingBox();
            if (bb.isValid() && bb.extent().length() > 0.0)
            {
                //double minClipDist = bb.extent().length()/10000.0;
                m_camera->setClipPlanesFromBoundingBox(bb, DEF_NEAR_PLANE);
            }
        }
    }

    TestSnippet::onPaintEvent(postEventAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VisualPriority::onPropertyChanged(Property* property, PostEventAction* postEventAction)
{
    *postEventAction = REDRAW;

    if (property == m_propAutomaticClipping)
    {
        if (!m_propAutomaticClipping->value())
        {
            m_camera->setProjectionAsPerspective(40, DEF_NEAR_PLANE, DEF_FAR_PLANE);
        }

        return;
    }

    m_theModel->removeAllParts();
    buildModel();
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VisualPriority::buildModel()
{
    // Box
    // --------------------------
    {
        BoxGenerator gen;
        gen.setCenterAndExtent(Vec3d(0, 0, 0), Vec3d(4, 4, 4));
        gen.setSubdivisions(12, 8, 4);
        GeometryBuilderFaceList builder;
        gen.generate(&builder);
        ref<Vec3fArray> vertexArray = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        {
            // Box surface
            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->setFromFaceList(*faceList);
            geo->computeNormals();
            addAsSurfacePart(geo.p(), Color3::YELLOW, 0);
        }
        {
            // Box mesh
            MeshEdgeExtractor ee;
            ee.addFaceList(*faceList);

            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->addPrimitiveSet(new PrimitiveSetIndexedUInt(PT_LINES, ee.lineIndices().p()));
            addAsLinesPart(geo.p(), Color3::RED, 0);
        }
        {
            // Box outline
            OutlineEdgeExtractor ee(Math::toRadians(60.0), *vertexArray);
            ee.addFaceList(*faceList);

            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->addPrimitiveSet(new PrimitiveSetIndexedUInt(PT_LINES, ee.lineIndices().p()));
            addAsLinesPart(geo.p(), Color3::WHITE, 0.1);
        }
    }

    // Two patches 'glued to box'
    // --------------------------
    {
        GeometryBuilderFaceList builder;
        PatchGenerator gen;
        gen.setExtent(2, 4);
        gen.setSubdivisions(10, 5);

        gen.setOrigin(Vec3d(-1, -1, 2));
        gen.setAxes(Vec3d::X_AXIS, Vec3d::Y_AXIS);
        gen.generate(&builder);

        gen.setOrigin(Vec3d(2, -1,1));
        gen.setAxes(-Vec3d::Z_AXIS, Vec3d::Y_AXIS);
        gen.generate(&builder);

        ref<Vec3fArray> vertexArray = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        {
            // Surface
            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->setFromFaceList(*faceList);
            geo->computeNormals();
            addAsSurfacePart(geo.p(), Color3::BLUE, 1.0);
        }
        {
            // Mesh on the patch
            MeshEdgeExtractor ee;
            ee.addFaceList(*faceList);

            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->addPrimitiveSet(new PrimitiveSetIndexedUInt(PT_LINES, ee.lineIndices().p()));
            addAsLinesPart(geo.p(), Color3::DARK_CYAN, 1.0);
        }
    }

    // Bottom (floor) patch
    // --------------------------
    {
        PatchGenerator gen;
        gen.setOrigin(Vec3d(-7.5, -2, 10));
        gen.setAxes(Vec3d::X_AXIS, -Vec3d::Z_AXIS);
        gen.setExtent(15, 20);
        gen.setSubdivisions(3, 4);

        GeometryBuilderFaceList builder;
        gen.generate(&builder);
        ref<Vec3fArray> vertexArray = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        {
            // Surface
            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->setFromFaceList(*faceList);
            geo->computeNormals();
            addAsSurfacePart(geo.p(), Color3::LIGHT_GRAY, 0);
        }
        {
            // Mesh 
            MeshEdgeExtractor ee;
            ee.addFaceList(*faceList);

            ref<DrawableGeo> geo = new DrawableGeo;
            geo->setVertexArray(vertexArray.p());
            geo->addPrimitiveSet(new PrimitiveSetIndexedUInt(PT_LINES, ee.lineIndices().p()));
            addAsLinesPart(geo.p(), Color3::WHITE, 0);
        }
    }

    // Overlapping boxes, transformed
    // --------------------------
    {
        {
            BoxGenerator gen;
            gen.setOriginAndExtent(Vec3d(0, 0, 0), Vec3d(3, 2, 2));
            gen.setSubdivisions(2, 2, 2);
            GeometryBuilderFaceList builder;
            gen.generate(&builder);

            const Mat4d rm = Mat4d::fromRotation(Vec3d::Z_AXIS, Math::toRadians(30.0));

            {
                Mat4d m = rm*Mat4d::fromTranslation(Vec3d(5, 0, 0));

                ref<DrawableGeo> geo = new DrawableGeo;
                geo->setVertexArray(builder.vertices().p());
                geo->setFromFaceList(*builder.faceList());
                //geo->transform(m);
                geo->computeNormals();

                Part* part = addAsSurfacePart(geo.p(), Color3::RED, 0);
                setTransformationMatrixOnPart(part, m);
            }
            {
                Mat4d m = rm*Mat4d::fromTranslation(Vec3d(7, 0, 0));

                ref<DrawableGeo> geo = new DrawableGeo;
                geo->setVertexArray(builder.vertices().p());
                geo->setFromFaceList(*builder.faceList());
                //geo->transform(m);
                geo->computeNormals();

                Part* part = addAsSurfacePart(geo.p(), Color3::GREEN, 0.5);
                setTransformationMatrixOnPart(part, m);
            }
            {
                Mat4d m = rm*Mat4d::fromTranslation(Vec3d(6, 1, 0));

                ref<DrawableGeo> geo = new DrawableGeo;
                geo->setVertexArray(builder.vertices().p());
                geo->setFromFaceList(*builder.faceList());
                //geo->transform(m);
                geo->computeNormals();

                Part* part = addAsSurfacePart(geo.p(), Color3::BLUE, 1.0);
                setTransformationMatrixOnPart(part, m);
            }
        }
    }


    m_theModel->updateBoundingBoxesRecursive();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part* VisualPriority::addAsSurfacePart(DrawableGeo* geo, const Color3f& color, double liftFactor)
{
    ref<Effect> eff = new Effect;

    if (m_propUsePolygonOffset->value())
    {
        cvf::RenderStatePolygonOffset* polyOffset = new cvf::RenderStatePolygonOffset;
        polyOffset->configurePolygonPositiveOffset();
        eff->setRenderState(polyOffset);
    }

    if (m_propUseShaders->value())
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

    if (m_propUseEyeLift->value())
    {
        if (liftFactor != 0)
        {
            ref<Transform> trans = new Transform;
            trans->setEyeLiftFactor(liftFactor);
            part->setTransform(trans.p());
        }
    }

    m_theModel->addPart(part.p());

    return part.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Part* VisualPriority::addAsLinesPart(DrawableGeo* geo, const Color3f& color, double liftFactor)
{
    ref<Effect> eff = new Effect;
    //eff->setRenderState(new cvf::Depth(true, cvf::Depth::LEQUAL));

    if (m_propUseShaders->value())
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

    if (m_propUseEyeLift->value())
    {
        if (liftFactor != 0)
        {
            ref<Transform> trans = new Transform;
            trans->setEyeLiftFactor(liftFactor);
            part->setTransform(trans.p());
        }
    }

    m_theModel->addPart(part.p());

    return part.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void VisualPriority::setTransformationMatrixOnPart(Part* part, const Mat4d& mat)
{
     ref<Transform> trans = part->transform();
     if (trans.isNull())
     {
         trans = new Transform;
     }

     trans->setLocalTransform(mat);

     part->setTransform(trans.p());
}


} // namespace snip

