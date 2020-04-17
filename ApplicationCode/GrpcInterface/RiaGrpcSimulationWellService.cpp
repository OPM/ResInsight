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

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigSimWellData.h"

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
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( findCase( request->case_id() ) );
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
        switch ( currentWellResult->wellResultFrame( tsIdx ).m_productionType )
        {
            case RigWellResultFrame::PRODUCER:
                wellType = "Producer";
                break;
            case RigWellResultFrame::OIL_INJECTOR:
                wellType = "OilInjector";
                break;
            case RigWellResultFrame::WATER_INJECTOR:
                wellType = "WaterInjector";
                break;
            case RigWellResultFrame::GAS_INJECTOR:
                wellType = "GasInjector";
                break;
        }

        wellStatus = currentWellResult->wellResultFrame( tsIdx ).m_isOpen;
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
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( findCase( request->case_id() ) );
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
        const RigWellResultFrame& wellResFrame = currentWellResult->wellResultFrame( tsIdx );
        std::vector<RigGridBase*> grids;
        eclipseCase->eclipseCaseData()->allGrids( &grids );

        for ( size_t bIdx = 0; bIdx < wellResFrame.m_wellResultBranches.size(); ++bIdx )
        {
            const std::vector<RigWellResultPoint>& branchResPoints =
                wellResFrame.m_wellResultBranches[bIdx].m_branchResultPoints;
            for ( size_t rpIdx = 0; rpIdx < branchResPoints.size(); ++rpIdx )
            {
                const RigWellResultPoint& resPoint = branchResPoints[rpIdx];

                if ( resPoint.isCell() )
                {
                    rips::SimulationWellCellInfo* cellInfo = reply->add_data();
                    size_t                        i;
                    size_t                        j;
                    size_t                        k;
                    size_t                        gridIdx = resPoint.m_gridIndex;
                    grids[gridIdx]->ijkFromCellIndex( resPoint.m_gridCellIndex, &i, &j, &k );

                    Vec3i* ijk = new Vec3i;
                    ijk->set_i( i );
                    ijk->set_j( j );
                    ijk->set_k( k );

                    cellInfo->set_allocated_ijk( ijk );
                    cellInfo->set_grid_index( gridIdx );
                    cellInfo->set_is_open( resPoint.m_isOpen );
                    cellInfo->set_branch_id( resPoint.m_ertBranchId );
                    cellInfo->set_segment_id( resPoint.m_ertSegmentId );
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
    typedef RiaGrpcSimulationWellService Self;

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
