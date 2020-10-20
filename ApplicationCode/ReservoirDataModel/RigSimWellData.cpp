/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RigSimWellData.h"

#include <map>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSimWellData::RigSimWellData()
    : m_isMultiSegmentWell( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellResultFrame& RigSimWellData::wellResultFrame( size_t resultTimeStepIndex ) const
{
    CVF_ASSERT( resultTimeStepIndex < m_resultTimeStepIndexToWellTimeStepIndex.size() );

    size_t wellTimeStepIndex = m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex];
    CVF_ASSERT( wellTimeStepIndex < m_wellCellsTimeSteps.size() );

    return m_wellCellsTimeSteps[wellTimeStepIndex];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimWellData::computeMappingFromResultTimeIndicesToWellTimeIndices( const std::vector<QDateTime>& simulationTimeSteps )
{
    m_resultTimeStepIndexToWellTimeStepIndex.clear();
    if ( m_wellCellsTimeSteps.size() == 0 ) return;

    m_resultTimeStepIndexToWellTimeStepIndex.resize( simulationTimeSteps.size(), cvf::UNDEFINED_SIZE_T );

    if ( false )
    {
        qDebug() << "Well TimeStamps";
        for ( size_t i = 0; i < m_wellCellsTimeSteps.size(); i++ )
        {
            qDebug() << m_wellCellsTimeSteps[i].m_timestamp.toString();
        }

        qDebug() << "Result TimeStamps";
        for ( size_t i = 0; i < simulationTimeSteps.size(); i++ )
        {
            qDebug() << simulationTimeSteps[i].toString();
        }
    }

    size_t wellTimeStepIndex = 0;
    for ( size_t resultTimeStepIndex = 0; resultTimeStepIndex < simulationTimeSteps.size(); resultTimeStepIndex++ )
    {
        while ( wellTimeStepIndex < m_wellCellsTimeSteps.size() &&
                m_wellCellsTimeSteps[wellTimeStepIndex].m_timestamp < simulationTimeSteps[resultTimeStepIndex] )
        {
            wellTimeStepIndex++;
        }

        if ( wellTimeStepIndex < m_wellCellsTimeSteps.size() &&
             m_wellCellsTimeSteps[wellTimeStepIndex].m_timestamp == simulationTimeSteps[resultTimeStepIndex] )
        {
            m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex] = wellTimeStepIndex;
        }
        else
        {
            m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex] = cvf::UNDEFINED_SIZE_T;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSimWellData::hasWellResult( size_t resultTimeStepIndex ) const
{
    if ( resultTimeStepIndex >= m_resultTimeStepIndexToWellTimeStepIndex.size() )
    {
        return false;
    }

    size_t wellTimeStepIndex = m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex];

    return wellTimeStepIndex != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSimWellData::hasAnyValidCells( size_t resultTimeStepIndex ) const
{
    if ( resultTimeStepIndex >= m_resultTimeStepIndexToWellTimeStepIndex.size() )
    {
        return false;
    }

    size_t wellTimeStepIndex = m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex];

    if ( wellTimeStepIndex == cvf::UNDEFINED_SIZE_T ) return false;

    if ( wellResultFrame( resultTimeStepIndex ).m_wellHead.isCell() ) return true;

    const std::vector<RigWellResultBranch>& resBranches = wellResultFrame( resultTimeStepIndex ).m_wellResultBranches;

    for ( size_t i = 0; i < resBranches.size(); ++i )
    {
        for ( size_t cIdx = 0; cIdx < resBranches[i].m_branchResultPoints.size(); ++cIdx )
        {
            if ( resBranches[i].m_branchResultPoints[cIdx].isCell() ) return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------

bool operator==( const RigWellResultPoint& p1, const RigWellResultPoint& p2 )
{
    return p1.m_gridIndex == p2.m_gridIndex && p1.m_gridCellIndex == p2.m_gridCellIndex &&
           p1.m_ertBranchId == p2.m_ertBranchId && p1.m_ertSegmentId == p2.m_ertSegmentId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimWellData::computeStaticWellCellPath() const
{
    if ( m_wellCellsTimeSteps.size() == 0 ) return;

    // Mapping of Branch ERT ID to ResultPoint list
    std::map<int, std::list<RigWellResultPoint>> staticWellBranches;

    // Add ResultCell data from the first timestep to the final result.

    for ( size_t bIdx = 0; bIdx < m_wellCellsTimeSteps[0].m_wellResultBranches.size(); ++bIdx )
    {
        int branchErtId = m_wellCellsTimeSteps[0].m_wellResultBranches[bIdx].m_ertBranchId;
        const std::vector<RigWellResultPoint>& frameCells =
            m_wellCellsTimeSteps[0].m_wellResultBranches[bIdx].m_branchResultPoints;

        std::list<RigWellResultPoint>& branch = staticWellBranches[branchErtId];

        for ( size_t cIdx = 0; cIdx < frameCells.size(); ++cIdx )
        {
            branch.push_back( frameCells[cIdx] );
        }
    }

    for ( size_t tIdx = 1; tIdx < m_wellCellsTimeSteps.size(); ++tIdx )
    {
        // Merge well branches separately

        for ( size_t bIdx = 0; bIdx < m_wellCellsTimeSteps[tIdx].m_wellResultBranches.size(); ++bIdx )
        {
            int branchId = m_wellCellsTimeSteps[tIdx].m_wellResultBranches[bIdx].m_ertBranchId;
            const std::vector<RigWellResultPoint>& resBranch =
                m_wellCellsTimeSteps[tIdx].m_wellResultBranches[bIdx].m_branchResultPoints;

            std::list<RigWellResultPoint>&          stBranch = staticWellBranches[branchId];
            std::list<RigWellResultPoint>::iterator sEndIt;
            size_t                                  rStartIdx = -1;
            size_t                                  rEndIdx   = -1;

            // First detect if we have cells on the start of the result frame, that is not in the static frame
            {
                sEndIt     = stBranch.begin();
                bool found = false;
                if ( !stBranch.empty() )
                {
                    for ( rEndIdx = 0; !found && rEndIdx < resBranch.size(); ++rEndIdx )
                    {
                        if ( ( *sEndIt ) == ( resBranch[rEndIdx] ) )
                        {
                            found = true;
                            break;
                        }
                    }
                }

                if ( found )
                {
                    if ( rEndIdx > 0 )
                    {
                        // Found cells in start, merge them in
                        for ( size_t cIdx = 0; cIdx < rEndIdx; ++cIdx )
                        {
                            stBranch.insert( sEndIt, resBranch[cIdx] );
                        }
                    }
                }
                else
                {
                    // The result probably starts later in the well
                    rEndIdx = 0;
                }

                rStartIdx = rEndIdx;
            }

            // Now find all result cells in ranges between pairs in the static path
            // If the result has items that "compete" with those in the static path,
            // those items are inserted after the ones in the static path. This
            // is not necessarily correct. They could be in front, and also merged in
            // strange ways. A geometric test could make this more robust, but we will
            // not solve before we see that it actually ends up as a problem

            if ( sEndIt != stBranch.end() ) ++sEndIt;
            for ( ; sEndIt != stBranch.end(); ++sEndIt )
            {
                bool found = false;
                for ( rEndIdx += 1; !found && rEndIdx < resBranch.size(); ++rEndIdx )
                {
                    if ( ( *sEndIt ) == ( resBranch[rEndIdx] ) )
                    {
                        found = true;
                        break;
                    }
                }

                if ( found )
                {
                    if ( rEndIdx - rStartIdx > 1 )
                    {
                        // Found cell range in result that we do not have in the static result, merge them in
                        for ( size_t cIdx = rStartIdx + 1; cIdx < rEndIdx; ++cIdx )
                        {
                            stBranch.insert( sEndIt, resBranch[cIdx] );
                        }
                    }
                }
                else
                {
                    // The static path probably has some extra cells
                    rEndIdx = rStartIdx;
                }

                rStartIdx = rEndIdx;
            }

            // Then add cells from the end of the resultpath not present in the static path
            for ( size_t cIdx = rEndIdx + 1; cIdx < resBranch.size(); ++cIdx )
            {
                stBranch.push_back( resBranch[cIdx] );
            }
        }
    }

    // Populate the static well info

    std::map<int, std::list<RigWellResultPoint>>::iterator bIt;

    m_staticWellCells.m_wellResultBranches.clear();
    m_staticWellCells.m_wellHead = m_wellCellsTimeSteps[0].m_wellHead;

    for ( bIt = staticWellBranches.begin(); bIt != staticWellBranches.end(); ++bIt )
    {
        // Copy from first time step
        RigWellResultBranch rigBranch;
        rigBranch.m_ertBranchId = bIt->first;

        std::list<RigWellResultPoint>&          branch = bIt->second;
        std::list<RigWellResultPoint>::iterator cIt;
        for ( cIt = branch.begin(); cIt != branch.end(); ++cIt )
        {
            rigBranch.m_branchResultPoints.push_back( *cIt );
        }

        m_staticWellCells.m_wellResultBranches.push_back( rigBranch );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimWellData::setMultiSegmentWell( bool isMultiSegmentWell )
{
    m_isMultiSegmentWell = isMultiSegmentWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSimWellData::isMultiSegmentWell() const
{
    return m_isMultiSegmentWell;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultFrame::WellProductionType RigSimWellData::wellProductionType( size_t resultTimeStepIndex ) const
{
    if ( hasWellResult( resultTimeStepIndex ) )
    {
        const RigWellResultFrame& wResFrame = wellResultFrame( resultTimeStepIndex );
        return wResFrame.m_productionType;
    }
    else
    {
        return RigWellResultFrame::UNDEFINED_PRODUCTION_TYPE;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellResultFrame& RigSimWellData::staticWellCells() const
{
    // Make sure we have computed the static representation of the well
    if ( m_staticWellCells.m_wellResultBranches.size() == 0 )
    {
        computeStaticWellCellPath();
    }

    return m_staticWellCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSimWellData::isOpen( size_t resultTimeStepIndex ) const
{
    if ( hasWellResult( resultTimeStepIndex ) )
    {
        const RigWellResultFrame& wResFrame = wellResultFrame( resultTimeStepIndex );
        return wResFrame.m_isOpen;
    }
    else
    {
        return false;
    }
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

    if ( m_wellHead.m_gridCellIndex == gridCellIndex && m_wellHead.m_gridIndex == gridIndex )
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
            if ( m_wellResultBranches[wb].m_branchResultPoints[wc].m_gridCellIndex == gridCellIndex &&
                 m_wellResultBranches[wb].m_branchResultPoints[wc].m_gridIndex == gridIndex )
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
