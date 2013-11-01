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
#include "cvfLibViewing.h"

#include "cvfPatchGenerator.h"
#include "cvfGeometryUtils.h"
#include "cvfGeometryBuilderFaceList.h"
#include "cvfArrowGenerator.h"

#include "snipBlendTest.h"

#include "cvfuSampleFactory.h"
#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
BlendTest::BlendTest()
{
    m_opacity = 0.5;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool BlendTest::onInitialize()
{
    m_model = createModel();

    m_renderSequence->firstRendering()->scene()->addModel(m_model.p());

    m_renderSequence->firstRendering()->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    RenderQueueSorterBasic* sorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::BACK_TO_FRONT);
    m_renderSequence->firstRendering()->setRenderQueueSorter(sorter);
    
    BoundingBox bb = m_model->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }

    return true;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void BlendTest::onKeyPressEvent(KeyEvent* keyEvent)
{
    if (keyEvent->character() == 'T')
    {
        m_opacity -= 0.1f;
        //m_alphaUniform->set(m_opacity);
    }
    if (keyEvent->character() == 't')
    {
        m_opacity += 0.1f;
        //m_alphaUniform->set(m_opacity);
    }
    
    if (keyEvent->key() == Key_D)
    {
        m_depth->enableDepthTest(!m_depth->isDepthTestEnabled());
    }

    if (keyEvent->key() == Key_C)
    {
        m_cullFace->enable(!m_cullFace->isEnabled());
    }

    if (keyEvent->key() == Key_B)
    {
        RenderQueueSorterBasic* sorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::BACK_TO_FRONT);
        m_renderSequence->firstRendering()->setRenderQueueSorter(sorter);
    }

    if (keyEvent->key() == Key_S)
    {
        RenderQueueSorterBasic* sorter = new RenderQueueSorterBasic(RenderQueueSorterBasic::EFFECT_ONLY);
        m_renderSequence->firstRendering()->setRenderQueueSorter(sorter);
    }

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> BlendTest::helpText() const
{
    std::vector<cvf::String> helpText;

    helpText.push_back("'T' decrease opacity");
    helpText.push_back("'t' increase opacity");
    helpText.push_back("'d' toggle depth test");
    helpText.push_back("'b' back to front sorting");
    helpText.push_back("'s' default effect only sorting");

    return helpText;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<ModelBasicList> BlendTest::createModel()
{
    ref<ModelBasicList> model = new ModelBasicList;

    ShaderProgramGenerator spGen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    spGen.configureStandardHeadlightColor();
    ref<ShaderProgram> shaderProg = spGen.generate();

    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(3, 3, 3));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(2, 5, 3));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        Part* part = parts[i].p();
        Effect* eff = part->effect();

        RenderStateMaterial_FF* mat = static_cast<RenderStateMaterial_FF*>(part->effect()->renderStateOfType(RenderState::MATERIAL_FF));
        if (mat)
        {
            eff->setUniform(new UniformFloat("u_color", Color4f(mat->frontDiffuse(), m_opacity)));
        }

        eff->setShaderProgram(shaderProg.p());

        model->addPart(part);
    }

    {
        PatchGenerator gen;
        gen.setOrigin(Vec3d(0, 0, 0));
        gen.setExtent(10, 10);
        gen.setSubdivisions(10, 10);
        {
            GeometryBuilderDrawableGeo builder;
            gen.generate(&builder);

            ref<DrawableGeo> geo = builder.drawableGeo();
            ref<Part> part = new Part;
            part->setDrawable(geo.p());

            ref<Effect> eff = new Effect;
            eff->setShaderProgram(shaderProg.p());
            eff->setUniform(new UniformFloat("u_color", Color4f(0, 1, 0, m_opacity)));

            part->setEffect(eff.p());
            model->addPart(part.p());
        }
    }

    {
        GeometryBuilderFaceList builder;
        GeometryUtils::createSphere(1, 50, 50, &builder);

        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        Mat4f matr;
        matr.setTranslation(Vec3f(5,2,5));
        size_t i;
        for (i = 0; i < vertices->size(); i++)
        {
            Vec3f v = vertices->get(i);
            v.transformPoint(matr);
            vertices->set(i, v);
        }

        ref<DrawableGeo> geo1 = new DrawableGeo;
        geo1->setVertexArray(vertices.p());   
        geo1->setFromFaceList(*faceList);
        geo1->computeNormals();

        ref<Part> part = new Part;
        part->setDrawable(geo1.p());

        ref<Effect> eff1 = new Effect;
        eff1->setShaderProgram(shaderProg.p());
        eff1->setUniform(new UniformFloat("u_color", Color4f(1, 0, 0, m_opacity)));
        
        part->setEffect(eff1.p());

        model->addPart(part.p());
    }

    {
        GeometryBuilderFaceList builder;
        ArrowGenerator gen;
        gen.generate(&builder);

        ref<Vec3fArray> vertices = builder.vertices();
        ref<UIntArray> faceList = builder.faceList();

        Mat4f matr = Mat4f::fromRotation(Vec3f(1,1,0), static_cast<float>(cvf::Math::toRadians(90.0)));
        matr.setTranslation(Vec3f(7,5,2));
        size_t i;
        for (i = 0; i < vertices->size(); i++)
        {
            Vec3f v = vertices->get(i);
            v.transformPoint(matr);
            vertices->set(i, v);
        }

        ref<DrawableGeo> geo1 = new DrawableGeo;
        geo1->setVertexArray(vertices.p());   
        geo1->setFromFaceList(*faceList);
        geo1->computeNormals();

        ref<Part> part = new Part;
        part->setDrawable(geo1.p());

        ref<Effect> eff1 = new Effect;
        eff1->setShaderProgram(shaderProg.p());
        eff1->setUniform(new UniformFloat("u_color", Color4f(1, 1, 1, m_opacity)));

        part->setEffect(eff1.p());

        model->addPart(part.p());
    }

    model->updateBoundingBoxesRecursive();

    m_depth = new RenderStateDepth(false);
    m_blending = new RenderStateBlending;
    m_blending->configureTransparencyBlending();

    m_cullFace = new RenderStateCullFace;

    for (i = 0; i < model->partCount(); i++)
    {
        Part* part = model->part(i);
        CVF_ASSERT(part);

        part->effect()->setRenderState(m_blending.p());
        part->effect()->setRenderState(m_depth.p());            
        part->effect()->setRenderState(m_cullFace.p());
    }

    return model;
}

} // namespace snip
