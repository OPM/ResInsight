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
#include "cvfTrace.h"
#include "cvfSystem.h"

#ifdef WIN32
#pragma warning (push)
#pragma warning (disable: 4668)
#include <windows.h>
#pragma warning (pop)
#else
#include <cstdio>
#include <cstdarg>
#endif

#ifdef CVF_ANDROID
#include <android/log.h>
#endif

namespace cvf {

//==================================================================================================
///
/// \class cvf::Trace
/// \ingroup Core
///
/// Class for writing debug text to console, DevStudio output window and file (future)
///
/// TODO: Create file output.
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Write debug text to console, DevStudio output window and file (future)
//--------------------------------------------------------------------------------------------------
void Trace::show(String message)
{
    showTraceOutput(message, true);
}


//--------------------------------------------------------------------------------------------------
/// Write printf formatted debug text to console, DevStudio output window and file (future)
//--------------------------------------------------------------------------------------------------
void Trace::show(const char* format, ...)
{
    // Create the printf-style string and send it to the console and trace file
    // TODO! Note: var-arg is solved locally here, but needs to be revisited e.g. in String class
    va_list argList;
    va_start(argList, format);

    const int maxFormatLength = 4000;
    char temp[maxFormatLength + 1];

#ifdef WIN32
    _vsnprintf_s(temp, maxFormatLength, format, argList);
#elif defined(CVF_ANDROID)
    __android_log_print(ANDROID_LOG_DEBUG, "CVF_TAG", format, argList);
#else
    vsprintf(temp, format, argList);
#endif

    va_end(argList);    

    showTraceOutput(temp, true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Trace::showFileLineNumber(const String& file, int line, const String& message)
{
    String tmp = file + "(" + String(line) + ")";

    if (!message.isEmpty())
    {
        tmp += ": msg: " + message;
    }

    Trace::show(tmp);
}


//--------------------------------------------------------------------------------------------------
/// Show the trace output in console and DevStudio output window
//--------------------------------------------------------------------------------------------------
void Trace::showTraceOutput(String text, bool addNewLine)
{
#ifdef WIN32
    AllocConsole();

    HANDLE hStdOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hStdOutputHandle)
    {
        unsigned long iDum = 0;
        CharArray ascii = text.toAscii();
        DWORD stringLength = static_cast<DWORD>(System::strlen(ascii.ptr()));

        WriteConsoleA(hStdOutputHandle, ascii.ptr(), stringLength, &iDum, NULL);
        if (addNewLine) WriteConsole(hStdOutputHandle, "\n", 1, &iDum, NULL);
    }
#elif defined(CVF_ANDROID)
    __android_log_print(ANDROID_LOG_DEBUG, "CVF_TAG", "%s", text.toAscii().ptr());
#else
    fprintf(stderr, "%s", text.toAscii().ptr());
    if (addNewLine) 
    {
        fprintf(stderr, "\n");
    }
#endif

    // Show output in "Output window" in Visual Studio
#if defined(WIN32) && defined(_DEBUG)
    // Alternativly use OutputDebugStringA(text.toAscii().ptr()); if this does not work on some platforms
    _RPT0(_CRT_WARN, text.toAscii().ptr());
    if (addNewLine)
    {
        _RPT0(_CRT_WARN, "\n");
    }
#endif
}


} // namespace cvf

