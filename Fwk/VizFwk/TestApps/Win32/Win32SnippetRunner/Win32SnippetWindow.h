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

class Win32OpenGLContext;
class Win32MessageKicker;


//==================================================================================================
//
// 
//
//==================================================================================================
class Win32SnippetWindow
{
public:
    Win32SnippetWindow();
    ~Win32SnippetWindow();

    bool    create(HWND parentWnd, cvfu::TestSnippet* snippet);
    void    destroy();
    HWND    windowHandle();

private:
    LRESULT CALLBACK        wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK msgRouter(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    static void             registerWndClass();

private:
    HWND                         m_hwnd;            // Window handle
    ref<cvf::OpenGLContextGroup> m_contextGroup;    // The context group
    ref<Win32OpenGLContext>      m_openGLContext;   // This window's OpenGL context
    ref<cvfu::TestSnippet>       m_snippet;         // Pointer to current snippet

    Win32MessageKicker*     m_messageKicker;        // Used for posting a predefined message to ourselves at a fixed interval. Used for animation
    ref<cvf::Timer>         m_currAnimationTime;    // Keeps track of current animation time.
};


