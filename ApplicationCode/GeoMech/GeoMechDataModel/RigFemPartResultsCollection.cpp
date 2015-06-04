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
        }
        else if (resPos == RIG_INTEGRATION_POINT)
        {
            fieldCompNames = m_readerInterface->scalarIntegrationPointFieldAndComponentNames();
        }
    }

    return fieldCompNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::findOrLoadScalarResult(int partIndex,
                                                                      const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(partIndex < m_femPartResults.size());
    CVF_ASSERT(m_readerInterface.notNull());

    RigFemScalarResultFrames* frames = m_femPartResults[partIndex]->findScalarResult(resVarAddr);
    if (frames) return frames;

    std::vector<std::string> stepNames =  m_readerInterface->stepNames();
    frames = m_femPartResults[partIndex]->createScalarResult( resVarAddr);

    for (int stepIndex = 0; stepIndex < static_cast<int>(stepNames.size()); ++stepIndex)
    {
    std::vector<double > frameTimes = m_readerInterface->frameTimes(stepIndex);

    for (int fIdx = 1; (size_t)fIdx < frameTimes.size() && fIdx < 2 ; ++fIdx)  // Read only the second frame
    {
        std::vector<float>* frameData = &(frames->frameData(stepIndex));
        switch (resVarAddr.resultPosType)
        {
            case RIG_NODAL:
                m_readerInterface->readScalarNodeField(resVarAddr.fieldName, resVarAddr.componentName, partIndex, stepIndex, fIdx, frameData);
            break;
            case RIG_ELEMENT_NODAL:
                m_readerInterface->readScalarElementNodeField(resVarAddr.fieldName, resVarAddr.componentName, partIndex, stepIndex, fIdx, frameData);
            break;
            case RIG_INTEGRATION_POINT:
                m_readerInterface->readScalarIntegrationPointField(resVarAddr.fieldName, resVarAddr.componentName, partIndex, stepIndex, fIdx, frameData);
           break;
        }
    }
    }
    return frames;
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
void RigFemPartResultsCollection::minMaxScalarValues(const RigFemResultAddress& resVarAddr, int frameIndex, 
                                            double* localMin, double* localMax)
{
    minMaxScalarValuesInternal(resVarAddr, frameIndex, localMin, localMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::minMaxScalarValues(const RigFemResultAddress& resVarAddr,  
                                            double* globalMin, double* globalMax)
{
    minMaxScalarValuesInternal(resVarAddr, -1, globalMin, globalMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::minMaxScalarValuesInternal(const RigFemResultAddress& resVarAddr, int frameIndex, double* overallMin, double* overallMax)
{
    CVF_ASSERT(overallMax && overallMin);

    double min = HUGE_VAL;
    double max = -HUGE_VAL;

    for (int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            RigFemScalarResultFrames* frames = findOrLoadScalarResult(pIdx, resVarAddr);
            if (frames)
            {
                double lmin; 
                double lmax;

                RigStatisticsDataCache* stats =  frames->statistics();
                if (frameIndex == -1)
                {
                    stats->minMaxCellScalarValues(lmin, lmax);
                }
                else
                {
                    stats->minMaxCellScalarValues(frameIndex, lmin, lmax);
                }
                min = lmin < min ? lmin: min;
                max = lmax > max ? lmax: max;
            }
        }
    }

    *overallMax = max;
    *overallMin = min;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZero(const RigFemResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero)
{
    posNegClosestToZeroInternal(resVarAddr, frameIndex, localPosClosestToZero, localNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZero(const RigFemResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero)
{
    posNegClosestToZeroInternal(resVarAddr, -1, globalPosClosestToZero, globalNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZeroInternal(const RigFemResultAddress& resVarAddr, int frameIndex, 
                                                     double* overallPosClosestToZero, double* overallNegClosestToZero)
{
    CVF_ASSERT(overallPosClosestToZero && overallNegClosestToZero);

    double posClosestToZero = HUGE_VAL;
    double negClosestToZero = -HUGE_VAL;

    for (int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            RigFemScalarResultFrames* frames = findOrLoadScalarResult(pIdx, resVarAddr);
            if (frames)
            {
                double partNeg, partPos;

                RigStatisticsDataCache* stats =  frames->statistics();
                if (frameIndex == -1)
                {
                    stats->posNegClosestToZero(partPos, partNeg);
                }
                else
                {
                    stats->posNegClosestToZero(frameIndex, partPos, partNeg);
                }
               
                if (partNeg > negClosestToZero && partNeg < 0) negClosestToZero = partNeg;
                if (partPos < posClosestToZero && partPos > 0) posClosestToZero = partPos;
                
            }
        }
    }

    *overallPosClosestToZero = posClosestToZero;
    *overallNegClosestToZero = negClosestToZero; 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::meanCellScalarValues(const RigFemResultAddress& resVarAddr, double* meanValue)
{
    CVF_ASSERT(meanValue);

    double mean = 0;
    size_t meanContribCount = 0;

    for (int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            RigFemScalarResultFrames* frames = findOrLoadScalarResult(pIdx, resVarAddr);
            if (frames)
            {
                double localMean = 0; 

                RigStatisticsDataCache* stats = frames->statistics();
                stats->meanCellScalarValues(localMean);

                mean += localMean;
                meanContribCount++;
            }
        }
    }

    *meanValue = meanContribCount > 0 ? mean/meanContribCount : 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RigFemPartResultsCollection::frameCount()
{
    return static_cast<int>(stepNames().size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::assertResultsLoaded( const RigFemResultAddress& resVarAddr)
{
    for (int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            findOrLoadScalarResult(pIdx, resVarAddr);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<float>& RigFemPartResultsCollection::resultValues(const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex)
{
    RigFemScalarResultFrames* scalarResults = findOrLoadScalarResult(partIndex, resVarAddr);
    return scalarResults->frameData(frameIndex);
}


