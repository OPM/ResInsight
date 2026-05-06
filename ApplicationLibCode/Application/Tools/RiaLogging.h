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

#pragma once

#include <chrono>
#include <concepts>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <QString>

template <typename T>
concept StdStringLike = std::same_as<std::remove_cvref_t<T>, std::string> || std::same_as<std::remove_cvref_t<T>, std::string_view>;

enum class RILogLevel
{
    RI_LL_DISABLED = 0,
    RI_LL_ERROR    = 1,
    RI_LL_WARNING  = 2,
    RI_LL_INFO     = 3,
    RI_LL_DEBUG    = 4
};

//==================================================================================================
//
// Logger interface for the application
//
//==================================================================================================
class RiaLogger
{
public:
    virtual ~RiaLogger() {}

    virtual int  level() const            = 0;
    virtual void setLevel( int logLevel ) = 0;

    virtual void error( const char* message )   = 0;
    virtual void warning( const char* message ) = 0;
    virtual void info( const char* message )    = 0;
    virtual void debug( const char* message )   = 0;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RiaLogging
{
public:
    static std::vector<RiaLogger*> loggerInstances();
    static void                    appendLoggerInstance( std::unique_ptr<RiaLogger> loggerInstance );

    static RILogLevel                logLevelBasedOnPreferences();
    static std::optional<RILogLevel> parseLogLevelString( const QString& logLevelString );

    static void error( const QString& message, const QString& logKeyword = "" );
    static void warning( const QString& message, const QString& logKeyword = "" );
    static void info( const QString& message, const QString& logKeyword = "" );
    static void debug( const QString& message, const QString& logKeyword = "" );

    template <StdStringLike Msg>
    static void error( const Msg& message, std::string_view logKeyword = "" )
    {
        const std::string_view svMsg{ message };
        if ( !isKeywordEnabled( logKeyword ) ) return;
        if ( isSameMessage( svMsg ) ) return;
        const std::string buf{ svMsg };
        for ( const auto& logger : sm_logger )
        {
            if ( logger && logger->level() >= int( RILogLevel::RI_LL_ERROR ) )
            {
#pragma omp critical( critical_section_logging )
                logger->error( buf.c_str() );
            }
        }
        setLastMessage( svMsg );
    }

    template <StdStringLike Msg>
    static void warning( const Msg& message, std::string_view logKeyword = "" )
    {
        const std::string_view svMsg{ message };
        if ( !isKeywordEnabled( logKeyword ) ) return;
        if ( isSameMessage( svMsg ) ) return;
        const std::string buf{ svMsg };
        for ( const auto& logger : sm_logger )
        {
            if ( logger && logger->level() >= int( RILogLevel::RI_LL_WARNING ) )
            {
#pragma omp critical( critical_section_logging )
                logger->warning( buf.c_str() );
            }
        }
        setLastMessage( svMsg );
    }

    template <StdStringLike Msg>
    static void info( const Msg& message, std::string_view logKeyword = "" )
    {
        const std::string_view svMsg{ message };
        if ( !isKeywordEnabled( logKeyword ) ) return;
        if ( isSameMessage( svMsg ) ) return;
        const std::string buf{ svMsg };
        for ( const auto& logger : sm_logger )
        {
            if ( logger && logger->level() >= int( RILogLevel::RI_LL_INFO ) )
            {
#pragma omp critical( critical_section_logging )
                logger->info( buf.c_str() );
            }
        }
        setLastMessage( svMsg );
    }

    template <StdStringLike Msg>
    static void debug( const Msg& message, std::string_view logKeyword = "" )
    {
        const std::string_view svMsg{ message };
        if ( !isKeywordEnabled( logKeyword ) ) return;
        if ( isSameMessage( svMsg ) ) return;
        const std::string buf{ svMsg };
        for ( const auto& logger : sm_logger )
        {
            if ( logger && logger->level() >= int( RILogLevel::RI_LL_DEBUG ) )
            {
#pragma omp critical( critical_section_logging )
                logger->debug( buf.c_str() );
            }
        }
        setLastMessage( svMsg );
    }

    static std::chrono::time_point<std::chrono::high_resolution_clock> currentTime();
    static void logElapsedTime( std::string_view message, const std::chrono::time_point<std::chrono::high_resolution_clock>& startTime );

private:
    static void setLastMessage( std::string_view message );
    static bool isSameMessage( std::string_view message );
    static bool isKeywordEnabled( std::string_view keyword );

private:
    static std::vector<std::unique_ptr<RiaLogger>>                     sm_logger;
    static std::string                                                 sm_lastMessage;
    static std::chrono::time_point<std::chrono::high_resolution_clock> sm_lastMessageTime;
};

//==================================================================================================
//
//==================================================================================================
class RiuMessageLoggerBase : public RiaLogger
{
public:
    explicit RiuMessageLoggerBase();

    int  level() const override;
    void setLevel( int logLevel ) override;

    void error( const char* message ) override;
    void warning( const char* message ) override;
    void info( const char* message ) override;
    void debug( const char* message ) override;

protected:
    virtual void writeMessageToLogger( const std::string& str ) = 0;

private:
    void writeMessageWithPrefixToLogger( const char* prefix, const char* message );

private:
    int m_logLevel;
};

//==================================================================================================
//
//==================================================================================================
class RiaStdOutLogger : public RiuMessageLoggerBase
{
public:
    void writeMessageToLogger( const std::string& str ) override;
};

//==================================================================================================
//
//==================================================================================================
class RiaThreadSafeLogger
{
public:
    void error( const QString& message );
    void warning( const QString& message );
    void info( const QString& message );
    void debug( const QString& message );

    std::vector<QString> messages() const;

private:
    std::vector<QString> m_messages;
};
