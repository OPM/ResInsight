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
#include "RiaGrpcSimulationWellService.h"

#include "RiaGrpcCallbacks.h"
#include "RiaGrpcHelper.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "RimCase.h"
#include "RimEclipseCase.h"

#include "cvfCollection.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcSimulationWellService::GetSimulationWellStatus( grpc::ServerContext*               context,
                                                                    const rips::SimulationWellRequest* request,
                                                                    rips::SimulationWellStatus*        reply )

{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( request->case_id() ) );
    if ( !eclipseCase )
    {
        return grpc::Status( grpc::NOT_FOUND, "Case not found" );
    }

    // First find the well result for the correct well
    cvf::ref<RigSimWellData> currentWellResult = findWellResult( eclipseCase, request->well_name() );
    if ( currentWellResult.isNull() )
    {
        return grpc::Status( grpc::NOT_FOUND, "Well not found" );
    }

    size_t tsIdx = static_cast<size_t>( request->timestep() );

    QString wellType   = "NotDefined";
    bool    wellStatus = false;
    if ( currentWellResult->hasWellResult( tsIdx ) )
    {
        switch ( currentWellResult->wellResultFrame( tsIdx )->productionType() )
        {
            case RiaDefines::WellProductionType::PRODUCER:
                wellType = "Producer";
                break;
            case RiaDefines::WellProductionType::OIL_INJECTOR:
                wellType = "OilInjector";
                break;
            case RiaDefines::WellProductionType::WATER_INJECTOR:
                wellType = "WaterInjector";
                break;
            case RiaDefines::WellProductionType::GAS_INJECTOR:
                wellType = "GasInjector";
                break;
        }

        wellStatus = currentWellResult->wellResultFrame( tsIdx )->isOpen();
    }

    reply->set_well_type( wellType.toStdString() );
    reply->set_is_open( wellStatus );

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcSimulationWellService::GetSimulationWellCells( grpc::ServerContext*               context,
                                                                   const rips::SimulationWellRequest* request,
                                                                   rips::SimulationWellCellInfoArray* reply )
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( request->case_id() ) );
    if ( !eclipseCase )
    {
        return grpc::Status( grpc::NOT_FOUND, "Case not found" );
    }

    // First find the well result for the correct well
    cvf::ref<RigSimWellData> currentWellResult = findWellResult( eclipseCase, request->well_name() );
    if ( currentWellResult.isNull() )
    {
        return grpc::Status( grpc::NOT_FOUND, "Well not found" );
    }

    size_t tsIdx = static_cast<size_t>( request->timestep() );
    if ( currentWellResult->hasWellResult( tsIdx ) )
    {
        // Fetch results
        const RigWellResultFrame* wellResFrame = currentWellResult->wellResultFrame( tsIdx );
        std::vector<RigGridBase*> grids;
        eclipseCase->eclipseCaseData()->allGrids( &grids );

        for ( size_t bIdx = 0; bIdx < wellResFrame->wellResultBranches().size(); ++bIdx )
        {
            const std::vector<RigWellResultPoint> branchResPoints = wellResFrame->branchResultPointsFromBranchIndex( bIdx );
            for ( size_t rpIdx = 0; rpIdx < branchResPoints.size(); ++rpIdx )
            {
                const RigWellResultPoint& resPoint = branchResPoints[rpIdx];

                if ( resPoint.isCell() )
                {
                    rips::SimulationWellCellInfo* cellInfo = reply->add_data();
                    size_t                        i;
                    size_t                        j;
                    size_t                        k;
                    const size_t                  gridIdx = resPoint.gridIndex();
                    grids[gridIdx]->ijkFromCellIndex( resPoint.cellIndex(), &i, &j, &k );

                    Vec3i* ijk = new Vec3i;
                    ijk->set_i( i );
                    ijk->set_j( j );
                    ijk->set_k( k );

                    cellInfo->set_allocated_ijk( ijk );
                    cellInfo->set_grid_index( gridIdx );
                    cellInfo->set_is_open( resPoint.isOpen() );
                    cellInfo->set_branch_id( resPoint.branchId() );
                    cellInfo->set_segment_id( resPoint.segmentId() );
                }
            }
        }
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::ref<RigSimWellData> RiaGrpcSimulationWellService::findWellResult( const RimEclipseCase* eclipseCase,
                                                                       const std::string&    wellName )
{
    const cvf::Collection<RigSimWellData>& allWellRes = eclipseCase->eclipseCaseData()->wellResults();
    for ( size_t tsIdx = 0; tsIdx < allWellRes.size(); ++tsIdx )
    {
        if ( allWellRes[tsIdx]->m_wellName.toStdString() == wellName )
        {
            return allWellRes[tsIdx];
        }
    }

    return cvf::ref<RigSimWellData>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcSimulationWellService::createCallbacks()
{
    using Self = RiaGrpcSimulationWellService;

    return {
        new RiaGrpcUnaryCallback<Self, SimulationWellRequest, SimulationWellStatus>( this,
                                                                                     &Self::GetSimulationWellStatus,
                                                                                     &Self::RequestGetSimulationWellStatus ),
        new RiaGrpcUnaryCallback<Self, SimulationWellRequest, SimulationWellCellInfoArray>( this,
                                                                                            &Self::GetSimulationWellCells,
                                                                                            &Self::RequestGetSimulationWellCells ),
    };
}

static bool RiaGrpcSimulationWellService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcSimulationWellService>(
        typeid( RiaGrpcSimulationWellService ).hash_code() );
