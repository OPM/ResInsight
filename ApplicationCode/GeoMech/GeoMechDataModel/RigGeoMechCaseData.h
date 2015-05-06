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
#include <string>
#include <map>
#include <vector>
#include "RigFemResultPosEnum.h"
#include "cvfCollection.h"
#include "RigFemPartResults.h"

class RifGeoMechReaderInterface;
class RigFemPartCollection;
class RigFemScalarResultFrames;

class RigGeoMechCaseData: public cvf::Object
{
public:
    RigGeoMechCaseData(const std::string& fileName);
    ~RigGeoMechCaseData();

    bool                                 openAndReadFemParts();

    RigFemPartCollection*                femParts();
    const RigFemPartCollection*          femParts() const;

    std::map<std::string, std::vector<std::string> > scalarFieldAndComponentNames(RigFemResultPosEnum resPos);
    std::vector<std::string>             stepNames();
    RigFemScalarResultFrames*            findOrLoadScalarResult(size_t partIndex, 
                                                     size_t stepIndex,
                                                     RigFemResultPosEnum resultPosType,
                                                     const std::string& fieldName,
                                                     const std::string& componentName);

private:
    std::string                          m_geoMechCaseFileName;
    cvf::ref<RigFemPartCollection>       m_femParts;
    cvf::Collection<RigFemPartResults>   m_femPartResults;
    cvf::ref<RifGeoMechReaderInterface>  m_readerInterface;
};

