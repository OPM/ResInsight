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
#include "RigFemResultAddress.h"

class RifGeoMechReaderInterface;
class RigFemPartCollection;
class RigFemScalarResultFrames;

class RigGeoMechCaseData: public cvf::Object
{
public:
    RigGeoMechCaseData(const std::string& fileName);
    ~RigGeoMechCaseData();

    bool                                 openAndReadFemParts(std::string* errorMessage);

    RigFemPartCollection*                femParts();
    const RigFemPartCollection*          femParts() const;

    std::map<std::string, std::vector<std::string> > scalarFieldAndComponentNames(RigFemResultPosEnum resPos);
    std::vector<std::string>             stepNames();
    void                                 assertResultsLoaded(const RigFemResultAddress& resVarAddr);
    RigFemScalarResultFrames*            findOrLoadScalarResult(int partIndex,
                                                                const RigFemResultAddress& resVarAddr);

    int frameCount();

    void minMaxScalarValues (const RigFemResultAddress& resVarAddr, int frameIndex,  double* localMin, double* localMax);
    void posNegClosestToZero(const RigFemResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero);
    void minMaxScalarValues (const RigFemResultAddress& resVarAddr, double* globalMin, double* globalMax);
    void posNegClosestToZero(const RigFemResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero);
    void meanCellScalarValues(const RigFemResultAddress& resVarAddr, double* meanValue);

private:
    void minMaxScalarValuesInternal(const RigFemResultAddress& resVarAddr, int frameIndex, 
                                    double* overallMin, double* overallMax);
    void posNegClosestToZeroInternal(const RigFemResultAddress& resVarAddr, int frameIndex, 
                                     double* localPosClosestToZero, double* localNegClosestToZero);

    std::string                          m_geoMechCaseFileName;
    cvf::ref<RigFemPartCollection>       m_femParts;
    cvf::Collection<RigFemPartResults>   m_femPartResults;
    cvf::ref<RifGeoMechReaderInterface>  m_readerInterface;
};

