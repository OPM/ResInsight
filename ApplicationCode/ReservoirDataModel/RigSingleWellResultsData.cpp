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

#include "RigSingleWellResultsData.h"
#include <map>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellResultFrame& RigSingleWellResultsData::wellResultFrame(size_t resultTimeStepIndex) const
{
    CVF_ASSERT(resultTimeStepIndex < m_resultTimeStepIndexToWellTimeStepIndex.size());

    size_t wellTimeStepIndex = m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex];
    CVF_ASSERT(wellTimeStepIndex < m_wellCellsTimeSteps.size());

    return m_wellCellsTimeSteps[wellTimeStepIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigSingleWellResultsData::computeMappingFromResultTimeIndicesToWellTimeIndices(const std::vector<QDateTime>& resultTimes)
{
    m_resultTimeStepIndexToWellTimeStepIndex.clear();
    if (m_wellCellsTimeSteps.size() == 0) return;

    m_resultTimeStepIndexToWellTimeStepIndex.resize(resultTimes.size(), cvf::UNDEFINED_SIZE_T);

    if (false)
    {
        qDebug() << "Well TimeStamps";
        for (size_t i = 0; i < m_wellCellsTimeSteps.size(); i++)
        {
            qDebug() << m_wellCellsTimeSteps[i].m_timestamp.toString();

        }

        qDebug() << "Result TimeStamps";
        for (size_t i = 0; i < resultTimes.size(); i++)
        {
            qDebug() << resultTimes[i].toString();
        }

    }

    int resultIdx = 0;
    size_t wellIdx = 0;
    size_t activeWellIdx = cvf::UNDEFINED_SIZE_T;

    while (wellIdx <= m_wellCellsTimeSteps.size() && resultIdx < static_cast<int>(resultTimes.size()))
    {
        if (wellIdx < m_wellCellsTimeSteps.size() && m_wellCellsTimeSteps[wellIdx].m_timestamp <= resultTimes[resultIdx])
        {
            activeWellIdx = wellIdx;
            wellIdx++;
        }

        CVF_ASSERT(resultIdx < static_cast<int>(m_resultTimeStepIndexToWellTimeStepIndex.size()));
        m_resultTimeStepIndexToWellTimeStepIndex[resultIdx] = activeWellIdx;

        resultIdx++;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigSingleWellResultsData::hasWellResult(size_t resultTimeStepIndex) const
{
    size_t wellTimeStepIndex = m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex];

    return wellTimeStepIndex != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigSingleWellResultsData::firstResultTimeStep() const
{
    size_t i = 0;
    for(i = 0; i < m_resultTimeStepIndexToWellTimeStepIndex.size(); ++i)
    {
        if (m_resultTimeStepIndexToWellTimeStepIndex[i] != cvf::UNDEFINED_SIZE_T) return i;
    }

    return cvf::UNDEFINED_SIZE_T;
}

bool operator== (const RigWellResultPoint& p1, const RigWellResultPoint& p2)
{
    return 
        p1.m_gridIndex == p2.m_gridIndex 
        && p1.m_gridCellIndex == p2.m_gridCellIndex
        && p1.m_ertBranchId == p2.m_ertBranchId
        && p1.m_ertSegmentId == p2.m_ertSegmentId;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigSingleWellResultsData::computeStaticWellCellPath()
{
    if (m_wellCellsTimeSteps.size() == 0) return;

    std::map < int, std::list< RigWellResultPoint > > staticWellBranches;

    // Add ResultCell data from the first timestep to the final result.

    for (size_t bIdx = 0; bIdx < m_wellCellsTimeSteps[0].m_wellResultBranches.size(); ++bIdx)
    {
        int branchNumber = m_wellCellsTimeSteps[0].m_wellResultBranches[bIdx].m_ertBranchId;
        std::vector<RigWellResultPoint>& frameCells = m_wellCellsTimeSteps[0].m_wellResultBranches[bIdx].m_branchResultPoints;
       
        std::list< RigWellResultPoint >& branch =  staticWellBranches[branchNumber];

        for(size_t cIdx = 0; cIdx < frameCells.size(); ++cIdx)
        {
            branch.push_back(frameCells[cIdx]);
        }
    }



    for (size_t tIdx = 1; tIdx < m_wellCellsTimeSteps.size(); ++tIdx)
    {
        // Merge well branches separately

        for (size_t bIdx = 0; bIdx < m_wellCellsTimeSteps[tIdx].m_wellResultBranches.size(); ++bIdx)
        {
            int branchId = m_wellCellsTimeSteps[tIdx].m_wellResultBranches[bIdx].m_ertBranchId;
            std::vector<RigWellResultPoint>& resBranch = m_wellCellsTimeSteps[tIdx].m_wellResultBranches[bIdx].m_branchResultPoints;

            std::list< RigWellResultPoint >& stBranch =  staticWellBranches[branchId];
            std::list< RigWellResultPoint >::iterator it;
            std::list< RigWellResultPoint >::iterator sStartIt;
            std::list< RigWellResultPoint >::iterator sEndIt;
            size_t  rStartIdx;
            size_t  rEndIdx;

            // First detect if we have cells on the start of the result frame, that is not in the static frame
            {
                sEndIt = stBranch.begin();
                bool found = false;
                if (stBranch.size())
                {
                    for (rEndIdx = 0; !found && rEndIdx < resBranch.size(); ++rEndIdx)
                    {
                        if ((*sEndIt) == (resBranch[rEndIdx])) { found = true; break; }
                    }
                }

                if (found)
                {
                    if (rEndIdx > 0)
                    {
                        // Found cells in start, merge them in
                        for (size_t cIdx = 0; cIdx < rEndIdx; ++cIdx)
                        {
                            stBranch.insert(sEndIt, resBranch[cIdx]);
                        }
                    }
                }
                else 
                {
                    // The result probably starts later in the well
                    rEndIdx = 0;
                }

                sStartIt = sEndIt;
                rStartIdx = rEndIdx;
            }

            // Now find all result cells in ranges between pairs in the static path
            // If the result has items that "compete" with those in the static path, 
            // those items are inserted after the ones in the static path. This 
            // is not neccesarily correct. They could be in front, and also merged in 
            // strange ways. A geometric test could make this more robust, but we will 
            // not solve before we see that it actually ends up as a problem

            if (sEndIt !=  stBranch.end()) ++sEndIt;
            for ( ; sEndIt != stBranch.end() ; ++sEndIt)
            {
                bool found = false;
                for (rEndIdx += 1; !found && rEndIdx < resBranch.size(); ++rEndIdx)
                {
                    if ((*sEndIt) == (resBranch[rEndIdx])) { found = true; break; }
                }

                if (found)
                 {
                    if (rEndIdx - rStartIdx > 1)
                    {
                        // Found cell range in result that we do not have in the static result, merge them in
                        for (size_t cIdx = rStartIdx + 1; cIdx < rEndIdx; ++cIdx)
                        {
                            stBranch.insert(sEndIt, resBranch[cIdx]);
                        }
                    }
                }
                else
                {
                    // The static path probably has some extra cells
                    rEndIdx = rStartIdx;
                }

                sStartIt = sEndIt;
                rStartIdx = rEndIdx;
            }

            // Then add cells from the end of the resultpath not present in the static path
            for (size_t cIdx = rEndIdx + 1; cIdx < resBranch.size(); ++cIdx)
            {
                stBranch.push_back(resBranch[cIdx]);
            }
        }
    }

    // Populate the static well info 

    std::map < int, std::list< RigWellResultPoint > >::iterator bIt;

    m_staticWellCells.m_wellResultBranches.clear();
    m_staticWellCells.m_wellHead = m_wellCellsTimeSteps[0].m_wellHead;

    for (bIt = staticWellBranches.begin(); bIt != staticWellBranches.end(); ++bIt)
    {
        if (bIt->first >= m_wellCellsTimeSteps[0].m_wellResultBranches.size())
        {
            continue;
        }

        // Copy from first time step
        RigWellResultBranch rigBranch = m_wellCellsTimeSteps[0].m_wellResultBranches[bIt->first];
        rigBranch.m_ertBranchId = bIt->first;

        // Clear well cells, and insert the collection of well cells for the static situation
        rigBranch.m_branchResultPoints.clear();

        std::list< RigWellResultPoint >& branch =  bIt->second;
        std::list< RigWellResultPoint >::iterator cIt;
        for (cIt = branch.begin(); cIt != branch.end(); ++cIt)
        {
            RigWellResultPoint rwc = *cIt;
            rwc.m_isOpen = false; // Reset the dynamic property
            rigBranch.m_branchResultPoints.push_back(*cIt);
        }

        m_staticWellCells.m_wellResultBranches.push_back(rigBranch);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigSingleWellResultsData::setMultiSegmentWell(bool isMultiSegmentWell)
{
    m_isMultiSegmentWell = isMultiSegmentWell;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigSingleWellResultsData::isMultiSegmentWell() const
{
    return m_isMultiSegmentWell;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellResultPoint* RigWellResultFrame::findResultCell(size_t gridIndex, size_t gridCellIndex) const
{
    CVF_ASSERT(gridIndex != cvf::UNDEFINED_SIZE_T && gridCellIndex != cvf::UNDEFINED_SIZE_T);

    if (m_wellHead.m_gridCellIndex == gridCellIndex && m_wellHead.m_gridIndex == gridIndex )
    {
        return &m_wellHead;
    }

    for (size_t wb = 0; wb < m_wellResultBranches.size(); ++wb)
    {
        for (size_t wc = 0; wc < m_wellResultBranches[wb].m_branchResultPoints.size(); ++wc)
        {
            if (   m_wellResultBranches[wb].m_branchResultPoints[wc].m_gridCellIndex == gridCellIndex  
                && m_wellResultBranches[wb].m_branchResultPoints[wc].m_gridIndex == gridIndex  )
            {
                return &(m_wellResultBranches[wb].m_branchResultPoints[wc]);
            }
        }
    }

    return NULL;
}
