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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFlowDiagResults::RigFlowDiagResults(RimFlowDiagSolution* flowSolution, size_t timeStepCount)
 : m_flowDiagSolution()
{

    m_flowDagSolverInterface = new RigFlowDiagSolverInterface(flowSolution);

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
const std::vector<double>* RigFlowDiagResults::findOrCalculateResult(const RigFlowDiagResultAddress& resVarAddr, size_t frameIndex)
{
   
    std::vector<double>* frameData = findScalarResultFrame(resVarAddr, frameIndex);

    if ( frameData ) return frameData;

    frameData = calculateDerivedResult(resVarAddr, frameIndex);

    if ( frameData ) return frameData;

    // We need to access the native data from the opm solver

    if (!m_hasAtemptedNativeResults[frameIndex])
    {
        RigFlowDiagTimeStepResult nativeTimestepResults =  m_flowDagSolverInterface->calculate(frameIndex);

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

