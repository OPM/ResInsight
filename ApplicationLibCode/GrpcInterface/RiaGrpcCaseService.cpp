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
#include "RiaGrpcCaseService.h"

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

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaActiveCellInfoStateHandler::RiaActiveCellInfoStateHandler()
    : m_request( nullptr )
    , m_eclipseCase( nullptr )
    , m_activeCellInfo( nullptr )
    , m_currentCellIdx( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellInfoStateHandler::init( const rips::CellInfoRequest* request )
{
    CAF_ASSERT( request );
    m_request = request;

    m_porosityModel  = RiaDefines::PorosityModelType( m_request->porosity_model() );
    RimCase* rimCase = RiaGrpcServiceInterface::findCase( m_request->case_request().id() );
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
grpc::Status RiaActiveCellInfoStateHandler::assignNextActiveCellInfoData( rips::CellInfo* cellInfo )
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->globalCellArray();

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
void RiaActiveCellInfoStateHandler::assignCellInfoData( rips::CellInfo*             cellInfo,
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
RigActiveCellInfo* RiaActiveCellInfoStateHandler::activeCellInfo() const
{
    return m_activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigCell>& RiaActiveCellInfoStateHandler::reservoirCells() const
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->globalCellArray();
    return reservoirCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellInfoStateHandler::assignReply( rips::CellInfoArray* reply )
{
    const size_t packageSize    = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( sizeof( rips::CellInfo ) );
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
grpc::Status RiaActiveCellInfoStateHandler::assignNextActiveCellCenter( rips::Vec3d* cellCenter )
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->globalCellArray();

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
void RiaActiveCellInfoStateHandler::assignCellCenter( rips::Vec3d*                cellCenter,
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
grpc::Status RiaActiveCellInfoStateHandler::assignCellCentersReply( rips::CellCenters* reply )
{
    const size_t packageSize    = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( sizeof( rips::Vec3d ) );
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
Status RiaActiveCellInfoStateHandler::assignNextActiveCellCorners( rips::CellCorners* cellCorners )
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->globalCellArray();

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
void RiaActiveCellInfoStateHandler::assignCellCorners( rips::CellCorners*          corners,
                                                       const std::vector<RigCell>& reservoirCells,
                                                       size_t                      cellIdx )
{
    cvf::Vec3d   cornerVerts[8];
    RigGridBase* grid = m_eclipseCase->eclipseCaseData()->mainGrid();
    grid->cellCornerVertices( cellIdx, cornerVerts );
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
Status RiaActiveCellInfoStateHandler::assignCellCornersReply( rips::CellCornersArray* reply )
{
    const size_t packageSize    = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( sizeof( rips::CellCorners ) );
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetGridCount( grpc::ServerContext*     context,
                                               const rips::CaseRequest* request,
                                               rips::GridCount*         reply )
{
    RimCase* rimCase = findCase( request->id() );

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    if ( eclipseCase )
    {
        int gridCount = (int)eclipseCase->mainGrid()->gridCount();
        reply->set_count( gridCount );
        return Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellCount( grpc::ServerContext*         context,
                                               const rips::CellInfoRequest* request,
                                               rips::CellCount*             reply )
{
    RimCase* rimCase = findCase( request->case_request().id() );

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    if ( eclipseCase )
    {
        auto               porosityModel  = RiaDefines::PorosityModelType( request->porosity_model() );
        RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( porosityModel );
        reply->set_active_cell_count( (int)activeCellInfo->reservoirActiveCellCount() );
        reply->set_reservoir_cell_count( (int)activeCellInfo->reservoirCellCount() );
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetTimeSteps( grpc::ServerContext*     context,
                                               const rips::CaseRequest* request,
                                               rips::TimeStepDates*     reply )
{
    RimCase* rimCase = findCase( request->id() );

    if ( rimCase )
    {
        std::vector<QDateTime> timeStepDates = rimCase->timeStepDates();
        for ( QDateTime dateTime : timeStepDates )
        {
            rips::TimeStepDate* date = reply->add_dates();
            date->set_year( dateTime.date().year() );
            date->set_month( dateTime.date().month() );
            date->set_day( dateTime.date().day() );
            date->set_hour( dateTime.time().hour() );
            date->set_minute( dateTime.time().minute() );
            date->set_second( dateTime.time().second() );
        }
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetDaysSinceStart( grpc::ServerContext*     context,
                                                    const rips::CaseRequest* request,
                                                    rips::DaysSinceStart*    reply )
{
    RimCase* rimCase = findCase( request->id() );

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    if ( eclipseCase )
    {
        RigEclipseResultAddress addrToMaxTimeStepCountResult;
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            eclipseCase->eclipseCaseData()
                ->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                ->maxTimeStepCount( &addrToMaxTimeStepCountResult );
            if ( !addrToMaxTimeStepCountResult.isValid() )
            {
                return grpc::Status( grpc::NOT_FOUND, "Invalid result. No time steps found." );
            }
        }

        std::vector<double> daysSinceSimulationStart = eclipseCase->eclipseCaseData()
                                                           ->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                                           ->daysSinceSimulationStart( addrToMaxTimeStepCountResult );

        for ( auto days : daysSinceSimulationStart )
        {
            reply->add_day_decimals( days );
        }
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcCaseService::GetCaseInfo( grpc::ServerContext* context, const rips::CaseRequest* request, rips::CaseInfo* reply )
{
    RimCase* rimCase = findCase( request->id() );
    if ( rimCase )
    {
        qint64  caseId      = rimCase->caseId();
        qint64  caseGroupId = -1;
        QString caseName, caseType;
        RiaSocketTools::getCaseInfoFromCase( rimCase, caseId, caseName, caseType, caseGroupId );

        reply->set_id( caseId );
        reply->set_group_id( caseGroupId );
        reply->set_name( caseName.toStdString() );
        reply->set_type( caseType.toStdString() );
        return Status::OK;
    }
    return Status( grpc::NOT_FOUND, "Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetPdmObject( grpc::ServerContext*     context,
                                               const rips::CaseRequest* request,
                                               rips::PdmObject*         reply )
{
    RimCase* rimCase = findCase( request->id() );
    if ( rimCase )
    {
        copyPdmObjectFromCafToRips( rimCase, reply );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellInfoForActiveCells( grpc::ServerContext*           context,
                                                            const rips::CellInfoRequest*   request,
                                                            rips::CellInfoArray*           reply,
                                                            RiaActiveCellInfoStateHandler* stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellCenterForActiveCells( grpc::ServerContext*           context,
                                                              const rips::CellInfoRequest*   request,
                                                              rips::CellCenters*             reply,
                                                              RiaActiveCellInfoStateHandler* stateHandler )
{
    return stateHandler->assignCellCentersReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellCornersForActiveCells( grpc::ServerContext*           context,
                                                               const rips::CellInfoRequest*   request,
                                                               rips::CellCornersArray*        reply,
                                                               RiaActiveCellInfoStateHandler* stateHandler )
{
    return stateHandler->assignCellCornersReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSelectedCellsStateHandler::RiaSelectedCellsStateHandler()
    : m_request( nullptr )
    , m_eclipseCase( nullptr )
    , m_currentItem( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaSelectedCellsStateHandler::init( const rips::CaseRequest* request )
{
    CAF_ASSERT( request );
    m_request = request;

    RimCase* rimCase = RiaGrpcServiceInterface::findCase( m_request->id() );
    m_eclipseCase    = dynamic_cast<RimEclipseCase*>( rimCase );

    if ( !m_eclipseCase )
    {
        return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
    }

    if ( !m_eclipseCase->eclipseCaseData() || !m_eclipseCase->eclipseCaseData()->mainGrid() )
    {
        return grpc::Status( grpc::NOT_FOUND, "Eclipse Case Data not found" );
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaSelectedCellsStateHandler::assignNextSelectedCell( rips::SelectedCell*                          cell,
                                                             const std::vector<RiuEclipseSelectionItem*>& items )
{
    while ( m_currentItem < items.size() )
    {
        size_t itemToTry = m_currentItem++;

        const RiuEclipseSelectionItem* item = items[itemToTry];
        CVF_ASSERT( item->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT );
        assignSelectedCell( cell, item );
        return grpc::Status::OK;
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSelectedCellsStateHandler::assignSelectedCell( rips::SelectedCell* cell, const RiuEclipseSelectionItem* item )
{
    CVF_ASSERT( item->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT );
    size_t i = -1;
    size_t j = -1;
    size_t k = -1;
    item->m_resultDefinition->eclipseCase()
        ->eclipseCaseData()
        ->grid( item->m_gridIndex )
        ->ijkFromCellIndex( item->m_gridLocalCellIndex, &i, &j, &k );

    cell->set_grid_index( item->m_gridIndex );
    rips::Vec3i* ijk = new rips::Vec3i;
    ijk->set_i( (int)i );
    ijk->set_j( (int)j );
    ijk->set_k( (int)k );
    cell->set_allocated_ijk( ijk );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaSelectedCellsStateHandler::assignReply( rips::SelectedCells* reply )
{
    std::vector<RiuSelectionItem*> items;
    Riu3dSelectionManager::instance()->selectedItems( items );

    // Only eclipse cases are currently supported. Also filter by case.
    std::vector<RiuEclipseSelectionItem*> eclipseItems;
    for ( auto item : items )
    {
        RiuEclipseSelectionItem* eclipseItem = dynamic_cast<RiuEclipseSelectionItem*>( item );
        if ( eclipseItem && eclipseItem->m_resultDefinition->eclipseCase()->caseId == m_request->id() )
        {
            eclipseItems.push_back( eclipseItem );
        }
    }

    const size_t packageSize    = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( sizeof( rips::SelectedCell ) );
    size_t       indexInPackage = 0u;
    reply->mutable_cells()->Reserve( (int)packageSize );
    for ( ; indexInPackage < packageSize && m_currentItem < eclipseItems.size(); ++indexInPackage )
    {
        rips::SelectedCell singleSelectedCell;
        grpc::Status       singleSelectedCellStatus = assignNextSelectedCell( &singleSelectedCell, eclipseItems );
        if ( singleSelectedCellStatus.ok() )
        {
            rips::SelectedCell* allocSelectedCell = reply->add_cells();
            *allocSelectedCell                    = singleSelectedCell;
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
grpc::Status RiaGrpcCaseService::GetSelectedCells( grpc::ServerContext*          context,
                                                   const rips::CaseRequest*      request,
                                                   rips::SelectedCells*          reply,
                                                   RiaSelectedCellsStateHandler* stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetReservoirBoundingBox( grpc::ServerContext*     context,
                                                          const rips::CaseRequest* request,
                                                          rips::BoundingBox*       reply )
{
    RimCase* rimCase = findCase( request->id() );
    if ( rimCase )
    {
        cvf::BoundingBox boundingBox = rimCase->reservoirBoundingBox();
        reply->set_min_x( boundingBox.min().x() );
        reply->set_min_y( boundingBox.min().y() );
        reply->set_min_z( boundingBox.min().z() );
        reply->set_max_x( boundingBox.max().x() );
        reply->set_max_y( boundingBox.max().y() );
        reply->set_max_z( boundingBox.max().z() );
        return Status::OK;
    }
    return Status( grpc::NOT_FOUND, "Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCoarseningInfoArray( grpc::ServerContext*       context,
                                                         const rips::CaseRequest*   request,
                                                         rips::CoarseningInfoArray* reply )
{
    RimEclipseCase* rimCase = dynamic_cast<RimEclipseCase*>( findCase( request->id() ) );
    if ( rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid() )
    {
        for ( size_t gridIdx = 0; gridIdx < rimCase->eclipseCaseData()->gridCount(); gridIdx++ )
        {
            RigGridBase* grid = rimCase->eclipseCaseData()->grid( gridIdx );

            size_t localCoarseningBoxCount = grid->coarseningBoxCount();
            for ( size_t boxIdx = 0; boxIdx < localCoarseningBoxCount; boxIdx++ )
            {
                size_t i1, i2, j1, j2, k1, k2;
                grid->coarseningBox( boxIdx, &i1, &i2, &j1, &j2, &k1, &k2 );

                rips::CoarseningInfo* coarseningInfo = reply->add_data();

                rips::Vec3i* min = new rips::Vec3i;
                min->set_i( (int)i1 );
                min->set_j( (int)j1 );
                min->set_k( (int)k1 );
                coarseningInfo->set_allocated_min( min );

                rips::Vec3i* max = new rips::Vec3i;
                max->set_i( (int)i2 );
                max->set_j( (int)j2 );
                max->set_k( (int)k2 );
                coarseningInfo->set_allocated_max( max );
            }
        }
        return Status::OK;
    }
    return Status( grpc::NOT_FOUND, "Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcCaseService::createCallbacks()
{
    typedef RiaGrpcCaseService Self;

    return { new RiaGrpcUnaryCallback<Self, CaseRequest, GridCount>( this, &Self::GetGridCount, &Self::RequestGetGridCount ),
             new RiaGrpcUnaryCallback<Self, CellInfoRequest, CellCount>( this, &Self::GetCellCount, &Self::RequestGetCellCount ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, TimeStepDates>( this, &Self::GetTimeSteps, &Self::RequestGetTimeSteps ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, DaysSinceStart>( this,
                                                                          &Self::GetDaysSinceStart,
                                                                          &Self::RequestGetDaysSinceStart ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, CaseInfo>( this, &Self::GetCaseInfo, &Self::RequestGetCaseInfo ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, PdmObject>( this, &Self::GetPdmObject, &Self::RequestGetPdmObject ),
             new RiaGrpcServerToClientStreamCallback<Self,
                                                     CellInfoRequest,
                                                     CellInfoArray,
                                                     RiaActiveCellInfoStateHandler>( this,
                                                                                     &Self::GetCellInfoForActiveCells,
                                                                                     &Self::RequestGetCellInfoForActiveCells,
                                                                                     new RiaActiveCellInfoStateHandler ),
             new RiaGrpcServerToClientStreamCallback<Self,
                                                     CellInfoRequest,
                                                     CellCenters,
                                                     RiaActiveCellInfoStateHandler>( this,
                                                                                     &Self::GetCellCenterForActiveCells,
                                                                                     &Self::RequestGetCellCenterForActiveCells,
                                                                                     new RiaActiveCellInfoStateHandler ),
             new RiaGrpcServerToClientStreamCallback<Self,
                                                     CellInfoRequest,
                                                     CellCornersArray,
                                                     RiaActiveCellInfoStateHandler>( this,
                                                                                     &Self::GetCellCornersForActiveCells,
                                                                                     &Self::RequestGetCellCornersForActiveCells,
                                                                                     new RiaActiveCellInfoStateHandler ),
             new RiaGrpcServerToClientStreamCallback<Self,
                                                     CaseRequest,
                                                     SelectedCells,
                                                     RiaSelectedCellsStateHandler>( this,
                                                                                    &Self::GetSelectedCells,
                                                                                    &Self::RequestGetSelectedCells,
                                                                                    new RiaSelectedCellsStateHandler ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, BoundingBox>( this,
                                                                       &Self::GetReservoirBoundingBox,
                                                                       &Self::RequestGetReservoirBoundingBox ),

             new RiaGrpcUnaryCallback<Self, CaseRequest, CoarseningInfoArray>( this,
                                                                               &Self::GetCoarseningInfoArray,
                                                                               &Self::RequestGetCoarseningInfoArray ) };
}

static bool RiaGrpcCaseService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcCaseService>( typeid( RiaGrpcCaseService ).hash_code() );
