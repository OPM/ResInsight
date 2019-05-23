/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
//////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "RiaLogging.h"

#include <iostream>
#include <list>
#include <memory>
#include <mutex>

class RiaGrpcServerImpl;

//==================================================================================================
//
// The GRPC server.
//
//==================================================================================================
class RiaGrpcServer
{
public:    
    RiaGrpcServer(int portNumber);
    ~RiaGrpcServer();

    int portNumber() const;
    bool isRunning() const;
    void run();
    void runInThread();
    void processAllQueuedRequests();
    void quit();
    static int findAvailablePortNumber(int defaultPortNumber);

private:
    void initialize();

private:
    RiaGrpcServerImpl* m_serverImpl;
};