/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RigActiveCellInfo.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo::RigActiveCellInfo()
    : m_reservoirActiveCellCount( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setReservoirCellCount( size_t reservoirCellCount )
{
    m_reservoirCellToActiveCell.resize( reservoirCellCount, ActiveCellIndex( cvf::UNDEFINED_SIZE_T ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::reservoirCellCount() const
{
    return m_reservoirCellToActiveCell.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigActiveCellInfo::isActive( ReservoirCellIndex reservoirCellIndex ) const
{
    if ( m_reservoirCellToActiveCell.empty() )
    {
        return true;
    }

    CVF_TIGHT_ASSERT( reservoirCellIndex.value() < m_reservoirCellToActiveCell.size() );

    return m_reservoirCellToActiveCell[reservoirCellIndex.value()].value() != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigActiveCellInfo::isActive( size_t reservoirCellIndex ) const
{
    return isActive( ReservoirCellIndex( reservoirCellIndex ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ActiveCellIndex RigActiveCellInfo::cellResultIndex( ReservoirCellIndex reservoirCellIndex ) const
{
    CVF_TIGHT_ASSERT( reservoirCellIndex.value() < m_reservoirCellToActiveCell.size() );

    return m_reservoirCellToActiveCell[reservoirCellIndex.value()];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::cellResultIndex( size_t reservoirCellIndex ) const
{
    return cellResultIndex( ReservoirCellIndex( reservoirCellIndex ) ).value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setCellResultIndex( ReservoirCellIndex reservoirCellIndex, ActiveCellIndex reservoirCellResultIndex )
{
    CVF_TIGHT_ASSERT( reservoirCellResultIndex < m_reservoirCellToActiveCell.size() );

    m_reservoirCellToActiveCell[reservoirCellIndex.value()] = reservoirCellResultIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setCellResultIndex( size_t reservoirCellIndex, size_t reservoirCellResultIndex )
{
    setCellResultIndex( ReservoirCellIndex( reservoirCellIndex ), ActiveCellIndex( reservoirCellResultIndex ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<ReservoirCellIndex> RigActiveCellInfo::activeReservoirCellIndices() const
{
    return m_activeReservoirCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGridCount( size_t gridCount )
{
    m_perGridActiveCellInfo.resize( gridCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGridActiveCellCounts( size_t gridIndex, size_t activeCellCount )
{
    CVF_ASSERT( gridIndex < m_perGridActiveCellInfo.size() );

    m_perGridActiveCellInfo[gridIndex].setActiveCellCount( activeCellCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::computeDerivedData()
{
    m_reservoirActiveCellCount = 0;

    for ( size_t i = 0; i < m_perGridActiveCellInfo.size(); i++ )
    {
        m_reservoirActiveCellCount += m_perGridActiveCellInfo[i].activeCellCount();
    }

    for ( size_t i = 0; i < m_reservoirCellToActiveCell.size(); i++ )
    {
        if ( m_reservoirCellToActiveCell[i].value() != cvf::UNDEFINED_SIZE_T )
        {
            m_activeReservoirCells.push_back( ReservoirCellIndex( i ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::reservoirActiveCellCount() const
{
    return m_reservoirActiveCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setIjkBoundingBox( const RigBoundingBoxIjk<caf::VecIjk0>& boundingBox )
{
    m_ijkBoundingBox = boundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigBoundingBoxIjk<caf::VecIjk0>& RigActiveCellInfo::ijkBoundingBox() const
{
    return m_ijkBoundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::gridActiveCellCounts( size_t gridIndex ) const
{
    if ( gridIndex >= m_perGridActiveCellInfo.size() ) return 0;

    return m_perGridActiveCellInfo[gridIndex].activeCellCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::BoundingBox RigActiveCellInfo::geometryBoundingBox() const
{
    return m_activeCellsBoundingBox;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::setGeometryBoundingBox( cvf::BoundingBox bb )
{
    m_activeCellsBoundingBox = bb;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::clear()
{
    m_perGridActiveCellInfo.clear();
    m_reservoirCellToActiveCell.clear();
    m_reservoirActiveCellCount = 0;
    m_ijkBoundingBox           = RigBoundingBoxIjk<caf::VecIjk0>();
    m_activeCellsBoundingBox.reset();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::addLgr( size_t cellCount )
{
    size_t currentGridCount          = m_perGridActiveCellInfo.size();
    size_t currentActiveCellCount    = reservoirActiveCellCount();
    size_t currentReservoirCellCount = reservoirCellCount();

    setGridCount( currentGridCount + 1 );
    setGridActiveCellCounts( currentGridCount, cellCount );
    setReservoirCellCount( currentReservoirCellCount + cellCount );

    computeDerivedData();

    for ( size_t i = 0; i < cellCount; i++ )
    {
        setCellResultIndex( ReservoirCellIndex( currentReservoirCellCount + i ), ActiveCellIndex( currentActiveCellCount + i ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo::GridActiveCellCounts::GridActiveCellCounts()
    : m_activeCellCount( 0 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellInfo::GridActiveCellCounts::activeCellCount() const
{
    return m_activeCellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigActiveCellInfo::GridActiveCellCounts::setActiveCellCount( size_t activeCellCount )
{
    m_activeCellCount = activeCellCount;
}
