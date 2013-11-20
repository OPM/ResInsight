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
#include "cvfuWavefrontObjImport.h"
#include "cvfuSampleFactory.h"
#include "cvfuSnippetPropertyPublisher.h"

#include "snipClipping.h"

namespace snip {

//==================================================================================================
//
// 
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Clipping::Clipping()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Clipping::~Clipping()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Clipping::onInitialize()
{
    createAndPublishProperties();

    ref<ModelBasicList> myModel = new ModelBasicList;

    ref<DrawableGeo> geo;
    WavefrontObjImport imp;
    imp.readFile(m_testDataDir + "dragon_10k.obj");
    //imp.readFile(m_testDataDir + "teapot.obj");
    GeometryBuilderDrawableGeo builder;
    imp.buildGeometry(&builder);
    geo = builder.drawableGeo();
    geo->weldVertices(0.00001);
    geo->computeNormals();

    ref<Part> part = new Part;
    part->setDrawable(geo.p());

    m_clipPlaneSet =  new ClipPlaneSet;
    m_renderSequence->firstRendering()->addDynamicUniformSet(m_clipPlaneSet.p());
    configureClipPlaneSetFromProperties();

    ref<Effect> eff = createClippingFixedHeadlightEffect(Color3f(Color3::SKY_BLUE));
    part->setEffect(eff.p());

    myModel->addPart(part.p());

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
void Clipping::createAndPublishProperties()
{
    m_propEnableXClip = new PropertyBool("X clip plane", false);
    m_propEnableYClip = new PropertyBool("Y clip plane", false);
    m_propEnableZClip = new PropertyBool("Z clip plane", false);
    m_propertyPublisher->publishProperty(m_propEnableXClip.p());
    m_propertyPublisher->publishProperty(m_propEnableYClip.p());
    m_propertyPublisher->publishProperty(m_propEnableZClip.p());

    m_propEnableUserClip = new PropertyBool("User clip plane", true);
    m_propertyPublisher->publishProperty(m_propEnableUserClip.p());

    m_propPosX = new PropertyDouble("X pos", 0);
    m_propPosY = new PropertyDouble("Y pos", 0);
    m_propPosZ = new PropertyDouble("Z pos", 0);
    m_propPosX->setDecimals(3);
    m_propPosY->setDecimals(3);
    m_propPosZ->setDecimals(3);
    m_propPosY->setGuiStep(0.001);
    m_propPosZ->setGuiStep(0.001);
    m_propPosX->setGuiStep(0.001);
    m_propPosY->setGuiStep(0.001);
    m_propPosZ->setGuiStep(0.001);
    m_propertyPublisher->publishProperty(m_propPosX.p());
    m_propertyPublisher->publishProperty(m_propPosY.p());
    m_propertyPublisher->publishProperty(m_propPosZ.p());

    m_propNormalX = new PropertyDouble("X normal", 1);
    m_propNormalY = new PropertyDouble("Y normal", 0);
    m_propNormalZ = new PropertyDouble("Z normal", 0);
    m_propertyPublisher->publishProperty(m_propNormalX.p());
    m_propertyPublisher->publishProperty(m_propNormalY.p());
    m_propertyPublisher->publishProperty(m_propNormalZ.p());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Clipping::configureClipPlaneSetFromProperties()
{
    CVF_ASSERT(m_clipPlaneSet.notNull());
    m_clipPlaneSet->clear();

    if (m_propEnableXClip->value())
    {
        m_clipPlaneSet->addPlane(Plane(1, 0, 0, -0.02));
    }

    if (m_propEnableYClip->value())
    {
        m_clipPlaneSet->addPlane(Plane(0, 1, 0, -0.07));
    }

    if (m_propEnableZClip->value())
    {
        m_clipPlaneSet->addPlane(Plane(0, 0, 1, 0.02));
    }

    if (m_propEnableUserClip->value())
    {
        Vec3d planePos(m_propPosX->value(), m_propPosY->value(), m_propPosZ->value());
        Vec3d planeNorm(m_propNormalX->value(), m_propNormalY->value(), m_propNormalZ->value());
        if (!planeNorm.isZero())
        {
            Plane wcClipPlane;
            wcClipPlane.setFromPointAndNormal(planePos, planeNorm);
            m_clipPlaneSet->addPlane(wcClipPlane);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Effect> Clipping::createClippingFixedHeadlightEffect(Color3f objectColor)
{
//     cvf::ShaderProgramGenerator gen("ClippingFixedHeadlight", cvf::ShaderSourceProvider::instance());
//     gen.addVertexCodeFromFile("Clipping_Vert");
//     gen.addFragmentCode(ShaderSourceRepository::src_Color);
//     gen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
//     gen.addFragmentCodeFromFile("Clipping_Frag");

    cvf::ShaderProgramGenerator gen("ClippingFixedHeadlight", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(ShaderSourceRepository::calcClipDistances);
    gen.addVertexCode(ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(ShaderSourceRepository::checkDiscard_ClipDistances);
    gen.addFragmentCode(ShaderSourceRepository::src_Color);
    gen.addFragmentCode(ShaderSourceRepository::light_SimpleHeadlight);
    gen.addFragmentCode(ShaderSourceRepository::fs_Standard);

    ref<ShaderProgram> prog = gen.generate();

    // Link and show log to see any warnings
    prog->linkProgram(m_openGLContext.p());
    if (!prog->programInfoLog(m_openGLContext.p()).isEmpty()) Trace::show(prog->programInfoLog(m_openGLContext.p()));

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new UniformFloat("u_color", cvf::Color4f(objectColor)));

    CVF_ASSERT(m_clipPlaneSet.notNull());

    size_t uniformCount = m_clipPlaneSet->uniformSet()->count();
    size_t i;
    for (i = 0; i < uniformCount; i++)
    {
        eff->setUniform(m_clipPlaneSet->uniformSet()->uniform(i));
    }

    return eff;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Clipping::onPaintEvent(PostEventAction* postEventAction)
{
    // Once we get this working also on ATI we should probably encapsulate this setting in a render state
    //glEnable(GL_CLIP_DISTANCE0);

    TestSnippet::onPaintEvent(postEventAction);

    //glDisable(GL_CLIP_DISTANCE0);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Clipping::onPropertyChanged(Property* property, PostEventAction* postEventAction)
{
    CVF_UNUSED(property);

    configureClipPlaneSetFromProperties();

    *postEventAction = REDRAW;
}


} // namespace snip



