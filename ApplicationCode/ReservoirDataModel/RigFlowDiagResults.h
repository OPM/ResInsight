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
#pragma once

#include "RigFlowDiagResultAddress.h"

#include "cvfBase.h"
#include "cvfObject.h"

#include <vector>
#include <map>
#include <string>

#include "RigFlowDiagResultFrames.h"
#include "RigStatisticsDataCache.h"
#include "cafPdmPointer.h"

class RigFlowDiagSolverInterface;
class RimFlowDiagSolution;
class RigActiveCellInfo;

class RigFlowDiagResults: public cvf::Object
{
public:
    RigFlowDiagResults(RimFlowDiagSolution* flowSolution, size_t timeStepCount);
    virtual ~RigFlowDiagResults();

    const std::vector<double>* resultValues(const RigFlowDiagResultAddress& resVarAddr,  size_t frameIndex);
    size_t                     timeStepCount() { return m_timeStepCount; }
    const RigActiveCellInfo *  activeCellInfo(const RigFlowDiagResultAddress& resVarAddr);


    void                                             minMaxScalarValues (const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* localMin, double* localMax);
    void                                             minMaxScalarValues (const RigFlowDiagResultAddress& resVarAddr, double* globalMin, double* globalMax);
    void                                             posNegClosestToZero(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero);
    void                                             posNegClosestToZero(const RigFlowDiagResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero);
    void                                             meanScalarValue(const RigFlowDiagResultAddress& resVarAddr, double* meanValue);
    void                                             meanScalarValue(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* meanValue);
    void                                             p10p90ScalarValues(const RigFlowDiagResultAddress& resVarAddr, double* p10, double* p90);
    void                                             p10p90ScalarValues(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* p10, double* p90);
    void                                             sumScalarValue(const RigFlowDiagResultAddress& resVarAddr, double* sum);
    void                                             sumScalarValue(const RigFlowDiagResultAddress& resVarAddr, int frameIndex, double* sum);
    const std::vector<size_t>&                       scalarValuesHistogram(const RigFlowDiagResultAddress& resVarAddr);
    const std::vector<size_t>&                       scalarValuesHistogram(const RigFlowDiagResultAddress& resVarAddr, int frameIndex);
    const std::vector<int>&                          uniqueCellScalarValues(const RigFlowDiagResultAddress& resVarAddr);
    const std::vector<int>&                          uniqueCellScalarValues(const RigFlowDiagResultAddress& resVarAddr, int frameIndex);

private:
    const std::vector<double>* findOrCalculateResult (const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);
    std::vector<double>*       calculateDerivedResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);
    RigStatisticsDataCache*    statistics(const RigFlowDiagResultAddress& resVarAddr);

    void                       calculateFractionWeightedTOF  (size_t timeStepIdx, std::set<std::string> selectedTracerNames);
    void                       calculateSumOfFractions       ( size_t timeStepIdx, std::set<std::string> selectedTracerNames);
    void                       calculateTracerWithMaxFraction( size_t timeStepIdx, std::set<std::string> selectedTracerNames); // Needs a tracer index
    void                       calculateCommunication        ( size_t timeStepIdx, std::set<std::string> selectedTracerNames);
                               
    RigFlowDiagResultFrames*   createScalarResult(const RigFlowDiagResultAddress& resVarAddr);
    RigFlowDiagResultFrames*   findScalarResult  (const RigFlowDiagResultAddress& resVarAddr) ;
    std::vector<double>*       findScalarResultFrame  (const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);

    //void                       deleteScalarResult(const RigFlowDiagResultAddress& resVarAddr);

    size_t m_timeStepCount;

    std::map< RigFlowDiagResultAddress, cvf::ref<RigFlowDiagResultFrames> >  m_resultSets;
    std::map< RigFlowDiagResultAddress, cvf::ref<RigStatisticsDataCache>  >  m_resultStatistics;

    RigFlowDiagSolverInterface* solverInterface();

    caf::PdmPointer<RimFlowDiagSolution> m_flowDiagSolution;

    std::vector<bool> m_hasAtemptedNativeResults;
};


