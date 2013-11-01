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


//==================================================================================================
//
// Class to periodically kick/post a windows message
//
//==================================================================================================
class Win32MessageKicker 
{
public:
    Win32MessageKicker(HWND destinationWnd, unsigned int messageToSend);
    ~Win32MessageKicker();

    void    start(unsigned int intervalMSec);
    void    stop();

private:
    void                 startOneShotTimer();
    void                 deleteCurrentTimer();
    void                 postTheMessage();
    static VOID CALLBACK timerCallbackProc(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

private:
    HWND         m_destinationWnd;
    unsigned int m_messageToSend;
    unsigned int m_intervalMsec;
    bool         m_isKickerStarted;
    HANDLE       m_timerHandle;
};


