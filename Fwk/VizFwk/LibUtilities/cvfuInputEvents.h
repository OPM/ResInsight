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

#include "cvfString.h"
#include "cvfuInputTypes.h"

namespace cvfu {


//==================================================================================================
//
// MouseEvent
//
//==================================================================================================
class MouseEvent
{
public:
    MouseEvent(int x, int y, MouseButtons buttonsDown, KeyboardModifiers modifiersDown);

    int                 x() const;
    int                 y() const;
    MouseButtons        buttons() const;
    KeyboardModifiers   modifiers() const;

    void                setRequestedAction(PostEventAction actionRequest);
    PostEventAction     requestedAction() const;

    cvf::String         toString() const;

private:
    int                 m_x;                        // X-coordinate
    int                 m_y;                        // OpenGL style Y-coordinate with origin in lower left corner of the window
    MouseButtons        m_mouseButtonsDown;         // Combination of mouse buttons that are down
    KeyboardModifiers   m_keyboardModifiersDown;    // Combination of modifier keys that are down. 
    PostEventAction     m_requestedAction;
};



//==================================================================================================
//
// KeyEvent
//
//==================================================================================================
class KeyEvent
{
public:
    KeyEvent(Key key, unsigned short unicodeChar);

    Key                 key() const;
    char                character() const;
    unsigned short      unicodeCharacter() const;

    void                setRequestedAction(PostEventAction actionRequest);
    PostEventAction     requestedAction() const;

    cvf::String         toString() const;
    static cvf::String  toString(Key key);

public:
    Key                 m_key;
    unsigned short      m_unicodeChar;
    PostEventAction     m_requestedAction;
};


}


