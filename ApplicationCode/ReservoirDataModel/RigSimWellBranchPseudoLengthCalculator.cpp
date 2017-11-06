/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RigSimWellBranchPseudoLengthCalculator.h"

#include "RigSimulationWellCoordsAndMD.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigSimWellBranchPseudoLengthCalculator::RigSimWellBranchPseudoLengthCalculator(const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds): m_pipeBranchesCLCoords(pipeBranchesCLCoords),
m_pipeBranchesCellIds(pipeBranchesCellIds)
{
    m_pipeBranchesCellMDs.resize(pipeBranchesCLCoords.size());
    calculatePseudoLength(0, 0.0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigSimWellBranchPseudoLengthCalculator::calculatePseudoLength(size_t branchIdx, double startPseudoLengthFromTop)
{
    const std::vector<RigWellResultPoint>& branchCells =  m_pipeBranchesCellIds[branchIdx];
    const std::vector <cvf::Vec3d>& branchClPoints = m_pipeBranchesCLCoords[branchIdx];

    RigSimulationWellCoordsAndMD mdCalculator(branchClPoints);

    // Set the pseudo lengths for this branch
    m_pipeBranchesCellMDs[branchIdx] = mdCalculator.measuredDepths();
    for ( double& depth : m_pipeBranchesCellMDs[branchIdx] )
    {
        depth += startPseudoLengthFromTop;
    }

    // Find the downstream branches, and calculate for them as well, using the correct start offset

    int clSegIdx = static_cast<int>(branchCells.size()) - 1;

    while ( clSegIdx >= 0 )
    {
        RigWellResultPoint  previousResultPoint = branchCells[clSegIdx];
        --clSegIdx;
        while ( clSegIdx >= 0  && previousResultPoint.isEqual(branchCells[clSegIdx]) ) { --clSegIdx; }

        int cellUpperPointIndex = clSegIdx + 1;

        double pseudoLengthFromTop_upper = mdCalculator.measuredDepths()[cellUpperPointIndex] + startPseudoLengthFromTop;

        std::vector<size_t> downStreamBranchIndices = findDownStreamBranchIdxs(branchCells[cellUpperPointIndex]);
        for ( size_t dsBidx : downStreamBranchIndices )
        {
            if ( dsBidx != branchIdx && !m_pipeBranchesCellMDs[dsBidx].size() ) // Not this branch or done before 
            {
                calculatePseudoLength(dsBidx, pseudoLengthFromTop_upper);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigSimWellBranchPseudoLengthCalculator::findDownStreamBranchIdxs(const RigWellResultPoint& connectionPoint)
{
    std::vector<size_t> downStreamBranchIdxs;

    for ( size_t bIdx = 0; bIdx < m_pipeBranchesCellIds.size(); ++bIdx )
    {
        if ( m_pipeBranchesCellIds[bIdx][0].m_gridIndex == connectionPoint.m_gridIndex
            && m_pipeBranchesCellIds[bIdx][0].m_gridCellIndex == connectionPoint.m_gridCellIndex
            && m_pipeBranchesCellIds[bIdx][0].m_ertBranchId == connectionPoint.m_ertBranchId
            && m_pipeBranchesCellIds[bIdx][0].m_ertSegmentId == connectionPoint.m_ertSegmentId )
        {
            downStreamBranchIdxs.push_back(bIdx);
        }
    }
    return downStreamBranchIdxs;
}
