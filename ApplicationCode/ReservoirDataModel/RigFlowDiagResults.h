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

class RigFlowDiagResults: public cvf::Object
{
public:
    RigFlowDiagResults(RimFlowDiagSolution* flowSolution, size_t timeStepCount);
    virtual ~RigFlowDiagResults();

    const std::vector<double>* resultValues(const RigFlowDiagResultAddress& resVarAddr,  size_t frameIndex);

private:
    const std::vector<double>* findOrCalculateResult (const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);
    std::vector<double>*       calculateDerivedResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex);
    void                       calculateFractionWeightedTOF  ( size_t timeStepIdx, std::set<std::string> selectedTracerNames);
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

    cvf::ref<RigFlowDiagSolverInterface>                                     m_flowDagSolverInterface;

    caf::PdmPointer<RimFlowDiagSolution> m_flowDiagSolution;

    std::vector<bool> m_hasAtemptedNativeResults;
};


