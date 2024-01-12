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

#include "cvfuPartCompoundGenerator.h"

#include "QMVFactory.h"



//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMVModelFactory::QMVModelFactory(bool useShaders, const cvf::OpenGLCapabilities& capabilities)
:   m_useShaders(useShaders),
    m_capabilities(capabilities)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Model> QMVModelFactory::createSphereAndBox()
{
    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    {
        cvf::GeometryBuilderDrawableGeo builder;
        cvf::GeometryUtils::createSphere(2, 10, 10, &builder);

        cvf::ref<cvf::Effect> eff = new cvf::Effect;
        if (m_useShaders)
        {
            cvf::ref<cvf::ShaderProgram> prog = createProgramStandardHeadlightColor();
            eff->setShaderProgram(prog.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::GREEN)));
        }
        else
        {
            eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::BLUE));
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("MySphere");
        part->setDrawable(0, builder.drawableGeo().p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    {
        cvf::GeometryBuilderDrawableGeo builder;
        cvf::GeometryUtils::createBox(cvf::Vec3f(5, 0, 0), 2, 3, 4, &builder);

        cvf::ref<cvf::Effect> eff = new cvf::Effect;
        if (m_useShaders)
        {
            cvf::ref<cvf::ShaderProgram> prog = createProgramUnlit();
            eff->setShaderProgram(prog.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::GREEN)));
        }
        else
        {
            eff->setRenderState(new cvf::RenderStateMaterial_FF(cvf::Color3::RED));
        }

        cvf::ref<cvf::Part> part = new cvf::Part;
        part->setName("MyBox");
        part->setDrawable(0, builder.drawableGeo().p());
        part->setEffect(eff.p());

        model->addPart(part.p());
    }

    model->updateBoundingBoxesRecursive();

    return model;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Model> QMVModelFactory::createSpheres() 
{
    cvfu::PartCompoundGenerator gen;
    gen.setUseShaders(m_useShaders);
    gen.setPartDistribution(cvf::Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(cvf::Vec3f(3,3,3));
    gen.setOrigin(cvf::Vec3f(-1.5f, -1.5f, -1.5f));

    cvf::Collection<cvf::Part> parts;
    gen.generateSpheres(20, 20, &parts);

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    cvf::ref<cvf::ShaderProgram> prog = createProgramStandardHeadlightColor();

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        cvf::Part* part = parts[i].p();
        if (m_useShaders)
        {
            cvf::Effect* eff = part->effect();
            eff->setShaderProgram(prog.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::INDIGO)));
        }
        model->addPart(part);
    }

    model->updateBoundingBoxesRecursive();

    return model;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Model> QMVModelFactory::createBoxes() 
{
    cvfu::PartCompoundGenerator gen;
    gen.setUseShaders(m_useShaders);
    gen.setPartDistribution(cvf::Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(cvf::Vec3f(3,3,3));
    gen.setOrigin(cvf::Vec3f(-1.5f, -1.5f, -1.5f));

    cvf::Collection<cvf::Part> parts;
    gen.generateBoxes(&parts);

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    cvf::ref<cvf::ShaderProgram> prog = createProgramStandardHeadlightColor();

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        cvf::Part* part = parts[i].p();
        if (m_useShaders)
        {
            cvf::Effect* eff = part->effect();
            eff->setShaderProgram(prog.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::CYAN)));
        }
        model->addPart(part);
    }

    model->updateBoundingBoxesRecursive();

    return model;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Model> QMVModelFactory::createTriangles() 
{
    cvfu::PartCompoundGenerator gen;
    gen.setUseShaders(m_useShaders);
    gen.setPartDistribution(cvf::Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(cvf::Vec3f(3,3,3));
    gen.setOrigin(cvf::Vec3f(-1.5f, -1.5f, -1.5f));

    cvf::Collection<cvf::Part> parts;
    gen.generateTriangles(&parts);

    cvf::ref<cvf::ModelBasicList> model = new cvf::ModelBasicList;

    cvf::ref<cvf::ShaderProgram> prog = createProgramStandardHeadlightColor();

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        cvf::Part* part = parts[i].p();
        if (m_useShaders)
        {
            cvf::Effect* eff = part->effect();
            eff->setShaderProgram(prog.p());
            eff->setUniform(new cvf::UniformFloat("u_color", cvf::Color4f(cvf::Color3::GOLD)));
        }
        model->addPart(part);
    }

    model->updateBoundingBoxesRecursive();

    return model;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::ShaderProgram> QMVModelFactory::createProgramStandardHeadlightColor()
{
    cvf::ShaderProgramGenerator gen("StandardHeadlightColor", cvf::ShaderSourceProvider::instance());
    gen.configureStandardHeadlightColor();
    cvf::ref<cvf::ShaderProgram> prog = gen.generate();
    return prog;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::ShaderProgram> QMVModelFactory::createProgramUnlit()
{
    cvf::ShaderProgramGenerator gen("Unlit", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Color);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);
    cvf::ref<cvf::ShaderProgram> prog = gen.generate();
    return prog;
}


//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QMVSceneFactory::QMVSceneFactory(QMVModelFactory* modelFactory)
:   m_modelFactory(modelFactory)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Scene> QMVSceneFactory::createNumberedScene(int sceneNumber) 
{
    cvf::ref<cvf::Model> model;
    switch (sceneNumber)
    {
        case 0:     model = m_modelFactory->createSphereAndBox();  break;
        case 1:     model = m_modelFactory->createSpheres();       break;
        case 2:     model = m_modelFactory->createBoxes();         break;
        default:    model = m_modelFactory->createTriangles();     break;
    }

    return createFromModel(model.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::Scene> QMVSceneFactory::createFromModel(cvf::Model* model) 
{
    cvf::ref<cvf::Scene> scene = new cvf::Scene;

    if (model)
    {
        scene->addModel(model);
    }

    return scene;
}



//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::RenderSequence> QMVRenderSequenceFactory::createFromScene(cvf::Scene* scene) 
{
    cvf::ref<cvf::Rendering> rendering = new cvf::Rendering;
    rendering->renderEngine()->enableItemCountUpdate(true);
    rendering->setScene(scene);

    cvf::Camera* cam = rendering->camera();
    rendering->addOverlayItem(new cvf::OverlayAxisCross(cam, new cvf::FixedAtlasFont(cvf::FixedAtlasFont::STANDARD)));

    if (scene)
    {
        cvf::BoundingBox bb = scene->boundingBox();
        if (bb.isValid())
        {
            cam->fitView(bb, -cvf::Vec3d::Z_AXIS, cvf::Vec3d::Y_AXIS);
        }
    }

    cvf::ref<cvf::RenderSequence> renderSeq = new cvf::RenderSequence;
    renderSeq->addRendering(rendering.p());

    return renderSeq;
}
