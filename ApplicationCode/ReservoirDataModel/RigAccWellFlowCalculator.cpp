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
#include "RigSimulationWellCoordsAndMD.h"

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
    m_connectionFlowPrBranch.resize(m_pipeBranchesCellIds.size());
    m_pseudoLengthFlowPrBranch.resize(m_pipeBranchesCellIds.size());
    m_tvdFlowPrBranch.resize(m_pipeBranchesCellIds.size());

    if (isWellFlowConsistent(isProducer))
    {

        for ( const auto& it: (*m_tracerCellFractionValues) ) m_tracerNames.push_back(it.first);

        m_tracerNames.push_back(RIG_RESERVOIR_TRACER_NAME);

        calculateAccumulatedFlowPrConnection(0, 1);
        calculateFlowPrPseudoLength(0, 0.0);
        sortTracers();
        groupSmallContributions();
    }
    else
    {
        m_tracerCellFractionValues = nullptr;
        m_cellIndexCalculator = RigEclCellIndexCalculator(nullptr, nullptr);
        m_tracerNames.push_back(RIG_FLOW_TOTAL_NAME);

        calculateAccumulatedFlowPrConnection(0, 1);
        calculateFlowPrPseudoLength(0, 0.0);

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
    m_connectionFlowPrBranch.resize(m_pipeBranchesCellIds.size());
    m_pseudoLengthFlowPrBranch.resize(m_pipeBranchesCellIds.size());
    m_tvdFlowPrBranch.resize(m_pipeBranchesCellIds.size());

    m_tracerNames.push_back(RIG_FLOW_TOTAL_NAME);
    
    calculateAccumulatedFlowPrConnection(0, 1);
    calculateFlowPrPseudoLength(0, 0.0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::connectionNumbersFromTop(size_t branchIdx) const
{
    return m_connectionFlowPrBranch[branchIdx].depthValuesFromTop;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::accumulatedFlowPrConnection(size_t branchIdx) const
{
    return accumulatedTracerFlowPrConnection(RIG_FLOW_TOTAL_NAME, branchIdx);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::accumulatedTracerFlowPrConnection(const QString& tracerName, size_t branchIdx) const
{
    auto flowPrTracerIt =  m_connectionFlowPrBranch[branchIdx].accFlowPrTracer.find(tracerName);
    if ( flowPrTracerIt != m_connectionFlowPrBranch[branchIdx].accFlowPrTracer.end())
    {
        return flowPrTracerIt->second;
    }
    else 
    {
        CVF_ASSERT(false);
        static std::vector<double> dummy;
        return dummy;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::flowPrConnection(size_t branchIdx) const
{
    return tracerFlowPrConnection(RIG_FLOW_TOTAL_NAME, branchIdx);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::tracerFlowPrConnection(const QString& tracerName, size_t branchIdx) const
{
    auto flowPrTracerIt =  m_connectionFlowPrBranch[branchIdx].flowPrTracer.find(tracerName);
    if ( flowPrTracerIt != m_connectionFlowPrBranch[branchIdx].flowPrTracer.end())
    {
        return flowPrTracerIt->second;
    }
    else 
    {
        CVF_ASSERT(false);
        static std::vector<double> dummy;
        return dummy;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::pseudoLengthFromTop(size_t branchIdx) const
{
    return m_pseudoLengthFlowPrBranch[branchIdx].depthValuesFromTop;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::accumulatedFlowPrPseudoLength(size_t branchIdx) const
{
    return accumulatedTracerFlowPrPseudoLength(RIG_FLOW_TOTAL_NAME, branchIdx);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::accumulatedTracerFlowPrPseudoLength(const QString& tracerName, size_t branchIdx) const
{
    auto flowPrTracerIt =  m_pseudoLengthFlowPrBranch[branchIdx].accFlowPrTracer.find(tracerName);
    if ( flowPrTracerIt != m_pseudoLengthFlowPrBranch[branchIdx].accFlowPrTracer.end())
    {
        return flowPrTracerIt->second;
    }
    else 
    {
        CVF_ASSERT(false);
        static std::vector<double> dummy;
        return dummy;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::flowPrPseudoLength(size_t branchIdx) const
{
    return tracerFlowPrPseudoLength(RIG_FLOW_TOTAL_NAME, branchIdx);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RigAccWellFlowCalculator::tracerFlowPrPseudoLength(const QString& tracerName, size_t branchIdx) const
{
    auto flowPrTracerIt =  m_pseudoLengthFlowPrBranch[branchIdx].flowPrTracer.find(tracerName);
    if ( flowPrTracerIt != m_pseudoLengthFlowPrBranch[branchIdx].flowPrTracer.end())
    {
        return flowPrTracerIt->second;
    }
    else 
    {
        CVF_ASSERT(false);
        static std::vector<double> dummy;
        return dummy;
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, double> > RigAccWellFlowCalculator::totalWellFlowPrTracer() const 
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
std::vector<std::pair<QString, double> > RigAccWellFlowCalculator::totalTracerFractions() const
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
bool  RigAccWellFlowCalculator::isWellFlowConsistent( bool isProducer) const
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

    std::vector<double> accFlowPrTracer(m_tracerNames.size(), 0.0); 

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

        std::vector<double> flowPrTracer = calculateFlowPrTracer(branchCells, clSegIdx);

        for (size_t tIdx = 0; tIdx < flowPrTracer.size(); ++tIdx)
        {
            accFlowPrTracer[tIdx] += flowPrTracer[tIdx];
        }

        // Add the total accumulated (fraction) flows from any branches connected to this cell

        size_t connNumFromTop = connectionIndexFromTop(resPointToConnectionIndexFromBottom, clSegIdx) + startConnectionNumberFromTop;

        std::vector<size_t> downStreamBranchIndices = findDownStreamBranchIdxs(branchCells[clSegIdx]);
        for ( size_t dsBidx : downStreamBranchIndices )
        {
            BranchFlow &downStreamBranchFlow = m_connectionFlowPrBranch[dsBidx];
            if ( dsBidx != branchIdx &&  downStreamBranchFlow.depthValuesFromTop.size() == 0 ) // Not this branch or already calculated
            {
                calculateAccumulatedFlowPrConnection(dsBidx, connNumFromTop);
                addDownStreamBranchFlow(accFlowPrTracer, downStreamBranchFlow);
            }
        }

        // Push back the accumulated result into the storage

        BranchFlow& branchFlow =  m_connectionFlowPrBranch[branchIdx];

        storeFlowOnDepth(branchFlow, connNumFromTop, accFlowPrTracer, flowPrTracer);

        --clSegIdx;

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::calculateFlowPrPseudoLength(size_t branchIdx, double startPseudoLengthFromTop)
{
    const std::vector<RigWellResultPoint>& branchCells =  m_pipeBranchesCellIds[branchIdx];
    const std::vector <cvf::Vec3d>& branchClPoints = m_pipeBranchesCLCoords[branchIdx];

    RigSimulationWellCoordsAndMD mdCalculator(branchClPoints);

    size_t prevConnIndx = -1;
    int clSegIdx = static_cast<int>(branchCells.size()) - 1;

    std::vector<double> accFlowPrTracer(m_tracerNames.size(), 0.0); 

    BranchFlow& branchFlow =  m_pseudoLengthFlowPrBranch[branchIdx];
    
    RigWellResultPoint previousResultPoint;

    while ( clSegIdx >= 0 )
    {
        int cellBottomPointIndex = -1;
        int cellUpperPointIndex = -1;
        int currentSegmentIndex = -1;

        // Find the complete cell span
        {
            cellBottomPointIndex = clSegIdx + 1;

            previousResultPoint = branchCells[clSegIdx];
            --clSegIdx;
            while ( clSegIdx >= 0  && previousResultPoint.isEqual(branchCells[clSegIdx]) ) { --clSegIdx; }

            cellUpperPointIndex = clSegIdx + 1;
            currentSegmentIndex = cellUpperPointIndex;
        }

        std::vector<double> flowPrTracer = calculateFlowPrTracer(branchCells, currentSegmentIndex);

        double pseudoLengthFromTop_lower = mdCalculator.measuredDepths()[cellBottomPointIndex] + startPseudoLengthFromTop;

        // Push back the new start-of-cell flow, with the previously accumulated result into the storage

        storeFlowOnDepth(branchFlow, pseudoLengthFromTop_lower, accFlowPrTracer, flowPrTracer);

        // Accumulate the connection-cell's fraction flows 

        for (size_t tIdx = 0; tIdx < flowPrTracer.size(); ++tIdx)
        {
            accFlowPrTracer[tIdx] += flowPrTracer[tIdx];
        }

        double pseudoLengthFromTop_upper = mdCalculator.measuredDepths()[cellUpperPointIndex] + startPseudoLengthFromTop;

        // Push back the accumulated result into the storage

        storeFlowOnDepth(branchFlow, pseudoLengthFromTop_upper, accFlowPrTracer, flowPrTracer);

        // Add the total accumulated (fraction) flows from any branches connected to this cell

        std::vector<size_t> downStreamBranchIndices = findDownStreamBranchIdxs(branchCells[cellUpperPointIndex]);
        for ( size_t dsBidx : downStreamBranchIndices )
        {
            BranchFlow &downStreamBranchFlow = m_pseudoLengthFlowPrBranch[dsBidx];
            if ( dsBidx != branchIdx &&  downStreamBranchFlow.depthValuesFromTop.size() == 0 ) // Not this branch or already calculated
            {
                calculateFlowPrPseudoLength(dsBidx, pseudoLengthFromTop_upper);
                addDownStreamBranchFlow(accFlowPrTracer, downStreamBranchFlow);
            }
        }

        // Push back the accumulated result after adding the branch result into the storage

        if (downStreamBranchIndices.size()) storeFlowOnDepth(branchFlow, pseudoLengthFromTop_upper, accFlowPrTracer, flowPrTracer);

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::addDownStreamBranchFlow(std::vector<double> &accFlowPrTracer, const BranchFlow &downStreamBranchFlow) const
{
    size_t tracerIdx = 0;
    for ( const auto & tracerName: m_tracerNames )
    {
        auto tracerFlowPair = downStreamBranchFlow.accFlowPrTracer.find(tracerName);
        if ( tracerFlowPair != downStreamBranchFlow.accFlowPrTracer.end() )
        {
            accFlowPrTracer[tracerIdx] +=  tracerFlowPair->second.back(); // The topmost accumulated value in the branch
        }
        tracerIdx++;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::storeFlowOnDepth(BranchFlow &branchFlow, double depthValue, const std::vector<double>& accFlowPrTracer, const std::vector<double>& flowPrTracer)
{
    size_t tracerIdx = 0;
    for ( const auto & tracerName: m_tracerNames )
    {
        branchFlow.accFlowPrTracer[tracerName].push_back(accFlowPrTracer[tracerIdx]);
        branchFlow.flowPrTracer[tracerName].push_back(flowPrTracer[tracerIdx]);
        tracerIdx++;
    }

    branchFlow.depthValuesFromTop.push_back(depthValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RigAccWellFlowCalculator::calculateFlowPrTracer(const std::vector<RigWellResultPoint>& branchCells, 
                                                                    int clSegIdx) const
{
    std::vector<double> flowPrTracer(m_tracerNames.size(), 0.0);
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
                if ( cellTracerFraction != HUGE_VAL && cellTracerFraction == cellTracerFraction )
                {
                    double tracerFlow = cellTracerFraction * branchCells[clSegIdx].flowRate();
                    flowPrTracer[tracerIdx]     = tracerFlow;

                    totalTracerFractionInCell += cellTracerFraction;
                }
                tracerIdx++;
            }

            double reservoirFraction    = 1.0 - totalTracerFractionInCell;
            double reservoirTracerFlow  = reservoirFraction * branchCells[clSegIdx].flowRate();
            flowPrTracer[tracerIdx]     = reservoirTracerFlow;
        }
    }
    else
    {
        flowPrTracer[0] = branchCells[clSegIdx].flowRate();
    }

    return flowPrTracer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigAccWellFlowCalculator::wrpToConnectionIndexFromBottom(const std::vector<RigWellResultPoint> &branchCells) const
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
std::vector<size_t> RigAccWellFlowCalculator::findDownStreamBranchIdxs(const RigWellResultPoint& connectionPoint) const
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

    for ( BranchFlow& brRes : m_connectionFlowPrBranch )
    {
        groupSmallTracers( brRes.accFlowPrTracer, tracersToGroup);
        groupSmallTracers( brRes.flowPrTracer, tracersToGroup);
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::groupSmallTracers(std::map<QString, std::vector<double>> &branchFlowSet, std::vector<QString> tracersToGroup)
{
    if ( branchFlowSet.empty() ) return;

    size_t depthCount = branchFlowSet.begin()->second.size();
    std::vector<double> groupedAccFlowValues(depthCount, 0.0);

    for ( const QString& tracername:tracersToGroup )
    {
        auto it = branchFlowSet.find(tracername);

        if ( it != branchFlowSet.end() )
        {
            const std::vector<double>& tracerVals = it->second;
            for ( size_t cIdx = 0; cIdx < groupedAccFlowValues.size(); ++cIdx )
            {
                groupedAccFlowValues[cIdx] += tracerVals[cIdx];
            }
        }

        branchFlowSet.erase(it);
    }

    branchFlowSet[RIG_TINY_TRACER_GROUP_NAME] = groupedAccFlowValues;
}

