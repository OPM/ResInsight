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

#include "RigFemResultAddress.h"

#include "cvfCollection.h"
#include "cvfObject.h"
#include <map>
#include <vector>

class RifGeoMechReaderInterface;
class RigFemScalarResultFrames;
class RigFemPartResultsCollection;
class RigFemPartResults;
class RigStatisticsDataCache;
class RigFemPartCollection;
class RigFormationNames;

class RigFemPartResultsCollection: public cvf::Object
{
public:
    RigFemPartResultsCollection(RifGeoMechReaderInterface* readerInterface, const RigFemPartCollection * femPartCollection);
    ~RigFemPartResultsCollection();

    void                                             setActiveFormationNames(RigFormationNames* activeFormationNames);
    RigFormationNames*                               activeFormationNames();

    std::map<std::string, std::vector<std::string> > scalarFieldAndComponentNames(RigFemResultPosEnum resPos);
    std::vector<std::string>                         stepNames();
    bool                                             assertResultsLoaded(const RigFemResultAddress& resVarAddr);
    const std::vector<float>&                        resultValues(const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex); 
    int                                              partCount() const;
    int                                              frameCount();


    void                                             minMaxScalarValues (const RigFemResultAddress& resVarAddr, int frameIndex,  double* localMin, double* localMax);
    void                                             minMaxScalarValues (const RigFemResultAddress& resVarAddr, double* globalMin, double* globalMax);
    void                                             posNegClosestToZero(const RigFemResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero);
    void                                             posNegClosestToZero(const RigFemResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero);
    void                                             meanScalarValue(const RigFemResultAddress& resVarAddr, double* meanValue);
    void                                             meanScalarValue(const RigFemResultAddress& resVarAddr, int frameIndex, double* meanValue);
    void                                             p10p90ScalarValues(const RigFemResultAddress& resVarAddr, double* p10, double* p90);
    void                                             p10p90ScalarValues(const RigFemResultAddress& resVarAddr, int frameIndex, double* p10, double* p90);
    void                                             sumScalarValue(const RigFemResultAddress& resVarAddr, double* sum);
    void                                             sumScalarValue(const RigFemResultAddress& resVarAddr, int frameIndex, double* sum);
    const std::vector<size_t>&                       scalarValuesHistogram(const RigFemResultAddress& resVarAddr);
    const std::vector<size_t>&                       scalarValuesHistogram(const RigFemResultAddress& resVarAddr, int frameIndex);

private:
    RigFemScalarResultFrames*                        findOrLoadScalarResult(int partIndex,
                                                                            const RigFemResultAddress& resVarAddr);

    RigFemScalarResultFrames*                        calculateDerivedResult(int partIndex, const RigFemResultAddress& resVarAddr);

    RigFemScalarResultFrames*                        calculateBarConvertedResult(int partIndex, const RigFemResultAddress &convertedResultAddr, const std::string fieldNameToConvert);
    RigFemScalarResultFrames*                        calculateEnIpPorBarResult(int partIndex, const RigFemResultAddress &convertedResultAddr);
    RigFemScalarResultFrames*                        calculateTimeLapseResult(int partIndex, const RigFemResultAddress& resVarAddr);
    RigFemScalarResultFrames*                        calculateMeanStressSMSEM(int partIndex, const RigFemResultAddress& resVarAddr);
    RigFemScalarResultFrames*                        calculateDeviatoricStress(int partIndex, const RigFemResultAddress& resVarAddr);

    cvf::Collection<RigFemPartResults>               m_femPartResults;
    cvf::ref<RifGeoMechReaderInterface>              m_readerInterface;
    cvf::cref<RigFemPartCollection>                  m_femParts;
    cvf::ref<RigFormationNames>                      m_activeFormationNamesData;

    RigStatisticsDataCache*                          statistics(const RigFemResultAddress& resVarAddr);
    std::vector< RigFemResultAddress>                getResAddrToComponentsToRead(const RigFemResultAddress& resVarAddr);
    std::map<RigFemResultAddress, cvf::ref<RigStatisticsDataCache> >  m_resultStatistics;
};



