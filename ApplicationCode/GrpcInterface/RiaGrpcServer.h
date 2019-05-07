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

#ifdef WIN32
// GRPC does a lot of tricks that give warnings on MSVC but works fine
#pragma warning(push)
#pragma warning(disable : 4251 4702 4005)
#endif

#include "RiaGrpcServerCallData.h"
#include "RiaGrpcServiceInterface.h"
#include "RiaGrpcGridInfoService.h"
#include "RiaGrpcProjectInfoService.h"

#include <QString>

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <mutex>

#include "CaseInfo.grpc.pb.h"
#include "ProjectInfo.grpc.pb.h"

class RimEclipseCase;

//==================================================================================================
//
// The GRPC server implementation
//
//==================================================================================================
class RiaGrpcServer
{
public:
    ~RiaGrpcServer();
    void run();
    void runInThread();
    void initialize();
    void processOneRequest();
    void quit();
private:
    void waitForNextRequest();
    void process(RiaGrpcServerCallMethod* method);

private:
    std::unique_ptr<grpc::ServerCompletionQueue> m_completionQueue;
    std::unique_ptr<grpc::Server>                m_server;
    RiaGrpcProjectInfoService                    m_projectService;
    RiaGrpcGridInfoService                       m_gridService;
    std::list<RiaGrpcServerCallMethod*>          m_receivedRequests;
    std::mutex                                   m_requestMutex;
};

#ifdef WIN32
#pragma warning(pop)
#endif
