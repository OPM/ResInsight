/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RigFlowDiagResults.h"
#include "RigFlowDiagSolverInterface.h"
#include "RimFlowDiagSolution.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseCase.h"
#include "RigCaseData.h"
#include "RigFlowDiagStatCalc.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagResults::RigFlowDiagResults(RimFlowDiagSolution* flowSolution, size_t timeStepCount)
 : m_flowDiagSolution(flowSolution)
{

    m_timeStepCount = timeStepCount;
    m_hasAtemptedNativeResults.resize(timeStepCount, false);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagResults::~RigFlowDiagResults()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigFlowDiagResults::resultValues(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex)
{
    CVF_ASSERT(m_timeStepCount != cvf::UNDEFINED_SIZE_T); // Forgotten to call init

    return findOrCalculateResult(resVarAddr, frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigActiveCellInfo * RigFlowDiagResults::activeCellInfo(const RigFlowDiagResultAddress& resVarAddr)
{
    RimEclipseResultCase* eclCase;
    m_flowDiagSolution->firstAncestorOrThisOfType(eclCase);
    
    return eclCase->reservoirData()->activeCellInfo(RifReaderInterface::MATRIX_RESULTS); // Todo: base on resVarAddr member
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<double>* RigFlowDiagResults::findOrCalculateResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex)
{
   
    std::vector<double>* frameData = findScalarResultFrame(resVarAddr, frameIndex);

    if ( frameData ) return frameData;

    frameData = calculateDerivedResult(resVarAddr, frameIndex);

    if ( frameData ) return frameData;

    // We need to access the native data from the opm solver

    if (!m_hasAtemptedNativeResults[frameIndex])
    {
        
        RigFlowDiagTimeStepResult nativeTimestepResults =  solverInterface()->calculate(frameIndex, 
                                                                                        m_flowDiagSolution->allInjectorTracerActiveCellIndices(frameIndex),
                                                                                        m_flowDiagSolution->allProducerTracerActiveCellIndices(frameIndex));

        std::map<RigFlowDiagResultAddress, std::vector<double> >& nativeResults = nativeTimestepResults.nativeResults();

        for ( auto& resIt: nativeResults )
        {
            RigFlowDiagResultFrames*  nativeResFrames = findScalarResult(resIt.first);
            if ( !nativeResFrames ) nativeResFrames = createScalarResult(resIt.first);

            nativeResFrames->frameData(frameIndex).swap(resIt.second);
        }

        m_hasAtemptedNativeResults[frameIndex] = true;
    }

    return findScalarResultFrame(resVarAddr, frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigFlowDiagResults::findScalarResultFrame(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex)
{
    RigFlowDiagResultFrames*  resFrames = findScalarResult  (resVarAddr);

    if ( resFrames )
    {
        std::vector<double>& frame = resFrames->frameData(frameIndex);
        if ( frame.size() ) return(&frame);
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagSolverInterface* RigFlowDiagResults::solverInterface()
{
    RimEclipseResultCase* eclCase;
    m_flowDiagSolution->firstAncestorOrThisOfType(eclCase);

    return eclCase->flowDiagSolverInterface();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagResultFrames* RigFlowDiagResults::createScalarResult(const RigFlowDiagResultAddress& resVarAddr)
{
    cvf::ref<RigFlowDiagResultFrames> newFrameSet = new RigFlowDiagResultFrames(m_timeStepCount);
    m_resultSets[resVarAddr] = newFrameSet;

    return newFrameSet.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagResultFrames* RigFlowDiagResults::findScalarResult(const RigFlowDiagResultAddress& resVarAddr)
{
    decltype(m_resultSets)::iterator it = m_resultSets.find(resVarAddr);

    if ( it == m_resultSets.end() ) return nullptr;

    return it->second.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double>* RigFlowDiagResults::calculateDerivedResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex)
{
    return nullptr; // Todo
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStatisticsDataCache* RigFlowDiagResults::statistics(const RigFlowDiagResultAddress& resVarAddr)
{
    RigStatisticsDataCache* statCache = m_resultStatistics[resVarAddr].p();
    if ( !statCache )
    {
        RigFlowDiagStatCalc* calculator = new RigFlowDiagStatCalc(this, resVarAddr);
        statCache = new RigStatisticsDataCache(calculator);
        m_resultStatistics[resVarAddr] = statCache;
    }

    return statCache;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::minMaxScalarValues(const RigFlowDiagResultAddress& resVarAddr, int frameIndex,
                                                     double* localMin, double* localMax)
{
    this->statistics(resVarAddr)->minMaxCellScalarValues(frameIndex, *localMin, *localMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::minMaxScalarValues(const RigFlowDiagResultAddress& resVarAddr,
                                                     double* globalMin, double* globalMax)
{
    this->statistics(resVarAddr)->minMaxCellScalarValues(*globalMin, *globalMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::posNegClosestToZero(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero)
{
    this->statistics(resVarAddr)->posNegClosestToZero(frameIndex, *localPosClosestToZero, *localNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::posNegClosestToZero(const RigFlowDiagResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero)
{
    this->statistics(resVarAddr)->posNegClosestToZero(*globalPosClosestToZero, *globalNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::meanScalarValue(const RigFlowDiagResultAddress& resVarAddr, double* meanValue)
{
    CVF_ASSERT(meanValue);

    this->statistics(resVarAddr)->meanCellScalarValues(*meanValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::meanScalarValue(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* meanValue)
{
    this->statistics(resVarAddr)->meanCellScalarValues(frameIndex, *meanValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::p10p90ScalarValues(const RigFlowDiagResultAddress& resVarAddr, double* p10, double* p90)
{
    this->statistics(resVarAddr)->p10p90CellScalarValues(*p10, *p90);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::p10p90ScalarValues(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* p10, double* p90)
{
    this->statistics(resVarAddr)->p10p90CellScalarValues(frameIndex, *p10, *p90);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::sumScalarValue(const RigFlowDiagResultAddress& resVarAddr, double* sum)
{
    CVF_ASSERT(sum);

    this->statistics(resVarAddr)->sumCellScalarValues(*sum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::sumScalarValue(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* sum)
{
    CVF_ASSERT(sum);

    this->statistics(resVarAddr)->sumCellScalarValues(frameIndex, *sum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFlowDiagResults::scalarValuesHistogram(const RigFlowDiagResultAddress& resVarAddr)
{
    return this->statistics(resVarAddr)->cellScalarValuesHistogram();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFlowDiagResults::scalarValuesHistogram(const RigFlowDiagResultAddress& resVarAddr, int frameIndex)
{
    return this->statistics(resVarAddr)->cellScalarValuesHistogram(frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigFlowDiagResults::uniqueCellScalarValues(const RigFlowDiagResultAddress& resVarAddr)
{
    return this->statistics(resVarAddr)->uniqueCellScalarValues();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<int>& RigFlowDiagResults::uniqueCellScalarValues(const RigFlowDiagResultAddress& resVarAddr, int frameIndex)
{
    return this->statistics(resVarAddr)->uniqueCellScalarValues(frameIndex);
}
