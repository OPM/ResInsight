/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RigActiveCellGrid.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellGrid::RigActiveCellGrid()
{
    m_invalidCell.setInvalid( true );
    for ( size_t i = 0; i < 8; i++ )
        m_invalidCell.cornerIndices()[i] = 0;

    m_cells.push_back( m_invalidCell );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellGrid::~RigActiveCellGrid()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellGrid::initializeActiveMapIndex( const std::vector<int> activeMatrixIndexes, const std::vector<int> activeFracIndexes )
{
    m_globalToActiveMap.resize( activeMatrixIndexes.size() );
    size_t activeCells = 1; // first cell is our "invalid cell" used for all non-active cells to save space
    m_activeToGlobalMap.push_back( 0 );

    for ( size_t i = 0; i < activeMatrixIndexes.size(); i++ )
    {
        if ( ( activeMatrixIndexes[i] < 0 ) && ( activeFracIndexes[i] < 0 ) )
        {
            m_globalToActiveMap[i] = 0;
            continue;
        }
        m_activeToGlobalMap.push_back( i );
        m_globalToActiveMap[i] = activeCells++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellGrid::cellIndexFromIJK( size_t i, size_t j, size_t k ) const
{
    auto index = RigGridBase::cellIndexFromIJK( i, j, k );
    return m_globalToActiveMap[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellGrid::cellIndexFromIJKUnguarded( size_t i, size_t j, size_t k ) const
{
    auto index = RigGridBase::cellIndexFromIJKUnguarded( i, j, k );
    return m_globalToActiveMap[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigActiveCellGrid::ijkFromCellIndex( size_t cellIndex, size_t* i, size_t* j, size_t* k ) const
{
    auto index = m_activeToGlobalMap[cellIndex];
    return RigGridBase::ijkFromCellIndex( index, i, j, k );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellGrid::ijkFromCellIndexUnguarded( size_t cellIndex, size_t* i, size_t* j, size_t* k ) const
{
    auto index = m_activeToGlobalMap[cellIndex];
    RigGridBase::ijkFromCellIndexUnguarded( index, i, j, k );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCell& RigActiveCellGrid::cell( size_t gridLocalCellIndex )
{
    auto index = m_globalToActiveMap[gridLocalCellIndex];
    return RigGridBase::cell( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCell& RigActiveCellGrid::cell( size_t gridLocalCellIndex ) const
{
    auto index = m_globalToActiveMap[gridLocalCellIndex];
    return RigGridBase::cell( index );
}
