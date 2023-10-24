/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023     Equinor ASA
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

#include "RigFaultReactivationModelGenerator.h"

#include "RiaApplication.h"

#include "RigFault.h"
#include "RigMainGrid.h"

#include "RimCellFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimGridView.h"
#include "RimUserDefinedIndexFilter.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModelGenerator::RigFaultReactivationModelGenerator( cvf::Vec3d                         position,
                                                                        cvf::Vec3d                         normal,
                                                                        size_t                             cellIndex,
                                                                        cvf::StructGridInterface::FaceType face )
    : m_startPosition( position )
    , m_startFace( face )
    , m_normal( normal )
    , m_cellIndex( cellIndex )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFaultReactivationModelGenerator::~RigFaultReactivationModelGenerator()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setFault( const RigFault* fault )
{
    m_fault = fault;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::setGrid( const RigMainGrid* grid )
{
    m_grid = grid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFaultReactivationModelGenerator::generateGeometry()
{
    size_t i, j, k;

    RimEclipseView* view = dynamic_cast<RimEclipseView*>( RiaApplication::instance()->activeGridView() );

    auto cellFilters = view->cellFilterCollection();
    auto eCase       = cellFilters->firstAncestorOfType<RimEclipseCase>();

    std::vector<size_t> cellColumn;

    // get ijk of start cell
    m_grid->ijkFromCellIndexUnguarded( m_cellIndex, &i, &j, &k );

    cellColumn.push_back( m_cellIndex );

    // build column of cells behind fault
    for ( auto kLayer = 0; kLayer < m_grid->cellCountK(); kLayer++ )
    {
        auto cellIdx = m_grid->cellIndexFromIJKUnguarded( i, j, kLayer );

        if ( cellIdx != m_cellIndex ) cellColumn.push_back( cellIdx );
    }

    auto filter = cellFilters->addNewUserDefinedIndexFilter( eCase, cellColumn );
    filter->setName( "Behind fault column" );

    auto   oppositeStartFace  = cvf::StructGridInterface::oppositeFace( m_startFace );
    bool   bFoundOppositeCell = false;
    size_t oppositeCellIdx    = 0;

    for ( auto backCellIdx : cellColumn )
    {
        for ( auto& faultFace : m_fault->faultFaces() )
        {
            if ( ( faultFace.m_nativeFace == m_startFace ) && ( faultFace.m_nativeReservoirCellIndex == backCellIdx ) )
            {
                qDebug() << "Found opposite start cell: " + QString::number( faultFace.m_oppositeReservoirCellIndex );
                bFoundOppositeCell = true;
                oppositeCellIdx    = faultFace.m_oppositeReservoirCellIndex;
                break;
            }
            else if ( ( faultFace.m_nativeFace == oppositeStartFace ) && ( faultFace.m_oppositeReservoirCellIndex == backCellIdx ) )
            {
                qDebug() << "Found backwards opposite start cell: " + QString::number( faultFace.m_nativeReservoirCellIndex );
                bFoundOppositeCell = true;
                oppositeCellIdx    = faultFace.m_nativeReservoirCellIndex;
                break;
            }
        }

        if ( bFoundOppositeCell ) break;
    }

    // todo - build cell column for the k column that oppositeCellIdx sits in.
}
