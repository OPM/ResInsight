//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#include "cvfBase.h"
#include "cvfLogger.h"
#include "cvfTrace.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::Logger
/// \ingroup Core
///
/// Logger class
/// 
/// Currently, output is written using Trace, and special formatting of the string makes it possible
/// to navigate to source code using F4 in Visual Studio. See http://msdn.microsoft.com/en-us/library/yxkt8b26.aspx
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
Logger::Logger()
:   m_debugLogging(false)
{
}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
Logger::~Logger()
{

}


//--------------------------------------------------------------------------------------------------
/// Use file and line to create a specially formatted string for Visual Studio. 
/// 
/// \param message  The actual error message 
/// \param fileName Use system macro __FILE__ for source code file name
/// \param line     Use system macro __LINE__ for source code line number
///
/// __FILE__ and __LINE__ are used to create the variables used to navigate to the line in the 
/// source code file the error message was logged at.
//--------------------------------------------------------------------------------------------------
void Logger::error(const String& message, const char* fileName, int lineNumber)
{
    String tmp;
    tmp = String(fileName) + "(" + String(lineNumber) + "): error: " + message;

    Trace::show(tmp);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Logger::enableDebug(bool enableDebugLogging)
{
    m_debugLogging = enableDebugLogging;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool Logger::isDebugEnabled() const
{
    return m_debugLogging;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void Logger::debug(const String& message, const char* /*fileName*/, int /*lineNumber*/)
{
    if (m_debugLogging)
    {
        // For now, don't report file and line
        String tmp;
        tmp = "debug: " + message;

        Trace::show(tmp);
    }
}


} // namespace cvf

