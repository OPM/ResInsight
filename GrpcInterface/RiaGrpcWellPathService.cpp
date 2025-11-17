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
#include "RiaWellPathDataToGrpcConverter.h"

#include "Commands/CompletionExportCommands/RicWellPathExportCompletionDataFeatureImpl.h"
#include "Commands/CompletionExportCommands/RicWellPathExportMswTableData.h"

#include "CompletionsMsw/RigMswTableData.h"
#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigMainGrid.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletionSettings.h"

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
        return grpc::Status( grpc::NOT_FOUND, "Well path " + request->wellpath_name() + " not found" );
    }

    auto compdata = RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( wellPath, eclipseCase );
    for ( const auto& cd : compdata )
    {
        SimulatorCompdatEntry* grpcData = reply->add_compdat();
        RiaWellPathDataToGrpcConverter::copyCompdatToGrpc( cd, grpcData );
    }

    auto ijPos = RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ( eclipseCase, wellPath );
    auto compSettings = wellPath->completionSettings();

    SimulatorWelspecsEntry* grpcData = reply->add_welspecs();
    RiaWellPathDataToGrpcConverter::copyWelspecsToGrpc( compSettings,
                                                        grpcData,
                                                        eclipseCase,
                                                        ijPos.second.x(),
                                                        ijPos.second.y() );

    // Multisegment wells

    int  timeStep         = 0;
    auto mswDataContainer = RicWellPathExportMswTableData::extractSingleWellMswData( eclipseCase, wellPath, timeStep );
    if ( mswDataContainer.has_value() )
    {
        auto tables = mswDataContainer.value();

        if ( tables.hasWelsegsData() )
        {
            SimulatorWelsegsEntry* grpcSegData = reply->add_welsegs();
            RiaWellPathDataToGrpcConverter::copyWelsegsToGrpc( tables, grpcSegData );
        }

        RiaWellPathDataToGrpcConverter::copyCompsegsToGrpc( tables, reply );
        RiaWellPathDataToGrpcConverter::copyWsegvalvToGrpc( tables, reply );
        RiaWellPathDataToGrpcConverter::copyWsegaicdToGrpc( tables, reply );
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcWellPathService::GetCompletionDataUnified( grpc::ServerContext*                      context,
                                                               const rips::SimulatorTableUnifiedRequest* request,
                                                               rips::SimulatorTableData*                 reply )
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( request->case_id() ) );
    if ( !eclipseCase )
    {
        return grpc::Status( grpc::NOT_FOUND, "Case not found" );
    }

    auto proj = RimProject::current();
    if ( !proj )
    {
        return grpc::Status( grpc::INTERNAL, "No current project" );
    }

    // Process each well and merge all data into the single reply
    for ( const std::string& wellPathName : request->wellpath_names() )
    {
        RimWellPath* wellPath = proj->wellPathByName( QString::fromStdString( wellPathName ) );
        if ( !wellPath )
        {
            // Continue processing other wells instead of failing the entire request
            continue;
        }

        // Add completion data for this well
        auto compdata = RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( wellPath, eclipseCase );
        for ( const auto& cd : compdata )
        {
            SimulatorCompdatEntry* grpcData = reply->add_compdat();
            RiaWellPathDataToGrpcConverter::copyCompdatToGrpc( cd, grpcData );
        }

        // Add welspecs data for this well
        auto ijPos = RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ( eclipseCase, wellPath );
        auto compSettings = wellPath->completionSettings();

        SimulatorWelspecsEntry* grpcData = reply->add_welspecs();
        RiaWellPathDataToGrpcConverter::copyWelspecsToGrpc( compSettings,
                                                            grpcData,
                                                            eclipseCase,
                                                            ijPos.second.x(),
                                                            ijPos.second.y() );

        // Add multisegment well data for this well
        int timeStep = 0;
        auto mswDataContainer = RicWellPathExportMswTableData::extractSingleWellMswData( eclipseCase, wellPath, timeStep );
        if ( mswDataContainer.has_value() )
        {
            auto tables = mswDataContainer.value();

            if ( tables.hasWelsegsData() )
            {
                SimulatorWelsegsEntry* grpcSegData = reply->add_welsegs();
                RiaWellPathDataToGrpcConverter::copyWelsegsToGrpc( tables, grpcSegData );
            }

            RiaWellPathDataToGrpcConverter::copyCompsegsToGrpc( tables, reply );
            RiaWellPathDataToGrpcConverter::copyWsegvalvToGrpc( tables, reply );
            RiaWellPathDataToGrpcConverter::copyWsegaicdToGrpc( tables, reply );
        }
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcWellPathService::createCallbacks()
{
    using Self = RiaGrpcWellPathService;

    return { new RiaGrpcUnaryCallback<Self, SimulatorTableRequest, SimulatorTableData>( this,
                                                                                        &Self::GetCompletionData,
                                                                                        &Self::RequestGetCompletionData ),
             new RiaGrpcUnaryCallback<Self, SimulatorTableUnifiedRequest, SimulatorTableData>( this,
                                                                                               &Self::GetCompletionDataUnified,
                                                                                               &Self::RequestGetCompletionDataUnified ) };
}

static bool RiaGrpcWellPathService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcWellPathService>(
    typeid( RiaGrpcWellPathService ).hash_code() );
