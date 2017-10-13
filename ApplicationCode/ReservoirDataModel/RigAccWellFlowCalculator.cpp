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

#include "RigSimWellData.h"
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

#define USE_WELL_PHASE_RATES

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigAccWellFlowCalculator::RigAccWellFlowCalculator(const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                                                   const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds,
                                                   const std::map<QString, const std::vector<double>* >& tracerCellFractionValues,
                                                   const RigEclCellIndexCalculator& cellIndexCalculator,
                                                   double smallContribThreshold,
                                                   bool isProducer)
                                                 : m_pipeBranchesCLCoords(pipeBranchesCLCoords),
                                                   m_pipeBranchesCellIds(pipeBranchesCellIds),
                                                   m_tracerCellFractionValues(&tracerCellFractionValues),
                                                   m_cellIndexCalculator(cellIndexCalculator),
                                                   m_smallContributionsThreshold(smallContribThreshold),
                                                   m_isProducer(isProducer)
{
    m_connectionFlowPrBranch.resize(m_pipeBranchesCellIds.size());
    m_pseudoLengthFlowPrBranch.resize(m_pipeBranchesCellIds.size());


    for ( const auto& it: (*m_tracerCellFractionValues) ) m_tracerNames.push_back(it.first);

    m_tracerNames.push_back(RIG_RESERVOIR_TRACER_NAME);

    calculateAccumulatedFlowPrConnection(0, 1);
    calculateFlowPrPseudoLength(0, 0.0);
    sortTracers();
    groupSmallContributions();

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
                                                   m_smallContributionsThreshold(smallContribThreshold),
                                                   m_isProducer(true)
{
    m_connectionFlowPrBranch.resize(m_pipeBranchesCellIds.size());
    m_pseudoLengthFlowPrBranch.resize(m_pipeBranchesCellIds.size());

#ifdef USE_WELL_PHASE_RATES
    m_tracerNames.push_back(RIG_FLOW_OIL_NAME);
    m_tracerNames.push_back(RIG_FLOW_GAS_NAME);
    m_tracerNames.push_back(RIG_FLOW_WATER_NAME);
#else
    m_tracerNames.push_back(RIG_FLOW_TOTAL_NAME);
#endif

    calculateAccumulatedFlowPrConnection(0, 1);
    calculateFlowPrPseudoLength(0, 0.0);
#ifdef USE_WELL_PHASE_RATES
    sortTracers();
#endif
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
const std::vector<double>& RigAccWellFlowCalculator::trueVerticalDepth(size_t branchIdx) const
{
    return m_pseudoLengthFlowPrBranch[branchIdx].trueVerticalDepth;
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
bool  RigAccWellFlowCalculator::isWellFlowConsistent() const
{
    bool isConsistent = true;
    for (const std::vector <RigWellResultPoint> & branch :  m_pipeBranchesCellIds)
    {
        for (const RigWellResultPoint& wrp : branch)
        {
            isConsistent = isFlowRateConsistent(wrp.flowRate());

            if (!isConsistent) break;
        }        
        if (!isConsistent) break;
    }
    return isConsistent;
}
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------

std::vector<double> RigAccWellFlowCalculator::calculateAccumulatedFractions(const std::vector<double>& accumulatedFlowPrTracer ) const
{
    double totalFlow = 0.0;
    for ( double tracerFlow: accumulatedFlowPrTracer)
    {
        totalFlow += tracerFlow;
    } 

    std::vector<double> flowFractionsPrTracer(accumulatedFlowPrTracer.size(), 0.0);
    
    if (totalFlow == 0.0  || !isFlowRateConsistent(totalFlow)) // If we have no accumulated flow, we set all the flow associated to the last tracer, which is the reservoir 
    {
        flowFractionsPrTracer.back() = 1.0;
        return flowFractionsPrTracer;
    }

    for ( size_t tIdx = 0; tIdx < accumulatedFlowPrTracer.size(); ++tIdx) 
    {
        double tracerFlow = accumulatedFlowPrTracer[tIdx];
        flowFractionsPrTracer[tIdx] = tracerFlow / totalFlow;
    }

    return flowFractionsPrTracer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigAccWellFlowCalculator::isConnectionFlowConsistent(const RigWellResultPoint &wellCell) const
{
    if (!m_tracerCellFractionValues) return true; // No flow diagnostics. 

    return  isFlowRateConsistent (wellCell.flowRate());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigAccWellFlowCalculator::isFlowRateConsistent(double flowRate) const
{
    if (!m_tracerCellFractionValues) return true; // No flow diagnostics. 

    return  (flowRate >= 0.0 && m_isProducer) ||  (flowRate <= 0.0 && !m_isProducer);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::calculateAccumulatedFlowPrConnection(size_t branchIdx, size_t startConnectionNumberFromTop)
{
    const std::vector<RigWellResultPoint>& branchCells =  m_pipeBranchesCellIds[branchIdx];

    std::vector<size_t> resPointUniqueIndexFromBottom =  wrpToUniqueWrpIndexFromBottom(branchCells);

    size_t prevConnIndx = -1;
    int clSegIdx = static_cast<int>(branchCells.size()) - 1;

    std::vector<double> accFlowPrTracer(m_tracerNames.size(), 0.0); 

    while ( clSegIdx >= 0 )
    {
        // Skip point if referring to the same cell as the previous centerline segment did
        {
            if ( resPointUniqueIndexFromBottom[clSegIdx] == prevConnIndx )
            {
                --clSegIdx;
                continue;
            }

            prevConnIndx = resPointUniqueIndexFromBottom[clSegIdx];
        }

        // Accumulate the connection-cell's fraction flows 

        const RigWellResultPoint& wellCell = branchCells[clSegIdx];

        std::vector<double> flowPrTracer = calculateWellCellFlowPrTracer(wellCell, accFlowPrTracer);

        addDownStreamBranchFlow(&accFlowPrTracer, flowPrTracer);

        if (!isConnectionFlowConsistent(wellCell))
        { 
            // Associate all the flow with the reservoir tracer for inconsistent flow direction
            flowPrTracer = std::vector<double> (flowPrTracer.size(), 0.0 );
            flowPrTracer.back() = wellCell.flowRate();
        }

        // Add the total accumulated (fraction) flows from any branches connected to this cell

        size_t connNumFromTop = connectionIndexFromTop(resPointUniqueIndexFromBottom, clSegIdx) + startConnectionNumberFromTop;

        std::vector<size_t> downStreamBranchIndices = findDownStreamBranchIdxs(branchCells[clSegIdx]);
        for ( size_t dsBidx : downStreamBranchIndices )
        {
            BranchFlow &downStreamBranchFlow = m_connectionFlowPrBranch[dsBidx];
            if ( dsBidx != branchIdx &&  downStreamBranchFlow.depthValuesFromTop.size() == 0 ) // Not this branch or already calculated
            {
                calculateAccumulatedFlowPrConnection(dsBidx, connNumFromTop);
                std::vector<double> accBranchFlowPrTracer = accumulatedDsBranchFlowPrTracer(downStreamBranchFlow);
                addDownStreamBranchFlow(&accFlowPrTracer, accBranchFlowPrTracer);
                if (m_pipeBranchesCellIds[dsBidx].size() <= 3)
                {
                    // Short branch. Will not be visible. Show branch flow as addition to this connections direct flow
                    addDownStreamBranchFlow(&flowPrTracer, accBranchFlowPrTracer);
                }
            }
        }

        // Push back the accumulated result into the storage

        BranchFlow& branchFlow =  m_connectionFlowPrBranch[branchIdx];

        storeFlowOnDepth(&branchFlow, connNumFromTop, accFlowPrTracer, flowPrTracer);

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
        const RigWellResultPoint& wellCell =  branchCells[currentSegmentIndex];
        std::vector<double> flowPrTracerToAccumulate = calculateWellCellFlowPrTracer( wellCell, accFlowPrTracer);

        double pseudoLengthFromTop_lower = mdCalculator.measuredDepths()[cellBottomPointIndex] + startPseudoLengthFromTop;
        double tvd_lower = -mdCalculator.wellPathPoints()[cellBottomPointIndex][2];

        // Push back the new start-of-cell flow, with the previously accumulated result into the storage

        std::vector<double> flowPrTracer;
        if (!isConnectionFlowConsistent(wellCell))
        { 
            // Associate all the flow with the reservoir tracer for inconsistent flow direction
            flowPrTracer = std::vector<double> (flowPrTracerToAccumulate.size(), 0.0 );
            flowPrTracer.back() = wellCell.flowRate();
        }
        else
        {
            flowPrTracer = flowPrTracerToAccumulate;
        }

        storeFlowOnDepthWTvd(&branchFlow, pseudoLengthFromTop_lower, tvd_lower, accFlowPrTracer, flowPrTracer);

        // Accumulate the connection-cell's fraction flows 

        addDownStreamBranchFlow(&accFlowPrTracer, flowPrTracerToAccumulate);

        double pseudoLengthFromTop_upper = mdCalculator.measuredDepths()[cellUpperPointIndex] + startPseudoLengthFromTop;
        double tvd_upper = -mdCalculator.wellPathPoints()[cellUpperPointIndex][2];

        // Push back the accumulated result into the storage

        storeFlowOnDepthWTvd(&branchFlow, pseudoLengthFromTop_upper, tvd_upper, accFlowPrTracer, flowPrTracer);

        // Add the total accumulated (fraction) flows from any branches connected to this cell

        std::vector<size_t> downStreamBranchIndices = findDownStreamBranchIdxs(branchCells[cellUpperPointIndex]);
        for ( size_t dsBidx : downStreamBranchIndices )
        {
            BranchFlow &downStreamBranchFlow = m_pseudoLengthFlowPrBranch[dsBidx];
            if ( dsBidx != branchIdx &&  downStreamBranchFlow.depthValuesFromTop.size() == 0 ) // Not this branch or already calculated
            {
                calculateFlowPrPseudoLength(dsBidx, pseudoLengthFromTop_upper);
                std::vector<double> accBranchFlowPrTracer = accumulatedDsBranchFlowPrTracer(downStreamBranchFlow);
                addDownStreamBranchFlow(&accFlowPrTracer, accBranchFlowPrTracer);
                if (m_pipeBranchesCellIds[dsBidx].size() <= 3)
                {
                    // Short branch. Will not be visible. Show branch flow as addition to this connections direct flow
                    addDownStreamBranchFlow(&flowPrTracer, accBranchFlowPrTracer);
                }
            }
        }

        // Push back the accumulated result after adding the branch result into the storage

        if (downStreamBranchIndices.size()) storeFlowOnDepthWTvd(&branchFlow, pseudoLengthFromTop_upper, tvd_upper, accFlowPrTracer, flowPrTracer);

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::addDownStreamBranchFlow(std::vector<double> *accFlowPrTracer, 
                                                       const std::vector<double>& accBranchFlowPrTracer) const
{

    double totalThisBranchFlow = 0.0;
    for ( double tracerFlow: *accFlowPrTracer)
    {
        totalThisBranchFlow += tracerFlow;
    } 

    double totalDsBranchFlow = 0.0;
    for ( double tracerFlow: accBranchFlowPrTracer)
    {
        totalDsBranchFlow += tracerFlow;
    } 

    bool isAccumulationConsistent = isFlowRateConsistent(totalThisBranchFlow); // If inconsistent, is it always only the Reservoir tracer that has the flow ?
    bool isBranchConsistent = isFlowRateConsistent(totalDsBranchFlow);


    if (isAccumulationConsistent == isBranchConsistent)
    {
        for ( size_t tracerIdx = 0; tracerIdx < (*accFlowPrTracer).size() ; ++tracerIdx )
        {
            (*accFlowPrTracer)[tracerIdx] +=  accBranchFlowPrTracer[tracerIdx];
        }
        return;
    }

    double totalAccFlow = totalThisBranchFlow + totalDsBranchFlow;

    if (!isFlowRateConsistent(totalAccFlow))
    {
        // Reset the accumulated values, as everything must be moved to the "Reservoir" tracer.
        for (double& val : (*accFlowPrTracer) ) val = 0.0; 

        // Put all flow into the Reservoir tracer
        accFlowPrTracer->back() = totalThisBranchFlow + totalDsBranchFlow; 

        return;
    }

    // We will end up with a consistent accumulated flow, and need to keep the accumulated distribution in this branch
    // or to use the ds branch distribution

    std::vector<double> accFractionsPrTracer;

    if ( !isAccumulationConsistent && isBranchConsistent )
    {
        accFractionsPrTracer = calculateAccumulatedFractions(accBranchFlowPrTracer);
    }
    else if ( isAccumulationConsistent && !isBranchConsistent )
    {
        accFractionsPrTracer = calculateAccumulatedFractions(*accFlowPrTracer);
    }

    // Set the accumulated values to the totalFlow times the tracer fraction selected.

    for (size_t tIdx = 0; tIdx < accFlowPrTracer->size(); ++tIdx)
    {
        (*accFlowPrTracer)[tIdx] = accFractionsPrTracer[tIdx] * (totalAccFlow);
    }

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::storeFlowOnDepth(BranchFlow* branchFlow, double depthValue, const std::vector<double>& accFlowPrTracer, const std::vector<double>& flowPrTracer)
{
    size_t tracerIdx = 0;
    for ( const auto & tracerName: m_tracerNames )
    {
        branchFlow->accFlowPrTracer[tracerName].push_back(accFlowPrTracer[tracerIdx]);
        branchFlow->flowPrTracer[tracerName].push_back(flowPrTracer[tracerIdx]);
        tracerIdx++;
    }

    branchFlow->depthValuesFromTop.push_back(depthValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigAccWellFlowCalculator::storeFlowOnDepthWTvd(BranchFlow *branchFlow, double depthValue, double trueVerticalDepth, const std::vector<double>& accFlowPrTracer, const std::vector<double>& flowPrTracer)
{
    size_t tracerIdx = 0;
    for ( const auto & tracerName: m_tracerNames )
    {
        branchFlow->accFlowPrTracer[tracerName].push_back(accFlowPrTracer[tracerIdx]);
        branchFlow->flowPrTracer[tracerName].push_back(flowPrTracer[tracerIdx]);
        tracerIdx++;
    }

    branchFlow->depthValuesFromTop.push_back(depthValue);
    branchFlow->trueVerticalDepth.push_back(trueVerticalDepth);
}

std::vector<double> RigAccWellFlowCalculator::accumulatedDsBranchFlowPrTracer(const BranchFlow &downStreamBranchFlow) const
{
    std::vector<double> accBranchFlowPrTracer(m_tracerNames.size(), 0.0);

    size_t tracerIdx = 0;
    for ( const auto & tracerName: m_tracerNames )
    {
        const auto trNameAccFlowsPair = downStreamBranchFlow.accFlowPrTracer.find(tracerName);
        if ( trNameAccFlowsPair != downStreamBranchFlow.accFlowPrTracer.end())
        {
            accBranchFlowPrTracer[tracerIdx] = trNameAccFlowsPair->second.back();
        }
        tracerIdx++;
    }

    return accBranchFlowPrTracer;
}

//--------------------------------------------------------------------------------------------------
/// Calculate the flow pr tracer. If inconsistent flow, keep the existing fractions constant
//--------------------------------------------------------------------------------------------------
std::vector<double> RigAccWellFlowCalculator::calculateWellCellFlowPrTracer(const RigWellResultPoint& wellCell, 
                                                                            const std::vector<double>& currentAccumulatedFlowPrTracer) const
{
    std::vector<double> flowPrTracer(m_tracerNames.size(), 0.0);

    if ( !isConnectionFlowConsistent(wellCell) )
    {
        double flowRate = wellCell.flowRate();
        flowPrTracer = calculateAccumulatedFractions(currentAccumulatedFlowPrTracer);
        for (double & accFraction: flowPrTracer)
        {
            accFraction *= flowRate;
        }

        return flowPrTracer;
    }
  
    if ( m_tracerCellFractionValues )
    {
        if ( wellCell.isCell() && wellCell.m_isOpen )
        {
            size_t resCellIndex = m_cellIndexCalculator.resultCellIndex(wellCell.m_gridIndex,
                                                                        wellCell.m_gridCellIndex);
            size_t tracerIdx = 0;
            double totalTracerFractionInCell = 0.0;
            for ( const auto & tracerFractionValsPair: (*m_tracerCellFractionValues) )
            {
                const std::vector<double>* fractionVals = tracerFractionValsPair.second ;
                if ( fractionVals )
                {
                    double cellTracerFraction = (*fractionVals)[resCellIndex];
                    if ( cellTracerFraction != HUGE_VAL && cellTracerFraction == cellTracerFraction )
                    {
                        double tracerFlow = cellTracerFraction * wellCell.flowRate();
                        flowPrTracer[tracerIdx]     = tracerFlow;

                        totalTracerFractionInCell += cellTracerFraction;
                    }
                }
                tracerIdx++;
            }

            double reservoirFraction    = 1.0 - totalTracerFractionInCell;
            double reservoirTracerFlow  = reservoirFraction * wellCell.flowRate();
            flowPrTracer[tracerIdx]     = reservoirTracerFlow;
        }
    }
    else
    {
        #ifdef USE_WELL_PHASE_RATES
        flowPrTracer[0] = wellCell.oilRate();
        flowPrTracer[1] = wellCell.gasRate();
        flowPrTracer[2] = wellCell.waterRate();
        #else
        flowPrTracer[0] = wellCell.flowRate();
        #endif
    }
 
    return flowPrTracer;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RigAccWellFlowCalculator::wrpToUniqueWrpIndexFromBottom(const std::vector<RigWellResultPoint> &branchCells) const
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
        
        if (mainBranchAccFlow.size()) totalFlow = - fabs( mainBranchAccFlow.back() ); // Based on size in reverse order (biggest to least)

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
        bool hasConsistentWellFlow = isWellFlowConsistent();

        std::vector<std::pair<QString, double> > totalTracerFractions = this->totalTracerFractions();

        if ( totalTracerFractions.size() < 5 ) return; // No grouping for few legend items


        for ( const auto& tracerPair : totalTracerFractions )
        {
            if ( fabs(tracerPair.second) <= m_smallContributionsThreshold  
                 && (hasConsistentWellFlow || tracerPair.first != RIG_RESERVOIR_TRACER_NAME) ) // Do not group the Reservoir tracer if the well flow is inconsistent, because cross flow is shown as the reservoir fraction
            { 
                tracersToGroup.push_back(tracerPair.first);
            }
        }
    }

    if ( tracersToGroup.size() < 2 ) return; // Must at least group two ...

    // Concatenate the values for each branch, erasing the tracers being grouped, replaced with the concatenated values

    for ( BranchFlow& brRes : m_connectionFlowPrBranch )
    {
        groupSmallTracers( &brRes.accFlowPrTracer, tracersToGroup);
        groupSmallTracers( &brRes.flowPrTracer, tracersToGroup);
    }

    for ( BranchFlow& brRes : m_pseudoLengthFlowPrBranch )
    {
        groupSmallTracers( &brRes.accFlowPrTracer, tracersToGroup);
        groupSmallTracers( &brRes.flowPrTracer, tracersToGroup);
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
void RigAccWellFlowCalculator::groupSmallTracers(std::map<QString, std::vector<double> >* branchFlowSet, const std::vector<QString>& tracersToGroup)
{
    if ( branchFlowSet->empty() ) return;

    size_t depthCount = branchFlowSet->begin()->second.size();
    std::vector<double> groupedAccFlowValues(depthCount, 0.0);

    for ( const QString& tracername:tracersToGroup )
    {
        auto it = branchFlowSet->find(tracername);

        if ( it != branchFlowSet->end() )
        {
            const std::vector<double>& tracerVals = it->second;
            for ( size_t cIdx = 0; cIdx < groupedAccFlowValues.size(); ++cIdx )
            {
                groupedAccFlowValues[cIdx] += tracerVals[cIdx];
            }
        }

        branchFlowSet->erase(it);
    }

    (*branchFlowSet)[RIG_TINY_TRACER_GROUP_NAME] = groupedAccFlowValues;
}

