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

#include <stdlib.h>
#include "RigFemPartResults.h"
#include "RigFemResultAddress.h"

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
void RigFemPartResults::initResultSteps(const std::vector<std::string>& stepNames)
{
    m_stepNames = stepNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResults::createScalarResult(const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(m_stepNames.size());

    RigFemScalarResultFrames * resFrames = new RigFemScalarResultFrames(static_cast<int>(m_stepNames.size()));
    resultSets[resVarAddr] = resFrames;
    return resFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResults::findScalarResult(const RigFemResultAddress& resVarAddr)
{
    return resultSets[resVarAddr].p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResults::deleteScalarResult(const RigFemResultAddress& resVarAddr)
{
    resultSets.erase(resVarAddr); // Refcounting is supposed to destroy the data.
    
    if (resVarAddr.representsAllTimeLapses())
    {
        std::vector<RigFemResultAddress> addressesToDelete;
        for (auto it : resultSets)
        {
            if (it.first.resultPosType == resVarAddr.resultPosType
                && it.first.fieldName == resVarAddr.fieldName
                &&  it.first.componentName == resVarAddr.componentName)
                { 
                    addressesToDelete. push_back(it.first);
                }
        }

        for (RigFemResultAddress& addr: addressesToDelete)
        {
            resultSets.erase(addr);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress> RigFemPartResults::loadedResults() const
{
    std::vector<RigFemResultAddress> currentResults;
    for (const auto& result : resultSets)
    {
        currentResults.push_back(result.first);
    }
    return currentResults;
}
