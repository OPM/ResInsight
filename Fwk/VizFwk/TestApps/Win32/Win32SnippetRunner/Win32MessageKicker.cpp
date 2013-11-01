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
#include "Win32MessageKicker.h"



//==================================================================================================
//
// Win32MessageKicker
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Win32MessageKicker::Win32MessageKicker(HWND destinationWnd, unsigned int messageToSend)
:   m_destinationWnd(destinationWnd),
    m_messageToSend(messageToSend),
    m_intervalMsec(0),
    m_isKickerStarted(false),
    m_timerHandle(0)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Win32MessageKicker::~Win32MessageKicker()
{
    stop();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32MessageKicker::start(unsigned int intervalMSec)
{
    stop();

    m_intervalMsec = intervalMSec;
    m_isKickerStarted = true;
    startOneShotTimer();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32MessageKicker::stop()
{
    deleteCurrentTimer();
    m_isKickerStarted = false;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32MessageKicker::startOneShotTimer()
{
    // Delete any existing
    deleteCurrentTimer();
    
    BOOL success = ::CreateTimerQueueTimer(&m_timerHandle, NULL, Win32MessageKicker::timerCallbackProc, this, m_intervalMsec, 0, WT_EXECUTEINTIMERTHREAD);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32MessageKicker::deleteCurrentTimer()
{
    if (m_timerHandle != 0)
    {
        DeleteTimerQueueTimer(NULL, m_timerHandle, NULL);
        m_timerHandle = 0;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Win32MessageKicker::postTheMessage()
{
    if (m_destinationWnd != NULL)
    {
        ::SendNotifyMessage(m_destinationWnd, m_messageToSend, 0, 0);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
VOID CALLBACK Win32MessageKicker::timerCallbackProc(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    Win32MessageKicker* kicker = reinterpret_cast<Win32MessageKicker*>(lpParameter);
    kicker->postTheMessage();
    kicker->startOneShotTimer();
}

