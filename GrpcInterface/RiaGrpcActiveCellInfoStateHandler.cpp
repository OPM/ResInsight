/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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
#include "RiaGrpcActiveCellInfoStateHandler.h"

#include "RiaGrpcCallbacks.h"
#include "RiaGrpcHelper.h"
#include "RiaSocketTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"

#include "Riu3dSelectionManager.h"

#include <array>

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcActiveCellInfoStateHandler::RiaGrpcActiveCellInfoStateHandler()
    : m_request( nullptr )
    , m_eclipseCase( nullptr )
    , m_activeCellInfo( nullptr )
    , m_currentCellIdx( 0u )
    , m_porosityModel( RiaDefines::PorosityModelType::MATRIX_MODEL )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcActiveCellInfoStateHandler::init( const rips::CellInfoRequest* request )
{
    CAF_ASSERT( request );
    m_request = request;

    m_porosityModel  = RiaDefines::PorosityModelType( m_request->porosity_model() );
    RimCase* rimCase = RiaGrpcHelper::findCase( m_request->case_request().id() );
    m_eclipseCase    = dynamic_cast<RimEclipseCase*>( rimCase );

    if ( !m_eclipseCase )
    {
        return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
    }

    if ( !m_eclipseCase->eclipseCaseData() || !m_eclipseCase->eclipseCaseData()->mainGrid() )
    {
        return grpc::Status( grpc::NOT_FOUND, "Eclipse Case Data not found" );
    }

    m_activeCellInfo = m_eclipseCase->eclipseCaseData()->activeCellInfo( m_porosityModel );

    if ( !m_activeCellInfo )
    {
        return grpc::Status( grpc::NOT_FOUND, "Active Cell Info not found" );
    }

    size_t globalCoarseningBoxCount = 0;

    for ( size_t gridIdx = 0; gridIdx < m_eclipseCase->eclipseCaseData()->gridCount(); gridIdx++ )
    {
        m_globalCoarseningBoxIndexStart.push_back( globalCoarseningBoxCount );

        RigGridBase* grid = m_eclipseCase->eclipseCaseData()->grid( gridIdx );

        size_t localCoarseningBoxCount = grid->coarseningBoxCount();
        globalCoarseningBoxCount += localCoarseningBoxCount;
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcActiveCellInfoStateHandler::assignNextActiveCellInfoData( rips::CellInfo* cellInfo )
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->reservoirCells();

    while ( m_currentCellIdx < reservoirCells.size() )
    {
        size_t cellIdxToTry = m_currentCellIdx++;
        if ( m_activeCellInfo->isActive( cellIdxToTry ) )
        {
            assignCellInfoData( cellInfo, reservoirCells, cellIdxToTry );
            return grpc::Status::OK;
        }
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcActiveCellInfoStateHandler::assignCellInfoData( rips::CellInfo*             cellInfo,
                                                            const std::vector<RigCell>& reservoirCells,
                                                            size_t                      cellIdx )
{
    RigGridBase* grid = reservoirCells[cellIdx].hostGrid();
    CVF_ASSERT( grid != nullptr );
    size_t cellIndex = reservoirCells[cellIdx].gridLocalCellIndex();

    size_t i, j, k;
    grid->ijkFromCellIndex( cellIndex, &i, &j, &k );

    size_t       pi, pj, pk;
    RigGridBase* parentGrid = nullptr;

    if ( grid->isMainGrid() )
    {
        pi         = i;
        pj         = j;
        pk         = k;
        parentGrid = grid;
    }
    else
    {
        size_t parentCellIdx = reservoirCells[cellIdx].parentCellIndex();
        parentGrid           = ( static_cast<RigLocalGrid*>( grid ) )->parentGrid();
        CVF_ASSERT( parentGrid != nullptr );
        parentGrid->ijkFromCellIndex( parentCellIdx, &pi, &pj, &pk );
    }

    cellInfo->set_grid_index( (int)grid->gridIndex() );
    cellInfo->set_parent_grid_index( (int)parentGrid->gridIndex() );

    size_t coarseningIdx = reservoirCells[cellIdx].coarseningBoxIndex();
    if ( coarseningIdx != cvf::UNDEFINED_SIZE_T )
    {
        size_t globalCoarseningIdx = m_globalCoarseningBoxIndexStart[grid->gridIndex()] + coarseningIdx;
        cellInfo->set_coarsening_box_index( (int)globalCoarseningIdx );
    }
    else
    {
        cellInfo->set_coarsening_box_index( -1 );
    }
    {
        rips::Vec3i* local_ijk = new rips::Vec3i;
        local_ijk->set_i( (int)i );
        local_ijk->set_j( (int)j );
        local_ijk->set_k( (int)k );
        cellInfo->set_allocated_local_ijk( local_ijk );
    }
    {
        rips::Vec3i* parent_ijk = new rips::Vec3i;
        parent_ijk->set_i( (int)pi );
        parent_ijk->set_j( (int)pj );
        parent_ijk->set_k( (int)pk );
        cellInfo->set_allocated_parent_ijk( parent_ijk );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RiaGrpcActiveCellInfoStateHandler::activeCellInfo() const
{
    return m_activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigCell>& RiaGrpcActiveCellInfoStateHandler::reservoirCells() const
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->reservoirCells();
    return reservoirCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcActiveCellInfoStateHandler::assignReply( rips::CellInfoArray* reply )
{
    const size_t packageSize    = RiaGrpcHelper::numberOfDataUnitsInPackage( sizeof( rips::CellInfo ) );
    size_t       indexInPackage = 0u;
    reply->mutable_data()->Reserve( (int)packageSize );

    // Stream until you've reached the package size or total cell count. Whatever comes first.
    // If you've reached the package size you'll come back for another round.
    for ( ; indexInPackage < packageSize && m_currentCellIdx < m_activeCellInfo->reservoirCellCount(); ++indexInPackage )
    {
        rips::CellInfo singleCellInfo;
        grpc::Status   singleCellInfoStatus = assignNextActiveCellInfoData( &singleCellInfo );
        if ( singleCellInfoStatus.ok() )
        {
            rips::CellInfo* allocCellInfo = reply->add_data();
            *allocCellInfo                = singleCellInfo;
        }
        else
        {
            break;
        }
    }
    if ( indexInPackage > 0u )
    {
        return Status::OK;
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcActiveCellInfoStateHandler::assignNextActiveCellCenter( rips::Vec3d* cellCenter )
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->reservoirCells();

    while ( m_currentCellIdx < reservoirCells.size() )
    {
        size_t cellIdxToTry = m_currentCellIdx++;
        if ( m_activeCellInfo->isActive( cellIdxToTry ) )
        {
            assignCellCenter( cellCenter, reservoirCells, cellIdxToTry );
            return grpc::Status::OK;
        }
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcActiveCellInfoStateHandler::assignCellCenter( rips::Vec3d*                cellCenter,
                                                          const std::vector<RigCell>& reservoirCells,
                                                          size_t                      cellIdx )

{
    cvf::Vec3d center = reservoirCells[cellIdx].center();

    RiaGrpcHelper::convertVec3dToPositiveDepth( &center );

    cellCenter->set_x( center.x() );
    cellCenter->set_y( center.y() );
    cellCenter->set_z( center.z() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcActiveCellInfoStateHandler::assignCellCentersReply( rips::CellCenters* reply )
{
    const size_t packageSize    = RiaGrpcHelper::numberOfDataUnitsInPackage( sizeof( rips::Vec3d ) );
    size_t       indexInPackage = 0u;
    reply->mutable_centers()->Reserve( (int)packageSize );
    for ( ; indexInPackage < packageSize && m_currentCellIdx < m_activeCellInfo->reservoirCellCount(); ++indexInPackage )
    {
        rips::Vec3d  singleCellCenter;
        grpc::Status singleCellCenterStatus = assignNextActiveCellCenter( &singleCellCenter );
        if ( singleCellCenterStatus.ok() )
        {
            rips::Vec3d* allocCellCenter = reply->add_centers();
            *allocCellCenter             = singleCellCenter;
        }
        else
        {
            break;
        }
    }
    if ( indexInPackage > 0u )
    {
        return Status::OK;
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcActiveCellInfoStateHandler::assignNextActiveCellCorners( rips::CellCorners* cellCorners )
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->reservoirCells();

    while ( m_currentCellIdx < reservoirCells.size() )
    {
        size_t cellIdxToTry = m_currentCellIdx++;
        if ( m_activeCellInfo->isActive( cellIdxToTry ) )
        {
            assignCellCorners( cellCorners, reservoirCells, cellIdxToTry );
            return grpc::Status::OK;
        }
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcActiveCellInfoStateHandler::assignCellCorners( rips::CellCorners*          corners,
                                                           const std::vector<RigCell>& reservoirCells,
                                                           size_t                      cellIdx )
{
    RigGridBase*              grid        = m_eclipseCase->eclipseCaseData()->mainGrid();
    std::array<cvf::Vec3d, 8> cornerVerts = grid->cellCornerVertices( cellIdx );
    for ( cvf::Vec3d& corner : cornerVerts )
    {
        RiaGrpcHelper::convertVec3dToPositiveDepth( &corner );
    }

    RiaGrpcHelper::setCornerValues( corners->mutable_c0(), cornerVerts[0] );
    RiaGrpcHelper::setCornerValues( corners->mutable_c1(), cornerVerts[1] );
    RiaGrpcHelper::setCornerValues( corners->mutable_c2(), cornerVerts[2] );
    RiaGrpcHelper::setCornerValues( corners->mutable_c3(), cornerVerts[3] );
    RiaGrpcHelper::setCornerValues( corners->mutable_c4(), cornerVerts[4] );
    RiaGrpcHelper::setCornerValues( corners->mutable_c5(), cornerVerts[5] );
    RiaGrpcHelper::setCornerValues( corners->mutable_c6(), cornerVerts[6] );
    RiaGrpcHelper::setCornerValues( corners->mutable_c7(), cornerVerts[7] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcActiveCellInfoStateHandler::assignCellCornersReply( rips::CellCornersArray* reply )
{
    const size_t packageSize    = RiaGrpcHelper::numberOfDataUnitsInPackage( sizeof( rips::CellCorners ) );
    size_t       indexInPackage = 0u;
    reply->mutable_cells()->Reserve( (int)packageSize );
    for ( ; indexInPackage < packageSize && m_currentCellIdx < m_activeCellInfo->reservoirCellCount(); ++indexInPackage )
    {
        rips::CellCorners singleCellCorners;
        grpc::Status      singleCellCornersStatus = assignNextActiveCellCorners( &singleCellCorners );
        if ( singleCellCornersStatus.ok() )
        {
            rips::CellCorners* allocCellCorners = reply->add_cells();
            *allocCellCorners                   = singleCellCorners;
        }
        else
        {
            break;
        }
    }
    if ( indexInPackage > 0u )
    {
        return Status::OK;
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}
