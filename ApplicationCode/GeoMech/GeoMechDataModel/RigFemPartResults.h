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
#include "RigFemResultAddress.h"

//==================================================================================================
/// 
//==================================================================================================

class RigFemPartResults : public cvf::Object
{
public:
    RigFemPartResults();
    ~RigFemPartResults();

    void initResultSteps(const std::vector<std::string>& stepNames);

    RigFemScalarResultFrames*        createScalarResult(const RigFemResultAddress& resVarAddr);
    RigFemScalarResultFrames*        findScalarResult(const RigFemResultAddress& resVarAddr);
    void                             deleteScalarResult(const RigFemResultAddress& resVarAddr);
    std::vector<RigFemResultAddress> loadedResults() const;

private:

    std::vector<std::string> m_stepNames;
    std::map<RigFemResultAddress, cvf::ref<RigFemScalarResultFrames> >  resultSets;
};

