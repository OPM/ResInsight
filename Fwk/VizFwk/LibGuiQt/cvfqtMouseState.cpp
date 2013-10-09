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
#include "cvfMath.h"

#include "cvfqtMouseState.h"

#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>

#include <math.h>

namespace cvfqt {



//==================================================================================================
///
/// \class cvfqt::MouseState
/// \ingroup GuiQt
///
/// Helper class for storing mouse positions and button states. 
/// Should be reset whenever widget looses focus.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
MouseState::MouseState()
{
	m_cleanButtonClickTolerance = 3;

    reset();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MouseState::updateFromMouseEvent(QMouseEvent* event)
{
    // Always update with current state
    m_mouseButtonState = event->buttons();
    m_keyboardModifierFlags = event->modifiers();


    // Now do the events themselves

    // Mouse press events
    if (event->type() == QEvent::MouseButtonPress)
    {
        // Start by clearing them
        m_cleanButtonPressButton = Qt::NoButton;
        m_cleanButtonClickButton = Qt::NoButton;;
        m_cleanButtonPressPosX = cvf::UNDEFINED_INT;
        m_cleanButtonPressPosY = cvf::UNDEFINED_INT;

        Qt::MouseButton buttonPressed = event->button();

        if (numMouseButtonsInState(m_mouseButtonState) == 1)
        {
            m_cleanButtonPressButton = buttonPressed;
            m_cleanButtonPressPosX = event->x();
            m_cleanButtonPressPosY = event->y();
        }
    }

    // Mouse button release events
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        // Clear it now, might set it later
        m_cleanButtonClickButton = Qt::NoButton;

        Qt::MouseButton buttonReleased = event->button();

        // Check if we have a clean press/release sequence
        if (m_cleanButtonPressButton == buttonReleased &&
            m_cleanButtonPressPosX != cvf::UNDEFINED_INT    &&
            m_cleanButtonPressPosY != cvf::UNDEFINED_INT)
        {
            // We have a candidate, check if movement is within tolerance
            if (cvf::Math::abs(double((m_cleanButtonPressPosX - event->x()))) <= m_cleanButtonClickTolerance &&
                cvf::Math::abs(double((m_cleanButtonPressPosY - event->y()))) <= m_cleanButtonClickTolerance)
            {
                m_cleanButtonClickButton = buttonReleased;
            }

            m_cleanButtonPressButton = Qt::NoButton;;
            m_cleanButtonPressPosX = cvf::UNDEFINED_INT;
            m_cleanButtonPressPosY = cvf::UNDEFINED_INT;
        }
    }

    else if (event->type() == QEvent::MouseMove)
    {
        // For now nothing to do
    }
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MouseState::updateFromMouseEvent(QGraphicsSceneMouseEvent* event)
{
    // Always update with current state
    m_mouseButtonState      = event->buttons();
    m_keyboardModifierFlags = event->modifiers();


    // Now do the events themselves
    if (event->type() == QEvent::GraphicsSceneMousePress)
    {
        // Start by clearing them
        m_cleanButtonPressButton = Qt::NoButton;
        m_cleanButtonClickButton = Qt::NoButton;;
        m_cleanButtonPressPosX = cvf::UNDEFINED_INT;
        m_cleanButtonPressPosY = cvf::UNDEFINED_INT;

        Qt::MouseButton buttonPressed = event->button();

        if (numMouseButtonsInState(m_mouseButtonState) == 1)
        {
            m_cleanButtonPressButton = buttonPressed;
            m_cleanButtonPressPosX = event->screenPos().x();
            m_cleanButtonPressPosY = event->screenPos().y();
        }
    }

    // Mouse button release events
    else if (event->type() == QEvent::GraphicsSceneMouseRelease)
    {
        // Clear it now, might set it later
        m_cleanButtonClickButton = Qt::NoButton;

        Qt::MouseButton buttonReleased = event->button();

        // Check if we have a clean press/release sequence
        if (m_cleanButtonPressButton == buttonReleased &&
            m_cleanButtonPressPosX != cvf::UNDEFINED_INT    &&
            m_cleanButtonPressPosY != cvf::UNDEFINED_INT)
        {
            // We have a candidate, check if movement is within tolerance
            if (cvf::Math::abs(double((m_cleanButtonPressPosX - event->screenPos().x()))) <= m_cleanButtonClickTolerance &&
                cvf::Math::abs(double((m_cleanButtonPressPosY - event->screenPos().y()))) <= m_cleanButtonClickTolerance)
            {
                m_cleanButtonClickButton = buttonReleased;
            }

            m_cleanButtonPressButton = Qt::NoButton;;
            m_cleanButtonPressPosX = cvf::UNDEFINED_INT;
            m_cleanButtonPressPosY = cvf::UNDEFINED_INT;
        }
    }

    else if (event->type() == QEvent::GraphicsSceneMouseMove)
    {
        // For now nothing to do
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void MouseState::reset()
{
	m_mouseButtonState = Qt::NoButton;

	m_cleanButtonPressButton = Qt::NoButton;
	m_cleanButtonPressPosX = cvf::UNDEFINED_INT;
    m_cleanButtonPressPosY = cvf::UNDEFINED_INT;
	m_cleanButtonClickButton = Qt::NoButton;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::MouseButtons MouseState::mouseButtonState() const
{
	return m_mouseButtonState;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Qt::KeyboardModifiers MouseState::keyboardModifierFlags() const
{
	return m_keyboardModifierFlags;
}


//--------------------------------------------------------------------------------------------------
/// Get button that was cleanly clicked if any
//--------------------------------------------------------------------------------------------------
Qt::MouseButton MouseState::cleanButtonClickButton() const
{
	return m_cleanButtonClickButton;
}


//--------------------------------------------------------------------------------------------------
/// Static helper function to determine the number of mouse buttons pressed
//--------------------------------------------------------------------------------------------------
int MouseState::numMouseButtonsInState(Qt::MouseButtons buttonState)
{
	int iNum = 0;

	if (buttonState & Qt::LeftButton)	iNum++;
	if (buttonState & Qt::RightButton)	iNum++;
	if (buttonState & Qt::MidButton)	iNum++;

	return iNum;
}


} // namespace cvfqt


