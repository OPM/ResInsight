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
#include "cvfCodeLocation.h"

namespace cvf {

class LogEvent;
class LogDestination;



//==================================================================================================
//
// Logger class
//
//==================================================================================================
class Logger : public Object
{
public:
    enum Level
    {
        LL_ERROR = 1,  
        LL_WARNING,
        LL_INFO,
        LL_DEBUG
    };

public:
    Logger(const String& loggerName, int logLevel, LogDestination* logDestination);
    ~Logger();

    const String&   name() const;
    int             level() const;
    void            setLevel(int logLevel);
    LogDestination* destination();
    void            setDestination(LogDestination* logDestination);

    void            error(const String& message);
    void            error(const String& message, const CodeLocation& location);
    void            warning(const String& message);
    void            warning(const String& message, const CodeLocation& location);
    void            info(const String& message);
    void            info(const String& message, const CodeLocation& location);
    void            debug(const String& message, const CodeLocation& location);

    bool            isErrorEnabled() const      { return m_logLevel >= LL_ERROR; }
    bool            isWarningEnabled() const    { return m_logLevel >= LL_WARNING; }
    bool            isInfoEnabled() const       { return m_logLevel >= LL_INFO; }
    bool            isDebugEnabled() const      { return m_logLevel >= LL_DEBUG; }

private:
    void            log(const String& message, Logger::Level messageLevel, const CodeLocation& location);

private:
    String              m_name;         // Logger name
    int                 m_logLevel;     // Logging level, all messages with a level less than or equal to this level will be logged
    ref<LogDestination> m_destination;

    CVF_DISABLE_COPY_AND_ASSIGN(Logger);
};


// Helper macros for writing log messages to a logger
#define CVF_LOG_ERROR(theLogger, theMessage)    if ((theLogger)->isErrorEnabled())    { (theLogger)->error((theMessage), CVF_CODE_LOCATION); }
#define CVF_LOG_WARNING(theLogger, theMessage)  if ((theLogger)->isWarningEnabled())  { (theLogger)->warning((theMessage), CVF_CODE_LOCATION); }
#define CVF_LOG_INFO(theLogger, theMessage)     if ((theLogger)->isInfoEnabled())     { (theLogger)->info((theMessage), CVF_CODE_LOCATION); }
#define CVF_LOG_DEBUG(theLogger, theMessage)    if ((theLogger)->isDebugEnabled())    { (theLogger)->debug((theMessage), CVF_CODE_LOCATION); }



} // cvf


