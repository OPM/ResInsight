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
        RiaGrpcWellPathService::copyCompdatToGrpc( cd, grpcData );
    }

    auto ijPos = RicWellPathExportCompletionDataFeatureImpl::wellPathUpperGridIntersectionIJ( eclipseCase, wellPath );
    auto compSettings = wellPath->completionSettings();

    SimulatorWelspecsEntry* grpcData = reply->add_welspecs();
    RiaGrpcWellPathService::copyWelspecsToGrpc( compSettings, grpcData, eclipseCase, ijPos.second.x(), ijPos.second.y() );

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcWellPathService::copyWelspecsToGrpc( const RimWellPathCompletionSettings* compSettings,
                                                 rips::SimulatorWelspecsEntry*        grpcData,
                                                 RimEclipseCase*                      eclipseCase,
                                                 int                                  gridI,
                                                 int                                  gridJ )
{
    grpcData->set_well_name( compSettings->wellNameForExport().toStdString() );
    grpcData->set_group_name( compSettings->groupNameForExport().toStdString() );
    grpcData->set_grid_i( gridI );
    grpcData->set_grid_j( gridJ );
    if ( compSettings->referenceDepth().has_value() )
    {
        grpcData->set_bhp_depth( compSettings->referenceDepth().value() );
    }
    grpcData->set_phase( compSettings->wellTypeNameForExport().toStdString() );
    if ( compSettings->drainageRadius().has_value() )
    {
        grpcData->set_drainage_radius( compSettings->drainageRadius().value() );
    }
    grpcData->set_inflow_equation( compSettings->gasInflowEquationForExport().toStdString() );
    grpcData->set_auto_shut_in( compSettings->automaticWellShutInForExport().toStdString() );
    grpcData->set_cross_flow( compSettings->allowWellCrossFlowForExport().toStdString() );
    grpcData->set_pvt_num( compSettings->wellBoreFluidPVT() );
    grpcData->set_hydrostatic_density_calc( compSettings->hydrostaticDensityForExport().toStdString() );
    grpcData->set_fip_region( compSettings->fluidInPlaceRegion() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcWellPathService::copyCompdatToGrpc( const RigCompletionData& inputData, rips::SimulatorCompdatEntry* compDat )
{
    compDat->set_comment( inputData.metaDataString().toStdString() );
    compDat->set_well_name( inputData.wellName().toStdString() );
    compDat->set_grid_i( inputData.completionDataGridCell().localCellIndexI() );
    compDat->set_grid_j( inputData.completionDataGridCell().localCellIndexJ() );
    compDat->set_upper_k( inputData.completionDataGridCell().localCellIndexK() );
    compDat->set_lower_k( inputData.completionDataGridCell().localCellIndexK() );
    compDat->set_open_shut_flag( "OPEN" );
    if ( inputData.saturation() != inputData.defaultValue() )
    {
        compDat->set_saturation( inputData.saturation() );
    }
    if ( inputData.transmissibility() != inputData.defaultValue() )
    {
        compDat->set_transmissibility( inputData.transmissibility() );
    }
    if ( inputData.diameter() != inputData.defaultValue() )
    {
        compDat->set_diameter( inputData.diameter() );
    }
    if ( inputData.kh() != inputData.defaultValue() )
    {
        compDat->set_kh( inputData.kh() );
    }
    if ( inputData.skinFactor() != inputData.defaultValue() )
    {
        compDat->set_skin_factor( inputData.skinFactor() );
    }
    if ( inputData.dFactor() != inputData.defaultValue() )
    {
        compDat->set_d_factor( inputData.dFactor() );
    }
    compDat->set_direction( inputData.directionStringXYZ().toStdString() );
    if ( inputData.startMD().has_value() )
    {
        compDat->set_start_md( inputData.startMD().value() );
        compDat->set_end_md( inputData.endMD().value() );
    }
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
