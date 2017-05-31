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

class QString;

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

    virtual void        error(  const char* message) = 0;
    virtual void        warning(const char* message) = 0;
    virtual void        info(   const char* message) = 0;
    virtual void        debug(  const char* message) = 0;
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

    static void         error(  const QString& message);
    static void         warning(const QString& message);
    static void         info(   const QString& message);
    static void         debug(  const QString& message);

private:
    static RiaLogger*   sm_logger;
};

