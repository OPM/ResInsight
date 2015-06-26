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
#include "RigFemPartCollection.h"

#include "cafProgressInfo.h"
#include "cvfBoundingBox.h"
#include <QString>

#include <cmath>
#include <stdlib.h>
#include "RigFemNativeStatCalc.h"
#include "cafTensor3.h"
#include "cafProgressInfo.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartResultsCollection::RigFemPartResultsCollection(RifGeoMechReaderInterface* readerInterface, const RigFemPartCollection * femPartCollection)
{
    CVF_ASSERT(readerInterface);
    m_readerInterface = readerInterface;
    m_femParts = femPartCollection;

    m_femPartResults.resize(m_femParts->partCount());
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

    caf::ProgressInfo progress(frameCount, "Loading Results");

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

        progress.incrementProgress();
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
            fieldCompNames["NS"].push_back("S1");
            fieldCompNames["NS"].push_back("S2");
            fieldCompNames["NS"].push_back("S3");

            fieldCompNames["ST"].push_back("S11");
            fieldCompNames["ST"].push_back("S22");
            fieldCompNames["ST"].push_back("S33");
            fieldCompNames["ST"].push_back("S12");
            fieldCompNames["ST"].push_back("S13");
            fieldCompNames["ST"].push_back("S23");
            fieldCompNames["ST"].push_back("S1");
            fieldCompNames["ST"].push_back("S2");
            fieldCompNames["ST"].push_back("S3");

            fieldCompNames["Gamma"].push_back("Gamma1");
            fieldCompNames["Gamma"].push_back("Gamma2");
            fieldCompNames["Gamma"].push_back("Gamma3");
            fieldCompNames["Gamma"].push_back("Gamma11");
            fieldCompNames["Gamma"].push_back("Gamma22");
            fieldCompNames["Gamma"].push_back("Gamma33");

            fieldCompNames["NE"].push_back("E11");
            fieldCompNames["NE"].push_back("E22");
            fieldCompNames["NE"].push_back("E33");
            fieldCompNames["NE"].push_back("E12");
            fieldCompNames["NE"].push_back("E13");
            fieldCompNames["NE"].push_back("E23");
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
            fieldCompNames["NS"].push_back("S1");
            fieldCompNames["NS"].push_back("S2");
            fieldCompNames["NS"].push_back("S3");
 
            fieldCompNames["ST"].push_back("S11");
            fieldCompNames["ST"].push_back("S22");
            fieldCompNames["ST"].push_back("S33");
            fieldCompNames["ST"].push_back("S12");
            fieldCompNames["ST"].push_back("S13");
            fieldCompNames["ST"].push_back("S23");
            fieldCompNames["ST"].push_back("S1");
            fieldCompNames["ST"].push_back("S2");
            fieldCompNames["ST"].push_back("S3");

            fieldCompNames["Gamma"].push_back("Gamma1");
            fieldCompNames["Gamma"].push_back("Gamma2");
            fieldCompNames["Gamma"].push_back("Gamma3");
            fieldCompNames["Gamma"].push_back("Gamma11");
            fieldCompNames["Gamma"].push_back("Gamma22");
            fieldCompNames["Gamma"].push_back("Gamma33");

            fieldCompNames["NE"].push_back("E11");
            fieldCompNames["NE"].push_back("E22");
            fieldCompNames["NE"].push_back("E33");
            fieldCompNames["NE"].push_back("E12");
            fieldCompNames["NE"].push_back("E13");
            fieldCompNames["NE"].push_back("E23");
       }
    }

    return fieldCompNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDerivedResult(int partIndex, const RigFemResultAddress& resVarAddr)
{
    // ST[11, 22, 33, 12, 13, 23, 1, 2, 3], Gamma[1,2,3], NS[11,22,33,12,13,23, 1, 2, 3]

    if ((resVarAddr.fieldName == "NE") || (resVarAddr.fieldName == "NS") 
        && !(resVarAddr.componentName == "S1" || resVarAddr.componentName == "S2" || resVarAddr.componentName == "S3" ))
    {
        RigFemScalarResultFrames * srcDataFrames = NULL;
        if (resVarAddr.fieldName == "NE"){
            srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "E", resVarAddr.componentName));
        }else if (resVarAddr.fieldName == "NS"){
            srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "S", resVarAddr.componentName));
        }

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


    if (   (resVarAddr.fieldName == "NS" || resVarAddr.fieldName == "ST" )
        && (resVarAddr.componentName == "S1" || resVarAddr.componentName == "S2" || resVarAddr.componentName == "S3" ))
    {
        RigFemScalarResultFrames * ns11Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S11"));
        RigFemScalarResultFrames * ns22Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S22"));
        RigFemScalarResultFrames * ns33Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S33"));
        RigFemScalarResultFrames * ns12Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S12"));
        RigFemScalarResultFrames * ns13Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S13"));
        RigFemScalarResultFrames * ns23Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S23"));

        RigFemScalarResultFrames * ns1Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S1"));
        RigFemScalarResultFrames * ns2Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S2"));
        RigFemScalarResultFrames * ns3Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S3"));

        int frameCount = ns11Frames->frameCount();
        for (int fIdx = 0; fIdx < frameCount; ++fIdx)
        {
            const std::vector<float>& ns11 = ns11Frames->frameData(fIdx);
            const std::vector<float>& ns22 = ns22Frames->frameData(fIdx);
            const std::vector<float>& ns33 = ns33Frames->frameData(fIdx);
            const std::vector<float>& ns12 = ns12Frames->frameData(fIdx);
            const std::vector<float>& ns13 = ns13Frames->frameData(fIdx);
            const std::vector<float>& ns23 = ns23Frames->frameData(fIdx);

            std::vector<float>& ns1 = ns1Frames->frameData(fIdx);
            std::vector<float>& ns2 = ns2Frames->frameData(fIdx);
            std::vector<float>& ns3 = ns3Frames->frameData(fIdx);

            size_t valCount = ns11.size();
            
            ns1.resize(valCount);
            ns2.resize(valCount);
            ns3.resize(valCount);

            for (size_t vIdx = 0; vIdx < valCount; ++vIdx)
            {
                caf::Ten3f T(ns11[vIdx], ns22[vIdx], ns33[vIdx], ns12[vIdx], ns11[vIdx], ns11[vIdx] );

                cvf::Vec3f principals = T.calculatePrincipals(NULL);
                ns1[vIdx] = principals[0];
                ns2[vIdx] = principals[1];
                ns3[vIdx] = principals[2];
            }
        }

        RigFemScalarResultFrames* requestedPrincipal = this->findOrLoadScalarResult(partIndex,resVarAddr);
        
        return  requestedPrincipal;
    }

    if ( resVarAddr.fieldName == "ST" 
        &&  (   resVarAddr.componentName == "S11" 
            ||  resVarAddr.componentName == "S22"  
            ||  resVarAddr.componentName == "S33" ))
    {
        RigFemScalarResultFrames * srcNSDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "NS", resVarAddr.componentName));
        RigFemScalarResultFrames * srcPORDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_NODAL, "POR", ""));

        RigFemScalarResultFrames * dstDataFrames =  m_femPartResults[partIndex]->createScalarResult(resVarAddr);
        const RigFemPart * femPart = m_femParts->part(partIndex);
        int frameCount = srcNSDataFrames->frameCount();
        for (int fIdx = 0; fIdx < frameCount; ++fIdx)
        {
            const std::vector<float>& srcNSFrameData = srcNSDataFrames->frameData(fIdx);
            const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData(fIdx);
            
            std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);

            size_t valCount = srcNSFrameData.size();
            dstFrameData.resize(valCount);
            int nodeIdx = 0;
            for (size_t vIdx = 0; vIdx < valCount; ++vIdx)
            {
                nodeIdx = femPart->nodeIdxFromElementNodeResultIdx(vIdx);
                float por = srcPORFrameData[nodeIdx];
                if (por == std::numeric_limits<float>::infinity()) por = 0.0f;
                dstFrameData[vIdx] = srcNSFrameData[vIdx] + por;
            }
        }
        return dstDataFrames; 
    }

    if ( resVarAddr.fieldName == "ST" 
        &&  (   resVarAddr.componentName == "S12" 
            ||  resVarAddr.componentName == "S13"  
            ||  resVarAddr.componentName == "S23" ))
    {
        return this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "NS", resVarAddr.componentName));
    }

    if (resVarAddr.fieldName == "ST" && resVarAddr.componentName == "")
    {
        // Create and return an empty result
        return m_femPartResults[partIndex]->createScalarResult(resVarAddr);
    }


    if (resVarAddr.fieldName == "Gamma"
        &&  (   resVarAddr.componentName == "Gamma1"
             || resVarAddr.componentName == "Gamma2"
             || resVarAddr.componentName == "Gamma3"
             || resVarAddr.componentName == "Gamma11"
             || resVarAddr.componentName == "Gamma22"
             || resVarAddr.componentName == "Gamma33"
             ))
    {
        RigFemScalarResultFrames * srcDataFrames = NULL;
        if (resVarAddr.componentName == "Gamma1"){
            srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S1"));
        }else if (resVarAddr.componentName == "Gamma2"){
            srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S2"));
        }else if (resVarAddr.componentName == "Gamma3"){
            srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S3"));
        }else if (resVarAddr.componentName == "Gamma11"){
            srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S11"));
        }else if (resVarAddr.componentName == "Gamma22"){
            srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S22"));
        }else if (resVarAddr.componentName == "Gamma33"){
            srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S33"));
        }

        RigFemScalarResultFrames * srcPORDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_NODAL, "POR", ""));

        RigFemScalarResultFrames * dstDataFrames =  m_femPartResults[partIndex]->createScalarResult(resVarAddr);
        
        const RigFemPart * femPart = m_femParts->part(partIndex);
        int frameCount = srcDataFrames->frameCount();
        float inf = std::numeric_limits<float>::infinity();
        
        for (int fIdx = 0; fIdx < frameCount; ++fIdx)
        {
            const std::vector<float>& srcSTFrameData = srcDataFrames->frameData(fIdx);
            const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData(fIdx);
            
            std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);

            size_t valCount = srcSTFrameData.size();
            dstFrameData.resize(valCount);
            int nodeIdx = 0;
            for (size_t vIdx = 0; vIdx < valCount; ++vIdx)
            {
                nodeIdx = femPart->nodeIdxFromElementNodeResultIdx(vIdx);
                float por = srcPORFrameData[nodeIdx];

                if (por == inf || abs(por) < 0.01e6) 
                    dstFrameData[vIdx] = inf;
                else 
                    dstFrameData[vIdx] = srcSTFrameData[vIdx]/por;
            }
        }
        return dstDataFrames; 
    }

    if (resVarAddr.fieldName == "Gamma" && resVarAddr.componentName == "")
    {
        // Create and return an empty result
        return m_femPartResults[partIndex]->createScalarResult(resVarAddr);
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

