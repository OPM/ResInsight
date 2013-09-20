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
#include "cvfDebugTimer.h"
#include "cvfTimer.h"
#include "cvfTrace.h"

#include <cstdio>
#include <cstdarg>

namespace cvf {



//==================================================================================================
///
/// \class cvf::DebugTimer
/// \ingroup Core
///
/// Debug timer class for reporting timing info via the static Trace class
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor, starts the timer
///
/// \param prefix        This text will be prepended for all trace outputs. Can be NULL
/// \param operationMode Either NORMAL (the default) or DISABLED. Specifying DISABLED effectively
///                      disables all functionality in the class.
//--------------------------------------------------------------------------------------------------
DebugTimer::DebugTimer(const char* prefix, OperationMode operationMode) 
    : m_timer(NULL),
      m_prefix(NULL),
      m_messageCount(0)
{
    if (operationMode == NORMAL)
    {
        if (prefix)
        {
            m_prefix = new String(prefix);
        }

        m_timer = new Timer();
    }
}


//--------------------------------------------------------------------------------------------------
/// Destructor 
//--------------------------------------------------------------------------------------------------
DebugTimer::~DebugTimer()
{
    delete m_prefix;
    delete m_timer;
}


//--------------------------------------------------------------------------------------------------
/// Restart timer.
/// 
/// If msg is specified this method will show this text as trace output. Else nothing is shown.
//--------------------------------------------------------------------------------------------------
void DebugTimer::restart(const char* msg)
{
    if (!m_timer) return;

    m_timer->restart();

    if (msg)
    {
        String outputText = makeMessageStartString(msg);
        Trace::show(outputText);
    }
}


//--------------------------------------------------------------------------------------------------
/// Report elapsed time, in seconds. msg is appended if provided.
//--------------------------------------------------------------------------------------------------
void DebugTimer::reportTime(const char* msg)
{
    if (!m_timer) return;

    double time = m_timer->time();

    String outputText = makeMessageStartString(msg) + ": " + String::number(time, 'f', 6) + " sec";
    Trace::show(outputText);
}


//--------------------------------------------------------------------------------------------------
/// Report elapsed time, in milliseconds. msg is appended if provided.
//--------------------------------------------------------------------------------------------------
void DebugTimer::reportTimeMS(const char* msg )
{
    if (!m_timer) return;

    double timeMS = m_timer->time() * 1000.0;

    String outputText = makeMessageStartString(msg) + ": " + String::number(timeMS, 'f', 3) + " ms";
    Trace::show(outputText);
}


//--------------------------------------------------------------------------------------------------
/// Report lap time, in seconds. msg is appended if provided.
//--------------------------------------------------------------------------------------------------
void DebugTimer::reportLapTime(const char* msg)
{
    if (!m_timer) return;

    double lapTime = m_timer->lapTime();

    String outputText = makeMessageStartString(msg) + ": " + String::number(lapTime, 'f', 6) + " sec";
    Trace::show(outputText);
}


//--------------------------------------------------------------------------------------------------
/// Report lap time, in milliseconds. msg is appended if provided.
//--------------------------------------------------------------------------------------------------
void DebugTimer::reportLapTimeMS(const char* msg)
{
    if (!m_timer) return;

    double lapTimeMS = m_timer->lapTime() * 1000.0;

    String outputText = makeMessageStartString(msg) + ": " + String::number(lapTimeMS, 'f', 3) + " ms";
    Trace::show(outputText);
}


//--------------------------------------------------------------------------------------------------
/// Show trace output with message count, optional prefix and provided text.
//--------------------------------------------------------------------------------------------------
void DebugTimer::echoMessage(const char* format, ...)
{
    if (!m_timer) return;

    // Create the printf-style string and send it to the console and trace file
    // TODO! Note: var-arg is solved locally here, but needs to be revisited e.g. in String class
    va_list argList;
    va_start(argList, format);

    const int maxFormatLength = 4000;
    char temp[maxFormatLength + 1];

#ifdef WIN32
    _vsnprintf_s(temp, maxFormatLength, format, argList);
#else
    vsprintf(temp, format, argList);
#endif

    va_end(argList);    

    String outputText = makeMessageStartString(temp);

    Trace::show(outputText);
}


//--------------------------------------------------------------------------------------------------
/// Generate message string with message count, optional prefix and optional msg text.
//--------------------------------------------------------------------------------------------------
String DebugTimer::makeMessageStartString(const char* msg)
{
    String outputText = String(m_messageCount);
    m_messageCount++;

    if (m_prefix)
    {
        outputText += String(": ");
        outputText += *m_prefix;
    }

    if (msg)
    {
        outputText += ": ";
        outputText += msg;
    }

    return outputText;
}


} // namespace cvf

