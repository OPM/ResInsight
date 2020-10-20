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

class QMouseEvent;
class QGraphicsSceneMouseEvent;

#include <Qt>

namespace caf
{
//==================================================================================================
//
// Helper class for storing mouse positions and button states. Should be reset whenever widget looses focus.
//
//==================================================================================================
class QtMouseState
{
public:
    QtMouseState();

    void updateFromMouseEvent( QMouseEvent* event );
    void updateFromMouseEvent( QGraphicsSceneMouseEvent* event );
    void reset();

    Qt::MouseButtons      mouseButtonState() const;
    Qt::KeyboardModifiers keyboardModifierFlags() const;
    Qt::MouseButton       cleanButtonClickButton() const;

public:
    static int numMouseButtonsInState( Qt::MouseButtons buttonState );

private:
    Qt::MouseButtons m_mouseButtonState; // Stores current mouse button state (combination of mouse buttons that are down)
    Qt::KeyboardModifiers m_keyboardModifierFlags; // Stores current keyboard modifier flags (combination of keyboard
                                                   // modifier keys that are down)

    int m_cleanButtonClickTolerance; // The movement tolerance in pixels for considering a mouse button press/release
                                     // sequence a clean button click
    Qt::MouseButton m_cleanButtonPressButton; // The mouse button that was last pressed 'cleanly', that is without any
                                              // other mouse buttons down. Used to detect clean button clicks (for
                                              // showing context menus etc)
    int m_cleanButtonPressPosX; // The position of the mouse cursor in widget coordinates of the last clean button
                                // press. Used to check if cursor has moved when button is released
    int             m_cleanButtonPressPosY; //
    Qt::MouseButton m_cleanButtonClickButton; // The button (if any) that was last clicked 'cleanly'.
};

} // namespace caf
