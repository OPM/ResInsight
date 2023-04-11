/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RigWellResultFrame.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultFrame::RigWellResultFrame()
    : m_productionType( RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE )
    , m_isOpen( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellResultPoint* RigWellResultFrame::findResultCellWellHeadIncluded( size_t gridIndex, size_t gridCellIndex ) const
{
    const RigWellResultPoint* wellResultPoint = findResultCellWellHeadExcluded( gridIndex, gridCellIndex );
    if ( wellResultPoint ) return wellResultPoint;

    // If we could not find the cell among the real connections, we try the wellhead.
    // The wellhead does however not have a real connection state, and is rendering using pipe color
    // https://github.com/OPM/ResInsight/issues/4328

    // This behavior was different prior to release 2019.04 and was rendered as a closed connection (gray)
    // https://github.com/OPM/ResInsight/issues/712

    if ( m_wellHead.cellIndex() == gridCellIndex && m_wellHead.gridIndex() == gridIndex )
    {
        return &m_wellHead;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellResultPoint* RigWellResultFrame::findResultCellWellHeadExcluded( size_t gridIndex, size_t gridCellIndex ) const
{
    CVF_ASSERT( gridIndex != cvf::UNDEFINED_SIZE_T && gridCellIndex != cvf::UNDEFINED_SIZE_T );

    for ( size_t wb = 0; wb < m_wellResultBranches.size(); ++wb )
    {
        for ( size_t wc = 0; wc < m_wellResultBranches[wb].m_branchResultPoints.size(); ++wc )
        {
            if ( m_wellResultBranches[wb].m_branchResultPoints[wc].cellIndex() == gridCellIndex &&
                 m_wellResultBranches[wb].m_branchResultPoints[wc].gridIndex() == gridIndex )
            {
                return &( m_wellResultBranches[wb].m_branchResultPoints[wc] );
            }
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultPoint RigWellResultFrame::wellHeadOrStartCell() const
{
    if ( m_wellHead.isCell() ) return m_wellHead;

    for ( const RigWellResultBranch& resBranch : m_wellResultBranches )
    {
        for ( const RigWellResultPoint& wrp : resBranch.m_branchResultPoints )
        {
            if ( wrp.isCell() ) return wrp;
        }
    }

    return m_wellHead; // Nothing else to do
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellResultPoint> RigWellResultFrame::allResultPoints() const
{
    std::vector<RigWellResultPoint> allPoints;
    for ( const auto& resultBranch : m_wellResultBranches )
    {
        for ( const auto& resultPoint : resultBranch.m_branchResultPoints )
        {
            allPoints.push_back( resultPoint );
        }
    }
    return allPoints;
}
