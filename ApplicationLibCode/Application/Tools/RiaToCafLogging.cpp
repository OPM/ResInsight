/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RiaToCafLogging.h"

#include "RiaLogging.h"

//--------------------------------------------------------------------------------------------------
// Static member definitions
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RiaToCafLoggingBridge> RiaToCafLoggingBridge::s_instance;
bool                                   RiaCafLoggingManager::s_isInitialized = false;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaToCafLoggingBridge::RiaToCafLoggingBridge()
    : m_logLevel( static_cast<int>( RiaLogging::logLevelBasedOnPreferences() ) )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaToCafLoggingBridge::level() const
{
    return m_logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaToCafLoggingBridge::setLevel( int logLevel )
{
    m_logLevel = logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaToCafLoggingBridge::error( const QString& message )
{
    RiaLogging::error( QString( "CAF: %1" ).arg( message ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaToCafLoggingBridge::warning( const QString& message )
{
    RiaLogging::warning( QString( "CAF: %1" ).arg( message ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaToCafLoggingBridge::info( const QString& message )
{
    RiaLogging::info( QString( "CAF: %1" ).arg( message ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaToCafLoggingBridge::debug( const QString& message )
{
    RiaLogging::debug( QString( "CAF: %1" ).arg( message ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaToCafLoggingBridge::registerWithCafLogging()
{
    if ( !s_instance )
    {
        s_instance = std::make_shared<RiaToCafLoggingBridge>();
    }

    caf::PdmLogging::registerLogger( s_instance );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaToCafLoggingBridge::unregisterFromCafLogging()
{
    if ( s_instance )
    {
        caf::PdmLogging::unregisterLogger( s_instance );
        s_instance.reset();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::shared_ptr<RiaToCafLoggingBridge> RiaToCafLoggingBridge::instance()
{
    return s_instance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCafLoggingManager::initializeCafLogging()
{
    if ( !s_isInitialized )
    {
        RiaToCafLoggingBridge::registerWithCafLogging();
        s_isInitialized = true;

        // Log initialization success
        RiaLogging::info( "CAF logging bridge initialized successfully" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCafLoggingManager::shutdownCafLogging()
{
    if ( s_isInitialized )
    {
        RiaToCafLoggingBridge::unregisterFromCafLogging();
        s_isInitialized = false;

        // Log shutdown
        RiaLogging::info( "CAF logging bridge shut down" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaCafLoggingManager::isCafLoggingActive()
{
    return s_isInitialized && RiaToCafLoggingBridge::instance() != nullptr;
}