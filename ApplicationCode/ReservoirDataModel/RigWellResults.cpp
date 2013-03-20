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

#include "RigWellResults.h"
#include <map>

#include <QDebug>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigWellResultFrame& RigWellResults::wellResultFrame(size_t resultTimeStepIndex) const
{
    CVF_ASSERT(resultTimeStepIndex < m_resultTimeStepIndexToWellTimeStepIndex.size());

    size_t wellTimeStepIndex = m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex];
    CVF_ASSERT(wellTimeStepIndex < m_wellCellsTimeSteps.size());

    return m_wellCellsTimeSteps[wellTimeStepIndex];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellResults::computeMappingFromResultTimeIndicesToWellTimeIndices(const std::vector<QDateTime>& resultTimes)
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
bool RigWellResults::hasWellResult(size_t resultTimeStepIndex) const
{
    size_t wellTimeStepIndex = m_resultTimeStepIndexToWellTimeStepIndex[resultTimeStepIndex];

    return wellTimeStepIndex != cvf::UNDEFINED_SIZE_T;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigWellResults::firstResultTimeStep() const
{
    size_t i = 0;
    for(i = 0; i < m_resultTimeStepIndexToWellTimeStepIndex.size(); ++i)
    {
        if (m_resultTimeStepIndexToWellTimeStepIndex[i] != cvf::UNDEFINED_SIZE_T) return i;
    }

    return cvf::UNDEFINED_SIZE_T;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigWellResults::computeStaticWellCellPath()
{
    if (m_wellCellsTimeSteps.size() == 0) return;

    std::map < size_t, std::list< RigWellResultCell > > staticWellBranches;

    // Add ResultCell data from the first timestep to the final result.

    for (size_t bIdx = 0; bIdx < m_wellCellsTimeSteps[0].m_wellResultBranches.size(); ++bIdx)
    {
        size_t branchNumber = m_wellCellsTimeSteps[0].m_wellResultBranches[bIdx].m_branchNumber;
        std::vector<RigWellResultCell>& frameCells = m_wellCellsTimeSteps[0].m_wellResultBranches[bIdx].m_wellCells;
       
        std::list< RigWellResultCell >& branch =  staticWellBranches[branchNumber];

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
            size_t branchNumber = m_wellCellsTimeSteps[tIdx].m_wellResultBranches[bIdx].m_branchNumber;
            std::vector<RigWellResultCell>& resBranch = m_wellCellsTimeSteps[tIdx].m_wellResultBranches[bIdx].m_wellCells;

            std::list< RigWellResultCell >& stBranch =  staticWellBranches[branchNumber];
            std::list< RigWellResultCell >::iterator it;
            std::list< RigWellResultCell >::iterator sStartIt;
            std::list< RigWellResultCell >::iterator sEndIt;
            size_t  rStartIdx;
            size_t  rEndIdx;


            size_t sGridIdx;
            size_t sCellIdx;
            size_t rGridIdx; 
            size_t rCellIdx; 

            // First detect if we have cells on the start of the result frame, that is not in the static frame
            {
                sEndIt = stBranch.begin();
                bool found = false;
                if (stBranch.size())
                {
                    sGridIdx = sEndIt->m_gridIndex;
                    sCellIdx = sEndIt->m_gridCellIndex;

                    for (rEndIdx = 0; !found && rEndIdx < resBranch.size(); ++rEndIdx)
                    {
                        rGridIdx = resBranch[rEndIdx].m_gridIndex;
                        rCellIdx = resBranch[rEndIdx].m_gridCellIndex;

                        if (sGridIdx == rGridIdx && sCellIdx == rCellIdx) { found = true; break; }
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
                sGridIdx = sEndIt->m_gridIndex;
                sCellIdx = sEndIt->m_gridCellIndex;

                bool found = false;
                for (rEndIdx += 1; !found && rEndIdx < resBranch.size(); ++rEndIdx)
                {
                    rGridIdx = resBranch[rEndIdx].m_gridIndex;
                    rCellIdx = resBranch[rEndIdx].m_gridCellIndex;

                    if (sGridIdx == rGridIdx && sCellIdx == rCellIdx) { found = true; break; }
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

    std::map < size_t, std::list< RigWellResultCell > >::iterator bIt;

    m_staticWellCells.m_wellResultBranches.clear();
    m_staticWellCells.m_wellHead = m_wellCellsTimeSteps[0].m_wellHead;

    for (bIt = staticWellBranches.begin(); bIt != staticWellBranches.end(); ++bIt)
    {
        RigWellResultBranch rigBranch;
        rigBranch.m_branchNumber = bIt->first;

        std::list< RigWellResultCell >& branch =  bIt->second;
        std::list< RigWellResultCell >::iterator cIt;

        for (cIt = branch.begin(); cIt != branch.end(); ++cIt)
        {
            RigWellResultCell rwc = *cIt;
            rwc.m_isOpen = false; // Reset the dynamic property
            rigBranch.m_wellCells.push_back(*cIt);
        }

        m_staticWellCells.m_wellResultBranches.push_back(rigBranch);
    }
}

