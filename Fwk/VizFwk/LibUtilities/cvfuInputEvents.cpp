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
#include "cvfSystem.h"

#include "cvfuInputEvents.h"

namespace cvfu {



//==================================================================================================
///
/// \class cvfu::MouseEvent
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MouseEvent::MouseEvent(int x, int y, MouseButtons buttonsDown, KeyboardModifiers modifiersDown)
{
    m_x = x;
    m_y = y;
    m_mouseButtonsDown = buttonsDown;
    m_keyboardModifiersDown = modifiersDown;

    m_requestedAction = NONE;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int MouseEvent::x() const
{
    return m_x;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int MouseEvent::y() const
{
    return m_y;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MouseButtons MouseEvent::buttons() const
{
    return m_mouseButtonsDown;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
KeyboardModifiers MouseEvent::modifiers() const
{
    return m_keyboardModifiersDown;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MouseEvent::setRequestedAction(PostEventAction actionRequest)
{
    m_requestedAction = actionRequest;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PostEventAction MouseEvent::requestedAction() const
{
    return m_requestedAction;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::String MouseEvent::toString() const
{
    cvf::String btn;
    if (m_mouseButtonsDown.testFlag(LeftButton))   btn += "L";
    if (m_mouseButtonsDown.testFlag(MiddleButton)) btn += "M";
    if (m_mouseButtonsDown.testFlag(RightButton))  btn += "R";

    cvf::String mod;
    if (m_keyboardModifiersDown.testFlag(ShiftModifier))   mod += "S";
    if (m_keyboardModifiersDown.testFlag(ControlModifier)) mod += "C";

    cvf::String str = "MouseEvent:";
    str += " x=" + cvf::String(m_x) + " y=" + cvf::String(m_y) + " buttons=" + btn + " modifiers=" + mod;

    return str;
}



//==================================================================================================
///
/// \class cvfu::KeyEvent
/// \ingroup Utilities
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
KeyEvent::KeyEvent(Key key, unsigned short unicodeChar)
{
    m_key = key;
    m_unicodeChar = unicodeChar;

    m_requestedAction = NONE;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Key KeyEvent::key() const
{
    return m_key;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
char KeyEvent::character() const
{
    if (m_unicodeChar <= 0xff)
    {
        return static_cast<char>(m_unicodeChar);
    }
    else
    {
        return 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
unsigned short KeyEvent::unicodeCharacter() const
{
    return m_unicodeChar;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void KeyEvent::setRequestedAction(PostEventAction actionRequest)
{
    m_requestedAction = actionRequest;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PostEventAction KeyEvent::requestedAction() const
{
    return m_requestedAction;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::String KeyEvent::toString() const
{
    char szBuf[1024];
    cvf::String keyString = toString(m_key);
    unsigned char aChar = static_cast<unsigned char>(character());
    cvf::System::sprintf(szBuf, 1024, "KeyEvent: key=%s unicodeChar=0x%04x character=0x%02x", keyString.toAscii().ptr(), m_unicodeChar, aChar);
    
    cvf::String str(szBuf);
    return str;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::String KeyEvent::toString(Key key)
{
    switch (key)
    {
        case Key_None:          return "Key_None";        

        case Key_0:             return "Key_0";           
        case Key_1:             return "Key_1";           
        case Key_2:             return "Key_2";           
        case Key_3:             return "Key_3";           
        case Key_4:             return "Key_4";           
        case Key_5:             return "Key_5";           
        case Key_6:             return "Key_6";           
        case Key_7:             return "Key_7";           
        case Key_8:             return "Key_8";           
        case Key_9:             return "Key_9";           

        case Key_A:             return "Key_A";           
        case Key_B:             return "Key_B";           
        case Key_C:             return "Key_C";           
        case Key_D:             return "Key_D";           
        case Key_E:             return "Key_E";           
        case Key_F:             return "Key_F";           
        case Key_G:             return "Key_G";           
        case Key_H:             return "Key_H";           
        case Key_I:             return "Key_I";           
        case Key_J:             return "Key_J";           
        case Key_K:             return "Key_K";           
        case Key_L:             return "Key_L";           
        case Key_M:             return "Key_M";           
        case Key_N:             return "Key_N";           
        case Key_O:             return "Key_O";           
        case Key_P:             return "Key_P";           
        case Key_Q:             return "Key_Q";           
        case Key_R:             return "Key_R";           
        case Key_S:             return "Key_S";           
        case Key_T:             return "Key_T";           
        case Key_U:             return "Key_U";           
        case Key_V:             return "Key_V";           
        case Key_W:             return "Key_W";           
        case Key_X:             return "Key_X";           
        case Key_Y:             return "Key_Y";           
        case Key_Z:             return "Key_Z";           

        case Key_Return:        return "Key_Return";      
        case Key_Backspace:     return "Key_Backspace";   
        case Key_Tab:           return "Key_Tab";         
        case Key_Space:         return "Key_Space";       

        case Key_Control:       return "Key_Control";        
        case Key_Alt:           return "Key_Alt";         
        case Key_Shift:         return "Key_Shift";       
        case Key_Insert:        return "Key_Insert";      
        case Key_Delete:        return "Key_Delete";      
        case Key_Home:          return "Key_Home";        
        case Key_End:           return "Key_End";         
        case Key_PageUp:        return "Key_PageUp";      
        case Key_PageDown:      return "Key_PageDown";    
        case Key_Left:          return "Key_Left";        
        case Key_Right:         return "Key_Right";       
        case Key_Up:            return "Key_Up";          
        case Key_Down:          return "Key_Down";        
        case Key_Clear:         return "Key_Clear";       
        case Key_Escape:        return "Key_Escape";      
        case Key_Plus:          return "Key_Plus";      
        case Key_Minus:         return "Key_Minus";      
        case Key_Comma:         return "Key_Comma";      
        case Key_Period:        return "Key_Period";      
        case Key_F1:            return "Key_F1";          
        case Key_F2:            return "Key_F2";          
        case Key_F3:            return "Key_F3";          
        case Key_F4:            return "Key_F4";          
        case Key_F5:            return "Key_F5";          
        case Key_F6:            return "Key_F6";          
        case Key_F7:            return "Key_F7";          
        case Key_F8:            return "Key_F8";          
        case Key_F9:            return "Key_F9";          
        case Key_F10:           return "Key_F10";         
        case Key_F11:           return "Key_F11";         
        case Key_F12:           return "Key_F12";         

        case Key_Unknown:       return "Key_Unknown";     

        default:                return "UNKNOWN";     
    }
}


} // namespace cvfu

