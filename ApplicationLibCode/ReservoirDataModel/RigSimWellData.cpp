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
#include "RigWellResultFrame.h"
#include "RigWellResultPoint.h"

#include <map>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigSimWellData::RigSimWellData()
    : m_isMultiSegmentWell( false )
{
    m_staticWellCells = std::make_unique<RigWellResultFrame>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellResultFrame* RigSimWellData::wellResultFrame( size_t resultTimeStepIndex ) const
{
    CVF_ASSERT( resultTimeStepIndex < m_resultTimeStepIndexToWellTimeStepIndex.size() );

    size_t wellTimeStepIndex = m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex];
    CVF_ASSERT( wellTimeStepIndex < m_wellCellsTimeSteps.size() );

    return &( m_wellCellsTimeSteps[wellTimeStepIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimWellData::computeMappingFromResultTimeIndicesToWellTimeIndices( const std::vector<QDateTime>& simulationTimeSteps )
{
    m_resultTimeStepIndexToWellTimeStepIndex.clear();
    if ( m_wellCellsTimeSteps.empty() ) return;

    m_resultTimeStepIndexToWellTimeStepIndex.resize( simulationTimeSteps.size(), cvf::UNDEFINED_SIZE_T );

    size_t wellTimeStepIndex = 0;
    for ( size_t resultTimeStepIndex = 0; resultTimeStepIndex < simulationTimeSteps.size(); resultTimeStepIndex++ )
    {
        while ( wellTimeStepIndex < m_wellCellsTimeSteps.size() &&
                m_wellCellsTimeSteps[wellTimeStepIndex].timestamp() < simulationTimeSteps[resultTimeStepIndex] )
        {
            wellTimeStepIndex++;
        }

        if ( ( wellTimeStepIndex < m_wellCellsTimeSteps.size() ) &&
             ( m_wellCellsTimeSteps[wellTimeStepIndex].timestamp() == simulationTimeSteps[resultTimeStepIndex] ) )
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

    if ( wellResultFrame( resultTimeStepIndex )->wellHead().isCell() ) return true;

    const std::vector<RigWellResultBranch> resBranches = wellResultFrame( resultTimeStepIndex )->wellResultBranches();
    for ( const auto& branch : resBranches )
    {
        for ( const auto& branchResPoint : branch.branchResultPoints() )
        {
            if ( branchResPoint.isCell() ) return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool operator==( const RigWellResultPoint& p1, const RigWellResultPoint& p2 )
{
    // TODO : Remove when <=> operator has been added to RigWellResultPoint
    return p1.gridIndex() == p2.gridIndex() && p1.cellIndex() == p2.cellIndex() && p1.branchId() == p2.branchId() &&
           p1.segmentId() == p2.segmentId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigSimWellData::computeStaticWellCellPath() const
{
    if ( m_wellCellsTimeSteps.empty() ) return;

    // Mapping of Branch ERT ID to ResultPoint list
    std::map<int, std::list<RigWellResultPoint>> staticWellBranches;

    // Add ResultCell data from the first timestep to the final result.
    for ( const auto& wellResultBranch : m_wellCellsTimeSteps[0].wellResultBranches() )
    {
        const int                      branchErtId = wellResultBranch.ertBranchId();
        std::list<RigWellResultPoint>& branch      = staticWellBranches[branchErtId];

        for ( const auto& frameCell : wellResultBranch.branchResultPoints() )
        {
            branch.push_back( frameCell );
        }
    }

    bool doSkipTimeStep = true;
    for ( const auto& wellCellsTimeStep : m_wellCellsTimeSteps )
    {
        if ( doSkipTimeStep ) // Skip first
        {
            doSkipTimeStep = false;
            continue;
        }

        // Merge well branches separately
        for ( const auto& wellResultBranch : wellCellsTimeStep.wellResultBranches() )
        {
            const int                             branchId  = wellResultBranch.ertBranchId();
            const std::vector<RigWellResultPoint> resBranch = wellResultBranch.branchResultPoints();

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

    m_staticWellCells->clearWellResultBranches();
    m_staticWellCells->setWellHead( m_wellCellsTimeSteps[0].wellHead() );

    for ( const auto& [ertBranchId, resultPoints] : staticWellBranches )
    {
        // Copy from first time step
        RigWellResultBranch rigBranch;
        rigBranch.setErtBranchId( ertBranchId );

        for ( const auto& resultPoint : resultPoints )
        {
            rigBranch.addBranchResultPoint( resultPoint );
        }

        m_staticWellCells->addWellResultBranch( rigBranch );
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
void RigSimWellData::setWellName( const QString& wellName )
{
    m_wellName = wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellProductionType RigSimWellData::wellProductionType( size_t resultTimeStepIndex ) const
{
    if ( hasWellResult( resultTimeStepIndex ) )
    {
        const RigWellResultFrame* wResFrame = wellResultFrame( resultTimeStepIndex );
        return wResFrame->productionType();
    }
    else
    {
        return RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellResultFrame* RigSimWellData::staticWellResultFrame() const
{
    // Make sure we have computed the static representation of the well
    if ( m_staticWellCells->wellResultBranches().empty() )
    {
        computeStaticWellCellPath();
    }

    return m_staticWellCells.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigSimWellData::isOpen( size_t resultTimeStepIndex ) const
{
    if ( hasWellResult( resultTimeStepIndex ) )
    {
        const RigWellResultFrame* wResFrame = wellResultFrame( resultTimeStepIndex );
        return wResFrame->isOpen();
    }
    else
    {
        return false;
    }
}
