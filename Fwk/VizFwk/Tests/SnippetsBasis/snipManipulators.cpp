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
#include "cvfGeometryUtils.h"

#include "snipManipulators.h"

#include "cvfuInputEvents.h"
#include "cvfuPartCompoundGenerator.h"

namespace snip {


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Manipulators::Manipulators()
{
    m_perspectiveProjection = true;
    m_fovYDeg = 40;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Manipulators::onInitialize()
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

    m_renderSequence->firstRendering()->scene()->addModel(myModel.p());
    m_renderSequence->firstRendering()->addOverlayItem(new OverlayAxisCross(m_camera.p(), new FixedAtlasFont(FixedAtlasFont::STANDARD)));

    setDefaultProjectionAndViewpoint();

    {
        Vec3d pos(0, -1.5, 0);
        LocatorPanWalkRotate* loc = new LocatorPanWalkRotate(m_camera.p());
        loc->setPosition(pos);
        m_activeLocator = loc;
    }

//     {
//         Vec3d pos(0, -1.5, 0);
//         LocatorTranslateOnPlane* loc = new LocatorTranslateOnPlane(m_camera.p());
//         loc->setPosition(pos, -Vec3d::Y_AXIS);
//         m_activeLocator = loc;
//     }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Manipulators::setDefaultProjectionAndViewpoint()
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
void Manipulators::setProjection()
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
void Manipulators::onResizeEvent(int width, int height)
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
void Manipulators::onKeyPressEvent(KeyEvent* keyEvent)
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
void Manipulators::onPaintEvent(PostEventAction* postEventAction)
{
    TestSnippet::onPaintEvent(postEventAction);

    if (m_activeLocator.notNull())
    {
        if (ShaderProgram::supportedOpenGL(m_openGLContext.p()))
        {
            ShaderProgram::useNoProgram(m_openGLContext.p());
        }
        
        m_camera->viewport()->applyOpenGL(m_openGLContext.p(), Viewport::DO_NOT_CLEAR);
        m_camera->applyOpenGL();

        GeometryBuilderDrawableGeo builder;
        GeometryUtils::createBox(Vec3f::ZERO, 0.2f, 0.2f, 0.2f, &builder);
        ref<DrawableGeo> boxGeo = builder.drawableGeo();
        
        Mat4d transform;

        if (dynamic_cast<LocatorPanWalkRotate*>(m_activeLocator.p()))
        {
            LocatorPanWalkRotate* loc = dynamic_cast<LocatorPanWalkRotate*>(m_activeLocator.p());
            transform.setFromMatrix3(loc->orientation());
        }

        transform.translatePreMultiply(m_activeLocator->position());

        boxGeo->transform(transform);
        boxGeo->computeNormals();

        RenderStateLighting_FF lighting;
        lighting.applyOpenGL(m_openGLContext.p());

        RenderStateMaterial_FF mat(Color3f::ORANGE);
        mat.applyOpenGL(m_openGLContext.p());

        MatrixState matrixState(*m_camera);
        boxGeo->renderFixedFunction(m_openGLContext.p(), matrixState);
    }

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Manipulators::onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent)
{
    if (mouseEvent->modifiers() == (ShiftModifier | ControlModifier) && m_activeLocator.notNull())
    {
        int x = mouseEvent->x();
        int y = mouseEvent->y();

        if (dynamic_cast<LocatorPanWalkRotate*>(m_activeLocator.p()))
        {
            LocatorPanWalkRotate* loc = dynamic_cast<LocatorPanWalkRotate*>(m_activeLocator.p());
            if (mouseEvent->buttons() == LeftButton)
            {
                loc->setOperation(LocatorPanWalkRotate::PAN);
            }
            else if (mouseEvent->buttons() == RightButton)
            {
                loc->setOperation(LocatorPanWalkRotate::ROTATE);
            }
            else if (mouseEvent->buttons() == MiddleButton || 
                     mouseEvent->buttons() == (LeftButton | RightButton))
            {
                loc->setOperation(LocatorPanWalkRotate::WALK);
            }
        }

        m_activeLocator->start(x, y);

        mouseEvent->setRequestedAction(REDRAW);

        return;
    }

    TestSnippet::onMousePressEvent(buttonPressed, mouseEvent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Manipulators::onMouseMoveEvent(MouseEvent* mouseEvent)
{
    if (mouseEvent->modifiers() == (ShiftModifier | ControlModifier) && m_activeLocator.notNull())
    {
        int x = mouseEvent->x();
        int y = mouseEvent->y();
        if (m_activeLocator->update(x, y))
        {
            mouseEvent->setRequestedAction(REDRAW);
        }

        return;
    }

    TestSnippet::onMouseMoveEvent(mouseEvent);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Manipulators::onMouseReleaseEvent(MouseButton buttonReleased, MouseEvent* mouseEvent)
{
    TestSnippet::onMouseReleaseEvent(buttonReleased, mouseEvent);
}


} // namespace snip

