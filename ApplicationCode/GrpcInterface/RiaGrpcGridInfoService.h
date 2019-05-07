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

#include "GridInfo.grpc.pb.h"
#include "RiaGrpcServiceInterface.h"

#include <grpcpp/grpcpp.h>
#include <vector>

namespace ResInsight
{
class Case;
}

class RiaGrpcServerCallMethod;

//==================================================================================================
//
// gRPC-service answering requests about grid information for a given case
//
//==================================================================================================
class RiaGrpcGridInfoService final : public ResInsight::GridInfo::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GridCount(grpc::ServerContext* context, const ResInsight::Case* request, ResInsight::GridCountInfo* reply) override;
    grpc::Status AllDimensions(grpc::ServerContext* context, const ResInsight::Case* request, ResInsight::AllGridDimensions* reply) override;
    grpc::Status AllActiveCellInfos(grpc::ServerContext* context, const ResInsight::ActiveCellInfoRequest* request, ResInsight::ActiveCellInfos* reply) override;

public:
    std::vector<RiaGrpcServerCallMethod*> createCallbacks(grpc::ServerCompletionQueue* cq) override;
};




