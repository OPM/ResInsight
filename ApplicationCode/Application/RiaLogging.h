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


enum RILogLevel
{
    RI_LL_ERROR = 1,
    RI_LL_WARNING = 2,
    RI_LL_INFO = 3,
    RI_LL_DEBUG = 4
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

    virtual int         level() const = 0;
    virtual void        setLevel(int logLevel) = 0;

    virtual void        error(  const char* message, const char* fileName, int lineNumber) = 0;
    virtual void        warning(const char* message, const char* fileName, int lineNumber) = 0;
    virtual void        info(   const char* message, const char* fileName, int lineNumber) = 0;
    virtual void        debug(  const char* message, const char* fileName, int lineNumber) = 0;

protected:
    static const char*  shortFileName(const char* fileName);
};



//==================================================================================================
//
// 
//
//==================================================================================================
class RiaLogging
{
public:
    static RiaLogger*   loggerInstance();
    static void         setLoggerInstance(RiaLogger* loggerInstance);
    static void         deleteLoggerInstance();

private:
    static RiaLogger*   sm_logger;
};





// Helper macros for writing log messages
#define RI_LOG_ERROR_2(theLogger, theMessage)    if ((theLogger)->level() >= RI_LL_ERROR)    { (theLogger)->error((theMessage),     __FILE__, __LINE__); }
#define RI_LOG_WARNING_2(theLogger, theMessage)  if ((theLogger)->level() >= RI_LL_WARNING)  { (theLogger)->warning((theMessage),   __FILE__, __LINE__); }
#define RI_LOG_INFO_2(theLogger, theMessage)     if ((theLogger)->level() >= RI_LL_INFO)     { (theLogger)->info((theMessage),      __FILE__, __LINE__); }
#define RI_LOG_DEBUG_2(theLogger, theMessage)    if ((theLogger)->level() >= RI_LL_DEBUG)    { (theLogger)->debug((theMessage),     __FILE__, __LINE__); }

#define RI_LOG_ERROR(theMessage)         RI_LOG_ERROR_2(    RiaLogging::loggerInstance(), theMessage)   
#define RI_LOG_ERROR_QSTR(theMessage)    RI_LOG_ERROR_2(    RiaLogging::loggerInstance(), (theMessage).toLatin1().constData())   
#define RI_LOG_WARNING(theMessage)       RI_LOG_WARNING_2(  RiaLogging::loggerInstance(), theMessage) 
#define RI_LOG_WARNING_QSTR(theMessage)  RI_LOG_WARNING_2(  RiaLogging::loggerInstance(), (theMessage).toLatin1().constData())
#define RI_LOG_INFO(theMessage)          RI_LOG_INFO_2(     RiaLogging::loggerInstance(), theMessage)    
#define RI_LOG_INFO_QSTR(theMessage)     RI_LOG_INFO_2(     RiaLogging::loggerInstance(), (theMessage).toLatin1().constData())    
#define RI_LOG_DEBUG(theMessage)         RI_LOG_DEBUG_2(    RiaLogging::loggerInstance(), theMessage)   
#define RI_LOG_DEBUG_QSTR(theMessage)    RI_LOG_DEBUG_2(    RiaLogging::loggerInstance(), (theMessage).toLatin1().constData())    
