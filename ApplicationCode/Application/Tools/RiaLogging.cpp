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

#include "RiaLogging.h"

#include <iostream>
#include <sstream>

#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4668 )
// Define this one to tell windows.h to not define min() and max() as macros
#if defined WIN32 && !defined NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#pragma warning( pop )
#else
#include <cstdio>
#include <cstring>
#endif

#include "QString"

//==================================================================================================
//
//
//
//==================================================================================================
class RiaDefaultConsoleLogger : public RiaLogger
{
public:
    RiaDefaultConsoleLogger();

    int  level() const override;
    void setLevel( int logLevel ) override;
    void error( const char* message ) override;
    void warning( const char* message ) override;
    void info( const char* message ) override;
    void debug( const char* message ) override;

private:
    static void writeMessageToConsole( const char* prefix, const char* message );
    static void writeToConsole( const std::string& str );

private:
    int m_logLevel;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefaultConsoleLogger::RiaDefaultConsoleLogger()
    : m_logLevel( RI_LL_WARNING )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiaDefaultConsoleLogger::level() const
{
    return m_logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaDefaultConsoleLogger::setLevel( int logLevel )
{
    m_logLevel = logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaDefaultConsoleLogger::error( const char* message )
{
    writeMessageToConsole( "ERROR: ", message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaDefaultConsoleLogger::warning( const char* message )
{
    writeMessageToConsole( "warn:  ", message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaDefaultConsoleLogger::info( const char* message )
{
    writeMessageToConsole( "info:  ", message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaDefaultConsoleLogger::debug( const char* message )
{
    writeMessageToConsole( "debug: ", message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaDefaultConsoleLogger::writeMessageToConsole( const char* prefix, const char* message )
{
    std::ostringstream oss;

    //    VF_ASSERT(prefix);
    oss << prefix;

    if ( message )
    {
        oss << message << std::endl;
    }
    else
    {
        oss << "<no message>" << std::endl;
    }

    writeToConsole( oss.str() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaDefaultConsoleLogger::writeToConsole( const std::string& str )
{
#ifdef WIN32
    AllocConsole();
    HANDLE hStdOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    if ( hStdOutputHandle )
    {
        DWORD stringLength = static_cast<DWORD>( str.length() );

        unsigned long iDum = 0;
        WriteConsoleA( hStdOutputHandle, str.c_str(), stringLength, &iDum, nullptr );
    }
#else
    fputs( str.c_str(), stderr );
#endif
}

//==================================================================================================
//
//
//
//==================================================================================================

RiaLogger* RiaLogging::sm_logger = new RiaDefaultConsoleLogger;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaLogger* RiaLogging::loggerInstance()
{
    return sm_logger;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogging::setLoggerInstance( RiaLogger* loggerInstance )
{
    // Only delete if we're currently using our own default impl
    if ( dynamic_cast<RiaDefaultConsoleLogger*>( sm_logger ) )
    {
        delete sm_logger;
    }

    sm_logger = loggerInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogging::deleteLoggerInstance()
{
    delete sm_logger;
    sm_logger = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogging::error( const QString& message )
{
    if ( sm_logger && sm_logger->level() >= RI_LL_ERROR )
    {
#pragma omp critical( critical_section_logging )
        sm_logger->error( message.toLatin1().constData() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogging::warning( const QString& message )
{
    if ( sm_logger && sm_logger->level() >= RI_LL_WARNING )
    {
#pragma omp critical( critical_section_logging )
        sm_logger->warning( message.toLatin1().constData() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogging::info( const QString& message )
{
    if ( sm_logger && sm_logger->level() >= RI_LL_INFO )
    {
#pragma omp critical( critical_section_logging )
        sm_logger->info( message.toLatin1().constData() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaLogging::debug( const QString& message )
{
    if ( sm_logger && sm_logger->level() >= RI_LL_DEBUG )
    {
#pragma omp critical( critical_section_logging )
        sm_logger->debug( message.toLatin1().constData() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMessageLoggerBase::RiuMessageLoggerBase()
    : m_logLevel( RI_LL_WARNING )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMessageLoggerBase::level() const
{
    return m_logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessageLoggerBase::setLevel( int logLevel )
{
    m_logLevel = logLevel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessageLoggerBase::error( const char* message )
{
    writeMessageWithPrefixToLogger( "ERROR: ", message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessageLoggerBase::warning( const char* message )
{
    writeMessageWithPrefixToLogger( "warning: ", message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessageLoggerBase::info( const char* message )
{
    writeMessageWithPrefixToLogger( "info: ", message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessageLoggerBase::debug( const char* message )
{
    writeMessageWithPrefixToLogger( "debug: ", message );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMessageLoggerBase::writeMessageWithPrefixToLogger( const char* prefix, const char* message )
{
    std::ostringstream oss;

    oss << prefix;

    if ( message )
    {
        oss << message << std::endl;
    }
    else
    {
        oss << "<no message>" << std::endl;
    }

    writeMessageToLogger( oss.str() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaStdOutLogger::writeMessageToLogger( const std::string& str )
{
    std::cout << str;
}
