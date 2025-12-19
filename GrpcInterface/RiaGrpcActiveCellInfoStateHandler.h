/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "Case.grpc.pb.h"

#include "RiaPorosityModel.h"

#include <vector>

class RigCell;
class RigActiveCellInfo;
class RimEclipseCase;

//==================================================================================================
//
// State handler for streaming of active cell info
//
//==================================================================================================
class RiaGrpcActiveCellInfoStateHandler
{
    using Status = grpc::Status;

public:
    RiaGrpcActiveCellInfoStateHandler();

    Status init( const rips::CellInfoRequest* request );

    RigActiveCellInfo*          activeCellInfo() const;
    const std::vector<RigCell>& reservoirCells() const;

    // For cell info:
    Status assignNextActiveCellInfoData( rips::CellInfo* cellInfo );
    void   assignCellInfoData( rips::CellInfo* cellInfo, const std::vector<RigCell>& reservoirCells, size_t cellIdx );
    Status assignReply( rips::CellInfoArray* reply );

    // For cell centers:
    Status assignNextActiveCellCenter( rips::Vec3d* cellCenter );
    void   assignCellCenter( rips::Vec3d* cellCenter, const std::vector<RigCell>& reservoirCells, size_t cellIdx );
    Status assignCellCentersReply( rips::CellCenters* reply );

    // For cell corners:
    Status assignNextActiveCellCorners( rips::CellCorners* cellCorners );
    void assignCellCorners( rips::CellCorners* cellCorners, const std::vector<RigCell>& reservoirCells, size_t cellIdx );
    Status assignCellCornersReply( rips::CellCornersArray* reply );

protected:
    const rips::CellInfoRequest*  m_request;
    RimEclipseCase*               m_eclipseCase;
    RiaDefines::PorosityModelType m_porosityModel;
    RigActiveCellInfo*            m_activeCellInfo;
    std::vector<size_t>           m_globalCoarseningBoxIndexStart;
    size_t                        m_currentCellIdx;
};
