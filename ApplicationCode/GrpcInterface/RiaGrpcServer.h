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
#include "ResInsightGrid.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::CompletionQueue;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using ResInsight::Grid;
using ResInsight::ProjectInfo;
using ResInsight::Case;
using ResInsight::Int32Message;
using ResInsight::Vec3i;
using ResInsight::EclipseResultRequest;
using ResInsight::EclipseResultAddress;
using ResInsight::DoubleResult;
using ResInsight::Empty;

class RimEclipseCase;

// TODO: REMOVE
class RiaGrpcGridServiceImpl final : public ResInsight::Grid::AsyncService, public RiaGrpcServiceInterface
{
public:
    RimEclipseCase* getCase(int caseId) const;

    Status dimensions(ServerContext* context, const Case* request, Vec3i* reply) override;
    Status results(ServerContext* context, const EclipseResultRequest* request, DoubleResult* result) override;
    Status numberOfTimeSteps(ServerContext* context, const Case* request, Int32Message* reply) override;

    std::vector<RiaGrpcServerCallMethod*> createCallbacks(ServerCompletionQueue* cq) override;
};

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
    std::unique_ptr<ServerCompletionQueue> m_completionQueue;
    std::unique_ptr<Server>                m_server;
    RiaGrpcGridServiceImpl                 m_service;
    RiaGrpcProjectInfoServiceImpl          m_projectService;
    std::list<RiaGrpcServerCallMethod*>    m_receivedRequests;
    std::mutex                             m_requestMutex;
};

#ifdef WIN32
#pragma warning(pop)
#endif
