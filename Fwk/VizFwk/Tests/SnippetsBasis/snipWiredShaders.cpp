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

#include "snipWiredShaders.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool WiredShaders::onInitialize()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(3, 3, 1));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateSpheres(20, 20, &parts);

    ref<ModelBasicList> myModel = new ModelBasicList;
    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        myModel->addPart(parts[i].p());
    }

    {
        BoxGenerator gen;
        gen.setMinMax(Vec3d(-5, -5, -12), Vec3d(5, 5, -1));

        GeometryBuilderDrawableGeo builder;
        gen.generate(&builder);

        ref<Part> part = new Part;
        part->setDrawable(builder.drawableGeo().p());
        myModel->addPart(part.p());
    }

    myModel->updateBoundingBoxesRecursive();
    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());

    configAllParts_SimpleHeadlight();
    //configurePartsWithAlternatingFixedFunctionAndPhongShader();

    ref<OverlayAxisCross> axisCross = new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD));
    m_renderSequence->firstRendering()->addOverlayItem(axisCross.p());

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
void WiredShaders::configAllParts_MinimalVS_MagentaFS()
{
    cvf::ShaderProgramGenerator gen("MinimalVS_MagentaFS", cvf::ShaderSourceProvider::instance());

    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Minimal);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_FixedColorMagenta);

    ref<ShaderProgram> prog = gen.generate();

    linkProgramVerbose(prog.p());

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());

    setEffectForAllParts(eff.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WiredShaders::configAllParts_MinimalVS_UnlitRedFS()
{
    cvf::ShaderProgramGenerator gen("MinimalVS_MagentaFS", cvf::ShaderSourceProvider::instance());

    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Minimal);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Color);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);

    ref<ShaderProgram> prog = gen.generate();

    linkProgramVerbose(prog.p());

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new UniformFloat("u_color", Color4f(Color3::RED)));

    setEffectForAllParts(eff.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WiredShaders::configAllParts_MinimalVS_PassThroughGS_UnlitBlueFS()
{
    cvf::ShaderProgramGenerator gen("MinimalVS_MagentaFS", cvf::ShaderSourceProvider::instance());

    gen.addVertexCode(cvf::ShaderSourceRepository::vs_Minimal);
    gen.addFragmentCode(cvf::ShaderSourceRepository::src_Color);
    gen.addFragmentCode(cvf::ShaderSourceRepository::fs_Unlit);
    gen.addGeometryCode(cvf::ShaderSourceRepository::gs_PassThroughTriangle_v33);

    ref<ShaderProgram> prog = gen.generate();

    linkProgramVerbose(prog.p());

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new UniformFloat("u_color", Color4f(Color3::BLUE)));

    setEffectForAllParts(eff.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WiredShaders::configAllParts_SimpleHeadlight()
{
    cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    gen.configureStandardHeadlightColor();
    ref<ShaderProgram> prog = gen.generate();

    linkProgramVerbose(prog.p());

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new UniformFloat("u_color", Color4f(Color3::ORANGE)));

    setEffectForAllParts(eff.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WiredShaders::configParts_MixFixedFunctionAndShaderHeadlight()
{
    ref<Effect> effShader1 = new Effect;
    ref<Effect> effShader2 = new Effect;
    {
        cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
        gen.configureStandardHeadlightColor();
        ref<ShaderProgram> prog = gen.generate();

        effShader1->setShaderProgram(prog.p());
        effShader1->setUniform(new UniformFloat("u_color", Color4f(Color3::DARK_RED)));

        effShader2->setShaderProgram(prog.p());
        effShader2->setUniform(new UniformFloat("u_color", Color4f(Color3::SKY_BLUE)));
    }


    ref<Effect> effFixed = new Effect;
    effFixed->setRenderState(new RenderStateMaterial_FF(Color3::GREEN));


    Model* model = m_renderSequence->firstRendering()->scene()->model(0);
    CVF_ASSERT(model);

    Collection<Part> parts;
    model->allParts(&parts);

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        ref<Part> part = parts[i];
        if (i % 3 == 0)
        {
            part->setEffect(effFixed.p());
        }
        else if (i % 3 == 1)
        {
            part->setEffect(effShader1.p());
        }
        else
        {
            part->setEffect(effShader2.p());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WiredShaders::linkProgramVerbose(ShaderProgram* prog)
{
    // Link and show log to see any warnings
    prog->linkProgram(m_openGLContext.p());
    
    if (!prog->programInfoLog(m_openGLContext.p()).isEmpty()) 
    {
        Trace::show(prog->programInfoLog(m_openGLContext.p()));
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WiredShaders::setEffectForAllParts(Effect* effect)
{
    Model* model = m_renderSequence->firstRendering()->scene()->model(0);
    CVF_ASSERT(model);

    Collection<Part> parts;
    model->allParts(&parts);

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        ref<Part> part = parts[i];
        part->setEffect(effect);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void WiredShaders::onKeyPressEvent(KeyEvent* keyEvent)
{
    Key key = keyEvent->key();
    switch (key)
    {
        case Key_1: configAllParts_MinimalVS_MagentaFS();                 break;
        case Key_2: configAllParts_MinimalVS_UnlitRedFS();                break;
        case Key_3: configAllParts_MinimalVS_PassThroughGS_UnlitBlueFS(); break;
        case Key_4: configAllParts_SimpleHeadlight();                     break;
        case Key_5: configParts_MixFixedFunctionAndShaderHeadlight();     break;

        default: break;
    }

    keyEvent->setRequestedAction(REDRAW);
}


} // namespace snip



