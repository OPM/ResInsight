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
#include "cvfMutex.h"
#include "cvfLogger.h"

#include <map>

namespace cvf {

class LogDestination;



//==================================================================================================
//
// 
//
//==================================================================================================
class LogManager : public Object
{
public:
    LogManager();
    ~LogManager();

    static LogManager*  instance();
    static void         setInstance(LogManager* logManagerInstance);
    static void         shutdownInstance();

    Logger*             logger(const String& loggerName);
    Logger*             rootLogger();

    void                setLevelRecursive(const String& baseLoggerName, int logLevel);
    void                setDestinationRecursive(const String& baseLoggerName, LogDestination* logDestination);

private:
    Logger*             find(const String& loggerName);
    static String       nameOfParentLogger(const String& childLoggerName);

private:
    typedef std::map<String, cvf::ref<Logger> >  LoggerMap_T;
    LoggerMap_T                     m_loggerMap;
    Mutex                           m_mutex;
    static cvf::ref<LogManager>     sm_logManagerInstance;
    static Mutex                    sm_instanceMutex;

    CVF_DISABLE_COPY_AND_ASSIGN(LogManager);
};


// Helper macros for getting or creating a named logger
#define CVF_GET_LOGGER(loggerName)    (cvf::LogManager::instance()->logger(loggerName))


} // cvf


