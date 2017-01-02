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
    if (resVarAddr.isNativeResult()) return nullptr;


    if (resVarAddr.variableName == RIG_FLD_TOF_RESNAME)
    {
        std::vector<const std::vector<double>* > injectorTOFs;
        std::vector<const std::vector<double>* > injectorFractions;

        std::vector<const std::vector<double>* > producerTOFs;
        std::vector<const std::vector<double>* > producerFractions;

        for (const std::string& tracerName: resVarAddr.selectedTracerNames)
        {
            RimFlowDiagSolution::TracerStatusType tracerType = m_flowDiagSolution->tracerStatusInTimeStep(QString::fromStdString(tracerName), frameIndex);

            if ( tracerType == RimFlowDiagSolution::INJECTOR )
            {
                injectorTOFs.push_back( findOrCalculateResult(RigFlowDiagResultAddress(RIG_FLD_TOF_RESNAME, tracerName), frameIndex));
                injectorFractions.push_back(findOrCalculateResult(RigFlowDiagResultAddress(RIG_FLD_CELL_FRACTION_RESNAME, tracerName), frameIndex));
            }
            else if ( tracerType == RimFlowDiagSolution::PRODUCER )
            {
                producerTOFs.push_back(findOrCalculateResult(RigFlowDiagResultAddress(RIG_FLD_TOF_RESNAME, tracerName), frameIndex));
                producerFractions.push_back(findOrCalculateResult(RigFlowDiagResultAddress(RIG_FLD_CELL_FRACTION_RESNAME, tracerName), frameIndex));
            }
        }

        size_t activeCellCount =  this->activeCellInfo(resVarAddr)->reservoirActiveCellCount();

        std::vector<double> injectorTotalFractions;
        std::vector<double> injectorFractMultTof;

        calculateSumOfFractionAndFractionMultTOF(activeCellCount,  injectorFractions, injectorTOFs, &injectorTotalFractions,  &injectorFractMultTof);


        std::vector<double> producerTotalFractions;
        std::vector<double> producerFractMultTof;
        calculateSumOfFractionAndFractionMultTOF(activeCellCount, producerFractions, producerTOFs, &producerTotalFractions, &producerFractMultTof);

        RigFlowDiagResultFrames* averageTofFrames = this->createScalarResult(resVarAddr);
        std::vector<double>& averageTof = averageTofFrames->frameData(frameIndex);
        averageTof.resize(activeCellCount, HUGE_VAL);

        for (size_t acIdx = 0 ; acIdx < activeCellCount; ++acIdx)
        {
            if ( injectorTotalFractions[acIdx] == 0.0 && producerTotalFractions[acIdx] == 0.0 )
            {
                averageTof[acIdx] = HUGE_VAL;
            }
            else 
            {
                double retVal = 0.0;
                if (injectorTotalFractions[acIdx] != 0.0) retVal +=  (1.0/injectorTotalFractions[acIdx]) * injectorFractMultTof[acIdx];
                if (producerTotalFractions[acIdx] != 0.0) retVal +=  (1.0/producerTotalFractions[acIdx]) * producerFractMultTof[acIdx];
                averageTof[acIdx] = retVal;
            }
        }

        return &averageTof;
    }
    else if (resVarAddr.variableName == RIG_FLD_CELL_FRACTION_RESNAME)
    {
        std::vector<const std::vector<double>* > fractions;
        for ( const std::string& tracerName: resVarAddr.selectedTracerNames )
        {
            fractions.push_back(findOrCalculateResult(RigFlowDiagResultAddress(RIG_FLD_CELL_FRACTION_RESNAME, tracerName), frameIndex));
        }

        size_t activeCellCount =  this->activeCellInfo(resVarAddr)->reservoirActiveCellCount();

        RigFlowDiagResultFrames* sumOfFractionsFrames = this->createScalarResult(resVarAddr);
        std::vector<double>& sumOfFractions = sumOfFractionsFrames->frameData(frameIndex);
        sumOfFractions.resize(activeCellCount, HUGE_VAL);

        for ( size_t iIdx = 0; iIdx < fractions.size() ; ++iIdx )
        {
            const std::vector<double> * frInj = fractions[iIdx];

            if ( ! (frInj) ) continue;

            for ( size_t acIdx = 0 ; acIdx < activeCellCount; ++acIdx )
            {
                if ( (*frInj)[acIdx] == HUGE_VAL ) continue;

                if ((sumOfFractions)[acIdx] == HUGE_VAL) (sumOfFractions)[acIdx] = 0.0;

                (sumOfFractions)[acIdx] += (*frInj)[acIdx];
            }
        }

        return &sumOfFractions;
    }

    return nullptr; // Todo
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFlowDiagResults::calculateSumOfFractionAndFractionMultTOF(size_t activeCellCount,
                                                                  const std::vector<const std::vector<double> *> & fractions, 
                                                                  const std::vector<const std::vector<double> *> & TOFs,
                                                                  std::vector<double> *sumOfFractions,
                                                                  std::vector<double> *fractionMultTOF)
{
    sumOfFractions->resize(activeCellCount, 0.0);
    fractionMultTOF->resize(activeCellCount, 0.0);

    for ( size_t iIdx = 0; iIdx < fractions.size() ; ++iIdx )
    {
        const std::vector<double> * frInj = fractions[iIdx];
        const std::vector<double> * tofInj = TOFs[iIdx];

        if ( ! (frInj && tofInj) ) continue;

        for ( size_t acIdx = 0 ; acIdx < activeCellCount; ++acIdx )
        {
            if ( (*frInj)[acIdx] == HUGE_VAL ) continue;

            (*sumOfFractions)[acIdx] += (*frInj)[acIdx];
            (*fractionMultTOF)[acIdx]   += (*frInj)[acIdx] * (*tofInj)[acIdx];
        }
    }
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
