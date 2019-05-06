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

#include "RiaGrpcServiceInterface.h"
#include "ProjectInfo.grpc.pb.h"

#include <grpcpp/grpcpp.h>

namespace ResInsight
{
class Empty;
class Case;
}

using grpc::Status;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;

//==================================================================================================
//
// gRPC-service answering requests about project information
//
//==================================================================================================
class RiaGrpcProjectInfoServiceImpl final : public ResInsight::ProjectInfo::AsyncService, public RiaGrpcServiceInterface
{
public:
    Status GetCurrentCase(ServerContext* context, const ResInsight::Empty* request, ResInsight::Case* reply) override;

    std::vector<RiaGrpcServerCallMethod*> createCallbacks(ServerCompletionQueue* cq) override;
};


