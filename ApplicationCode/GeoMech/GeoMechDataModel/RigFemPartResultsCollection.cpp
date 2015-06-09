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
    CVF_ASSERT(partIndex < (int)(m_femPartResults.size()));
    CVF_ASSERT(m_readerInterface.notNull());
    CVF_ASSERT(resVarAddr.isValid());

    RigFemScalarResultFrames* frames = m_femPartResults[partIndex]->findScalarResult(resVarAddr);
    if (frames) return frames;

    std::vector<std::string> stepNames =  m_readerInterface->stepNames();
    frames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

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


