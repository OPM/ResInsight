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

#include <stdlib.h>
#include "RigGeoMechCaseData.h"
#include "RigFemPartCollection.h"
#include "RifGeoMechReaderInterface.h"

#ifdef USE_ODB_API 
#include "RifOdbReader.h"
#endif
#include "RigFemScalarResultFrames.h"
#include "RigStatisticsDataCache.h"
#include <cmath>
#include "cvfBoundingBox.h"
#include "cafProgressInfo.h"
#include <QString>

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

        caf::ProgressInfo progress(10, ""); // Here because the next call uses progress
        progress.setNextProgressIncrement(9);
        if (m_readerInterface->readFemParts(m_femParts.p()))
        {
            progress.incrementProgress();
            progress.setProgressDescription("Calculating element neighbors");

            // Initialize results containers
            m_femPartResults.resize(m_femParts->partCount());
            std::vector<std::string> stepNames = m_readerInterface->stepNames();
            for (int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx)
            {
                m_femPartResults[pIdx] = new RigFemPartResults;
                m_femPartResults[pIdx]->initResultSteps(stepNames);
            }

            // Calculate derived Fem data
            for (int pIdx = 0; pIdx < m_femParts->partCount(); ++pIdx)
            {
                m_femParts->part(pIdx)->assertNodeToElmIndicesIsCalculated();
                m_femParts->part(pIdx)->assertElmNeighborsIsCalculated();
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
RigFemScalarResultFrames* RigGeoMechCaseData::findOrLoadScalarResult(int partIndex,
                                                                      const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(partIndex < m_femParts->partCount());
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
std::vector<std::string> RigGeoMechCaseData::stepNames()
{
    CVF_ASSERT(m_readerInterface.notNull());
    return m_readerInterface->stepNames();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::minMaxScalarValues(const RigFemResultAddress& resVarAddr, int frameIndex, 
                                            double* localMin, double* localMax)
{
    minMaxScalarValuesInternal(resVarAddr, frameIndex, localMin, localMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::minMaxScalarValues(const RigFemResultAddress& resVarAddr,  
                                            double* globalMin, double* globalMax)
{
    minMaxScalarValuesInternal(resVarAddr, -1, globalMin, globalMax);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::minMaxScalarValuesInternal(const RigFemResultAddress& resVarAddr, int frameIndex, double* overallMin, double* overallMax)
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
void RigGeoMechCaseData::posNegClosestToZero(const RigFemResultAddress& resVarAddr, int frameIndex, double* localPosClosestToZero, double* localNegClosestToZero)
{
    posNegClosestToZeroInternal(resVarAddr, frameIndex, localPosClosestToZero, localNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::posNegClosestToZero(const RigFemResultAddress& resVarAddr, double* globalPosClosestToZero, double* globalNegClosestToZero)
{
    posNegClosestToZeroInternal(resVarAddr, -1, globalPosClosestToZero, globalNegClosestToZero);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::posNegClosestToZeroInternal(const RigFemResultAddress& resVarAddr, int frameIndex, 
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
int RigGeoMechCaseData::frameCount()
{
    return static_cast<int>(stepNames().size());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigGeoMechCaseData::assertResultsLoaded( const RigFemResultAddress& resVarAddr)
{
    for (int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx)
    {
        if (m_femPartResults[pIdx].notNull())
        {
            findOrLoadScalarResult(pIdx, resVarAddr);
        }
    }
}
