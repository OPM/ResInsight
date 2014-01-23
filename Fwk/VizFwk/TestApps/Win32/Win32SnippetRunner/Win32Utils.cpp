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
#include <WinUser.h>
#include "Win32Utils.h"


//==================================================================================================
//
// Win32Utils
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Translate data payload of a windows mouse message to a MouseEvent
/// 
/// The data payload that is translated is typically from a WM_MOUSEMOVE, WM_XBUTTONDOWN or 
/// WM_XBUTTONUP windows message.
//--------------------------------------------------------------------------------------------------
MouseEvent Win32Utils::translateMouseMessageToMouseEvent(int windowHeight, WPARAM wParam, LPARAM lParam)
{
    int xPos = LOWORD(lParam);
    int yPos = windowHeight - HIWORD(lParam);

    MouseButtons buttons = NoButton;
    if (wParam & MK_LBUTTON)    buttons |= LeftButton;
    if (wParam & MK_MBUTTON)    buttons |= MiddleButton;
    if (wParam & MK_RBUTTON)    buttons |= RightButton;

    KeyboardModifiers modifiers = NoModifier;
    if (wParam & MK_SHIFT)      modifiers |= ShiftModifier;
    if (wParam & MK_CONTROL)    modifiers |= ControlModifier;

    return MouseEvent(xPos, yPos, buttons, modifiers);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
KeyEvent Win32Utils::translateKeyDownMessageToKeyEvent(WPARAM wParam, LPARAM lParam)
{
    UINT virtualKeyCode = static_cast<UINT>(wParam);

    Key key = Key_Unknown;
    switch (virtualKeyCode)
    {
        case '0':               key = Key_0;            break;
        case '1':               key = Key_1;            break;
        case '2':               key = Key_2;            break;
        case '3':               key = Key_3;            break;
        case '4':               key = Key_4;            break;
        case '5':               key = Key_5;            break;
        case '6':               key = Key_6;            break;
        case '7':               key = Key_7;            break;
        case '8':               key = Key_8;            break;
        case '9':               key = Key_9;            break;

        case 'A':               key = Key_A;            break;
        case 'B':               key = Key_B;            break;
        case 'C':               key = Key_C;            break;
        case 'D':               key = Key_D;            break;
        case 'E':               key = Key_E;            break;
        case 'F':               key = Key_F;            break;
        case 'G':               key = Key_G;            break;
        case 'H':               key = Key_H;            break;
        case 'I':               key = Key_I;            break;
        case 'J':               key = Key_J;            break;
        case 'K':               key = Key_K;            break;
        case 'L':               key = Key_L;            break;
        case 'M':               key = Key_M;            break;
        case 'N':               key = Key_N;            break;
        case 'O':               key = Key_O;            break;
        case 'P':               key = Key_P;            break;
        case 'Q':               key = Key_Q;            break;
        case 'R':               key = Key_R;            break;
        case 'S':               key = Key_S;            break;
        case 'T':               key = Key_T;            break;
        case 'U':               key = Key_U;            break;
        case 'V':               key = Key_V;            break;
        case 'W':               key = Key_W;            break;
        case 'X':               key = Key_X;            break;
        case 'Y':               key = Key_Y;            break;
        case 'Z':               key = Key_Z;            break;

        case VK_RETURN:         key = Key_Return;       break;
        case VK_BACK:           key = Key_Backspace;    break;
        case VK_TAB:            key = Key_Tab;          break;
        case VK_SPACE:          key = Key_Space;        break;

        case VK_CONTROL:        key = Key_Control;      break;
        case VK_MENU:           key = Key_Alt;          break;
        case VK_SHIFT:          key = Key_Shift;        break;
        case VK_INSERT:         key = Key_Insert;       break;
        case VK_DELETE:         key = Key_Delete;       break;
        case VK_HOME:           key = Key_Home;         break;
        case VK_END:            key = Key_End;          break;
        case VK_PRIOR:          key = Key_PageUp;       break;
        case VK_NEXT:           key = Key_PageDown;     break;
        case VK_LEFT:           key = Key_Left;         break;
        case VK_RIGHT:          key = Key_Right;        break;
        case VK_UP:             key = Key_Up;           break;
        case VK_DOWN:           key = Key_Down;         break;
        case VK_CLEAR:          key = Key_Clear;        break;  
        case VK_ESCAPE:         key = Key_Escape;       break;
        case VK_OEM_PLUS:       key = Key_Plus;         break;
        case VK_OEM_MINUS:      key = Key_Minus;        break;
        case VK_OEM_COMMA:      key = Key_Comma;        break;
        case VK_OEM_PERIOD:     key = Key_Period;       break;
        case VK_F1:             key = Key_F1;           break;
        case VK_F2:             key = Key_F2;           break;
        case VK_F3:             key = Key_F3;           break;
        case VK_F4:             key = Key_F4;           break;
        case VK_F5:             key = Key_F5;           break;
        case VK_F6:             key = Key_F6;           break;
        case VK_F7:             key = Key_F7;           break;
        case VK_F8:             key = Key_F8;           break;
        case VK_F9:             key = Key_F9;           break;
        case VK_F10:            key = Key_F10;          break;
        case VK_F11:            key = Key_F11;          break;
        case VK_F12:            key = Key_F12;          break;
    }


    unsigned short unicodeCharacter = 0;

    // Last param should be MAPVK_VK_TO_VSC, but it seems to be missing from headers (still, value appears to be 0)
    UINT scanCode = ::MapVirtualKey(virtualKeyCode, 0);

    BYTE keyState[256];
    memset(keyState, 0, sizeof(keyState));
    ::GetKeyboardState(keyState);

    // Translate the virtual-key code and keyboard state to the corresponding Unicode character (or characters)
    WCHAR unicodeArr[4];
    memset(unicodeArr, 0, sizeof(unicodeArr));
    if (::ToUnicode(virtualKeyCode, scanCode, keyState, unicodeArr, 4, 0) == 1)
    {
        unicodeCharacter = unicodeArr[0];
    }

    KeyEvent keyEvent(key, unicodeCharacter);
    //Trace::show(keyEvent.toString());

    return keyEvent;
}
