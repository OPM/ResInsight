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

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <QString>

#ifdef WIN32
// GRPC does a lot of tricks that give warnings on MSVC but works fine
#pragma warning(push)
#pragma warning(disable : 4251 4702 4005)
#endif

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#include <iostream>

#include "ResInsightGrid.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using ResInsight::Grid;
using ResInsight::Case;
using ResInsight::Int32Message;
using ResInsight::Vec3i;
using ResInsight::EclipseResultRequest;
using ResInsight::EclipseResultAddress;
using ResInsight::DoubleResult;

class RimEclipseCase;

class RiaGridServiceImpl final : public ResInsight::Grid::Service
{
public:
    RimEclipseCase* getCase(int caseId) const;

    Status dimensions(ServerContext* context, const Case* request, Vec3i* reply);
    Status results(ServerContext* context, const EclipseResultRequest* request, DoubleResult* result);
    Status numberOfTimeSteps(ServerContext* context, const Case* request, Int32Message* reply);
};

class RiaGrpcServer
{
public:
    void runInOwnMainLoop();
    void run();
    void handleRequest();
private:
    std::unique_ptr<Server> server_;
};


#ifdef WIN32
#pragma warning(pop)
#endif
