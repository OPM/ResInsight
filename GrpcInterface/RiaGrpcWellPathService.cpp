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
//////////////////////////////////////////////////////////////////////////////////
#include "RiaGrpcWellPathService.h"

#include "RiaGrpcCallbacks.h"
#include "RiaGrpcHelper.h"

#include "../Commands/CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimWellPath.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcWellPathService::GetCompletionData( grpc::ServerContext*               context,
                                                        const rips::SimulatorTableRequest* request,
                                                        rips::SimulatorTableData*          reply )

{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( request->case_id() ) );
    if ( !eclipseCase )
    {
        return grpc::Status( grpc::NOT_FOUND, "Case not found" );
    }

    RimWellPath* wellPath = nullptr;

    if ( auto proj = RimProject::current() )
    {
        wellPath = proj->wellPathByName( QString::fromStdString( request->wellpath_name() ) );
    }

    if ( !wellPath )
    {
        return grpc::Status( grpc::NOT_FOUND, "Wellpath not found" );
    }

    if ( auto modWellPath = dynamic_cast<RimModeledWellPath*>( wellPath ) )
    {
        auto compdata = RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( modWellPath, eclipseCase );

        for ( const auto& cd : compdata )
        {
            SimulatorCompdatEntry* compDatData = reply->add_compdat();
            RiaGrpcWellPathService::copyCompDatToGrpc( cd, compDatData );
        }
    }
    else
    {
        return grpc::Status( grpc::NOT_FOUND, "Modeled well path not found" );
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcWellPathService::copyCompDatToGrpc( const RigCompletionData& inputData, rips::SimulatorCompdatEntry* compDat )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcWellPathService::createCallbacks()
{
    using Self = RiaGrpcWellPathService;

    return { new RiaGrpcUnaryCallback<Self, SimulatorTableRequest, SimulatorTableData>( this,
                                                                                        &Self::GetCompletionData,
                                                                                        &Self::RequestGetCompletionData ) };
}

static bool RiaGrpcWellPathService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcWellPathService>(
    typeid( RiaGrpcWellPathService ).hash_code() );
