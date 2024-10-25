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

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellGrid::RigActiveCellGrid()
    : m_totalCellCount( 0 )
    , m_totalActiveCellCount( 0 )
{
    m_invalidCell.setInvalid( true );
    m_invalidCell.setHostGrid( this );
    m_invalidCell.setSubGrid( nullptr );

    for ( size_t i = 0; i < 8; i++ )
    {
        m_invalidCell.cornerIndices()[i] = 0;
    }
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
RigCell& RigActiveCellGrid::cell( size_t gridLocalCellIndex )
{
    if ( m_nativeCells.contains( gridLocalCellIndex ) ) return m_nativeCells[gridLocalCellIndex];
    return m_invalidCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCell& RigActiveCellGrid::cell( size_t gridLocalCellIndex ) const
{
    if ( m_nativeCells.contains( gridLocalCellIndex ) ) return m_nativeCells.at( gridLocalCellIndex );
    return m_invalidCell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<size_t, RigCell>& RigActiveCellGrid::nativeCells()
{
    return m_nativeCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::map<size_t, RigCell>& RigActiveCellGrid::nativeCells() const
{
    return m_nativeCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellGrid::totalCellCount() const
{
    return m_totalCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellGrid::setTotalCellCount( size_t totalCellCount )
{
    m_totalCellCount = totalCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellGrid::totalActiveCellCount() const
{
    return m_totalActiveCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellGrid::setTotalActiveCellCount( size_t totalActiveCellCount )
{
    m_totalActiveCellCount = totalActiveCellCount;
}
