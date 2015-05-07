/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RigFemPartResults.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartResults::RigFemPartResults()
{
   
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartResults::~RigFemPartResults()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResults::initResultStages(const std::vector<std::string>& stageNames)
{
    m_femAnalysisStages.clear();
    m_femAnalysisStages.resize(stageNames.size());
    for (size_t sIdx = 0; sIdx < stageNames.size(); ++sIdx)
    {
        m_femAnalysisStages[sIdx].stageName = stageNames[sIdx];
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResults::createScalarResult(size_t stageIndex, 
                                                                RigFemResultPosEnum resultPosType, 
                                                                const std::string& fieldName, 
                                                                const std::string& componentName, 
                                                                const std::vector<double>& frameTimes)
{
    RigFemScalarResultFrames * resFrames = new RigFemScalarResultFrames(frameTimes);
    m_femAnalysisStages[stageIndex].resultSets[resultPosType][fieldName][componentName] = resFrames;
    return resFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResults::findScalarResult(size_t stageIndex, 
                                                              RigFemResultPosEnum resultPosType, 
                                                              const std::string& fieldName, 
                                                              const std::string& componentName)
{
    return m_femAnalysisStages[stageIndex].resultSets[resultPosType][fieldName][componentName].p();
}
