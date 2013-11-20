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
#include "cvfShaderSourceProvider.h"

#include "cvfuInputEvents.h"
#include "cvfuImageJpeg.h"
#include "cvfuWavefrontObjImport.h"
#include "cvfuSampleFactory.h"

#include "snipEnvironmentMapping.h"

namespace snip {

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool EnvironmentMapping::onInitialize()
{
    ref<ModelBasicList> myModel = new ModelBasicList;

    m_modelTransformMatrix = new Transform;
    m_lastAnimUpdateTimeStamp = 0;

    // Part with static texture
    //ref<TextureImage> img = cvfu::ImageJpeg::loadImage(m_testDataDir + "spheremap_uffizi_gallery.jpg");
    ref<TextureImage> img = cvfu::ImageJpeg::loadImage(m_testDataDir + "spheremap_metal.jpg");
    ref<Texture> envTexture = new Texture(img.p());

    ref<cvf::Sampler> sampler = new cvf::Sampler;
    sampler->setWrapMode(Sampler::CLAMP_TO_EDGE);
    sampler->setMinFilter(Sampler::LINEAR);
    sampler->setMagFilter(Sampler::LINEAR);
    ref<cvf::RenderStateTextureBindings> texBind = new cvf::RenderStateTextureBindings;
    texBind->addBinding(envTexture.p(), sampler.p(), "u_texture2D");

    // The 'main' geometry
    {
        ref<DrawableGeo> geo;

        WavefrontObjImport imp;
        imp.readFile(m_testDataDir + "teapot.obj");
        GeometryBuilderDrawableGeo builder;
        imp.buildGeometry(&builder);
        geo = builder.drawableGeo();
        geo->weldVertices(0.00001);
        geo->computeNormals();

        ref<Part> part = new Part;
        part->setDrawable(geo.p());

        ShaderProgramGenerator gen("EnvironmentMapping", ShaderSourceProvider::instance());
        gen.addVertexCode(ShaderSourceRepository::vs_EnvironmentMapping);
        gen.addFragmentCode(ShaderSourceRepository::src_Texture);
        gen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
        gen.addFragmentCode(ShaderSourceRepository::fs_Standard);

        ref<ShaderProgram> prog = gen.generate();

        // Link and show log to see any warnings
        prog->linkProgram(m_openGLContext.p());
        if (!prog->programInfoLog(m_openGLContext.p()).isEmpty()) Trace::show(prog->programInfoLog(m_openGLContext.p()));

        ref<Effect> eff = new Effect;
        eff->setRenderState(texBind.p());
        eff->setShaderProgram(prog.p());

        part->setEffect(eff.p());
        part->setTransform(m_modelTransformMatrix.p());

        myModel->addPart(part.p());
    }

    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());

    myModel->updateBoundingBoxesRecursive();
    BoundingBox bb = myModel->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, -Vec3d::Z_AXIS, Vec3d::Y_AXIS);
    }

    m_renderSequence->firstRendering()->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EnvironmentMapping::onUpdateAnimation(double animationTime, PostEventAction* postEventAction)
{
    TestSnippet::onUpdateAnimation(animationTime, postEventAction);

    if (Math::abs(animationTime - m_lastAnimUpdateTimeStamp) > 0.05)
    {
        Mat4d m = m_modelTransformMatrix->worldTransform();
        Mat4d r = Mat4d::fromRotation(Vec3d::Y_AXIS, Math::toRadians(1.0));
        m_modelTransformMatrix->setLocalTransform(m*r);
        m_lastAnimUpdateTimeStamp = animationTime;

        if (postEventAction)
        {
            *postEventAction = REDRAW;
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void EnvironmentMapping::onKeyPressEvent(KeyEvent* keyEvent)
{
    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> EnvironmentMapping::helpText() const
{
    std::vector<cvf::String> helpText;

    return helpText;
}

} // namespace snip
