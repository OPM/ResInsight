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


#include "cvfBase.h"
#include "cvfRenderSequence.h"
#include "cvfRendering.h"
#include "cvfScene.h"
#include "cvfModel.h"
#include "cvfCamera.h"
#include "cvfRenderQueueSorter.h"
#include "cvfRayIntersectSpec.h"
#include "cvfHitItemCollection.h"
#include "cvfHitItem.h"
#include "cvfOpenGL.h"
#include "cvfOpenGLResourceManager.h"
#include "cvfTrace.h"
#include "cvfDebugTimer.h"

#include "cvfuTestSnippet.h"
#include "cvfuInputEvents.h"
#include "cvfuSnippetPropertyPublisher.h"

namespace cvfu {

using cvf::ref;


//==================================================================================================
///
/// \class cvfu::TestSnippet
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TestSnippet::TestSnippet()
{
    m_propertyPublisher = new SnippetPropertyPublisher;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TestSnippet::~TestSnippet()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::setTestDataDir(const cvf::String& testDataDir)
{
    m_testDataDir = testDataDir;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool TestSnippet::initializeSnippet(cvf::OpenGLContext* oglContext)
{
    CVF_ASSERT(oglContext);
    m_openGLContext = oglContext;

    m_renderSequence = new cvf::RenderSequence;
    m_camera = new cvf::Camera;

    m_trackball = new cvf::ManipulatorTrackball;
    m_trackball->setCamera(m_camera.p());

    ref<cvf::Scene> scene = new cvf::Scene;
    ref<cvf::Rendering> rendering = new cvf::Rendering("snipRendering");
    rendering->setScene(scene.p());
    rendering->setCamera(m_camera.p());

    ref<cvf::RenderQueueSorter> sorter = new cvf::RenderQueueSorterBasic(cvf::RenderQueueSorterBasic::EFFECT_ONLY);
    rendering->setRenderQueueSorter(sorter.p());

    m_renderSequence->addRendering(rendering.p());

    m_openGLContext->makeCurrent();
    CVF_CHECK_OGL(m_openGLContext.p());
    bool initOK = onInitialize();
    CVF_CHECK_OGL(m_openGLContext.p());

    return initOK;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::destroySnippet()
{
    CVF_ASSERT(m_openGLContext.notNull());
    m_openGLContext->makeCurrent();

    if (m_renderSequence.notNull())
    {
        m_renderSequence->deleteOrReleaseOpenGLResources(m_openGLContext.p());
    }

    cvf::OpenGLResourceManager* resourceManager = m_openGLContext->resourceManager();
    CVF_ASSERT(resourceManager);
    resourceManager->deleteAllOpenGLResources(m_openGLContext.p());

    m_openGLContext = NULL;
    m_trackball = NULL;
    m_camera = NULL;
    m_renderSequence = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::RenderSequence* TestSnippet::renderSequence()
{
    return m_renderSequence.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::RenderSequence* TestSnippet::renderSequence() const
{
    return m_renderSequence.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::Camera* TestSnippet::camera()
{
    return m_camera.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const cvf::Camera* TestSnippet::camera() const
{
    return m_camera.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::onResizeEvent(int width, int height)
{
    cvf::uint uiWidth  = width  > 0 ? static_cast<cvf::uint>(width) : 0;
    cvf::uint uiHeight = height > 0 ? static_cast<cvf::uint>(height) : 0;
    if (m_camera.notNull())
    {
        m_camera->setViewport(0, 0, uiWidth, uiHeight);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::onUpdateAnimation(double animationTime, PostEventAction* postEventAction)
{
    CVF_UNUSED(animationTime);

    if (postEventAction)
    {
        *postEventAction = NONE;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::onPaintEvent(PostEventAction* postEventAction)
{
    CVF_ASSERT(m_openGLContext.notNull());
    m_openGLContext->makeCurrent();

    CVF_CHECK_OGL(m_openGLContext.p());
    m_renderSequence->render(m_openGLContext.p());
    CVF_CHECK_OGL(m_openGLContext.p());

    if (postEventAction)
    {
        *postEventAction = NONE;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent)
{
    //cvf::Trace::show("onMousePressEvent()   bn=%d  %s", buttonPressed, mouseEvent->toString().toAscii().ptr());
    CVF_UNUSED(buttonPressed);

    if (buttonPressed == LeftButton && mouseEvent->modifiers() == ControlModifier)
    {
        cvf::DebugTimer tim("Pick time");

        cvf::Rendering* r = m_renderSequence->firstRendering();
        ref<cvf::RayIntersectSpec> ris = r->rayIntersectSpecFromWindowCoordinates(mouseEvent->x(), mouseEvent->y());
        
        cvf::HitItemCollection hic;
        if (r->rayIntersect(*ris, &hic))
        {
            const cvf::HitItem* item = hic.firstItem();
            CVF_ASSERT(item && item->part());

            cvf::Vec3d rotPt = item->intersectionPoint();
            m_trackball->setRotationPoint(rotPt);
            cvf::Trace::show("Rotation point set to: %lf %lf %lf", rotPt.x(), rotPt.y(), rotPt.z());
        }

        tim.reportTimeMS();

        return;
    }

    cvf::ManipulatorTrackball::NavigationType navType = getNavigationTypeFromMouseButtons(mouseEvent->buttons(), mouseEvent->modifiers());
    m_trackball->startNavigation(navType, mouseEvent->x(), mouseEvent->y());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::onMouseMoveEvent(MouseEvent* mouseEvent)
{
    //cvf::Trace::show("onMouseMoveEvent()  %s", mouseEvent->toString().toAscii().ptr());
    
    cvf::ManipulatorTrackball::NavigationType navType = getNavigationTypeFromMouseButtons(mouseEvent->buttons(), mouseEvent->modifiers());
    if (navType != m_trackball->activeNavigation())
    {
        m_trackball->startNavigation(navType, mouseEvent->x(), mouseEvent->y());
    }

    bool needRedraw = m_trackball->updateNavigation(mouseEvent->x(), mouseEvent->y());
    if (needRedraw)
    {
        mouseEvent->setRequestedAction(REDRAW);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::onMouseReleaseEvent(MouseButton buttonReleased, MouseEvent* mouseEvent)
{
    //cvf::Trace::show("onMouseReleaseEvent() bn=%d  %s", buttonReleased, mouseEvent->toString().toAscii().ptr());
    CVF_UNUSED(buttonReleased);

    cvf::ManipulatorTrackball::NavigationType navType = getNavigationTypeFromMouseButtons(mouseEvent->buttons(), mouseEvent->modifiers());
    if (navType != cvf::ManipulatorTrackball::NONE)
    {
        m_trackball->startNavigation(navType, mouseEvent->x(), mouseEvent->y());
    }
    else
    {
        m_trackball->endNavigation();
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ManipulatorTrackball::NavigationType TestSnippet::getNavigationTypeFromMouseButtons(MouseButtons mouseButtons, KeyboardModifiers keyboardModifiers)
{
    if (mouseButtons == LeftButton)
    {
        return cvf::ManipulatorTrackball::PAN;
    }
    else if (mouseButtons == RightButton)
    {
        return cvf::ManipulatorTrackball::ROTATE;
    }
    else if (mouseButtons == MiddleButton || mouseButtons == (LeftButton | RightButton))
    {
        if (keyboardModifiers == ShiftModifier)
        {
            return cvf::ManipulatorTrackball::ZOOM;
        }
        else
        {
            return cvf::ManipulatorTrackball::WALK;
        }
    }
    else
    {
        return cvf::ManipulatorTrackball::NONE;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::onKeyPressEvent(KeyEvent* keyEvent)
{
    CVF_UNUSED(keyEvent);
    //cvf::Trace::show("onKeyPressEvent()  %s", keyEvent->toString().toAscii().ptr());
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
SnippetPropertyPublisher* TestSnippet::propertyPublisher()
{
    return m_propertyPublisher.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void TestSnippet::onPropertyChanged(Property* property, PostEventAction* postEventAction)
{
    CVF_UNUSED(property);
    CVF_UNUSED(postEventAction);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::String> TestSnippet::helpText() const
{
    std::vector<cvf::String> defaultHelpText;
    defaultHelpText.push_back("Default help text for this snippet. Please update.");

    return defaultHelpText;
}



} // namespace cvfu

