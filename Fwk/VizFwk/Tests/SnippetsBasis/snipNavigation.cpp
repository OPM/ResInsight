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

#include "snipNavigation.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"


namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Navigation::Navigation()
{
    m_perspectiveProjection = true;
    m_fovYDeg = 40;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Navigation::onInitialize()
{
    PartCompoundGenerator gen;
    gen.setPartDistribution(Vec3i(5, 5, 5));
    gen.setNumEffects(8);
    gen.useRandomEffectAssignment(false);
    gen.setExtent(Vec3f(3,3,3));
    gen.setOrigin(Vec3f(-1.5f, -1.5f, -1.5f));

    Collection<Part> parts;
    gen.generateBoxes(&parts);

    ref<ModelBasicList> myModel = new ModelBasicList;

    size_t i;
    for (i = 0; i < parts.size(); i++)
    {
        myModel->addPart(parts[i].p());
    }

    myModel->updateBoundingBoxesRecursive();

    cvf::Trace::show("BB: " + myModel->boundingBox().debugString());

    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());
    m_renderSequence->firstRendering()->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    setDefaultProjectionAndViewpoint();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Navigation::setDefaultProjectionAndViewpoint()
{
    m_perspectiveProjection = true;
    m_fovYDeg = 40;

    if (m_renderSequence.isNull() || m_camera.isNull())
    {
        return;
    }

    setProjection();

    BoundingBox bb = m_renderSequence->boundingBox();
    if (bb.isValid())
    {
        m_camera->fitView(bb, Vec3d::Y_AXIS, Vec3d::Z_AXIS);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Navigation::setProjection()
{
    if (m_camera->viewport()->width() <= 0 || m_camera->viewport()->height() <= 0)
    {
        return;
    }

    if (m_perspectiveProjection)
    {
        m_camera->setProjectionAsPerspective(m_fovYDeg, m_camera->nearPlane(), m_camera->farPlane());
    }
    else
    {
        BoundingBox bb = m_renderSequence->boundingBox();
        if (bb.isValid())
        {
            m_camera->setProjectionAsOrtho(bb.extent().length(), m_camera->nearPlane(), m_camera->farPlane());
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Navigation::onResizeEvent(int width, int height)
{
    if (m_renderSequence.isNull() || m_camera.isNull())
    {
        return;
    }

    m_camera->viewport()->set(0, 0, width, height);

    setProjection();

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Navigation::onKeyPressEvent(KeyEvent* keyEvent)
{
    Key key = keyEvent->key();
    char character = keyEvent->character();

    if (character == '+' || character == L'-')
    {
        if (character == '+')   m_fovYDeg += 2;
        else                    m_fovYDeg -= 2;
        m_fovYDeg = Math::clamp(m_fovYDeg, 2.0, 178.0);

        m_camera->setProjectionAsPerspective(m_fovYDeg, m_camera->nearPlane(), m_camera->farPlane());
        Trace::show("FOV set to %.1f", m_fovYDeg);
    }

    if (key == Key_P)
    {
        m_perspectiveProjection = true;
        setProjection();
        Trace::show("Use perspective projection");
    }

    if (key == Key_O)
    {
        m_perspectiveProjection = false;
        setProjection();
        Trace::show("Use orthographic projection");
    }

    if (key == Key_R)
    {
        setDefaultProjectionAndViewpoint();
        Trace::show("Reset to default projection and viewpoint");
    }

    keyEvent->setRequestedAction(REDRAW);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Navigation::onPaintEvent(PostEventAction* postEventAction)
{
    TestSnippet::onPaintEvent(postEventAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Navigation::onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent)
{
    TestSnippet::onMousePressEvent(buttonPressed, mouseEvent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Navigation::onMouseMoveEvent(MouseEvent* mouseEvent)
{
    TestSnippet::onMouseMoveEvent(mouseEvent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Navigation::onMouseReleaseEvent(MouseButton buttonReleased, MouseEvent* mouseEvent)
{
    TestSnippet::onMouseReleaseEvent(buttonReleased, mouseEvent);
}


} // namespace snip


