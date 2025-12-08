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

namespace internal
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void populateReply( RimEclipseCase* eclipseCase, RimWellPath* wellPath, ::rips::SimulatorTableData* reply )
{
    std::set<QString> gridNamesWithCompletions;
    auto compdata = RicWellPathExportCompletionDataFeatureImpl::completionDataForWellPath( wellPath, eclipseCase );
    for ( const auto& cd : compdata )
    {
        SimulatorCompdatEntry* grpcData = reply->add_compdat();
        RiaWellPathDataToGrpcConverter::copyCompdatToGrpc( cd, grpcData );

        auto completionCell = cd.completionDataGridCell();
        gridNamesWithCompletions.insert( completionCell.lgrName() );

        const auto wpimult = cd.wpimult();
        if ( wpimult != 0.0 && !RigCompletionData::isDefaultValue( wpimult ) )
        {
            const auto wellName = wellPath->completionSettings()->wellNameForExport();

            SimulatorWpimultEntry* grpcWpiData = reply->add_wpimult();
            grpcWpiData->set_well_name( wellName.toStdString() );
            grpcWpiData->set_pimult( wpimult );
            grpcWpiData->set_i( completionCell.localCellIndexI() + 1 );
            grpcWpiData->set_j( completionCell.localCellIndexJ() + 1 );
            grpcWpiData->set_k( completionCell.localCellIndexK() + 1 );
            grpcWpiData->set_grid_name( completionCell.lgrName().toStdString() );
        }
    }

    // See RicWellPathExportCompletionDataFeatureImpl::exportWelspeclToFile
    for ( const auto& gridName : gridNamesWithCompletions )
    {
        auto upperGridIntersection =
            RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ( eclipseCase, wellPath, gridName );

        auto       compSettings   = wellPath->completionSettings();
        cvf::Vec2i ijIntersection = std::get<1>( upperGridIntersection );

        SimulatorWelspecsEntry* grpcData = reply->add_welspecs();
        RiaWellPathDataToGrpcConverter::copyWelspecsToGrpc( compSettings,
                                                            grpcData,
                                                            ijIntersection.x(),
                                                            ijIntersection.y(),
                                                            gridName.toStdString() );
    }

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
}

} // namespace internal

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

    auto proj = RimProject::current();
    if ( !proj )
    {
        return grpc::Status( grpc::INTERNAL, "No current project" );
    }

    auto wellPath = proj->wellPathByName( QString::fromStdString( request->wellpath_name() ) );
    if ( !wellPath )
    {
        return grpc::Status( grpc::NOT_FOUND, "Well path " + request->wellpath_name() + " not found" );
    }

    internal::populateReply( eclipseCase, wellPath, reply );

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
        if ( !wellPath ) continue;

        internal::populateReply( eclipseCase, wellPath, reply );
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
