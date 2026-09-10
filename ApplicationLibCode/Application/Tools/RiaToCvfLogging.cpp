/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaToCvfLogging.h"

#include "RiaLogging.h"

#include "cvfLogEvent.h"
#include "cvfLogManager.h"

#include <algorithm>
#include <format>

namespace
{
// Using setLevelRecursive() with these as base names also affects any child loggers, e.g. the
// per-widget-instance loggers created underneath "cee.cvf.qt".
//
// NOTE: cvf::LogManager::logger() only inherits its parent's level/destination at the moment the
// logger is first created (i.e. the first time CVF_GET_LOGGER() is called for that name). It is not
// a live relationship, so a logger that does not exist yet would silently fall back to the root
// logger's defaults. Force-create them before applying the recursive settings.
const char* cvfLoggerPrefixes[] = { "cee.cvf", "cee.cvf.OpenGL", "cee.cvf.qt" };

// cvf is noisy at debug level, so only go there when the application itself is set to debug,
// e.g. with --loglevel debug.
cvf::Logger::Level cvfLevelFromRiaLogging()
{
    int appLevel = static_cast<int>( RILogLevel::RI_LL_DISABLED );
    for ( const RiaLogger* logger : RiaLogging::loggerInstances() )
    {
        appLevel = std::max( appLevel, logger->level() );
    }

    return appLevel >= static_cast<int>( RILogLevel::RI_LL_DEBUG ) ? cvf::Logger::LL_DEBUG : cvf::Logger::LL_INFO;
}
} // namespace

//--------------------------------------------------------------------------------------------------
// Static member definitions
//--------------------------------------------------------------------------------------------------
bool RiaCvfLoggingManager::s_isInitialized = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaToCvfLoggingBridge::log( const cvf::LogEvent& logEvent )
{
    const std::string message = std::format( "cvf[{}]: {}", logEvent.source().toStdString(), logEvent.message().toStdString() );

    switch ( logEvent.level() )
    {
        case cvf::Logger::LL_ERROR:
            RiaLogging::error( message );
            break;
        case cvf::Logger::LL_WARNING:
            RiaLogging::warning( message );
            break;
        case cvf::Logger::LL_INFO:
            RiaLogging::info( message );
            break;
        case cvf::Logger::LL_DEBUG:
        default:
            RiaLogging::debug( message );
            break;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCvfLoggingManager::initializeCvfLogging()
{
    if ( s_isInitialized ) return;

    // Route every cvf logger through RiaLogging. cvf defaults to LL_WARNING on a console
    // destination, so without this its diagnostics never reach the log file.
    cvf::LogManager::instance()->setDestinationRecursive( "", new RiaToCvfLoggingBridge );

    const cvf::Logger::Level level = cvfLevelFromRiaLogging();

    for ( const char* loggerPrefix : cvfLoggerPrefixes )
    {
        CVF_GET_LOGGER( loggerPrefix );
    }

    for ( const char* loggerPrefix : cvfLoggerPrefixes )
    {
        cvf::LogManager::instance()->setLevelRecursive( loggerPrefix, level );
        cvf::LogManager::instance()->setDestinationRecursive( loggerPrefix, new RiaToCvfLoggingBridge );
    }

    s_isInitialized = true;

    RiaLogging::debug( "cvf logging bridge initialized successfully" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCvfLoggingManager::shutdownCvfLogging()
{
    if ( !s_isInitialized ) return;

    for ( const char* loggerPrefix : cvfLoggerPrefixes )
    {
        cvf::LogManager::instance()->setLevelRecursive( loggerPrefix, cvf::Logger::LL_WARNING );
    }

    s_isInitialized = false;

    RiaLogging::debug( "cvf logging bridge shut down" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaCvfLoggingManager::isCvfLoggingActive()
{
    return s_isInitialized;
}
