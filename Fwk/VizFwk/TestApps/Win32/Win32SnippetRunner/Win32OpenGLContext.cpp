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


#include "stdafx.h"
#include "Win32OpenGLContext.h"



//==================================================================================================
//
// Win32OpenGLContext
//
// Currently only suitable for a single global OpenGL context
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Win32OpenGLContext::Win32OpenGLContext(OpenGLContextGroup* contextGroup)
:   cvf::OpenGLContext(contextGroup)
{
    m_hWnd = NULL;
    m_hDC = NULL;
    m_hRC = NULL;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Win32OpenGLContext::~Win32OpenGLContext()
{
    CVF_ASSERT(m_hWnd == NULL);
    CVF_ASSERT(m_hDC == NULL);
    CVF_ASSERT(m_hRC == NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Win32OpenGLContext::createHardwareContext(HWND hWnd)
{
    CVF_ASSERT(m_hWnd == NULL);
    CVF_ASSERT(m_hDC == NULL);
    CVF_ASSERT(m_hRC == NULL);


    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;

    HDC hDC = ::GetDC(hWnd);
    int pf = ChoosePixelFormat(hDC, &pfd);
    if (pf == 0) 
    {
        Trace::show("ChoosePixelFormat() failed: Cannot find a suitable pixel format");
        return false;
    } 
 
    if (SetPixelFormat(hDC, pf, &pfd) == FALSE) 
    {
        Trace::show("SetPixelFormat() failed: Cannot set format specified.");
	    return false;
    } 

    DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    HGLRC hRC = wglCreateContext(hDC);
    if (!hRC)
    {
        Trace::show("wglCreateContext() failed: Creation of rendering context failed.");
        return false;

    }

    wglMakeCurrent(hDC, hRC);

    m_hWnd = hWnd;
    m_hDC = hDC;
    m_hRC = hRC;

    if (initializeContext())
    {
        return true;
    }
    else
    {
        return false;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32OpenGLContext::shutdownContext()
{
    // Clears up resources and removes us from our group
    cvf::OpenGLContext::shutdownContext();

    if (m_hRC)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(m_hRC);
        m_hRC = NULL;
    }

    if (m_hWnd)
    {
        if (m_hDC)
        {
            ::ReleaseDC(m_hWnd, m_hDC);
            m_hDC = NULL;
        }

        m_hWnd = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32OpenGLContext::makeCurrent()
{
    if (m_hDC && m_hRC)
    {
        wglMakeCurrent(m_hDC, m_hRC);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Win32OpenGLContext::isCurrent() const
{
    if (m_hDC && m_hRC)
    {
        if (wglGetCurrentContext() == m_hRC)
        {
            return true;
        }
    }

    return false;
}

