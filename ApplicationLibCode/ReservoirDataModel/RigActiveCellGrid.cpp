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

#include "cvfAssert.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellGrid::RigActiveCellGrid()
{
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
void RigActiveCellGrid::transferActiveInformation( int                     gridIndex,
                                                   RigEclipseCaseData*     eclipseCaseData,
                                                   size_t                  totalActiveCells,
                                                   size_t                  matrixActiveCells,
                                                   size_t                  fractureActiveCells,
                                                   const std::vector<int>& activeMatrixIndexes,
                                                   const std::vector<int>& activeFracIndexes )
{
    if ( gridIndex == 0 )
    {
        m_globalToActiveMap.clear();
    }

    const auto totalCells = activeMatrixIndexes.size();

    const auto cellStartIndex = m_globalToActiveMap.size();

    m_globalToActiveMap.resize( cellStartIndex + totalCells );
    size_t activeCells       = cellStartIndex;
    size_t anInactiveCellIdx = cellStartIndex;

    for ( size_t i = 0; i < totalCells; i++ )
    {
        const auto globalCellIndex = cellStartIndex + i;
        if ( ( activeMatrixIndexes[i] < 0 ) && ( activeFracIndexes[i] < 0 ) )
        {
            m_globalToActiveMap[globalCellIndex] = totalActiveCells;
            anInactiveCellIdx                    = globalCellIndex;
            continue;
        }
        m_activeToGlobalMap.push_back( globalCellIndex );
        m_globalToActiveMap[i] = activeCells++;
    }
    m_activeToGlobalMap.push_back( anInactiveCellIdx );

    RigActiveCellInfo* activeCellInfo         = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    activeCellInfo->setReservoirCellCount( activeCellInfo->reservoirCellCount() + totalActiveCells + 1 );
    fractureActiveCellInfo->setReservoirCellCount( fractureActiveCellInfo->reservoirCellCount() + totalActiveCells + 1 );

    activeCellInfo->setGridCount( gridIndex + 1 );
    fractureActiveCellInfo->setGridCount( gridIndex + 1 );

    activeCellInfo->setGridActiveCellCounts( gridIndex, matrixActiveCells );
    fractureActiveCellInfo->setGridActiveCellCounts( gridIndex, fractureActiveCells );

    // TODO - update indexes here

#pragma omp parallel for
    for ( int opmCellIndex = 0; opmCellIndex < (int)totalCells; opmCellIndex++ )
    {
        auto activeCellIndex = m_globalToActiveMap[cellStartIndex + opmCellIndex];

        // active cell index
        int matrixActiveIndex = activeMatrixIndexes[opmCellIndex];
        if ( matrixActiveIndex != -1 )
        {
            activeCellInfo->setCellResultIndex( activeCellIndex, matrixActiveIndex );
        }

        int fractureActiveIndex = activeFracIndexes[opmCellIndex];
        if ( fractureActiveIndex != -1 )
        {
            fractureActiveCellInfo->setCellResultIndex( activeCellIndex, fractureActiveIndex );
        }
    }

    activeCellInfo->computeDerivedData();
    fractureActiveCellInfo->computeDerivedData();
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
    if ( cellIndex >= m_activeToGlobalMap.size() )
    {
        return false;
    }
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
    CVF_ASSERT( gridLocalCellIndex < m_globalToActiveMap.size() );

    const auto index = m_globalToActiveMap[gridLocalCellIndex];
    return m_cells[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCell& RigActiveCellGrid::cell( size_t gridLocalCellIndex ) const
{
    CVF_ASSERT( gridLocalCellIndex < m_globalToActiveMap.size() );

    const auto index = m_globalToActiveMap[gridLocalCellIndex];
    return m_cells[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCell& RigActiveCellGrid::nativeCell( size_t nativeCellIndex )
{
    CVF_ASSERT( nativeCellIndex < m_cells.size() );

    return m_cells[nativeCellIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCell& RigActiveCellGrid::nativeCell( size_t nativeCellIndex ) const
{
    CVF_ASSERT( nativeCellIndex < m_cells.size() );

    return m_cells[nativeCellIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellGrid::globalCellIndexToNative( size_t globalCellIndex ) const
{
    CVF_ASSERT( globalCellIndex < m_globalToActiveMap.size() );

    return m_globalToActiveMap[globalCellIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellGrid::nativeCellIndexToGlobal( size_t nativeCellIndex ) const
{
    CVF_ASSERT( nativeCellIndex < m_globalToActiveMap.size() );

    return m_activeToGlobalMap[nativeCellIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigActiveCellGrid::cellCount() const
{
    return m_cells.size();
}
