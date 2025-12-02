/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SimulatorTables.pb.h"

class RigCompletionData;
class RimWellPathCompletionSettings;
class RigMswTableData;

//==================================================================================================
///
/// Helper class for converting well path data to gRPC protobuf messages
///
//==================================================================================================
class RiaWellPathDataToGrpcConverter
{
public:
    static void copyCompdatToGrpc( const RigCompletionData& inputData, rips::SimulatorCompdatEntry* compDat );

    static void copyWelspecsToGrpc( const RimWellPathCompletionSettings* compSettings,
                                    rips::SimulatorWelspecsEntry*        grpcData,
                                    int                                  gridI,
                                    int                                  gridJ,
                                    std::string                          gridName );

    static void copyWelsegsToGrpc( const RigMswTableData& mswTableData, rips::SimulatorWelsegsEntry* grpcData );
    static void copyCompsegsToGrpc( const RigMswTableData& mswTableData, rips::SimulatorTableData* reply );
    static void copyWsegvalvToGrpc( const RigMswTableData& mswTableData, rips::SimulatorTableData* reply );
    static void copyWsegaicdToGrpc( const RigMswTableData& mswTableData, rips::SimulatorTableData* reply );
};