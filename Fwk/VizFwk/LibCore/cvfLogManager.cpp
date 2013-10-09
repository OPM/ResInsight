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
#include "cvfLogManager.h"
#include "cvfLogDestinationConsole.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::LogManager
/// \ingroup Core
///
/// 
///
//==================================================================================================

cvf::ref<LogManager> LogManager::sm_logManagerInstance;
Mutex                LogManager::sm_instanceMutex;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LogManager::LogManager()
{
    // Create the root logger
    ref<Logger> rootLogger = new Logger("", Logger::LL_WARNING, new LogDestinationConsole);
    m_loggerMap[""] = rootLogger;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LogManager::~LogManager()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
LogManager* LogManager::instance()
{
    Mutex::ScopedLock mutexLock(sm_instanceMutex);

    if (sm_logManagerInstance.isNull())
    {
        sm_logManagerInstance = new LogManager;
    }

    return sm_logManagerInstance.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LogManager::setInstance(LogManager* logManagerInstance)
{
    Mutex::ScopedLock mutexLock(sm_instanceMutex);

    sm_logManagerInstance = logManagerInstance;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void LogManager::shutdownInstance()
{
    Mutex::ScopedLock mutexLock(sm_instanceMutex);

    sm_logManagerInstance = NULL;
}


//--------------------------------------------------------------------------------------------------
/// Returns logger with the specified name
///
/// Will create the logger if it doesn't already exist. In this case, the newly created logger will
/// be initialized with the same logging level and appender as its parent.
//--------------------------------------------------------------------------------------------------
Logger* LogManager::logger(const String& loggerName)
{
    Mutex::ScopedLock mutexLock(m_mutex);

    ref<Logger> theLogger = find(loggerName);
    if (theLogger.isNull())
    {
        // Must create a new logger
        // Try and find parent (optionally we'll use the root logger) and use its settings to initialize level and appender
        String parentLoggerName = LogManager::nameOfParentLogger(loggerName);
        ref<Logger> parentLogger = find(parentLoggerName);
        if (parentLogger.isNull())
        {
            parentLogger = rootLogger();
        }

        CVF_ASSERT(parentLogger.notNull());

        theLogger = new Logger(loggerName, parentLogger->level(), parentLogger->destination());
        m_loggerMap[loggerName] = theLogger;
    }

    return theLogger.p();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Logger* LogManager::rootLogger()
{
    return logger(String());
}


//--------------------------------------------------------------------------------------------------
/// Sets the logging level of the given logger and all its descendants.
/// 
/// Specify an empty name to set logging level of all current loggers.
//--------------------------------------------------------------------------------------------------
void LogManager::setLevelRecursive(const String& baseLoggerName, int logLevel)
{
    Mutex::ScopedLock mutexLock(m_mutex);

    const size_t baseNameLength = baseLoggerName.size();
    const bool baseNameIsRoot = (baseNameLength == 0);

    for (LoggerMap_T::iterator it = m_loggerMap.begin(); it != m_loggerMap.end(); ++it)
    {
        Logger* logger = it->second.p();
        if (baseNameIsRoot)
        {
            logger->setLevel(logLevel);
        }
        else 
        {
            const String& loggerName = logger->name();
            if (loggerName.startsWith(baseLoggerName) && 
                ((loggerName.size() == baseNameLength) || (loggerName[baseNameLength] == '.')) )
            {
                logger->setLevel(logLevel);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// Sets the log destination for the specified logger and all its children.
//--------------------------------------------------------------------------------------------------
void LogManager::setDestinationRecursive(const String& baseLoggerName, LogDestination* logDestination)
{
    Mutex::ScopedLock mutexLock(m_mutex);

    const size_t baseNameLength = baseLoggerName.size();
    const bool baseNameIsRoot = (baseNameLength == 0);

    for (LoggerMap_T::iterator it = m_loggerMap.begin(); it != m_loggerMap.end(); ++it)
    {
        Logger* logger = it->second.p();
        if (baseNameIsRoot)
        {
            logger->setDestination(logDestination);
        }
        else 
        {
            const String& loggerName = logger->name();
            if (loggerName.startsWith(baseLoggerName) && 
                ((loggerName.size() == baseNameLength) || (loggerName[baseNameLength] == '.')) )
            {
                logger->setDestination(logDestination);
            }
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
Logger* LogManager::find(const String& loggerName)
{
    LoggerMap_T::iterator it = m_loggerMap.find(loggerName);
    if (it != m_loggerMap.end())
    {
        return it->second.p();
    }
    else
    {
        return NULL;
    }
}


//--------------------------------------------------------------------------------------------------
/// Determine name of the parent logger of \a childLoggerName
//--------------------------------------------------------------------------------------------------
String LogManager::nameOfParentLogger(const String& childLoggerName)
{
    std::wstring childName = childLoggerName.toStdWString();
    std::wstring::size_type pos = childName.rfind('.');
    if (pos != std::wstring::npos)
    {
        std::wstring parentName = childName.substr(0, pos);
        return parentName;
    }
    else
    {
        return String();
    }
}


} // namespace cvf

