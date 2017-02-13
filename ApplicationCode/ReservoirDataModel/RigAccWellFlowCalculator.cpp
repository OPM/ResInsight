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

#include "RigAccWellFlowCalculator.h"

#include "RigSingleWellResultsData.h"

#define RIG_FLOW_TOTAL_NAME "Total"
#define RIG_RESERVOIR_TRACER_NAME "Reservoir"
#define RIG_TINY_TRACER_GROUP_NAME "Other"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigAccWellFlowCalculator::RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                   const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds, 
                                                   const std::map<QString, const std::vector<double>* >& tracerCellFractionValues, 
                                                   const RigEclCellIndexCalculator cellIndexCalculator):
    m_pipeBranchesCLCoords(pipeBranchesCLCoords),
    m_pipeBranchesCellIds(pipeBranchesCellIds),
    m_tracerCellFractionValues(&tracerCellFractionValues),
    m_cellIndexCalculator(cellIndexCalculator)
{
    m_accConnectionFlowPrBranch.resize(m_pipeBranchesCellIds.size());
    for ( const auto& it: (*m_tracerCellFractionValues) ) m_tracerNames.push_back(it.first);
    
    m_tracerNames.push_back(RIG_RESERVOIR_TRACER_NAME);

    calculateAccumulatedFlowPrConnection(0, 1);
    sortTracers();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigAccWellFlowCalculator::RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds):
    m_pipeBranchesCLCoords(pipeBranchesCLCoords),
    m_pipeBranchesCellIds(pipeBranchesCellIds),
    m_tracerCellFractionValues(nullptr),
    m_cellIndexCalculator(RigEclCellIndexCalculator(nullptr, nullptr))
{
    m_accConnectionFlowPrBranch.resize(m_pipeBranchesCellIds.size());
    m_tracerNames.push_back(RIG_FLOW_TOTAL_NAME);
    calculateAccumulatedFlowPrConnection(0, 1);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::accumulatedTotalFlowPrConnection(size_t branchIdx) 
{
    CVF_ASSERT(m_accConnectionFlowPrBranch[branchIdx].accConnFlowFractionsPrTracer.find(RIG_FLOW_TOTAL_NAME) != m_accConnectionFlowPrBranch[branchIdx].accConnFlowFractionsPrTracer.end());

    return m_accConnectionFlowPrBranch[branchIdx].accConnFlowFractionsPrTracer[RIG_FLOW_TOTAL_NAME];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::accumulatedTracerFlowPrConnection(const QString& tracerName, size_t branchIdx) 
{
    CVF_ASSERT(m_accConnectionFlowPrBranch[branchIdx].accConnFlowFractionsPrTracer.find(tracerName) != m_accConnectionFlowPrBranch[branchIdx].accConnFlowFractionsPrTracer.end());

    return m_accConnectionFlowPrBranch[branchIdx].accConnFlowFractionsPrTracer[tracerName];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigAccWellFlowCalculator::connectionNumbersFromTop(size_t branchIdx) const
{
    return m_accConnectionFlowPrBranch[branchIdx].connectionNumbersFromTop;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::calculateAccumulatedFlowPrConnection(size_t branchIdx, size_t startConnectionNumberFromTop)
{
    const std::vector<RigWellResultPoint>& branchCells =  m_pipeBranchesCellIds[branchIdx];

    std::vector<size_t> resPointToConnectionIndexFromBottom =  wrpToConnectionIndexFromBottom(branchCells);

    size_t prevConnIndx = -1;
    int clSegIdx = static_cast<int>(branchCells.size()) - 1;

    std::map<QString, std::vector<double> >& accConnFlowFractionsPrTracer = m_accConnectionFlowPrBranch[branchIdx].accConnFlowFractionsPrTracer;
    std::vector<size_t>&                     connNumbersFromTop           = m_accConnectionFlowPrBranch[branchIdx].connectionNumbersFromTop;

    std::vector<double> accFlow;
    accFlow.resize(m_tracerNames.size(), 0.0);

    while ( clSegIdx >= 0 )
    {
        // Skip point if referring to the same cell as the previous centerline segment did
        {
            if ( resPointToConnectionIndexFromBottom[clSegIdx] == prevConnIndx )
            {
                --clSegIdx;
                continue;
            }

            prevConnIndx = resPointToConnectionIndexFromBottom[clSegIdx];
        }

        // Accumulate the connection-cell's fraction flows 

        if ( m_tracerCellFractionValues )
        {
            if ( branchCells[clSegIdx].isCell() && branchCells[clSegIdx].m_isOpen )
            {
                size_t resCellIndex = m_cellIndexCalculator.resultCellIndex(branchCells[clSegIdx].m_gridIndex,
                                                                            branchCells[clSegIdx].m_gridCellIndex);
                size_t tracerIdx = 0;
                double totalTracerFractionInCell = 0.0;
                for ( const auto & tracerFractionIt: (*m_tracerCellFractionValues) )
                {
                    double cellTracerFraction = (*tracerFractionIt.second)[resCellIndex];
                    if (cellTracerFraction != HUGE_VAL && cellTracerFraction == cellTracerFraction)
                    {
                        accFlow[tracerIdx] += cellTracerFraction * branchCells[clSegIdx].flowRate();
                        totalTracerFractionInCell += cellTracerFraction;
                    }
                    tracerIdx++;
                }

                double reservoirFraction = 1.0 - totalTracerFractionInCell;
                accFlow[tracerIdx] += reservoirFraction * branchCells[clSegIdx].flowRate();
            }
        }
        else
        {
            accFlow[0] +=  branchCells[clSegIdx].flowRate();
        }

        // Add the total accumulated (fraction) flows from any branches connected to this cell

        size_t connNumFromTop = connectionIndexFromTop(resPointToConnectionIndexFromBottom, clSegIdx) + startConnectionNumberFromTop;

        std::vector<size_t> downstreamBranches = findDownstreamBranchIdxs(branchCells[clSegIdx]);
        for ( size_t dsBidx : downstreamBranches )
        {
            if ( dsBidx != branchIdx &&  m_accConnectionFlowPrBranch[dsBidx].connectionNumbersFromTop.size() == 0 ) // Not this branch or already calculated
            {
                calculateAccumulatedFlowPrConnection(dsBidx, connNumFromTop);
                BranchResult& accConnFlowFractionsDsBranch = m_accConnectionFlowPrBranch[dsBidx];

                size_t tracerIdx = 0;
                for ( const auto & tracerName: m_tracerNames )
                {
                    accFlow[tracerIdx] +=  accConnFlowFractionsDsBranch.accConnFlowFractionsPrTracer[tracerName].back();
                    tracerIdx++;
                }
            }
        }

        // Push back the accumulated result into the storage

        size_t tracerIdx = 0;
        for ( const auto & tracerName: m_tracerNames )
        {
            accConnFlowFractionsPrTracer[tracerName].push_back(accFlow[tracerIdx]);
            tracerIdx++;
        }

        connNumbersFromTop.push_back(connNumFromTop);

        --clSegIdx;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigAccWellFlowCalculator::wrpToConnectionIndexFromBottom(const std::vector<RigWellResultPoint> &branchCells)
{
    std::vector<size_t> resPointToConnectionIndexFromBottom;
    resPointToConnectionIndexFromBottom.resize(branchCells.size(), -1);
 
    size_t connIdxFromBottom = 0;
    int clSegIdx = static_cast<int>(branchCells.size()) - 1;
    
    if (clSegIdx < 0) return resPointToConnectionIndexFromBottom;

    size_t prevGridIdx     =  branchCells[clSegIdx].m_gridIndex;
    size_t prevGridCellIdx = branchCells[clSegIdx].m_gridCellIndex;
    int    prevErtSegId    = branchCells[clSegIdx].m_ertSegmentId;
    int    prevErtBranchId = branchCells[clSegIdx].m_ertBranchId;

    while ( clSegIdx >= 0 )
    {
        if ( branchCells[clSegIdx].isValid()
            && (   branchCells[clSegIdx].m_gridIndex     != prevGridIdx
                || branchCells[clSegIdx].m_gridCellIndex != prevGridCellIdx
                || branchCells[clSegIdx].m_ertSegmentId  != prevErtSegId
                || branchCells[clSegIdx].m_ertBranchId   != prevErtBranchId) )
        {
            ++connIdxFromBottom;

            prevGridIdx     = branchCells[clSegIdx].m_gridIndex ;
            prevGridCellIdx = branchCells[clSegIdx].m_gridCellIndex;
            prevErtSegId    = branchCells[clSegIdx].m_ertSegmentId;
            prevErtBranchId = branchCells[clSegIdx].m_ertBranchId;
        }

        resPointToConnectionIndexFromBottom[clSegIdx] = connIdxFromBottom;

        --clSegIdx;
    }

    return resPointToConnectionIndexFromBottom;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigAccWellFlowCalculator::connectionIndexFromTop(const std::vector<size_t>& resPointToConnectionIndexFromBottom, size_t clSegIdx)
{
    return resPointToConnectionIndexFromBottom.front() - resPointToConnectionIndexFromBottom[clSegIdx];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigAccWellFlowCalculator::findDownstreamBranchIdxs(const RigWellResultPoint& connectionPoint)
{
    std::vector<size_t> downStreamBranchIdxs;

    for ( size_t bIdx = 0; bIdx < m_pipeBranchesCellIds.size(); ++bIdx )
    {
        if (   m_pipeBranchesCellIds[bIdx][0].m_gridIndex == connectionPoint.m_gridIndex
            && m_pipeBranchesCellIds[bIdx][0].m_gridCellIndex == connectionPoint.m_gridCellIndex 
            && m_pipeBranchesCellIds[bIdx][0].m_ertBranchId == connectionPoint.m_ertBranchId
            && m_pipeBranchesCellIds[bIdx][0].m_ertSegmentId == connectionPoint.m_ertSegmentId)
        {
            downStreamBranchIdxs.push_back(bIdx);
        }
    }
    return downStreamBranchIdxs;
}

void RigAccWellFlowCalculator::sortTracers()
{
    std::multimap<double, QString> sortedTracers;
    for (const QString& tracerName:  m_tracerNames)
    {
        const std::vector<double>& mainBranchAccFlow =  accumulatedTracerFlowPrConnection(tracerName, 0);
        double totalFlow = 0.0;
        if (mainBranchAccFlow.size()) totalFlow = - abs( mainBranchAccFlow.back() ); // Based on size in reverse order (biggest to least)
        sortedTracers.insert({totalFlow, tracerName});
    }

    m_tracerNames.clear();
    for (const auto& tracerPair : sortedTracers)
    {
        m_tracerNames.push_back(tracerPair.second);
    }
}