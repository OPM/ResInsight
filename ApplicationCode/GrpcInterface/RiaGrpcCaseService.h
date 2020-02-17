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

#include "Case.grpc.pb.h"

#include "RiaGrpcServiceInterface.h"
#include "RiaPorosityModel.h"

#include <vector>

namespace rips
{
class CaseRequest;
class PdmObject;
} // namespace rips

class RiaGrpcCallbackInterface;
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
    typedef grpc::Status Status;

public:
    RiaActiveCellInfoStateHandler();

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

protected:
    const rips::CellInfoRequest*  m_request;
    RimEclipseCase*               m_eclipseCase;
    RiaDefines::PorosityModelType m_porosityModel;
    RigActiveCellInfo*            m_activeCellInfo;
    std::vector<size_t>           m_globalCoarseningBoxIndexStart;
    size_t                        m_currentCellIdx;
};

//==================================================================================================
//
// gRPC-service answering requests about grid information for a given case
//
//==================================================================================================
class RiaGrpcCaseService final : public rips::Case::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status
                 GetGridCount( grpc::ServerContext* context, const rips::CaseRequest* request, rips::GridCount* reply ) override;
    grpc::Status GetCellCount( grpc::ServerContext*         context,
                               const rips::CellInfoRequest* request,
                               rips::CellCount*             reply ) override;
    grpc::Status GetTimeSteps( grpc::ServerContext*     context,
                               const rips::CaseRequest* request,
                               rips::TimeStepDates*     reply ) override;
    grpc::Status GetDaysSinceStart( grpc::ServerContext*     context,
                                    const rips::CaseRequest* request,
                                    rips::DaysSinceStart*    reply ) override;
    grpc::Status GetCaseInfo( grpc::ServerContext* context, const rips::CaseRequest* request, rips::CaseInfo* reply ) override;
    grpc::Status
                 GetPdmObject( grpc::ServerContext* context, const rips::CaseRequest* request, rips::PdmObject* reply ) override;
    grpc::Status GetCellInfoForActiveCells( grpc::ServerContext*           context,
                                            const rips::CellInfoRequest*   request,
                                            rips::CellInfoArray*           reply,
                                            RiaActiveCellInfoStateHandler* stateHandler );
    grpc::Status GetCellCenterForActiveCells( grpc::ServerContext*           context,
                                              const rips::CellInfoRequest*   request,
                                              rips::CellCenters*             reply,
                                              RiaActiveCellInfoStateHandler* stateHandler );
    grpc::Status GetReservoirBoundingBox( grpc::ServerContext*     context,
                                          const rips::CaseRequest* request,
                                          rips::BoundingBox*       reply );

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
