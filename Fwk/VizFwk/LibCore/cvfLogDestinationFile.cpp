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
#include "cvfLogDestinationFile.h"
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
//
// 
//
//==================================================================================================
class FileWrapper
{
public:
    FileWrapper(const String& fileName)
    :   m_fileName(fileName),
        m_filePtr(NULL)
    {
    }

    ~FileWrapper()
    {
        if (m_filePtr)
        {
            fclose(m_filePtr);
        }
    }

    bool open(const String& mode)
    {
        CVF_ASSERT(m_filePtr == NULL);

#ifdef WIN32
        if (_wfopen_s(&m_filePtr, m_fileName.c_str(), mode.c_str()) != 0)
        {
            m_filePtr = NULL;
        }
#else
        m_filePtr = ::fopen(m_fileName.toUtf8().ptr(), mode.toUtf8().ptr());
#endif

        return m_filePtr != NULL;
    }

    FILE* filePtr()
    {
        return m_filePtr;
    }

private:
    String  m_fileName;
    FILE*   m_filePtr;
};



//==================================================================================================
///
/// \class cvf::LogDestinationFile
/// \ingroup Core
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LogDestinationFile::LogDestinationFile(const String& fileName)
:   m_fileName(fileName)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LogDestinationFile::log(const LogEvent& logEvent)
{
    String str;
    bool addLocationInfo = false;

    Logger::Level logEventLevel = logEvent.level();
    if (logEventLevel == Logger::LL_ERROR)
    {
        str = logEvent.source() + "**ERROR**: " + logEvent.message();
        addLocationInfo = true;
    }
    else if (logEventLevel == Logger::LL_WARNING)
    {
        str = logEvent.source() + "(warn):  " + logEvent.message();
    }
    else if (logEventLevel == Logger::LL_INFO)
    {
        str = logEvent.source() + "(i): " + logEvent.message();
    }
    else if (logEventLevel == Logger::LL_DEBUG)
    {
        str = logEvent.source() + "(d): " + logEvent.message();
    }

    if (addLocationInfo)
    {
        str += "\n";
        str += String("        -func: %1\n").arg(logEvent.location().functionName());
        str += String("        -file: %1(%2)").arg(logEvent.location().shortFileName()).arg(logEvent.location().lineNumber());
    }

    CharArray charArrMsg = str.toAscii();
    const char* szMsg = charArrMsg.ptr();

    Mutex::ScopedLock lock(m_mutex);
    writeToFile(szMsg, true);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LogDestinationFile::writeToFile(const char* theString, bool addNewLine)
{
    FileWrapper file(m_fileName);
    if (m_firstTimeOpen)
    {
        if (!file.open("wt"))
        {
            return;
        }

        m_firstTimeOpen = false;
    }
    else
    {
        if (!file.open("at"))
        {
            return;
        }
    }

    if (file.filePtr() && theString)
    {
        if (addNewLine)
        {
            fprintf(file.filePtr(), "%s\n", theString);
        }
        else
        {
            fprintf(file.filePtr(), "%s", theString);
        }
    }
}


} // namespace cvf

