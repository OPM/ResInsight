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


#include "QSRStdInclude.h"
#include "QSRTranslateEvent.h"

#include <QMouseEvent>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfu::MouseEvent QSRTranslateEvent::translateMouseEvent(int widgetHeight, const QMouseEvent& event)
{
    int xPos = event.x();
    int yPos = widgetHeight - event.y();

    Qt::MouseButtons qtButtons = event.buttons();
    cvfu::MouseButtons buttons = cvfu::NoButton;
    if (qtButtons & Qt::LeftButton)     buttons |= cvfu::LeftButton;
    if (qtButtons & Qt::MidButton)      buttons |= cvfu::MiddleButton;
    if (qtButtons & Qt::RightButton)    buttons |= cvfu::RightButton;

    Qt::KeyboardModifiers qtModifiers = event.modifiers();
    cvfu::KeyboardModifiers modifiers = cvfu::NoModifier;
    if (qtModifiers & Qt::ShiftModifier)      modifiers |= cvfu::ShiftModifier;
    if (qtModifiers & Qt::ControlModifier)    modifiers |= cvfu::ControlModifier;

    return cvfu::MouseEvent(xPos, yPos, buttons, modifiers);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfu::MouseButton QSRTranslateEvent::translateMouseButton(Qt::MouseButton qtMouseButton)
{
    if (qtMouseButton == Qt::LeftButton)    return cvfu::LeftButton;
    if (qtMouseButton == Qt::MidButton)     return cvfu::MiddleButton;
    if (qtMouseButton == Qt::RightButton)   return cvfu::RightButton;

    return cvfu::NoButton;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvfu::KeyEvent QSRTranslateEvent::translateKeyEvent(const QKeyEvent& event)
{
    cvfu::Key key = cvfu::Key_Unknown;

    int qtKey = event.key();
    switch (qtKey)
    {
        case Qt::Key_0:  key = cvfu::Key_0; break;
        case Qt::Key_1:  key = cvfu::Key_1; break;
        case Qt::Key_2:  key = cvfu::Key_2; break;
        case Qt::Key_3:  key = cvfu::Key_3; break;
        case Qt::Key_4:  key = cvfu::Key_4; break;
        case Qt::Key_5:  key = cvfu::Key_5; break;
        case Qt::Key_6:  key = cvfu::Key_6; break;
        case Qt::Key_7:  key = cvfu::Key_7; break;
        case Qt::Key_8:  key = cvfu::Key_8; break;
        case Qt::Key_9:  key = cvfu::Key_9; break;

        case Qt::Key_A:  key = cvfu::Key_A; break;
        case Qt::Key_B:  key = cvfu::Key_B; break;
        case Qt::Key_C:  key = cvfu::Key_C; break;
        case Qt::Key_D:  key = cvfu::Key_D; break;
        case Qt::Key_E:  key = cvfu::Key_E; break;
        case Qt::Key_F:  key = cvfu::Key_F; break;
        case Qt::Key_G:  key = cvfu::Key_G; break;
        case Qt::Key_H:  key = cvfu::Key_H; break;
        case Qt::Key_I:  key = cvfu::Key_I; break;
        case Qt::Key_J:  key = cvfu::Key_J; break;
        case Qt::Key_K:  key = cvfu::Key_K; break;
        case Qt::Key_L:  key = cvfu::Key_L; break;
        case Qt::Key_M:  key = cvfu::Key_M; break;
        case Qt::Key_N:  key = cvfu::Key_N; break;
        case Qt::Key_O:  key = cvfu::Key_O; break;
        case Qt::Key_P:  key = cvfu::Key_P; break;
        case Qt::Key_Q:  key = cvfu::Key_Q; break;
        case Qt::Key_R:  key = cvfu::Key_R; break;
        case Qt::Key_S:  key = cvfu::Key_S; break;
        case Qt::Key_T:  key = cvfu::Key_T; break;
        case Qt::Key_U:  key = cvfu::Key_U; break;
        case Qt::Key_V:  key = cvfu::Key_V; break;
        case Qt::Key_W:  key = cvfu::Key_W; break;
        case Qt::Key_X:  key = cvfu::Key_X; break;
        case Qt::Key_Y:  key = cvfu::Key_Y; break;
        case Qt::Key_Z:  key = cvfu::Key_Z; break;

        case Qt::Key_Return:        key = cvfu::Key_Return;       break;
        case Qt::Key_Backspace:     key = cvfu::Key_Backspace;    break;
        case Qt::Key_Tab:           key = cvfu::Key_Tab;          break;
        case Qt::Key_Space:         key = cvfu::Key_Space;        break;

        case Qt::Key_Control:       key = cvfu::Key_Control;      break;
        case Qt::Key_Alt:           key = cvfu::Key_Alt;          break;
        case Qt::Key_Shift:         key = cvfu::Key_Shift;        break;
        case Qt::Key_Insert:        key = cvfu::Key_Insert;       break;
        case Qt::Key_Delete:        key = cvfu::Key_Delete;       break;
        case Qt::Key_Home:          key = cvfu::Key_Home;         break;
        case Qt::Key_End:           key = cvfu::Key_End;          break;
        case Qt::Key_PageUp:        key = cvfu::Key_PageUp;       break;
        case Qt::Key_PageDown:      key = cvfu::Key_PageDown;     break;
        case Qt::Key_Left:          key = cvfu::Key_Left;         break;
        case Qt::Key_Right:         key = cvfu::Key_Right;        break;
        case Qt::Key_Up:            key = cvfu::Key_Up;           break;
        case Qt::Key_Down:          key = cvfu::Key_Down;         break;
        case Qt::Key_Clear:         key = cvfu::Key_Clear;        break;  
        case Qt::Key_Escape:        key = cvfu::Key_Escape;       break;
        case Qt::Key_Plus:          key = cvfu::Key_Plus;         break;
        case Qt::Key_Minus:         key = cvfu::Key_Minus;        break;
        case Qt::Key_Comma:         key = cvfu::Key_Comma;        break;
        case Qt::Key_Period:        key = cvfu::Key_Period;       break;
        case Qt::Key_F1:            key = cvfu::Key_F1;           break;
        case Qt::Key_F2:            key = cvfu::Key_F2;           break;
        case Qt::Key_F3:            key = cvfu::Key_F3;           break;
        case Qt::Key_F4:            key = cvfu::Key_F4;           break;
        case Qt::Key_F5:            key = cvfu::Key_F5;           break;
        case Qt::Key_F6:            key = cvfu::Key_F6;           break;
        case Qt::Key_F7:            key = cvfu::Key_F7;           break;
        case Qt::Key_F8:            key = cvfu::Key_F8;           break;
        case Qt::Key_F9:            key = cvfu::Key_F9;           break;
        case Qt::Key_F10:           key = cvfu::Key_F10;          break;
        case Qt::Key_F11:           key = cvfu::Key_F11;          break;
        case Qt::Key_F12:           key = cvfu::Key_F12;          break;
    }


    unsigned short unicodeCharacter = 0;
    //char character = 0;

    QString qtText = event.text();
    if (qtText.length() == 1 )
    {
        QChar qtChar = qtText[0];
        unicodeCharacter = qtChar.unicode();
        //character = qtChar.toLatin1();
    }

	cvfu::KeyEvent keyEvent(key, unicodeCharacter);
    //Trace::show(keyEvent.toString());

    return keyEvent;
}


