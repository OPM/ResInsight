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

#include "Grid.grpc.pb.h"

#include "RiaGrpcServiceInterface.h"

#include <vector>

class RigGridBase;
class RimEclipseCase;

namespace rips
{
class GridRequest;
class CellCenters;
class GridDimensions;
}; // namespace rips

//==================================================================================================
//
// State handler for streaming of active cell info
//
//==================================================================================================
class RiaCellCenterStateHandler
{
    typedef grpc::Status Status;

public:
    RiaCellCenterStateHandler();
    grpc::Status init( const rips::GridRequest* request );
    grpc::Status assignReply( rips::CellCenters* reply );

protected:
    const rips::GridRequest* m_request;
    RimEclipseCase*          m_eclipseCase;
    size_t                   m_currentCellIdx;
    const RigGridBase*       m_grid;
};

class RiaGrpcGridService final : public rips::Grid::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetCellCenters( grpc::ServerContext*       context,
                                 const rips::GridRequest*   request,
                                 rips::CellCenters*         reply,
                                 RiaCellCenterStateHandler* stateHandler );

    grpc::Status GetDimensions( grpc::ServerContext*     context,
                                const rips::GridRequest* request,
                                rips::GridDimensions*    reply ) override;

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
