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


#pragma once

#include "cvfObject.h"
#include "cvfuInputTypes.h"
#include "cvfManipulatorTrackball.h"
#include "cvfCollection.h"
#include "cvfString.h"

namespace cvf {
    class OpenGLContext;
    class RenderSequence;
    class Camera;
}

namespace cvfu {

class MouseEvent;
class KeyEvent;
class Property;
class SnippetPropertyPublisher;



//==================================================================================================
//
// Base class for test snippets
//
//==================================================================================================
class TestSnippet : public cvf::Object
{
public:
    TestSnippet();
    virtual ~TestSnippet();

    virtual const char*         name() const = 0;

    void                        setTestDataDir(const cvf::String& testDataDir);
    bool                        initializeSnippet(cvf::OpenGLContext* oglContext);
    void                        destroySnippet();
    cvf::RenderSequence*        renderSequence();
    const cvf::RenderSequence*  renderSequence() const;
    cvf::Camera*                camera();
    const cvf::Camera*          camera() const;

    virtual bool                onInitialize() = 0;
    virtual void                onResizeEvent(int width, int height);
    virtual void                onUpdateAnimation(double animationTime, PostEventAction* postEventAction);
    virtual void                onPaintEvent(PostEventAction* postEventAction);

    virtual void                onMousePressEvent(MouseButton buttonPressed, MouseEvent* mouseEvent);
    virtual void                onMouseMoveEvent(MouseEvent* mouseEvent);
    virtual void                onMouseReleaseEvent(MouseButton buttonReleased, MouseEvent* mouseEvent);
    virtual void                onKeyPressEvent(KeyEvent* keyEvent);

    SnippetPropertyPublisher*   propertyPublisher();
    virtual void                onPropertyChanged(Property* property, PostEventAction* postEventAction);

    virtual std::vector<cvf::String> helpText() const;

private:
    static cvf::ManipulatorTrackball::NavigationType getNavigationTypeFromMouseButtons(MouseButtons mouseButtons, KeyboardModifiers keyboardModifiers);

protected:
    cvf::String                         m_testDataDir;          // Directory where test data resides (set by snippet runner)
    cvf::ref<cvf::OpenGLContext>        m_openGLContext;        // 
    cvf::ref<cvf::RenderSequence>       m_renderSequence;       // A render sequence configured with one Rendering containing one Scene
    cvf::ref<cvf::Camera>               m_camera;               // Direct reference to the Rendering's Camera
    cvf::ref<cvf::ManipulatorTrackball> m_trackball;            // Manipulator, already configured with the camera
    cvf::ref<SnippetPropertyPublisher>  m_propertyPublisher;    // For publishing any properties the snippet wants to expose
};

}


// Macro to use in header file when declaring a new snippet
#define CVFU_DECLARE_SNIPPET(SNIPPET_NAME) \
    virtual const char* name() const { return SNIPPET_NAME; }
