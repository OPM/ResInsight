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
#include "resource.h"
#include "Windowsx.h"

#include "Win32SnippetWindow.h"
#include "Win32PropertiesPanel.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include "SnippetFactoryBasis.h"



// Global Variable current instance
HINSTANCE g_hInstance = NULL;	


// Forward declarations of functions included in this code module:
LRESULT CALLBACK	MainWndProc(HWND, UINT, WPARAM, LPARAM);
LONG WINAPI         HandleCommandMessage(HWND hWnd, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    cvf::ShaderSourceProvider* shaderProvider = cvf::ShaderSourceProvider::instance();
    shaderProvider->setSourceRepository(new cvf::ShaderSourceRepositoryFile("../../../LibRender/glsl/"));
    shaderProvider->addFileSearchDirectory("../../../Tests/SnippetsBasis/Shaders/");
    shaderProvider->addFileSearchDirectory("./");

    const cvf::String testDataDir = "../../../Tests/TestData/";

    cvfu::SnippetFactory* factoryBasis = new SnippetFactoryBasis;
    factoryBasis->setTestDataDir(testDataDir);
    cvfu::SnippetRegistry::instance()->addFactory(factoryBasis);

    // Store instance handle in our global variable
    g_hInstance = hInstance; 

    // Register main window class
    WNDCLASSEX wcex;
    wcex.cbSize         = sizeof(WNDCLASSEX);
    wcex.style			= CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc	= MainWndProc;
    wcex.cbClsExtra		= 0;
    wcex.cbWndExtra		= 0;
    wcex.hInstance		= hInstance;
    wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32SNIPPETRUNNER));
    wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32SNIPPETRUNNER);
    wcex.lpszClassName	= _T("Win32SnippetRunnerMainWndClass");
    wcex.hIconSm		= NULL;
    RegisterClassEx(&wcex);

    const TCHAR* windowTitle = System::is64Bit() ? _T("Win32 Snippet Runner (64bit)") : _T("Win32 Snippet Runner");
    HWND hWnd = CreateWindow(_T("Win32SnippetRunnerMainWndClass"), windowTitle,
                             WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32SNIPPETRUNNER));

	// Main message loop
    MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

    DestroyWindow(hWnd);

	return (int) msg.wParam;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void ResizeChildWindowsHorizontal(HWND mainWnd, HWND leftChild, HWND rightChild, int leftChildWidth)
{
    RECT r;
    GetClientRect(mainWnd, &r);
    const int cx = r.right;
    const int cy = r.bottom;

    int rightPosX = 0;
    int rightSizeX = cx;

    if (leftChild)
    {
        MoveWindow(leftChild, 0, 0, CVF_MIN(leftChildWidth, cx), cy, TRUE);
        rightPosX += leftChildWidth;
        rightSizeX -= leftChildWidth;
    }

    if (rightChild)
    {
        MoveWindow(rightChild, rightPosX, 0, CVF_MAX(rightSizeX, 0), cy, TRUE);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static Win32SnippetWindow* sl_snippetWnd = NULL;
    static Win32PropertiesPanel* sl_propertiesPanelWnd = NULL;

    switch (message)
	{
        case WM_CREATE:
        {
            //ref<cvfu::TestSnippet> snippet = SnippetRegistry::instance()->createSnippet("snip::MinimalModel");
            //ref<cvfu::TestSnippet> snippet = SnippetRegistry::instance()->createSnippet("snip::Highlight");
            ref<cvfu::TestSnippet> snippet = SnippetRegistry::instance()->createSnippet("snip::Stencil");
            sl_snippetWnd = new Win32SnippetWindow;
            sl_snippetWnd->create(hWnd, snippet.p());
            SetFocus(sl_snippetWnd->windowHandle());

            //sl_propertiesPanelWnd = new Win32PropertiesPanel;
            //sl_propertiesPanelWnd->create(hWnd);

            return 0;
        }
        
        case WM_SIZE:
        {
            const int propPanelWidth = 200;
            HWND leftWnd = sl_propertiesPanelWnd ? sl_propertiesPanelWnd->windowHandle() : NULL;
            HWND rightWnd = sl_snippetWnd ? sl_snippetWnd->windowHandle() : NULL;
            ResizeChildWindowsHorizontal(hWnd, leftWnd, rightWnd, propPanelWidth);

            return 0;
        }

        case WM_COMMAND:
        {
            int wmId    = LOWORD(wParam);
            if (wmId == ID_FILE_SNIPPET_A || wmId == ID_FILE_SNIPPET_B)
            {
                ref<cvfu::TestSnippet> newSnippet;
                if (wmId == ID_FILE_SNIPPET_A) newSnippet = SnippetRegistry::instance()->createSnippet("snip::CubeMapping");
                if (wmId == ID_FILE_SNIPPET_B) newSnippet = SnippetRegistry::instance()->createSnippet("snip::PointSprites");

                sl_snippetWnd->destroy();
                delete sl_snippetWnd;
                sl_snippetWnd = new Win32SnippetWindow;
                sl_snippetWnd->create(hWnd, newSnippet.p());
                
                SetFocus(sl_snippetWnd->windowHandle());

                const int propPanelWidth = 200;
                HWND leftWnd = sl_propertiesPanelWnd ? sl_propertiesPanelWnd->windowHandle() : NULL;
                HWND rightWnd = sl_snippetWnd ? sl_snippetWnd->windowHandle() : NULL;
                ResizeChildWindowsHorizontal(hWnd, leftWnd, rightWnd, propPanelWidth);

                return 0;
            }

            LONG lRet = HandleCommandMessage(hWnd, wParam, lParam);
            return lRet;
        }

        case WM_DESTROY:
        {
            if (sl_propertiesPanelWnd)
            {
                sl_propertiesPanelWnd->destroy();
                delete sl_propertiesPanelWnd;
                sl_propertiesPanelWnd = NULL;
            }

            if (sl_snippetWnd)
            {
                sl_snippetWnd->destroy();
                delete sl_snippetWnd;
                sl_snippetWnd = NULL;
            }

            PostQuitMessage(0);
            break;
        }

        default:
    		return DefWindowProc(hWnd, message, wParam, lParam);
	}

    return 0;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LONG WINAPI HandleCommandMessage(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    int wmId    = LOWORD(wParam);
    int wmEvent = HIWORD(wParam);

    // Parse the menu selections:
    switch (wmId)
    {
        case IDM_ABOUT:
        {
            DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }

        case IDM_EXIT:
        {
            DestroyWindow(hWnd);
            break;
        }

        default:
        {
            // Unhandled WM_COMMAND message
            return 1;
        }
    }

    return 0;
}



//--------------------------------------------------------------------------------------------------
/// Message handler for about box.
//--------------------------------------------------------------------------------------------------
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	    case WM_INITDIALOG:
        {
		    return (INT_PTR)TRUE;
        }

	    case WM_COMMAND:
        {
		    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		    {
			    EndDialog(hDlg, LOWORD(wParam));
			    return (INT_PTR)TRUE;
		    }
            break;
        }
	}

    return (INT_PTR)FALSE;
}
