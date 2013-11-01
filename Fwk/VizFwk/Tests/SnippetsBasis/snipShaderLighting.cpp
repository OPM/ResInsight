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

#include "snipShaderLighting.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool ShaderLighting::onInitialize()
{
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

    //ref<Effect> eff = createFixedHeadlightPhongEffect(Color3f(Color3::SKY_BLUE));
    ref<Effect> eff = createPhongEffect(m_openGLContext.p(), Color3f(Color3::SKY_BLUE));
    //ref<Effect> eff = createHemisphereEffect();
    part->setEffect(eff.p());

    myModel->addPart(part.p());

    m_light = new PointLight;
    m_light->setPosition(Vec3d(-0.15, 0.1, 0));
    
    ref<Part> markerPart = SampleFactory::createUnlitSphere(0.005, Color3f::YELLOW);
    markerPart->setTransform(m_light->markerTransform());
    myModel->addPart(markerPart.p());

    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());
    m_renderSequence->firstRendering()->addGlobalDynamicUniformSet(m_light.p());

    m_locator = new LocatorPanWalkRotate(m_camera.p());

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
ref<Effect> ShaderLighting::createFixedHeadlightPhongEffect(OpenGLContext* oglContext, Color3f objectColor)
{
    cvf::ShaderProgramGenerator gen("SimpleHeadlight", cvf::ShaderSourceProvider::instance());
    gen.configureStandardHeadlightColor();
    ref<ShaderProgram> prog = gen.generate();

    // Link and show log to see any warnings
    prog->linkProgram(oglContext);
    if (!prog->programInfoLog(oglContext).isEmpty()) Trace::show(prog->programInfoLog(oglContext));

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new UniformFloat("u_color", cvf::Color4f(objectColor)));

    return eff;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Effect> ShaderLighting::createPhongEffect(OpenGLContext* oglContext, Color3f objectColor)
{
    cvf::ShaderProgramGenerator gen("PhongDual", cvf::ShaderSourceProvider::instance());
    gen.addVertexCode(ShaderSourceRepository::vs_Standard);
    gen.addFragmentCode(ShaderSourceRepository::src_Color);
    gen.addFragmentCode(ShaderSourceRepository::light_PhongDual);
    gen.addFragmentCode(ShaderSourceRepository::fs_Standard);
    ref<ShaderProgram> prog = gen.generate();

    // Link and show log to see any warnings
    prog->linkProgram(oglContext);
    if (!prog->programInfoLog(oglContext).isEmpty()) Trace::show(prog->programInfoLog(oglContext));

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());
    eff->setUniform(new UniformFloat("u_ambientIntensity", 0.2f));
    eff->setUniform(new UniformFloat("u_color", Color4f(objectColor)));
    
    // Headlight
    eff->setUniform(new UniformFloat("u_ecLightPosition2", Vec3f(0.5, 5.0, 7.0)));

    return eff;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
ref<Effect> ShaderLighting::createHemisphereEffect(OpenGLContext* oglContext)
{
    cvf::ShaderProgramGenerator gen("Hemisphere", cvf::ShaderSourceProvider::instance());
    gen.addVertexCodeFromFile("HemisphereLighting_Vert");
    gen.addFragmentCodeFromFile("HemisphereLighting_Frag");
    ref<ShaderProgram> prog = gen.generate();

    // Link and show log to see any warnings
    prog->linkProgram(oglContext);
    if (!prog->programInfoLog(oglContext).isEmpty()) Trace::show(prog->programInfoLog(oglContext));

    ref<Effect> eff = new Effect;
    eff->setShaderProgram(prog.p());

    return eff;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderLighting::onKeyPressEvent(KeyEvent* keyEvent)
{
    keyEvent->setRequestedAction(REDRAW);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderLighting::onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent)
{
    if (m_locator.notNull() && m_light.notNull())
    {
        if (mouseEvent->modifiers() == (ShiftModifier | ControlModifier))
        {
            int x = mouseEvent->x();
            int y = mouseEvent->y();

            if (mouseEvent->buttons() == MiddleButton || 
                mouseEvent->buttons() == (LeftButton | RightButton))
            {
                m_locator->setOperation(LocatorPanWalkRotate::WALK);
            }
            else
            {
                m_locator->setOperation(LocatorPanWalkRotate::PAN);
            }

            m_locator->setPosition(m_light->position());
            m_locator->start(x, y);

            mouseEvent->setRequestedAction(REDRAW);

            return;
        }
    }
    
    TestSnippet::onMousePressEvent(buttonPressed, mouseEvent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ShaderLighting::onMouseMoveEvent(MouseEvent* mouseEvent)
{
    if (m_locator.notNull() && m_light.notNull())
    {
        if (mouseEvent->modifiers() == (ShiftModifier | ControlModifier))
        {
            int x = mouseEvent->x();
            int y = mouseEvent->y();
            m_locator->update(x, y);
            m_light->setPosition(m_locator->position());

            mouseEvent->setRequestedAction(REDRAW);

            return;
        }
    }

    TestSnippet::onMouseMoveEvent(mouseEvent);
}


/*
class Subject : public Object
{

public:
    virtual void registerObserver(Observer* observer);
    virtual void removeObserver(Observer* observer);
    virtual void notifyObservers();

private:
    QVector<Observer*>   m_observers;
};


class Observer : public Object
{
public:
    virtual void update(Subject* modifiedSubject) = 0;
};
*/

} // namespace snip



