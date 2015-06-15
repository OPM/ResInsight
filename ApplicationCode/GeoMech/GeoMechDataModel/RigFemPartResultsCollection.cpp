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

#include "RigFemPartResultsCollection.h"

#include "RifGeoMechReaderInterface.h"

#ifdef USE_ODB_API 
#include "RifOdbReader.h"
#endif
#include "RigFemScalarResultFrames.h"
#include "RigStatisticsDataCache.h"
#include "RigFemPartResults.h"

#include "cafProgressInfo.h"
#include "cvfBoundingBox.h"
#include <QString>

#include <cmath>
#include <stdlib.h>
#include "RigFemNativeStatCalc.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartResultsCollection::RigFemPartResultsCollection(RifGeoMechReaderInterface* readerInterface, int partCount)
{
    CVF_ASSERT(readerInterface);
    m_readerInterface = readerInterface;
    m_femPartResults.resize(partCount);
    std::vector<std::string> stepNames = m_readerInterface->stepNames();
    for (int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx)
    {
        m_femPartResults[pIdx] = new RigFemPartResults;
        m_femPartResults[pIdx]->initResultSteps(stepNames);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartResultsCollection::~RigFemPartResultsCollection()
{

}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RigFemPartResultsCollection::scalarFieldAndComponentNames(RigFemResultPosEnum resPos)
{
    std::map<std::string, std::vector<std::string> >  fieldCompNames;

    if (m_readerInterface.notNull())
    {
        if (resPos == RIG_NODAL)
        {
            fieldCompNames = m_readerInterface->scalarNodeFieldAndComponentNames();
        }
        else if (resPos == RIG_ELEMENT_NODAL)
        {
            fieldCompNames = m_readerInterface->scalarElementNodeFieldAndComponentNames();
            fieldCompNames["NS"].push_back("S11");
            fieldCompNames["NS"].push_back("S22");
            fieldCompNames["NS"].push_back("S33");
            fieldCompNames["NS"].push_back("S12");
            fieldCompNames["NS"].push_back("S13");
            fieldCompNames["NS"].push_back("S23");
        }
        else if (resPos == RIG_INTEGRATION_POINT)
        {
            fieldCompNames = m_readerInterface->scalarIntegrationPointFieldAndComponentNames();
            fieldCompNames["NS"].push_back("S11");
            fieldCompNames["NS"].push_back("S22");
            fieldCompNames["NS"].push_back("S33");
            fieldCompNames["NS"].push_back("S12");
            fieldCompNames["NS"].push_back("S13");
            fieldCompNames["NS"].push_back("S23");
        }
    }

    return fieldCompNames;
}

//--------------------------------------------------------------------------------------------------
/// Will always return a valid object, but it can be empty
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::findOrLoadScalarResult(int partIndex,
                                                                              const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(partIndex < (int)(m_femPartResults.size()));
    CVF_ASSERT(m_readerInterface.notNull());
    CVF_ASSERT(resVarAddr.isValid());

    // If we have it in the cache, return it
    RigFemScalarResultFrames* frames = m_femPartResults[partIndex]->findScalarResult(resVarAddr);
    if (frames) return frames;

    // Check whether a derived result is requested

    frames = calculateDerivedResult(partIndex, resVarAddr);
    if (frames) return frames;

    // We need to read the data as bulk fields, and populate the correct scalar caches 

    std::vector< RigFemResultAddress> resultAddressOfComponents = this->getResAddrToComponentsToRead(resVarAddr);
    // 
    std::vector<RigFemScalarResultFrames*> resultsForEachComponent;
    for (size_t cIdx = 0; cIdx < resultAddressOfComponents.size(); ++cIdx)
    {
        resultsForEachComponent.push_back(m_femPartResults[partIndex]->createScalarResult(resultAddressOfComponents[cIdx]));
    }

    int frameCount =  this->frameCount();

    for (int stepIndex = 0; stepIndex < frameCount; ++stepIndex)
    {
        std::vector<double > frameTimes = m_readerInterface->frameTimes(stepIndex);

        for (int fIdx = 1; (size_t)fIdx < frameTimes.size() && fIdx < 2 ; ++fIdx)  // Read only the second frame
        {
            std::vector<std::vector<float>*> componentDataVectors;
            for (size_t cIdx = 0; cIdx < resultsForEachComponent.size(); ++cIdx)
            {
                componentDataVectors.push_back(&(resultsForEachComponent[cIdx]->frameData(stepIndex)));
            }

            switch (resVarAddr.resultPosType)
            {
                case RIG_NODAL:
                    m_readerInterface->readNodeField(resVarAddr.fieldName,  partIndex, stepIndex, fIdx, &componentDataVectors);
                    break;
                case RIG_ELEMENT_NODAL:
                    m_readerInterface->readElementNodeField(resVarAddr.fieldName,  partIndex, stepIndex, fIdx, &componentDataVectors);
                    break;
                case RIG_INTEGRATION_POINT:
                    m_readerInterface->readIntegrationPointField(resVarAddr.fieldName,  partIndex, stepIndex, fIdx, &componentDataVectors);
                    break;
            }
        }
    }

    // Now fetch the particular component requested, which should now exist and be read.
    frames = m_femPartResults[partIndex]->findScalarResult(resVarAddr);

    if (!frames) 
    {
        frames = m_femPartResults[partIndex]->createScalarResult(resVarAddr); // Create a dummy empty result, if the request did not specify the component.
    }

    return frames;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDerivedResult(int partIndex, const RigFemResultAddress& resVarAddr)
{
    // ST[11, 22, 33, 12, 13, 23, 1, 2, 3], Gamma[1,2,3], NS[11,22,33,12,13,23, 1, 2, 3]

    if (resVarAddr.fieldName == "NS")
    {
        RigFemScalarResultFrames * srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "S", resVarAddr.componentName));
        RigFemScalarResultFrames * dstDataFrames =  m_femPartResults[partIndex]->createScalarResult(resVarAddr);
        int frameCount = srcDataFrames->frameCount();
        for (int fIdx = 0; fIdx < frameCount; ++fIdx)
        {
            const std::vector<float>& srcFrameData = srcDataFrames->frameData(fIdx);
            std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
            size_t valCount = srcFrameData.size();
            dstFrameData.resize(valCount);
            for (size_t vIdx = 0; vIdx < valCount; ++vIdx)
            {
                dstFrameData[vIdx] = -srcFrameData[vIdx];
            }
        }
        return dstDataFrames;
    }

    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector< RigFemResultAddress> RigFemPartResultsCollection::getResAddrToComponentsToRead(const RigFemResultAddress& resVarAddr)
{
    std::map<std::string, std::vector<std::string> > fieldAndComponentNames;
    switch (resVarAddr.resultPosType)
    {
        case RIG_NODAL:
            fieldAndComponentNames = m_readerInterface->scalarNodeFieldAndComponentNames();
            break;
        case RIG_ELEMENT_NODAL:
            fieldAndComponentNames = m_readerInterface->scalarElementNodeFieldAndComponentNames();
            break;
        case RIG_INTEGRATION_POINT:
            fieldAndComponentNames = m_readerInterface->scalarIntegrationPointFieldAndComponentNames();
            break;
    }

    std::vector< RigFemResultAddress> resAddressToComponents;

    std::map<std::string, std::vector<std::string> >::iterator fcIt = fieldAndComponentNames.find(resVarAddr.fieldName);

    if (fcIt != fieldAndComponentNames.end())
    {
        std::vector<std::string> compNames = fcIt->second;
        for (size_t cIdx = 0; cIdx < compNames.size(); ++cIdx)
        {
            resAddressToComponents.push_back(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, compNames[cIdx]));
        }

        if (compNames.size() == 0) // This is a scalar field. Add one component named ""
        {
            CVF_ASSERT(resVarAddr.componentName == "");
            resAddressToComponents.push_back(resVarAddr);
        }
    }

    return resAddressToComponents;
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigFemPartResultsCollection::stepNames()
{
    CVF_ASSERT(m_readerInterface.notNull());
    return m_readerInterface->stepNames();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RigFemPartResultsCollection::frameCount()
{
    return static_cast<int>(stepNames().size());
}

//--------------------------------------------------------------------------------------------------
/// Returns whether any of the parts actually had any of the requested results
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultsCollection::assertResultsLoaded(const RigFemResultAddress& resVarAddr)
{
    if (!resVarAddr.isValid()) return false;

    bool foundResults = false;

    for (int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
             RigFemScalarResultFrames* scalarResults = findOrLoadScalarResult(pIdx, resVarAddr);
             for (int fIdx = 0; fIdx < scalarResults->frameCount(); ++fIdx)
             {
                 foundResults = foundResults || scalarResults->frameData(fIdx).size();
             }
        }
    }

    return foundResults;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<float>& RigFemPartResultsCollection::resultValues(const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex)
{
    CVF_ASSERT(resVarAddr.isValid());

    RigFemScalarResultFrames* scalarResults = findOrLoadScalarResult(partIndex, resVarAddr);
    return scalarResults->frameData(frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigStatisticsDataCache* RigFemPartResultsCollection::statistics(const RigFemResultAddress& resVarAddr)
{
    RigStatisticsDataCache* statCache = m_resultStatistics[resVarAddr].p();
    if (!statCache)
    {
        RigFemNativeStatCalc* calculator = new RigFemNativeStatCalc(this, resVarAddr);
        statCache = new RigStatisticsDataCache(calculator);
        m_resultStatistics[resVarAddr] = statCache;
    }

    return statCache;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::minMaxScalarValues(const RigFemResultAddress& resVarAddr, int frameIndex,
                                                     double* localMin, double* localMax)
{
    this->statistics(resVarAddr)->minMaxCellScalarValues(frameIndex, *localMin, *localMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::minMaxScalarValues(const RigFemResultAddress& resVarAddr,
                                                     double* globalMin, double* globalMax)
{
    this->statistics(resVarAddr)->minMaxCellScalarValues(*globalMin, *globalMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZero(const RigFemResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero)
{
    this->statistics(resVarAddr)->posNegClosestToZero(frameIndex, *localPosClosestToZero, *localNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZero(const RigFemResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero)
{
    this->statistics(resVarAddr)->posNegClosestToZero(*globalPosClosestToZero, *globalNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::meanScalarValue(const RigFemResultAddress& resVarAddr, double* meanValue)
{
    CVF_ASSERT(meanValue);

    this->statistics(resVarAddr)->meanCellScalarValues(*meanValue);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::p10p90ScalarValues(const RigFemResultAddress& resVarAddr, double* p10, double* p90)
{
    this->statistics(resVarAddr)->p10p90CellScalarValues(*p10, *p90);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFemPartResultsCollection::scalarValuesHistogram(const RigFemResultAddress& resVarAddr)
{
    return this->statistics(resVarAddr)->cellScalarValuesHistogram();
}

