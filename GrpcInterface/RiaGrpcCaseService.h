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
class CaseInfo;
} // namespace rips

class RiaGrpcCallbackInterface;
class RigCell;
class RigActiveCellInfo;
class RimEclipseCase;
class RiaGrpcActiveCellInfoStateHandler;
class RiaGrpcSelectedCellsStateHandler;

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
    grpc::Status GetCellInfoForActiveCells( grpc::ServerContext*               context,
                                            const rips::CellInfoRequest*       request,
                                            rips::CellInfoArray*               reply,
                                            RiaGrpcActiveCellInfoStateHandler* stateHandler );
    grpc::Status GetCellCenterForActiveCells( grpc::ServerContext*               context,
                                              const rips::CellInfoRequest*       request,
                                              rips::CellCenters*                 reply,
                                              RiaGrpcActiveCellInfoStateHandler* stateHandler );
    grpc::Status GetCellCornersForActiveCells( grpc::ServerContext*               context,
                                               const rips::CellInfoRequest*       request,
                                               rips::CellCornersArray*            reply,
                                               RiaGrpcActiveCellInfoStateHandler* stateHandler );
    grpc::Status GetSelectedCells( grpc::ServerContext*              context,
                                   const rips::CaseRequest*          request,
                                   rips::SelectedCells*              reply,
                                   RiaGrpcSelectedCellsStateHandler* stateHandler );
    grpc::Status GetReservoirBoundingBox( grpc::ServerContext*     context,
                                          const rips::CaseRequest* request,
                                          rips::BoundingBox*       reply ) override;
    grpc::Status GetCoarseningInfoArray( grpc::ServerContext*       context,
                                         const rips::CaseRequest*   request,
                                         rips::CoarseningInfoArray* reply ) override;

    grpc::Status GetDistanceToClosestFault( grpc::ServerContext*             context,
                                            const rips::ClosestFaultRequest* request,
                                            rips::ClosestFault*              reply ) override;

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
