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
#include "Win32PropertiesPanel.h"
#include "Windowsx.h"



//==================================================================================================
//
// Win32PropertiesPanel
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Win32PropertiesPanel::Win32PropertiesPanel()
:   m_hwnd(NULL)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Win32PropertiesPanel::~Win32PropertiesPanel()
{
    // Should already be destroyed
    CVF_ASSERT(m_hwnd == NULL);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Win32PropertiesPanel::create(HWND parentWnd)
{
    CVF_ASSERT(parentWnd);
    CVF_ASSERT(m_hwnd == NULL);

    // Is it safe to always do this?
    Win32PropertiesPanel::registerWndClass();

    m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
             _T("Win32SnippetRunnerPropertiesPanelWndClass"), NULL,
             WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,  
             0, 0,
             0, 0,
             parentWnd, 
             NULL, 
             ::GetModuleHandle(NULL), 
             this);

    if (!m_hwnd)
    {
        return false;
    }

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32PropertiesPanel::registerWndClass()
{
    // Register window class
    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style			= 0;  
    wcex.lpfnWndProc	= Win32PropertiesPanel::msgRouter;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= ::GetModuleHandle(NULL);
    wcex.hIcon			= NULL;
    wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);;
    wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
    wcex.lpszMenuName	= NULL;
    wcex.lpszClassName	= _T("Win32SnippetRunnerPropertiesPanelWndClass");
    wcex.hIconSm		= NULL;

    RegisterClassEx(&wcex);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32PropertiesPanel::destroy()
{
    if (m_hwnd)
    {
        ::DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
HWND Win32PropertiesPanel::windowHandle()
{
    return m_hwnd;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LRESULT CALLBACK Win32PropertiesPanel::wndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND sl_hwndCombo = NULL;

	switch (message)
	{
        case WM_CREATE:
        {
            HWND hwndStatic = CreateWindow(_T("STATIC"), _T("Heisann"), 
                WS_CHILD | WS_VISIBLE,
                0, 0, 50, 20,
                hwnd,
                NULL, 
                ::GetModuleHandle(NULL),
                NULL);

            SetWindowFont(hwndStatic, GetStockFont(DEFAULT_GUI_FONT), TRUE);

            sl_hwndCombo = CreateWindow(_T("COMBOBOX"), NULL, 
                                          CBS_DROPDOWNLIST |  WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                          50, 0, 149, 60,
                                          hwnd,
                                          NULL, 
                                          ::GetModuleHandle(NULL),
                                          NULL);

            SetWindowFont(sl_hwndCombo, GetStockFont(DEFAULT_GUI_FONT), TRUE);

            ComboBox_AddString(sl_hwndCombo, _T("Jalla1"));
            ComboBox_AddString(sl_hwndCombo, _T("Jalla2"));
            ComboBox_SetCurSel(sl_hwndCombo, 1);

            return 0;
        }
        
        case WM_SIZE:
        {
            return 0;
        }

        case WM_COMMAND:
        {
            int notifyCode = HIWORD(wParam);
            if ((HWND)lParam == sl_hwndCombo && notifyCode == CBN_SELCHANGE)
            {
                cvf::Trace::show("HitMe");
            }

            return 0;
        }

        default:
    		return DefWindowProc(hwnd, message, wParam, lParam);
	}

    return 0;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LRESULT CALLBACK Win32PropertiesPanel::msgRouter(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE) 
    {   
        // If this message gets sent then a new window has just been created.
        // Store the pointer as user data with the window
        CREATESTRUCT* createStruct = (LPCREATESTRUCT)lParam;
        Win32PropertiesPanel* windowObjPtr = reinterpret_cast<Win32PropertiesPanel*>(createStruct->lpCreateParams);        

        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)windowObjPtr);
    }

    // Pointer to the window that should receive the message
    Win32PropertiesPanel* destinationWindowObj = reinterpret_cast<Win32PropertiesPanel*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));        

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


