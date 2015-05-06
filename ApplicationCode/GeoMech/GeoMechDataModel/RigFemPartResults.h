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

#pragma once
#include "cvfObject.h"
#include "RigFemResultPosEnum.h"
#include <string>
#include <vector>
#include <map>

#include "RigFemScalarResultFrames.h"

//==================================================================================================
/// 
//==================================================================================================

class RigFemPartResults : public cvf::Object
{
public:
    RigFemPartResults();
    ~RigFemPartResults();

    void initResultStages( const std::vector<std::string>& stageNames);

    RigFemScalarResultFrames* createScalarResult( size_t stageIndex, 
                                                  RigFemResultPosEnum resultPosType,
                                                  const std::string& fieldName,
                                                  const std::string& componentName,
                                                  const std::vector<double>& frameTimes);

    RigFemScalarResultFrames* findScalarResult( size_t stageIndex, 
                                                RigFemResultPosEnum resultPosType,
                                                const std::string& fieldName,
                                                const std::string& componentName);

private:

    struct RigAnalysisStage
    {
        std::string stageName;
        std::map<RigFemResultPosEnum, std::map<std::string, std::map< std::string, cvf::ref<RigFemScalarResultFrames> > > > resultSets;
    };

    std::vector<RigAnalysisStage> m_femAnalysisStages;
 
};

