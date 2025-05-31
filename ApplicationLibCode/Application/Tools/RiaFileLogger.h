/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RiaLogging.h"

//==================================================================================================
//
//==================================================================================================
class RiaFileLogger : public RiaLogger
{
public:
    explicit RiaFileLogger( const std::string& filePathForLogFiles );
    ~RiaFileLogger() override;

    int  level() const override;
    void setLevel( int logLevel ) override;

    void error( const char* message ) override;
    void warning( const char* message ) override;
    void info( const char* message ) override;
    void debug( const char* message ) override;

    void flush();

private:
    int m_logLevel;

    class Impl;
    std::unique_ptr<Impl> m_impl;
};
