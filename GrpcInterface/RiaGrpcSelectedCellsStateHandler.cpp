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
#include "RiaGrpcSelectedCellsStateHandler.h"

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
RiaGrpcSelectedCellsStateHandler::RiaGrpcSelectedCellsStateHandler()
    : m_request( nullptr )
    , m_eclipseCase( nullptr )
    , m_currentItem( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGrpcSelectedCellsStateHandler::init( const rips::CaseRequest* request )
{
    CAF_ASSERT( request );
    m_request = request;

    RimCase* rimCase = RiaGrpcHelper::findCase( m_request->id() );
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
Status RiaGrpcSelectedCellsStateHandler::assignNextSelectedCell( rips::SelectedCell*                          cell,
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
void RiaGrpcSelectedCellsStateHandler::assignSelectedCell( rips::SelectedCell* cell, const RiuEclipseSelectionItem* item )
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
grpc::Status RiaGrpcSelectedCellsStateHandler::assignReply( rips::SelectedCells* reply )
{
    std::vector<RiuSelectionItem*> items;
    Riu3dSelectionManager::instance()->selectedItems( items );

    // Only eclipse cases are currently supported. Also filter by case.
    std::vector<RiuEclipseSelectionItem*> eclipseItems;
    for ( auto item : items )
    {
        RiuEclipseSelectionItem* eclipseItem = dynamic_cast<RiuEclipseSelectionItem*>( item );
        if ( eclipseItem && eclipseItem->m_resultDefinition->eclipseCase()->caseId() == m_request->id() )
        {
            eclipseItems.push_back( eclipseItem );
        }
    }

    const size_t packageSize    = RiaGrpcHelper::numberOfDataUnitsInPackage( sizeof( rips::SelectedCell ) );
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
