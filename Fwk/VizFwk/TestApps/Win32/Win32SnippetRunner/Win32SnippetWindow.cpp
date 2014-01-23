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
#include "Win32SnippetWindow.h"
#include "Win32OpenGLContext.h"
#include "Win32MessageKicker.h"
#include "Win32Utils.h"

// User defined message that will be sent from the message kicker to get periodic anim updates
const unsigned int W32SR_ANIM_UPDATE_MESSAGE = WM_USER + 1;



//==================================================================================================
//
// Win32SnippetWindow
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Win32SnippetWindow::Win32SnippetWindow()
:   m_hwnd(NULL),
    m_messageKicker(NULL)
{
    m_contextGroup = new cvf::OpenGLContextGroup;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Win32SnippetWindow::~Win32SnippetWindow()
{
    // Should already be destroyed
    CVF_ASSERT(m_hwnd == NULL);
    CVF_ASSERT(m_openGLContext.isNull());
    CVF_ASSERT(m_snippet.isNull());

    CVF_ASSERT(m_messageKicker == NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Win32SnippetWindow::create(HWND parentWnd, cvfu::TestSnippet* snippet)
{
    CVF_ASSERT(parentWnd);
    CVF_ASSERT(snippet);

    CVF_ASSERT(m_hwnd == NULL);
    CVF_ASSERT(m_openGLContext.isNull());

    // Is it safe to always do this?
    Win32SnippetWindow::registerWndClass();

    // Must set the snippet pointer first it will be accessed during window creation
    m_snippet = snippet;

    m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
             _T("Win32SnippetWindowClass"), NULL,
             WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,  
             0, 0,
             0, 0,
             parentWnd, 
             NULL, 
             ::GetModuleHandle(NULL), 
             this);

    if (!m_hwnd)
    {
        m_snippet = NULL;
        return false;
    }

    m_messageKicker = new Win32MessageKicker(m_hwnd, W32SR_ANIM_UPDATE_MESSAGE);
    m_messageKicker->start(5);

    m_currAnimationTime = new cvf::Timer;
    m_currAnimationTime->restart();

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32SnippetWindow::registerWndClass()
{
    // Register window class
    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Note: CS_OWNDC was added to be able to run with Mesa 7.7.x Does not seem to hurt, so left on.
    wcex.lpfnWndProc	= Win32SnippetWindow::msgRouter;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= ::GetModuleHandle(NULL);
    wcex.hIcon			= NULL;
    wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);;
    wcex.hbrBackground	= NULL;
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= _T("Win32SnippetWindowClass");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32SnippetWindow::destroy()
{
    if (m_hwnd)
    {
        ::DestroyWindow(m_hwnd);

        CVF_ASSERT(m_openGLContext.isNull());
        CVF_ASSERT(m_snippet.isNull());

        m_hwnd = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
HWND Win32SnippetWindow::windowHandle()
{
    return m_hwnd;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LRESULT CALLBACK Win32SnippetWindow::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
        case WM_CREATE:
        {
            // Create the render context
            CVF_ASSERT(m_contextGroup.notNull());
            CVF_ASSERT(m_openGLContext.isNull());
            m_openGLContext = new Win32OpenGLContext(m_contextGroup.p());
            bool createOK = m_openGLContext->createHardwareContext(hwnd);
            CVF_ASSERT(createOK);

            // Initialize the snippet
            m_openGLContext->makeCurrent();
            m_snippet->initializeSnippet(m_openGLContext.p());

            return 0;
        }
        
		case WM_MOUSEACTIVATE:
		{
			// Set keyboard focus to our window if activated with mouse
			SetFocus(hwnd);
            return 0;
		}

        case WM_SIZE:
        {
            if (m_snippet.notNull()) m_snippet->onResizeEvent(LOWORD(lParam), HIWORD(lParam));
            PostMessage(hwnd, WM_PAINT, 0, 0);
            return 0;
        }

        case W32SR_ANIM_UPDATE_MESSAGE:
        {
            if (m_snippet.notNull())
            {
                double animTime = m_currAnimationTime->time();
                cvfu::PostEventAction postEventAction = cvfu::NONE;
                m_snippet->onUpdateAnimation(animTime, &postEventAction);
                if (postEventAction == cvfu::REDRAW)
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }

            return 0;
        }
        
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
		    HDC hdc = BeginPaint(hwnd, &ps);

            cvfu::PostEventAction postEventAction = cvfu::NONE;

            if (m_snippet.notNull()) 
            {
                m_snippet->onPaintEvent(&postEventAction);
                SwapBuffers(hdc);            
            }

            EndPaint(hwnd, &ps);

            if (postEventAction == cvfu::REDRAW)
            {
                PostMessage(hwnd, WM_PAINT, 0, 0);
            }

            break;
        }

        case WM_MOUSEMOVE:
        {
            if (m_snippet.notNull())            
            {
                RECT r;
                GetClientRect(m_hwnd, &r);
                MouseEvent me = Win32Utils::translateMouseMessageToMouseEvent(r.bottom, wParam, lParam);
                m_snippet->onMouseMoveEvent(&me);
                if (me.requestedAction() == REDRAW) 
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }

            return 0;
        }

        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            if (m_snippet.notNull())            
            {
                MouseButton buttonPressed = NoButton;
                if      (message == WM_LBUTTONDOWN) buttonPressed = LeftButton;
                else if (message == WM_MBUTTONDOWN) buttonPressed = MiddleButton;
                else if (message == WM_RBUTTONDOWN) buttonPressed = RightButton;
                
                RECT r;
                GetClientRect(m_hwnd, &r);
                MouseEvent me = Win32Utils::translateMouseMessageToMouseEvent(r.bottom, wParam, lParam);                
                m_snippet->onMousePressEvent(buttonPressed, &me);
                if (me.requestedAction() == REDRAW) 
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }

            return 0;
        }

        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        {
            if (m_snippet.notNull())            
            {
                MouseButton buttonReleased = NoButton;
                if      (message == WM_LBUTTONUP) buttonReleased = LeftButton;
                else if (message == WM_MBUTTONUP) buttonReleased = MiddleButton;
                else if (message == WM_RBUTTONUP) buttonReleased = RightButton;
                
                RECT r;
                GetClientRect(m_hwnd, &r);
                MouseEvent me = Win32Utils::translateMouseMessageToMouseEvent(r.bottom, wParam, lParam);
                
                m_snippet->onMouseReleaseEvent(buttonReleased, &me);
                if (me.requestedAction() == REDRAW) 
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }

            return 0;
        }

        case WM_KEYDOWN:
        {
            if (m_snippet.notNull())            
            {
                KeyEvent ke = Win32Utils::translateKeyDownMessageToKeyEvent(wParam, lParam);

                m_snippet->onKeyPressEvent(&ke);
                if (ke.requestedAction() == REDRAW) 
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }

            return 0;
        }

        case WM_DESTROY:
        {
            if (m_messageKicker)
            {
                m_messageKicker->stop();
                delete m_messageKicker;
                m_messageKicker = NULL;
            }

            if (m_snippet.notNull())
            {
                m_snippet->destroySnippet();
                m_snippet = NULL;
            }

            if (m_openGLContext.notNull())
            {
                m_openGLContext->shutdownContext();
                m_openGLContext = NULL;
            }
        
            CVF_ASSERT(m_contextGroup.notNull());
            CVF_ASSERT(m_contextGroup->contextCount() == 0);

            break;
        }

        default:
    		return DefWindowProc(hwnd, message, wParam, lParam);
	}

    return 0;

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LRESULT CALLBACK Win32SnippetWindow::msgRouter(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE) 
    {   
        // If this message gets sent then a new window has just been created.
        // Store the pointer as user data with the window
        CREATESTRUCT* createStruct = (LPCREATESTRUCT)lParam;
        Win32SnippetWindow* windowObjPtr = reinterpret_cast<Win32SnippetWindow*>(createStruct->lpCreateParams);        

        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)windowObjPtr);
    }

    // Pointer to the window that should receive the message
    Win32SnippetWindow* destinationWindowObj = reinterpret_cast<Win32SnippetWindow*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));        

    if (destinationWindowObj)
    {
        return destinationWindowObj->wndProc(hwnd, message, wParam, lParam);
    }
    else
    {
        // For messages that arrive prior to WM_NCCREATE and the HWND <-> WindowPointer association was not made
        return ::DefWindowProc (hwnd, message, wParam, lParam);
    }
}


