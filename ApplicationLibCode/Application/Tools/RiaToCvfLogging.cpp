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

#include <format>

namespace
{
// The cvf logger name prefixes we bump to debug level while investigating #14714. Using
// setLevelRecursive() with these as base names also affects any child loggers, e.g. per-widget-
// instance loggers created underneath "cee.cvf.qt".
//
// NOTE: cvf::LogManager::logger() only inherits its parent's level/destination at the moment the
// logger is first created (i.e. the first time CVF_GET_LOGGER() is called for that name). It is not
// a live/recursive relationship. Loggers used by our new instrumentation ("cee.cvf.render.fbo",
// "cee.caf.viewer") do not exist yet at initialization time, so setLevelRecursive()/
// setDestinationRecursive() below would have no effect on them once they eventually get created,
// unless we force-create them here first, before applying the recursive settings.
const char* cvfLoggerPrefixesToInvestigate[] = { "cee.cvf.qt", "cee.cvf.render", "cee.cvf.render.fbo", "cee.caf.viewer" };
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

    // Route every cvf logger through RiaLogging, and bump the loggers relevant to the offscreen
    // FBO / OpenGL context investigation to debug level so their messages actually get emitted.
    cvf::LogManager::instance()->setDestinationRecursive( "", new RiaToCvfLoggingBridge );

    // Force-create the loggers we care about before applying the recursive level/destination
    // updates below. cvf::LogManager::logger() only inherits its parent's settings at creation
    // time, so any of these loggers not yet in existence would otherwise silently fall back to the
    // root logger's defaults (LL_WARNING + console) the first time application code touches them.
    for ( const char* loggerPrefix : cvfLoggerPrefixesToInvestigate )
    {
        CVF_GET_LOGGER( loggerPrefix );
    }

    for ( const char* loggerPrefix : cvfLoggerPrefixesToInvestigate )
    {
        cvf::LogManager::instance()->setLevelRecursive( loggerPrefix, cvf::Logger::LL_DEBUG );
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

    for ( const char* loggerPrefix : cvfLoggerPrefixesToInvestigate )
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
