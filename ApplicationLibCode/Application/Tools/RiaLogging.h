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
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <QString>

class QWidget;

enum class RILogLevel
{
    RI_LL_ERROR   = 1,
    RI_LL_WARNING = 2,
    RI_LL_INFO    = 3,
    RI_LL_DEBUG   = 4
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
// Timing class for measuring time spent in a scope. Will create a debug log message and the time spent.
//
// Usage:
//    {
//        RiaScopeTimer timer( "Message" );
//        // Code to measure
//    }
//
//==================================================================================================
class RiaScopeTimer
{
public:
    RiaScopeTimer( const QString& message );
    ~RiaScopeTimer();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
    QString                                                     m_message;
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

    static RILogLevel logLevelBasedOnPreferences();

    static void error( const QString& message );
    static void warning( const QString& message );
    static void info( const QString& message );
    static void debug( const QString& message );

    static void debugTimer( const QString& message, std::function<void()> callable );

    static void errorInMessageBox( QWidget* parent, const QString& title, const QString& text );

private:
    static std::vector<std::unique_ptr<RiaLogger>> sm_logger;
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
