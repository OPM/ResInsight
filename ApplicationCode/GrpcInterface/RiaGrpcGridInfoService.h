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

namespace rips
{
class Case;
}

class RiaGrpcServerCallMethod;
class RigCell;

//==================================================================================================
//
// gRPC-service answering requests about grid information for a given case
//
//==================================================================================================
class RiaGrpcGridInfoService final : public rips::GridInfo::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetGridCount(grpc::ServerContext* context, const rips::Case* request, rips::GridCount* reply) override;
    grpc::Status GetAllGridDimensions(grpc::ServerContext* context, const rips::Case* request, rips::AllGridDimensions* reply) override;
    grpc::Status GetAllActiveCellInfos(grpc::ServerContext* context, const rips::ActiveCellInfoRequest* request, rips::ActiveCellInfos* reply) override;
    grpc::Status StreamActiveCellInfo(grpc::ServerContext* context, const rips::ActiveCellInfoRequest* request, rips::ActiveCellInfo* reply, size_t* count);
    std::vector<RiaGrpcServerCallMethod*> createCallbacks() override;
    void assignActiveCellInfoData(rips::ActiveCellInfo* activeCellInfo, const std::vector<RigCell>& reservoirCells, size_t cIdx, const std::vector<size_t>& globalCoarseningBoxIndexStart);
};
