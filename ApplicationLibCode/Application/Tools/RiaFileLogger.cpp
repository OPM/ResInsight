/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RiaFileLogger.h"
#include "RiaPreferences.h"

#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

class RiaFileLogger::Impl
{
public:
    Impl( const std::string& fileName )
    {
        try
        {
            m_logger = spdlog::basic_logger_mt( "basic_logger", fileName );

            auto flushInterval = RiaPreferences::current()->loggerFlushInterval();
            spdlog::flush_every( std::chrono::milliseconds( flushInterval ) );
        }
        catch ( ... )
        {
        }
    }

    void log( const std::string& message )
    {
        if ( !m_logger ) return;

        m_logger->info( message );
    }

    void info( const std::string& message )
    {
        if ( !m_logger ) return;

        m_logger->info( message );
    }

    void debug( const std::string& message )
    {
        if ( !m_logger ) return;

        m_logger->debug( message );
    }

    void error( const std::string& message )
    {
        if ( !m_logger ) return;

        m_logger->error( message );
    }

    void warning( const std::string& message )
    {
        if ( !m_logger ) return;

        m_logger->warn( message );
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFileLogger::RiaFileLogger( const std::string& filename )
    : m_impl( new Impl( filename ) )
    , m_logLevel( int( RILogLevel::RI_LL_DEBUG ) )

{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaFileLogger::level() const
{
    return m_logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFileLogger::setLevel( int logLevel )
{
    m_logLevel = logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFileLogger::error( const char* message )
{
    m_impl->error( message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFileLogger::warning( const char* message )
{
    m_impl->warning( message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFileLogger::info( const char* message )
{
    m_impl->info( message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFileLogger::debug( const char* message )
{
    m_impl->debug( message );
}
