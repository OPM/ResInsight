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
#include "RiaPorosityModel.h"

#include <vector>

namespace rips
{
class Case;
}

class RiaAbstractGrpcCallback;
class RigCell;
class RigActiveCellInfo;
class RimEclipseCase;

//==================================================================================================
//
// State handler for streaming of active cell info
//
//==================================================================================================
class RiaActiveCellInfoStateHandler
{
public:
    typedef grpc::Status Status;

    RiaActiveCellInfoStateHandler();

    Status init(const rips::ActiveCellInfoRequest* request);
    Status assignNextActiveCellInfoData(rips::ActiveCellInfo* cellInfo);
    void   assignActiveCellInfoData(rips::ActiveCellInfo* cellInfo, const std::vector<RigCell>& reservoirCells, size_t cellIdx);
    Status assignReply(rips::ActiveCellInfoArray* reply);
    RigActiveCellInfo*          activeCellInfo() const;
    const std::vector<RigCell>& reservoirCells() const;

protected:
    const rips::ActiveCellInfoRequest* m_request;
    RimEclipseCase*                    m_eclipseCase;
    RiaDefines::PorosityModelType      m_porosityModel;
    RigActiveCellInfo*                 m_activeCellInfo;
    std::vector<size_t>                m_globalCoarseningBoxIndexStart;
    size_t                             m_currentCellIdx;
};

//==================================================================================================
//
// gRPC-service answering requests about grid information for a given case
//
//==================================================================================================
class RiaGrpcGridInfoService final : public rips::GridInfo::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetGridCount(grpc::ServerContext* context, const rips::Case* request, rips::GridCount* reply) override;
    grpc::Status GetGridDimensions(grpc::ServerContext* context, const rips::Case* request, rips::GridDimensions* reply) override;
    grpc::Status StreamActiveCellInfo(grpc::ServerContext*               context,
                                      const rips::ActiveCellInfoRequest* request,
                                      rips::ActiveCellInfoArray*         reply,
                                      RiaActiveCellInfoStateHandler*     stateHandler);
    std::vector<RiaAbstractGrpcCallback*> createCallbacks() override;
};
