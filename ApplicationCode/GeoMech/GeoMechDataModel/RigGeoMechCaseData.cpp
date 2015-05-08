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

#include "RigGeoMechCaseData.h"
#include "RigFemPartCollection.h"
#include "RifGeoMechReaderInterface.h"

#ifdef USE_ODB_API 
#include "RifOdbReader.h"
#endif
#include "RigFemScalarResultFrames.h"
#include "RigStatisticsDataCache.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData::RigGeoMechCaseData(const std::string& fileName)
{
    m_geoMechCaseFileName = fileName;
    m_femParts = new RigFemPartCollection();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigGeoMechCaseData::~RigGeoMechCaseData()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemPartCollection* RigGeoMechCaseData::femParts()
{
    return m_femParts.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const RigFemPartCollection* RigGeoMechCaseData::femParts() const
{
    return m_femParts.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RigGeoMechCaseData::openAndReadFemParts()
{

#ifdef USE_ODB_API    
    m_readerInterface = new RifOdbReader;
#endif

    if (m_readerInterface.notNull() && m_readerInterface->openFile(m_geoMechCaseFileName))
    {
        m_femParts = new RigFemPartCollection();

        if (m_readerInterface->readFemParts(m_femParts.p()))
        {
            // Initialize results containers
            m_femPartResults.resize(m_femParts->partCount());
            std::vector<std::string> stepNames = m_readerInterface->stepNames();
            for (int pIdx = 0; pIdx < m_femPartResults.size(); ++pIdx)
            {
                m_femPartResults[pIdx] = new RigFemPartResults;
                m_femPartResults[pIdx]->initResultStages(stepNames);
            }
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string> > RigGeoMechCaseData::scalarFieldAndComponentNames(RigFemResultPosEnum resPos)
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
RigFemScalarResultFrames* RigGeoMechCaseData::findOrLoadScalarResult(int partIndex, int stepIndex, 
                                                                      const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(partIndex < m_femParts->partCount());
    CVF_ASSERT(m_readerInterface.notNull());

    RigFemScalarResultFrames* frames = m_femPartResults[partIndex]->findScalarResult(stepIndex, resVarAddr);
    if (frames) return frames;

    std::vector<double > frameTimes = m_readerInterface->frameTimes((int)stepIndex);
    frames = m_femPartResults[partIndex]->createScalarResult( stepIndex, resVarAddr, frameTimes);

    for (int fIdx = 0; (size_t)fIdx < frameTimes.size(); ++fIdx)
    {
        std::vector<float>* frameData = &(frames->frameData(fIdx));
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

    return frames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigGeoMechCaseData::stepNames()
{
    CVF_ASSERT(m_readerInterface.notNull());
    return m_readerInterface->stepNames();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::minMaxScalarValues(const RigFemResultAddress& resVarAddr, int stepIndex, int frameIndex, 
                                            double* localMin, double* localMax)
{
    minMaxScalarValuesInternal(resVarAddr, stepIndex, frameIndex, localMin, localMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::minMaxScalarValues(const RigFemResultAddress& resVarAddr, int stepIndex, 
                                            double* globalMin, double* globalMax)
{
    minMaxScalarValuesInternal(resVarAddr, stepIndex, -1, globalMin, globalMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::minMaxScalarValuesInternal(const RigFemResultAddress& resVarAddr, int stepIndex, int frameIndex, double* overallMin, double* overallMax)
{
    CVF_ASSERT(overallMax && overallMin);

    double min = HUGE_VAL;
    double max = -HUGE_VAL;

    for (int pIdx = 0; pIdx < m_femPartResults.size(); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            RigFemScalarResultFrames* frames = m_femPartResults[pIdx]->findScalarResult(stepIndex, resVarAddr);
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
void RigGeoMechCaseData::posNegClosestToZero(const RigFemResultAddress& resVarAddr, int stepIndex, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero)
{
    posNegClosestToZeroInternal(resVarAddr, stepIndex, frameIndex, localPosClosestToZero, localNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::posNegClosestToZero(const RigFemResultAddress& resVarAddr, int stepIndex, double* globalPosClosestToZero, double* globalNegClosestToZero)
{
    posNegClosestToZeroInternal(resVarAddr, stepIndex, -1, globalPosClosestToZero, globalNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::posNegClosestToZeroInternal(const RigFemResultAddress& resVarAddr, int stepIndex, int frameIndex, 
                                                     double* overallPosClosestToZero, double* overallNegClosestToZero)
{
    CVF_ASSERT(overallPosClosestToZero && overallNegClosestToZero);

    double posClosestToZero = HUGE_VAL;
    double negClosestToZero = -HUGE_VAL;

    for (int pIdx = 0; pIdx < m_femPartResults.size(); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            RigFemScalarResultFrames* frames = m_femPartResults[pIdx]->findScalarResult(stepIndex, resVarAddr);
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
size_t RigGeoMechCaseData::frameCount(int stepIndex, const RigFemResultAddress& resVarAddr)
{
    size_t maxFrameCount = 0;
    for (int pIdx = 0; pIdx < m_femPartResults.size(); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            RigFemScalarResultFrames* frames = m_femPartResults[pIdx]->findScalarResult(stepIndex, resVarAddr);
            if (frames)
            {
                size_t frameCount = frames->frameCount();
                if (frameCount > maxFrameCount) maxFrameCount = frameCount;
            }
        }
    }

    return maxFrameCount;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::assertResultsLoaded(int stepIndex, const RigFemResultAddress& resVarAddr)
{
    for (int pIdx = 0; pIdx < m_femPartResults.size(); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            findOrLoadScalarResult(pIdx, stepIndex, resVarAddr);
        }
    }
}

