/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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
#include "spdlog/spdlog.h"
#include <spdlog/sinks/rotating_file_sink.h>

class RiaFileLogger::Impl
{
public:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Impl( const std::string& filePathForLogFiles )
    {
        try
        {
            // Automatically create unique files when size limit is reached. 5MB per file, keep 3 files
            auto fileName = filePathForLogFiles + "/resinsight.log";
            m_spdlogger   = spdlog::rotating_logger_mt( "rotating_logger", fileName, 1024 * 1024 * 5, 3 );

            auto flushInterval = 500;
            spdlog::flush_every( std::chrono::milliseconds( flushInterval ) );
        }
        catch ( ... )
        {
        }
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void log( const std::string& message )
    {
        if ( m_spdlogger ) m_spdlogger->info( message );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void info( const std::string& message )
    {
        if ( m_spdlogger ) m_spdlogger->info( message );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void debug( const std::string& message )
    {
        if ( m_spdlogger ) m_spdlogger->debug( message );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void error( const std::string& message )
    {
        if ( m_spdlogger ) m_spdlogger->error( message );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void warning( const std::string& message )
    {
        if ( m_spdlogger ) m_spdlogger->warn( message );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void flush()
    {
        if ( m_spdlogger ) m_spdlogger->flush();
    }

private:
    std::shared_ptr<spdlog::logger> m_spdlogger;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaFileLogger::RiaFileLogger( const std::string& filePathForLogFiles )
    : m_impl( new Impl( filePathForLogFiles ) )
    , m_logLevel( int( RILogLevel::RI_LL_DEBUG ) )

{
}

//--------------------------------------------------------------------------------------------------
/// The destructor must be located in the cpp file after the definition of RiaFileLogger::Impl to make sure the Impl class is defined when
/// the destructor of std::unique_ptr<Impl> is called
//--------------------------------------------------------------------------------------------------
RiaFileLogger::~RiaFileLogger() = default;

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaFileLogger::flush()
{
    if ( m_impl ) m_impl->flush();
}
