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

#include "cvfObject.h"
#include "cvfString.h"
#include "cvfLogger.h"

namespace cvf {


//==================================================================================================
//
// 
//
//==================================================================================================
class LogEvent
{
public:
    LogEvent();
    LogEvent(const String& source, const String& message, Logger::Level level, const CodeLocation& codeLocation);
    LogEvent(const LogEvent& other);

    const LogEvent& operator=(LogEvent rhs);

    const String&       source() const;
    Logger::Level       level() const;
    const String&       message() const;
    const CodeLocation& location() const;

private:
    void                swap(LogEvent& other);

private:
    String          m_source;       // Source of the log event, normally name of the logger
    String          m_message;      // The logged message
    Logger::Level   m_level;        // The log level of the event
    CodeLocation    m_codeLocation; // Source code location of log statement
};



} // cvf


