/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "SimulatorTables.grpc.pb.h"

#include "RiaGrpcServiceInterface.h"

#include "cvfObject.h"

#include <string>
#include <vector>

namespace rips
{
class SimulationWellRequest;
class SimulationWellStatus;
} // namespace rips

class RiaGrpcCallbackInterface;

class RimEclipseCase;
class RigCompletionData;

//==================================================================================================
//
// gRPC-service answering requests about grid information for a simulation wells
//
//==================================================================================================
class RiaGrpcWellPathService final : public rips::WellPath::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetCompletionData( ::grpc::ServerContext*               context,
                                    const ::rips::SimulatorTableRequest* request,
                                    ::rips::SimulatorTableData*          response ) override;

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;

private:
    static void copyCompDatToGrpc( const RigCompletionData& inputData, rips::SimulatorCompdatEntry* compDat );
};
