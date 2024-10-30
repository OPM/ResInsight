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
RigWellResultPoint RigWellResultFrame::findResultCellWellHeadIncluded( size_t gridIndex, size_t gridCellIndex ) const
{
    const RigWellResultPoint wellResultPoint = findResultCellWellHeadExcluded( gridIndex, gridCellIndex );
    if ( wellResultPoint.isValid() ) return wellResultPoint;

    // If we could not find the cell among the real connections, we try the wellhead.
    // The wellhead does however not have a real connection state, and is rendering using pipe color
    // https://github.com/OPM/ResInsight/issues/4328

    // This behavior was different prior to release 2019.04 and was rendered as a closed connection (gray)
    // https://github.com/OPM/ResInsight/issues/712

    if ( m_wellHead.cellIndex() == gridCellIndex && m_wellHead.gridIndex() == gridIndex )
    {
        return m_wellHead;
    }

    return RigWellResultPoint();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultPoint RigWellResultFrame::findResultCellWellHeadExcluded( size_t gridIndex, size_t gridCellIndex ) const
{
    CVF_ASSERT( gridIndex != cvf::UNDEFINED_SIZE_T && gridCellIndex != cvf::UNDEFINED_SIZE_T );

    for ( const auto& wellResultBranch : m_wellResultBranches )
    {
        for ( const auto& branchResultPoint : wellResultBranch.branchResultPoints() )
        {
            if ( branchResultPoint.cellIndex() == gridCellIndex && branchResultPoint.gridIndex() == gridIndex )
            {
                return branchResultPoint;
            }
        }
    }
    return RigWellResultPoint();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultFrame::setWellHead( RigWellResultPoint wellHead )
{
    m_wellHead = wellHead;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultPoint RigWellResultFrame::wellHead() const
{
    return m_wellHead;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultPoint RigWellResultFrame::wellHeadOrStartCell() const
{
    if ( m_wellHead.isCell() ) return m_wellHead;

    for ( const RigWellResultBranch& resBranch : m_wellResultBranches )
    {
        for ( const RigWellResultPoint& wrp : resBranch.branchResultPoints() )
        {
            if ( wrp.isCell() ) return wrp;
        }
    }

    return RigWellResultPoint(); // Nothing else matters
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellResultPoint> RigWellResultFrame::allResultPoints() const
{
    std::vector<RigWellResultPoint> allPoints;
    for ( const auto& resultBranch : m_wellResultBranches )
    {
        for ( const auto& resultPoint : resultBranch.branchResultPoints() )
        {
            allPoints.push_back( resultPoint );
        }
    }
    return allPoints;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultFrame::setIsOpen( bool isOpen )
{
    m_isOpen = isOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWellResultFrame::isOpen() const
{
    return m_isOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultFrame::setProductionType( RiaDefines::WellProductionType productionType )
{
    m_productionType = productionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellProductionType RigWellResultFrame::productionType() const
{
    return m_productionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultFrame::setTimestamp( const QDateTime& timeStamp )
{
    m_timestamp = timeStamp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RigWellResultFrame::timestamp() const
{
    return m_timestamp;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellResultBranch> RigWellResultFrame::wellResultBranches() const
{
    return m_wellResultBranches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultFrame::clearWellResultBranches()
{
    m_wellResultBranches.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultFrame::addWellResultBranch( const RigWellResultBranch& wellResultBranch )
{
    m_wellResultBranches.push_back( wellResultBranch );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigWellResultFrame::setWellResultBranches( const std::vector<RigWellResultBranch>& wellResultBranches )
{
    m_wellResultBranches = wellResultBranches;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWellResultPoint> RigWellResultFrame::branchResultPointsFromBranchIndex( size_t index ) const
{
    CVF_ASSERT( index < m_wellResultBranches.size() );
    return m_wellResultBranches[index].branchResultPoints();
}
