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
#include "cvfLogDestinationConsole.h"
#include "cvfLogEvent.h"

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable: 4668)
#include <windows.h>
#pragma warning (pop)
#else
#include <cstdio>
#include <cstdarg>
#endif

namespace cvf {



//==================================================================================================
///
/// \class cvf::LogDestinationConsole
/// \ingroup Core
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LogDestinationConsole::log(const LogEvent& logEvent)
{
    String str;
    bool addLocationInfo = false;

    Logger::Level logEventLevel = logEvent.level();
    if (logEventLevel == Logger::LL_ERROR)
    {
        str = "ERROR: " + logEvent.message();
        addLocationInfo = true;
    }
    else if (logEventLevel == Logger::LL_WARNING)
    {
        str = "warn:  " + logEvent.message();
    }
    else if (logEventLevel == Logger::LL_INFO)
    {
        str = "info:  " + logEvent.message();
    }
    else if (logEventLevel == Logger::LL_DEBUG)
    {
        str = "debug: " + logEvent.message();
    }

    if (addLocationInfo)
    {
        str += "\n";
        str += String("        -func: %1\n").arg(logEvent.location().functionName());
        str += String("        -file: %1(%2)").arg(logEvent.location().shortFileName()).arg(logEvent.location().lineNumber());
    }

    CharArray charArrMsg = str.toAscii();
    const char* szMsg = charArrMsg.ptr();

    {
        Mutex::ScopedLock lock(m_mutex);

#ifdef WIN32
        writeToWindowsConsole(szMsg, true);
#else
        writeToStderr(szMsg, true);
#endif
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LogDestinationConsole::writeToWindowsConsole(const char* theString, bool addNewLine)
{
#ifdef WIN32
    CVF_ASSERT(theString);

    AllocConsole();

    HANDLE hStdOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOutputHandle && theString)
    {
        DWORD stringLength = static_cast<DWORD>(System::strlen(theString));

        unsigned long iDum = 0;
        WriteConsoleA(hStdOutputHandle, theString, stringLength, &iDum, NULL);
        if (addNewLine) WriteConsole(hStdOutputHandle, "\n", 1, &iDum, NULL);
    }
#else
    CVF_UNUSED(theString);
    CVF_UNUSED(addNewLine);
#endif
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LogDestinationConsole::writeToStderr(const char* theString, bool addNewLine)
{
    CVF_ASSERT(theString);

    if (theString)
    {
        fprintf(stderr, "%s", theString);
        if (addNewLine) 
        {
            fprintf(stderr, "\n");
        }
    }
}


} // namespace cvf

