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
#include "RigMainGrid.h"
#include "RigActiveCellInfo.h"
#include "RigFlowDiagResults.h"

//==================================================================================================
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RigEclCellIndexCalculator::resultCellIndex(size_t gridIndex, size_t gridCellIndex) const
{
    const RigGridBase* grid = m_mainGrid->gridByIndex(gridIndex);
    size_t reservoirCellIndex = grid->reservoirCellIndex(gridCellIndex);

    return m_activeCellInfo->cellResultIndex(reservoirCellIndex);
}


//==================================================================================================
/// 
/// 
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigAccWellFlowCalculator::RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                                                   const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds,
                                                   const std::map<QString, const std::vector<double>* >& tracerCellFractionValues,
                                                   const RigEclCellIndexCalculator cellIndexCalculator,
                                                   double smallContribThreshold,
                                                   bool isProducer)
                                                 : m_pipeBranchesCLCoords(pipeBranchesCLCoords),
                                                   m_pipeBranchesCellIds(pipeBranchesCellIds),
                                                   m_tracerCellFractionValues(&tracerCellFractionValues),
                                                   m_cellIndexCalculator(cellIndexCalculator),
                                                   m_smallContributionsThreshold(smallContribThreshold)
{
    m_accConnectionFlowPrBranch.resize(m_pipeBranchesCellIds.size());

    if (isWellFlowConsistent(isProducer))
    {

        for ( const auto& it: (*m_tracerCellFractionValues) ) m_tracerNames.push_back(it.first);

        m_tracerNames.push_back(RIG_RESERVOIR_TRACER_NAME);

        calculateAccumulatedFlowPrConnection(0, 1);
        sortTracers();
        groupSmallContributions();
    }
    else
    {
        m_tracerCellFractionValues = nullptr;
        m_cellIndexCalculator = RigEclCellIndexCalculator(nullptr, nullptr);
        m_tracerNames.push_back(RIG_FLOW_TOTAL_NAME);

        calculateAccumulatedFlowPrConnection(0, 1);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigAccWellFlowCalculator::RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords, 
                                                   const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds,
                                                   double smallContribThreshold)
                                                 : m_pipeBranchesCLCoords(pipeBranchesCLCoords),
                                                   m_pipeBranchesCellIds(pipeBranchesCellIds),
                                                   m_tracerCellFractionValues(nullptr),
                                                   m_cellIndexCalculator(RigEclCellIndexCalculator(nullptr, nullptr)),
                                                   m_smallContributionsThreshold(smallContribThreshold)
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
    CVF_ASSERT(m_accConnectionFlowPrBranch[branchIdx].accFlowPrTracer.find(RIG_FLOW_TOTAL_NAME) != m_accConnectionFlowPrBranch[branchIdx].accFlowPrTracer.end());

    return m_accConnectionFlowPrBranch[branchIdx].accFlowPrTracer[RIG_FLOW_TOTAL_NAME];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::accumulatedTracerFlowPrConnection(const QString& tracerName, size_t branchIdx) 
{
    CVF_ASSERT(m_accConnectionFlowPrBranch[branchIdx].accFlowPrTracer.find(tracerName) != m_accConnectionFlowPrBranch[branchIdx].accFlowPrTracer.end());

    return m_accConnectionFlowPrBranch[branchIdx].accFlowPrTracer[tracerName];
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::connectionNumbersFromTop(size_t branchIdx) const
{
    return m_accConnectionFlowPrBranch[branchIdx].depthValuesFromTop;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, double> > RigAccWellFlowCalculator::totalWellFlowPrTracer() 
{
    std::vector<QString> tracerNames =  this->tracerNames();
    std::vector<std::pair<QString, double> > tracerWithValues;

    for (const QString& tracerName: tracerNames)
    {
        const std::vector<double>& accFlow = this->accumulatedTracerFlowPrConnection(tracerName, 0);
        tracerWithValues.push_back(std::make_pair(tracerName, accFlow.back()));
    }

    return tracerWithValues;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, double> > RigAccWellFlowCalculator::totalTracerFractions() 
{
    std::vector<std::pair<QString, double> > totalFlows = totalWellFlowPrTracer();

    float sumTracerFlows = 0.0f;
    for ( const auto& tracerVal : totalFlows)
    {
        sumTracerFlows += tracerVal.second;
    }

    if (sumTracerFlows == 0.0) totalFlows.clear();

    for (auto& tracerPair : totalFlows)
    {
        tracerPair.second = tracerPair.second/sumTracerFlows;
    }

    return totalFlows;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool  RigAccWellFlowCalculator::isWellFlowConsistent( bool isProducer)
{
    bool isConsistent = true;
    for (const std::vector <RigWellResultPoint> & branch :  m_pipeBranchesCellIds)
    {
        for (const RigWellResultPoint& wrp : branch)
        {
            if (isProducer)
                isConsistent = (wrp.flowRate() >= 0.0) ;
            else
                isConsistent = (wrp.flowRate() <= 0.0) ;

            if (!isConsistent) break;
        }        
        if (!isConsistent) break;
    }
    return isConsistent;
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

    std::map<QString, std::vector<double> >& accConnFlowFractionsPrTracer = m_accConnectionFlowPrBranch[branchIdx].accFlowPrTracer;
    std::vector<double>&                     connNumbersFromTop           = m_accConnectionFlowPrBranch[branchIdx].depthValuesFromTop;

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
            if ( dsBidx != branchIdx &&  m_accConnectionFlowPrBranch[dsBidx].depthValuesFromTop.size() == 0 ) // Not this branch or already calculated
            {
                calculateAccumulatedFlowPrConnection(dsBidx, connNumFromTop);
                BranchFlow& accConnFlowFractionsDsBranch = m_accConnectionFlowPrBranch[dsBidx];

                size_t tracerIdx = 0;
                for ( const auto & tracerName: m_tracerNames )
                {
                    accFlow[tracerIdx] +=  accConnFlowFractionsDsBranch.accFlowPrTracer[tracerName].back();
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------
/// Concatenate small tracers into an "Other" group
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::groupSmallContributions()
{

    if ( ! (m_smallContributionsThreshold > 0.0) ) return;

    // Find the tracers we need to group

    std::vector<QString> tracersToGroup;
    {
        std::vector<std::pair<QString, double> > totalTracerFractions = this->totalTracerFractions();

        if ( totalTracerFractions.size() < 5 ) return; // No grouping for few legend items


        for ( const auto& tracerPair : totalTracerFractions )
        {
            if ( abs(tracerPair.second) <= m_smallContributionsThreshold ) tracersToGroup.push_back(tracerPair.first);
        }
    }

    if ( tracersToGroup.size() < 2 ) return; // Must at least group two ...

    // Concatenate the values for each branch, erasing the tracers being grouped, replaced with the concatenated values

    for ( BranchFlow& brRes : m_accConnectionFlowPrBranch )
    {
        std::vector<double> groupedConnectionValues( brRes.depthValuesFromTop.size(), 0.0);

        for ( const QString& tracername:tracersToGroup )
        {
            auto it = brRes.accFlowPrTracer.find(tracername);

            if ( it != brRes.accFlowPrTracer.end() )
            {
                const std::vector<double>& tracerVals = it->second;
                for ( size_t cIdx = 0; cIdx < groupedConnectionValues.size(); ++cIdx )
                {
                    groupedConnectionValues[cIdx] += tracerVals[cIdx];
                }
            }

            brRes.accFlowPrTracer.erase(it);
        }

        brRes.accFlowPrTracer[RIG_TINY_TRACER_GROUP_NAME] = groupedConnectionValues;
    }

    // Remove the grouped tracer names from the tracerName list, and replace with the "Others" name

    std::vector<QString> filteredTracernames;
    for ( const QString& tracerName: m_tracerNames )
    {
        bool isDeleted = false;
        for ( const QString& deletedTracerName: tracersToGroup )
        {
            if ( tracerName == deletedTracerName ) { isDeleted = true; break; }
        }

        if ( !isDeleted ) filteredTracernames.push_back(tracerName);
    }

    m_tracerNames.swap(filteredTracernames);
    m_tracerNames.push_back(RIG_TINY_TRACER_GROUP_NAME);
    
}

