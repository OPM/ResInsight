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

#include "RiaApplication.h"

#include "RigFemNativeStatCalc.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResults.h"
#include "RigFemScalarResultFrames.h"
#include "RigFormationNames.h"
#include "RigStatisticsDataCache.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"

#include "cafProgressInfo.h"
#include "cvfBoundingBox.h"

#include "cafProgressInfo.h"
#include "cafTensor3.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"

#include <QString>

#include <stdlib.h>
#include <cmath>


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

    m_cohesion = 10.0;
    m_frictionAngleRad = cvf::Math::toRadians(30.0);

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
void RigFemPartResultsCollection::setActiveFormationNames(RigFormationNames* activeFormationNames)
{
    m_activeFormationNamesData  = activeFormationNames;

    RimProject* project = RiaApplication::instance()->project();
    if (project)
    {
        if (project->mainPlotCollection())
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if (plotCollection)
            {
                for (RimWellLogPlot* wellLogPlot : plotCollection->wellLogPlots)
                {
                    wellLogPlot->loadDataAndUpdate();
                }
            }
        }
    }

    this->deleteResult(RigFemResultAddress(RIG_FORMATION_NAMES, "Active Formation Names", ""));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFormationNames* RigFemPartResultsCollection::activeFormationNames()
{
    return m_activeFormationNamesData.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setCalculationParameters(double cohesion, double frictionAngleRad)
{
    m_cohesion = cohesion;
    m_frictionAngleRad = frictionAngleRad;

    // Todo, delete all dependent results
    this->deleteResult(RigFemResultAddress(RIG_ELEMENT_NODAL,     "SE", "SFI", RigFemResultAddress::ALL_TIME_LAPSES));
    this->deleteResult(RigFemResultAddress(RIG_INTEGRATION_POINT, "SE", "SFI", RigFemResultAddress::ALL_TIME_LAPSES));
    this->deleteResult(RigFemResultAddress(RIG_ELEMENT_NODAL,     "SE", "DSM", RigFemResultAddress::ALL_TIME_LAPSES));
    this->deleteResult(RigFemResultAddress(RIG_INTEGRATION_POINT, "SE", "DSM", RigFemResultAddress::ALL_TIME_LAPSES));
    this->deleteResult(RigFemResultAddress(RIG_ELEMENT_NODAL,     "SE", "FOS", RigFemResultAddress::ALL_TIME_LAPSES));
    this->deleteResult(RigFemResultAddress(RIG_INTEGRATION_POINT, "SE", "FOS", RigFemResultAddress::ALL_TIME_LAPSES));

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

    if (resultAddressOfComponents.size())
    {
        std::vector<RigFemScalarResultFrames*> resultsForEachComponent;
        for (size_t cIdx = 0; cIdx < resultAddressOfComponents.size(); ++cIdx)
        {
            resultsForEachComponent.push_back(m_femPartResults[partIndex]->createScalarResult(resultAddressOfComponents[cIdx]));
        }

        int frameCount = this->frameCount();
        caf::ProgressInfo progress(frameCount, "");
        progress.setProgressDescription(QString("Loading Native Result %1 %2").arg(resVarAddr.fieldName.c_str(), resVarAddr.componentName.c_str()));

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
                        m_readerInterface->readNodeField(resVarAddr.fieldName, partIndex, stepIndex, fIdx, &componentDataVectors);
                        break;
                    case RIG_ELEMENT_NODAL:
                        m_readerInterface->readElementNodeField(resVarAddr.fieldName, partIndex, stepIndex, fIdx, &componentDataVectors);
                        break;
                    case RIG_INTEGRATION_POINT:
                        m_readerInterface->readIntegrationPointField(resVarAddr.fieldName, partIndex, stepIndex, fIdx, &componentDataVectors);
                        break;
                }
            }

            progress.incrementProgress();
        }

        // Now fetch the particular component requested, which should now exist and be read.
        frames = m_femPartResults[partIndex]->findScalarResult(resVarAddr);
    }

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

    if (resPos == RIG_FORMATION_NAMES)
    {
        if (activeFormationNames()) fieldCompNames["Active Formation Names"];
    }

    if (m_readerInterface.notNull())
    {
        if (resPos == RIG_NODAL)
        {
            fieldCompNames = m_readerInterface->scalarNodeFieldAndComponentNames();
            fieldCompNames["POR-Bar"];
        }
        else if (resPos == RIG_ELEMENT_NODAL)
        {
            fieldCompNames = m_readerInterface->scalarElementNodeFieldAndComponentNames();

            fieldCompNames["SE"].push_back("SEM");
            fieldCompNames["SE"].push_back("SFI");
            fieldCompNames["SE"].push_back("DSM");
            fieldCompNames["SE"].push_back("FOS");

            fieldCompNames["SE"].push_back("S11");
            fieldCompNames["SE"].push_back("S22");
            fieldCompNames["SE"].push_back("S33");
            fieldCompNames["SE"].push_back("S12");
            fieldCompNames["SE"].push_back("S13");
            fieldCompNames["SE"].push_back("S23");
            fieldCompNames["SE"].push_back("S1");
            fieldCompNames["SE"].push_back("S2");
            fieldCompNames["SE"].push_back("S3");

            fieldCompNames["SE"].push_back("S1inc");
            fieldCompNames["SE"].push_back("S1azi");
            fieldCompNames["SE"].push_back("S2inc");
            fieldCompNames["SE"].push_back("S2azi");
            fieldCompNames["SE"].push_back("S3inc");
            fieldCompNames["SE"].push_back("S3azi");

            fieldCompNames["ST"].push_back("STM");
            fieldCompNames["ST"].push_back("Q");

            fieldCompNames["ST"].push_back("S11");
            fieldCompNames["ST"].push_back("S22");
            fieldCompNames["ST"].push_back("S33");
            fieldCompNames["ST"].push_back("S12");
            fieldCompNames["ST"].push_back("S13");
            fieldCompNames["ST"].push_back("S23");
            fieldCompNames["ST"].push_back("S1");
            fieldCompNames["ST"].push_back("S2");
            fieldCompNames["ST"].push_back("S3");

            fieldCompNames["ST"].push_back("S1inc");
            fieldCompNames["ST"].push_back("S1azi");
            fieldCompNames["ST"].push_back("S2inc");
            fieldCompNames["ST"].push_back("S2azi");
            fieldCompNames["ST"].push_back("S3inc");
            fieldCompNames["ST"].push_back("S3azi");

            fieldCompNames["Gamma"].push_back("Gamma1");
            fieldCompNames["Gamma"].push_back("Gamma2");
            fieldCompNames["Gamma"].push_back("Gamma3");
            fieldCompNames["Gamma"].push_back("Gamma11");
            fieldCompNames["Gamma"].push_back("Gamma22");
            fieldCompNames["Gamma"].push_back("Gamma33");

            fieldCompNames["NE"].push_back("EV");
            fieldCompNames["NE"].push_back("ED");
            fieldCompNames["NE"].push_back("E11");
            fieldCompNames["NE"].push_back("E22");
            fieldCompNames["NE"].push_back("E33");
            fieldCompNames["NE"].push_back("E12");
            fieldCompNames["NE"].push_back("E13");
            fieldCompNames["NE"].push_back("E23");
            fieldCompNames["NE"].push_back("E1");
            fieldCompNames["NE"].push_back("E2");
            fieldCompNames["NE"].push_back("E3");

       }
        else if (resPos == RIG_INTEGRATION_POINT)
        {
            fieldCompNames = m_readerInterface->scalarIntegrationPointFieldAndComponentNames();
           
            fieldCompNames["SE"].push_back("SEM");
            fieldCompNames["SE"].push_back("SFI");
            fieldCompNames["SE"].push_back("DSM");
            fieldCompNames["SE"].push_back("FOS");

            fieldCompNames["SE"].push_back("S11");
            fieldCompNames["SE"].push_back("S22");
            fieldCompNames["SE"].push_back("S33");
            fieldCompNames["SE"].push_back("S12");
            fieldCompNames["SE"].push_back("S13");
            fieldCompNames["SE"].push_back("S23");
            fieldCompNames["SE"].push_back("S1");
            fieldCompNames["SE"].push_back("S2");
            fieldCompNames["SE"].push_back("S3");

            fieldCompNames["SE"].push_back("S1inc");
            fieldCompNames["SE"].push_back("S1azi");
            fieldCompNames["SE"].push_back("S2inc");
            fieldCompNames["SE"].push_back("S2azi");
            fieldCompNames["SE"].push_back("S3inc");
            fieldCompNames["SE"].push_back("S3azi");

            fieldCompNames["ST"].push_back("STM");
            fieldCompNames["ST"].push_back("Q");

            fieldCompNames["ST"].push_back("S11");
            fieldCompNames["ST"].push_back("S22");
            fieldCompNames["ST"].push_back("S33");
            fieldCompNames["ST"].push_back("S12");
            fieldCompNames["ST"].push_back("S13");
            fieldCompNames["ST"].push_back("S23");
            fieldCompNames["ST"].push_back("S1");
            fieldCompNames["ST"].push_back("S2");
            fieldCompNames["ST"].push_back("S3");

            fieldCompNames["ST"].push_back("S1inc");
            fieldCompNames["ST"].push_back("S1azi");
            fieldCompNames["ST"].push_back("S2inc");
            fieldCompNames["ST"].push_back("S2azi");
            fieldCompNames["ST"].push_back("S3inc");
            fieldCompNames["ST"].push_back("S3azi");

            fieldCompNames["Gamma"].push_back("Gamma1");
            fieldCompNames["Gamma"].push_back("Gamma2");
            fieldCompNames["Gamma"].push_back("Gamma3");
            fieldCompNames["Gamma"].push_back("Gamma11");
            fieldCompNames["Gamma"].push_back("Gamma22");
            fieldCompNames["Gamma"].push_back("Gamma33");

            fieldCompNames["NE"].push_back("EV");
            fieldCompNames["NE"].push_back("ED");
            fieldCompNames["NE"].push_back("E11");
            fieldCompNames["NE"].push_back("E22");
            fieldCompNames["NE"].push_back("E33");
            fieldCompNames["NE"].push_back("E12");
            fieldCompNames["NE"].push_back("E13");
            fieldCompNames["NE"].push_back("E23");
            fieldCompNames["NE"].push_back("E1");
            fieldCompNames["NE"].push_back("E2");
            fieldCompNames["NE"].push_back("E3");

        }
        else if(resPos == RIG_ELEMENT_NODAL_FACE)
        {
            fieldCompNames["Plane"].push_back("Pinc");
            fieldCompNames["Plane"].push_back("Pazi");

            fieldCompNames["SE"].push_back("SN");
            fieldCompNames["SE"].push_back("TP");
            fieldCompNames["SE"].push_back("TPinc");
            fieldCompNames["SE"].push_back("TPH" );
            fieldCompNames["SE"].push_back("TPQV");
            fieldCompNames["SE"].push_back("FAULTMOB");
            fieldCompNames["SE"].push_back("PCRIT");

            fieldCompNames["ST"].push_back("SN");
            fieldCompNames["ST"].push_back("TP");
            fieldCompNames["ST"].push_back("TPinc");
            fieldCompNames["ST"].push_back("TPH");
            fieldCompNames["ST"].push_back("TPQV");
            fieldCompNames["ST"].push_back("FAULTMOB");
            fieldCompNames["ST"].push_back("PCRIT");

        }
    }

    return fieldCompNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateBarConvertedResult(int partIndex, const RigFemResultAddress &convertedResultAddr, const std::string& fieldNameToConvert)
{
    caf::ProgressInfo frameCountProgress(this->frameCount() * 2, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(convertedResultAddr.fieldName + ": " + convertedResultAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(convertedResultAddr.resultPosType, fieldNameToConvert, convertedResultAddr.componentName));
    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(convertedResultAddr);

    frameCountProgress.incrementProgress();

    int frameCount = srcDataFrames->frameCount();
    for (int fIdx = 0; fIdx < frameCount; ++fIdx)
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData(fIdx);
        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = srcFrameData.size();
        dstFrameData.resize(valCount);

        for (size_t vIdx = 0; vIdx < valCount; ++vIdx)
        {
            dstFrameData[vIdx] = 1.0e-5*srcFrameData[vIdx];
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// Convert POR NODAL result to POR-Bar Elment Nodal result
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateEnIpPorBarResult(int partIndex, const RigFemResultAddress &convertedResultAddr)
{
    caf::ProgressInfo frameCountProgress(this->frameCount() * 2, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(convertedResultAddr.fieldName + ": " + convertedResultAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_NODAL, "POR", ""));
    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(convertedResultAddr);

    frameCountProgress.incrementProgress();

    const RigFemPart * femPart = m_femParts->part(partIndex);
    float inf = std::numeric_limits<float>::infinity();

    int frameCount = srcDataFrames->frameCount();
    for (int fIdx = 0; fIdx < frameCount; ++fIdx)
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData(fIdx);
        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        
        if (!srcFrameData.size()) continue; // Create empty results if we have no POR result.

        size_t valCount = femPart->elementNodeResultCount();
        dstFrameData.resize(valCount, inf);

        int elementCount = femPart->elementCount();
        for (int elmIdx = 0; elmIdx < elementCount; ++elmIdx)
        {
            RigElementType elmType = femPart->elementType(elmIdx);

            int elmNodeCount = RigFemTypes::elmentNodeCount(elmType);

            if (elmType == HEX8P)
            {
                for (int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx)
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx(elmIdx, elmNodIdx);
                    int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx(elmNodResIdx);
                    dstFrameData[elmNodResIdx] = 1.0e-5*srcFrameData[nodeIdx];
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateTimeLapseResult(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.isTimeLapse());

    if (resVarAddr.fieldName != "Gamma")
    {
        caf::ProgressInfo frameCountProgress(this->frameCount() * 2, "");
        frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemResultAddress resVarNative(resVarAddr.resultPosType, resVarAddr.fieldName, resVarAddr.componentName);
        RigFemScalarResultFrames * srcDataFrames = this->findOrLoadScalarResult(partIndex, resVarNative);
        RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

        frameCountProgress.incrementProgress();

        int frameCount = srcDataFrames->frameCount();
        int baseFrameIdx = resVarAddr.timeLapseBaseFrameIdx;
        if ( baseFrameIdx >= frameCount ) return dstDataFrames;
        const std::vector<float>& baseFrameData = srcDataFrames->frameData(baseFrameIdx);

        for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
        {
            const std::vector<float>& srcFrameData = srcDataFrames->frameData(fIdx);
            std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
            size_t valCount = srcFrameData.size();
            dstFrameData.resize(valCount);

            for ( size_t vIdx = 0; vIdx < valCount; ++vIdx )
            {
                dstFrameData[vIdx] = srcFrameData[vIdx] - baseFrameData[vIdx];
            }

            frameCountProgress.incrementProgress();
        }

        return dstDataFrames;
    }
    else
    {
        // Gamma time lapse needs to be calculated as ST_dt / POR_dt and not Gamma - Gamma_baseFrame see github issue #937
        
        caf::ProgressInfo frameCountProgress(this->frameCount() * 3, "");
        frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemResultAddress totStressCompAddr(resVarAddr.resultPosType, "ST", "", resVarAddr.timeLapseBaseFrameIdx);
        {
            std::string scomp;
            std::string gcomp = resVarAddr.componentName;
            if ( gcomp == "Gamma1" )       scomp =  "S1";
            else if ( gcomp == "Gamma2" )  scomp =  "S2";
            else if ( gcomp == "Gamma3" )  scomp =  "S3";
            else if ( gcomp == "Gamma11" ) scomp =  "S11";
            else if ( gcomp == "Gamma22" ) scomp =  "S22";
            else if ( gcomp == "Gamma33" ) scomp =  "S33";
            totStressCompAddr.componentName = scomp;
        }

        RigFemScalarResultFrames * srcDataFrames = this->findOrLoadScalarResult(partIndex, totStressCompAddr);
        frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
        RigFemScalarResultFrames * srcPORDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_NODAL, "POR-Bar", "", resVarAddr.timeLapseBaseFrameIdx));
        RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

        frameCountProgress.incrementProgress();

        calculateGammaFromFrames(partIndex, srcDataFrames, srcPORDataFrames, dstDataFrames, &frameCountProgress); 

        return dstDataFrames;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateMeanStressSEM(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SEM");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 4, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());
    
    RigFemScalarResultFrames * sa11 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "SE", "S11"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * sa22 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "SE", "S22"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * sa33 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "SE", "S33"));

    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

    frameCountProgress.incrementProgress();

    int frameCount = sa11->frameCount();
    for(int fIdx = 0; fIdx < frameCount; ++fIdx)
    {
        const std::vector<float>& sa11Data = sa11->frameData(fIdx);
        const std::vector<float>& sa22Data = sa22->frameData(fIdx);
        const std::vector<float>& sa33Data = sa33->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = sa11Data.size();
        dstFrameData.resize(valCount);

        for(size_t vIdx = 0; vIdx < valCount; ++vIdx)
        {
            dstFrameData[vIdx] = (sa11Data[vIdx] + sa22Data[vIdx] + sa33Data[vIdx])/3.0f;
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateSFI(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SFI");
    caf::ProgressInfo frameCountProgress(this->frameCount() * 3, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * se1Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "SE", "S1"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * se3Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "SE", "S3"));

    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);
    
    frameCountProgress.incrementProgress();

    float cohPrFricAngle = (float)(m_cohesion/tan(m_frictionAngleRad));
    float sinFricAng = sin(m_frictionAngleRad);
     
    int frameCount = se1Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& se1Data = se1Frames->frameData(fIdx);
        const std::vector<float>& se3Data = se3Frames->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = se1Data.size();
        dstFrameData.resize(valCount);

        for ( size_t vIdx = 0; vIdx < valCount; ++vIdx )
        {
            float se1 = se1Data[vIdx];
            float se3 = se3Data[vIdx];
            float se1Se3Diff = se1-se3;

            if ( fabs(se1Se3Diff) < 1e-7 )
            {
                dstFrameData[vIdx] = std::numeric_limits<float>::infinity();
            }
            else
            {
                dstFrameData[vIdx] =  ((cohPrFricAngle + 0.5*(se1Data[vIdx] + se3Data[vIdx])) * sinFricAng ) / (0.5*(se1Data[vIdx] - se3Data[vIdx]));
            }
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDSM(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.fieldName == "SE" && resVarAddr.componentName == "DSM");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 3, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * se1Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "SE", "S1"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * se3Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "SE", "S3"));

    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

    frameCountProgress.incrementProgress();

    float tanFricAng = tan(m_frictionAngleRad);
    float cohPrTanFricAngle = (float)(m_cohesion/tanFricAng);
    int frameCount = se1Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& se1Data = se1Frames->frameData(fIdx);
        const std::vector<float>& se3Data = se3Frames->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = se1Data.size();
        dstFrameData.resize(valCount);

        for ( size_t vIdx = 0; vIdx < valCount; ++vIdx )
        {
            float se1 = se1Data[vIdx];
            float se3 = se3Data[vIdx];
            float pi_4 = 0.785398163397448309616f;
            float rho = 2.0f * ( atan( sqrt(( se1 + cohPrTanFricAngle)/(se3 + cohPrTanFricAngle)) ) - pi_4);
            
            {
                dstFrameData[vIdx] =  tan(rho)/tanFricAng;
            }
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateFOS(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.fieldName == "SE" && resVarAddr.componentName == "FOS");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 2, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * dsmFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "SE", "DSM"));

    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

    frameCountProgress.incrementProgress();

    int frameCount = dsmFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& dsmData = dsmFrames->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = dsmData.size();
        dstFrameData.resize(valCount);

        for ( size_t vIdx = 0; vIdx < valCount; ++vIdx )
        {
            float dsm = dsmData[vIdx];
            dstFrameData[vIdx] = 1.0f/dsm;
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateMeanStressSTM(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.fieldName == "ST" && resVarAddr.componentName == "STM");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 4, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());
    
    RigFemScalarResultFrames * st11 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S11"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * st22 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S22"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * st33 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S33"));

    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

    frameCountProgress.incrementProgress();

    int frameCount = st11->frameCount();
    for(int fIdx = 0; fIdx < frameCount; ++fIdx)
    {
        const std::vector<float>& st11Data = st11->frameData(fIdx);
        const std::vector<float>& st22Data = st22->frameData(fIdx);
        const std::vector<float>& st33Data = st33->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = st11Data.size();
        dstFrameData.resize(valCount);

        for(size_t vIdx = 0; vIdx < valCount; ++vIdx)
        {
            dstFrameData[vIdx] = (st11Data[vIdx] + st22Data[vIdx] + st33Data[vIdx])/3.0f;
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDeviatoricStress(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.fieldName == "ST"  && resVarAddr.componentName == "Q");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 5, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * st11 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S1"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * st22 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S2"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * st33 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "S3"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * stm = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "ST", "STM"));

    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

    frameCountProgress.incrementProgress();

    int frameCount = st11->frameCount();
    for(int fIdx = 0; fIdx < frameCount; ++fIdx)
    {
        const std::vector<float>& st11Data = st11->frameData(fIdx);
        const std::vector<float>& st22Data = st22->frameData(fIdx);
        const std::vector<float>& st33Data = st33->frameData(fIdx);

        const std::vector<float>& stmData = stm->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = st11Data.size();
        dstFrameData.resize(valCount);

        for(size_t vIdx = 0; vIdx < valCount; ++vIdx)
        {
            float stmVal = stmData[vIdx];
            float st11Corr = st11Data[vIdx] - stmVal;
            float st22Corr = st22Data[vIdx] - stmVal;
            float st33Corr = st33Data[vIdx] - stmVal;

            dstFrameData[vIdx] = sqrt (1.5*(st11Corr*st11Corr + st22Corr*st22Corr + st33Corr*st33Corr));
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateVolumetricStrain(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.fieldName == "NE" && resVarAddr.componentName == "EV");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 4, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * ea11 = nullptr;
    RigFemScalarResultFrames * ea22 = nullptr;
    RigFemScalarResultFrames * ea33 = nullptr;

    {
        ea11 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "NE", "E11"));
        frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
        ea22 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "NE", "E22"));
        frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
        ea33 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "NE", "E33"));
    }

    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

    frameCountProgress.incrementProgress();

    int frameCount = ea11->frameCount();
    for(int fIdx = 0; fIdx < frameCount; ++fIdx)
    {
        const std::vector<float>& ea11Data = ea11->frameData(fIdx);
        const std::vector<float>& ea22Data = ea22->frameData(fIdx);
        const std::vector<float>& ea33Data = ea33->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = ea11Data.size();
        dstFrameData.resize(valCount);

        for(size_t vIdx = 0; vIdx < valCount; ++vIdx)
        {
            dstFrameData[vIdx] = (ea11Data[vIdx] + ea22Data[vIdx] + ea33Data[vIdx]);
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDeviatoricStrain(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.fieldName == "NE" && resVarAddr.componentName == "ED");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 3, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * ea11 = nullptr;
    RigFemScalarResultFrames * ea33 = nullptr;
    {
        ea11 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "NE", "E1"));
        frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
        ea33 = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "NE", "E3"));
    }

    RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

    frameCountProgress.incrementProgress();

    int frameCount = ea11->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& ea11Data = ea11->frameData(fIdx);
        const std::vector<float>& ea33Data = ea33->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
        size_t valCount = ea11Data.size();
        dstFrameData.resize(valCount);

        for ( size_t vIdx = 0; vIdx < valCount; ++vIdx )
        {
            dstFrameData[vIdx] = 0.666666666666667f*(ea11Data[vIdx] - ea33Data[vIdx]);
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateSurfaceAlignedStress(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(   resVarAddr.componentName == "STH" || resVarAddr.componentName == "STQV" || resVarAddr.componentName == "SN"
               || resVarAddr.componentName == "TPH" || resVarAddr.componentName == "TPQV" || resVarAddr.componentName == "THQV"
               || resVarAddr.componentName == "TP"  || resVarAddr.componentName == "TPinc"
               || resVarAddr.componentName == "FAULTMOB" || resVarAddr.componentName == "PCRIT");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 7, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());

    RigFemScalarResultFrames * s11Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S11"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s22Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S22"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s33Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S33"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s12Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S12"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s23Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S23"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s13Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S13"));

    RigFemScalarResultFrames * SNFrames       = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "SN"));
    RigFemScalarResultFrames * STHFrames      = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "STH"));
    RigFemScalarResultFrames * STQVFrames     = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "STQV"));
    RigFemScalarResultFrames * TNHFrames      = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "TPH" ));
    RigFemScalarResultFrames * TNQVFrames     = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "TPQV"));
    RigFemScalarResultFrames * THQVFrames     = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "THQV"));
    RigFemScalarResultFrames * TPFrames       = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "TP"));
    RigFemScalarResultFrames * TPincFrames    = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "TPinc"));
    RigFemScalarResultFrames * FAULTMOBFrames = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "FAULTMOB"));
    RigFemScalarResultFrames * PCRITFrames    = m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "PCRIT"));

    frameCountProgress.incrementProgress();

    const RigFemPart * femPart = m_femParts->part(partIndex);
    const std::vector<cvf::Vec3f>& nodeCoordinates = femPart->nodes().coordinates;

    float tanFricAng = tan(m_frictionAngleRad);
    float cohPrTanFricAngle = (float)(m_cohesion/tanFricAng);

    int frameCount = s11Frames->frameCount();
    for(int fIdx = 0; fIdx < frameCount; ++fIdx)
    {
        const std::vector<float>& s11 = s11Frames->frameData(fIdx);
        const std::vector<float>& s22 = s22Frames->frameData(fIdx);
        const std::vector<float>& s33 = s33Frames->frameData(fIdx);
        const std::vector<float>& s12 = s12Frames->frameData(fIdx);
        const std::vector<float>& s23 = s23Frames->frameData(fIdx);
        const std::vector<float>& s13 = s13Frames->frameData(fIdx);

        std::vector<float>& SNDat       = SNFrames->frameData(fIdx);
        std::vector<float>& STHDat      = STHFrames->frameData(fIdx);
        std::vector<float>& STQVDat     = STQVFrames->frameData(fIdx);
        std::vector<float>& TNHDat      = TNHFrames->frameData(fIdx);
        std::vector<float>& TNQVDat     = TNQVFrames->frameData(fIdx);
        std::vector<float>& THQVDat     = THQVFrames->frameData(fIdx);
        std::vector<float>& TPDat       = TPFrames->frameData(fIdx);
        std::vector<float>& TincDat     = TPincFrames->frameData(fIdx);
        std::vector<float>& FAULTMOBDat = FAULTMOBFrames->frameData(fIdx);
        std::vector<float>& PCRITDat    = PCRITFrames->frameData(fIdx);

        // HACK ! Todo : make it robust against other elements than Hex8
        size_t valCount = s11.size() * 3; // Number of Elm Node Face results 24 = 4 * num faces = 3* numElmNodes 

        SNDat.resize(valCount);
        STHDat.resize(valCount);
        STQVDat.resize(valCount);
        TNHDat.resize(valCount);
        TNQVDat.resize(valCount);
        THQVDat.resize(valCount);
        TPDat.resize(valCount);
        TincDat.resize(valCount);
        FAULTMOBDat.resize(valCount);
        PCRITDat.resize(valCount);

        int elementCount = femPart->elementCount();
        for(int elmIdx = 0; elmIdx < elementCount; ++elmIdx)
        {
            RigElementType elmType = femPart->elementType(elmIdx);
            int faceCount = RigFemTypes::elmentFaceCount(elmType);
            const int* elmNodeIndices =  femPart->connectivities(elmIdx);

            int elmNodFaceResIdxElmStart = elmIdx * 24; // HACK should get from part

            for(int lfIdx = 0; lfIdx < faceCount; ++lfIdx)
            {
                int faceNodeCount = 0;
                const int*  localElmNodeIndicesForFace = RigFemTypes::localElmNodeIndicesForFace(elmType, lfIdx, &faceNodeCount);
                if(faceNodeCount == 4)
                {
                    int elmNodFaceResIdxFaceStart =  elmNodFaceResIdxElmStart + lfIdx*4; // HACK
                    cvf::Vec3f quadVxs[4];

                    quadVxs[0] = (nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[0]]]);
                    quadVxs[1] = (nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[1]]]);
                    quadVxs[2] = (nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[2]]]);
                    quadVxs[3] = (nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[3]]]);

                    cvf::Mat3f rotMx = cvf::GeometryTools::computePlaneHorizontalRotationMx(quadVxs[2] -quadVxs[0], quadVxs[3] - quadVxs[1] );

                    size_t qElmNodeResIdx[4];
                    qElmNodeResIdx[0] = femPart->elementNodeResultIdx(elmIdx, localElmNodeIndicesForFace[0]);
                    qElmNodeResIdx[1] = femPart->elementNodeResultIdx(elmIdx, localElmNodeIndicesForFace[1]);
                    qElmNodeResIdx[2] = femPart->elementNodeResultIdx(elmIdx, localElmNodeIndicesForFace[2]);
                    qElmNodeResIdx[3] = femPart->elementNodeResultIdx(elmIdx, localElmNodeIndicesForFace[3]);
                    
                    for (int qIdx = 0; qIdx < 4; ++qIdx)
                    {
                        size_t elmNodResIdx = qElmNodeResIdx[qIdx];
                        float t11 = s11[elmNodResIdx];
                        float t22 = s22[elmNodResIdx];
                        float t33 = s33[elmNodResIdx];
                        float t12 = s12[elmNodResIdx];
                        float t23 = s23[elmNodResIdx];
                        float t13 = s13[elmNodResIdx];

                        caf::Ten3f tensor(t11, t22, t33,
                                          t12, t23, t13);
                        caf::Ten3f xfTen = tensor.rotated(rotMx);
                        int elmNodFaceResIdx = elmNodFaceResIdxFaceStart + qIdx;

                        float szx = xfTen[caf::Ten3f::SZX];
                        float syz = xfTen[caf::Ten3f::SYZ];
                        float szz = xfTen[caf::Ten3f::SZZ];

                        STHDat[elmNodFaceResIdx]    = xfTen[caf::Ten3f::SXX];
                        STQVDat[elmNodFaceResIdx]   = xfTen[caf::Ten3f::SYY];
                        SNDat[elmNodFaceResIdx]     = xfTen[caf::Ten3f::SZZ];

                        TNHDat[elmNodFaceResIdx]    = xfTen[caf::Ten3f::SZX];
                        TNQVDat[elmNodFaceResIdx]   = xfTen[caf::Ten3f::SYZ];
                        THQVDat[elmNodFaceResIdx]   = xfTen[caf::Ten3f::SXY];

                        float TP = sqrt(szx*szx + syz*syz);
                        TPDat[elmNodFaceResIdx] = TP;

                        if (TP > 1e-5)
                        {
                            TincDat[elmNodFaceResIdx] = cvf::Math::toDegrees( acos(syz/TP) );
                        }
                        else
                        {
                            TincDat[elmNodFaceResIdx] = std::numeric_limits<float>::infinity();
                        }

                        FAULTMOBDat[elmNodFaceResIdx] = TP/(tanFricAng * (szz + cohPrTanFricAngle));
                        PCRITDat[elmNodFaceResIdx]  = szz - TP/tanFricAng; 
                    }
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedSurfStress = this->findOrLoadScalarResult(partIndex,resVarAddr);
    return requestedSurfStress;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateSurfaceAngles(int partIndex, const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT(resVarAddr.componentName == "Pazi" || resVarAddr.componentName == "Pinc");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 1, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));

    RigFemScalarResultFrames * PaziFrames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "Pazi"));
    RigFemScalarResultFrames * PincFrames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "Pinc"));

    const RigFemPart * femPart = m_femParts->part(partIndex);
    const std::vector<cvf::Vec3f>& nodeCoordinates = femPart->nodes().coordinates;
    int frameCount = this->frameCount();

    // HACK ! Todo : make it robust against other elements than Hex8
    size_t valCount = femPart->elementCount() * 24; // Number of Elm Node Face results 24 = 4 * num faces = 3* numElmNodes 

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        std::vector<float>& Pazi = PaziFrames->frameData(fIdx);
        std::vector<float>& Pinc = PincFrames->frameData(fIdx);

        Pazi.resize(valCount);
        Pinc.resize(valCount);

        int elementCount = femPart->elementCount();
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType(elmIdx);
            int faceCount = RigFemTypes::elmentFaceCount(elmType);
            const int* elmNodeIndices =  femPart->connectivities(elmIdx);

            int elmNodFaceResIdxElmStart = elmIdx * 24; // HACK should get from part

            for ( int lfIdx = 0; lfIdx < faceCount; ++lfIdx )
            {
                int faceNodeCount = 0;
                const int*  localElmNodeIndicesForFace = RigFemTypes::localElmNodeIndicesForFace(elmType, lfIdx, &faceNodeCount);
                if ( faceNodeCount == 4 )
                {
                    int elmNodFaceResIdxFaceStart =  elmNodFaceResIdxElmStart + lfIdx*4; // HACK
                    cvf::Vec3f quadVxs[4];

                    quadVxs[0] = (nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[0]]]);
                    quadVxs[1] = (nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[1]]]);
                    quadVxs[2] = (nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[2]]]);
                    quadVxs[3] = (nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[3]]]);

                    cvf::Mat3f rotMx = cvf::GeometryTools::computePlaneHorizontalRotationMx(quadVxs[2] -quadVxs[0], quadVxs[3] - quadVxs[1]);
                    OffshoreSphericalCoords sphCoord(cvf::Vec3f(rotMx.rowCol(0,2), rotMx.rowCol(1,2), rotMx.rowCol(2,2))); // Use Ez from the matrix as plane normal

                    for ( int qIdx = 0; qIdx < 4; ++qIdx )
                    {
                        int elmNodFaceResIdx = elmNodFaceResIdxFaceStart + qIdx;
                        Pazi[elmNodFaceResIdx] = cvf::Math::toDegrees( sphCoord.azi() ); 
                        Pinc[elmNodFaceResIdx] = cvf::Math::toDegrees( sphCoord.inc() ); 
                    }
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedPlaneAngle = this->findOrLoadScalarResult(partIndex, resVarAddr);
    return requestedPlaneAngle;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculatePrincipalStressValues(int partIndex, const RigFemResultAddress &resVarAddr )
{
    CVF_ASSERT(resVarAddr.componentName == "S1" || resVarAddr.componentName == "S2" || resVarAddr.componentName == "S3"
               || resVarAddr.componentName == "S1inc"
               || resVarAddr.componentName == "S1azi"
               || resVarAddr.componentName == "S2inc"
               || resVarAddr.componentName == "S2azi"
               || resVarAddr.componentName == "S3inc"
               || resVarAddr.componentName == "S3azi");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 7, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());


    RigFemScalarResultFrames * s11Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S11"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s22Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S22"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s33Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S33"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s12Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S12"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s13Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S13"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s23Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S23"));

    RigFemScalarResultFrames * s1Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S1"));
    RigFemScalarResultFrames * s2Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S2"));
    RigFemScalarResultFrames * s3Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S3"));

    RigFemScalarResultFrames * s1IncFrames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S1inc"));
    RigFemScalarResultFrames * s1AziFrames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S1azi"));
    RigFemScalarResultFrames * s2IncFrames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S2inc"));
    RigFemScalarResultFrames * s2AziFrames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S2azi"));
    RigFemScalarResultFrames * s3IncFrames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S3inc"));
    RigFemScalarResultFrames * s3AziFrames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "S3azi"));

    frameCountProgress.incrementProgress();

    int frameCount = s11Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& s11 = s11Frames->frameData(fIdx);
        const std::vector<float>& s22 = s22Frames->frameData(fIdx);
        const std::vector<float>& s33 = s33Frames->frameData(fIdx);
        const std::vector<float>& s12 = s12Frames->frameData(fIdx);
        const std::vector<float>& s13 = s13Frames->frameData(fIdx);
        const std::vector<float>& s23 = s23Frames->frameData(fIdx);

        std::vector<float>& s1 = s1Frames->frameData(fIdx);
        std::vector<float>& s2 = s2Frames->frameData(fIdx);
        std::vector<float>& s3 = s3Frames->frameData(fIdx);

        std::vector<float>& s1inc = s1IncFrames->frameData(fIdx);
        std::vector<float>& s1azi = s1AziFrames->frameData(fIdx);
        std::vector<float>& s2inc = s2IncFrames->frameData(fIdx);
        std::vector<float>& s2azi = s2AziFrames->frameData(fIdx);
        std::vector<float>& s3inc = s3IncFrames->frameData(fIdx);
        std::vector<float>& s3azi = s3AziFrames->frameData(fIdx);

        size_t valCount = s11.size();

        s1.resize(valCount);
        s2.resize(valCount);
        s3.resize(valCount);
        s1inc.resize(valCount);
        s1azi.resize(valCount);
        s2inc.resize(valCount);
        s2azi.resize(valCount);
        s3inc.resize(valCount);
        s3azi.resize(valCount);

        for ( size_t vIdx = 0; vIdx < valCount; ++vIdx )
        {
            caf::Ten3f T(s11[vIdx], s22[vIdx], s33[vIdx], s12[vIdx], s23[vIdx], s13[vIdx]);
            cvf::Vec3f principalDirs[3];
            cvf::Vec3f principals = T.calculatePrincipals(principalDirs);
            s1[vIdx] = principals[0];
            s2[vIdx] = principals[1];
            s3[vIdx] = principals[2];

            if ( principals[0] != std::numeric_limits<float>::infinity() )
            {
                OffshoreSphericalCoords sphCoord1(principalDirs[0]);
                s1inc[vIdx] = cvf::Math::toDegrees(sphCoord1.inc());
                s1azi[vIdx] = cvf::Math::toDegrees(sphCoord1.azi());
            }
            else
            {
                s1inc[vIdx] = std::numeric_limits<float>::infinity();
                s1azi[vIdx] =    std::numeric_limits<float>::infinity();
            }

            if ( principals[1] != std::numeric_limits<float>::infinity() )
            {
                OffshoreSphericalCoords sphCoord2(principalDirs[1]);
                s2inc[vIdx] = cvf::Math::toDegrees(sphCoord2.inc());
                s2azi[vIdx] = cvf::Math::toDegrees(sphCoord2.azi());
            }
            else
            {
                s2inc[vIdx] = std::numeric_limits<float>::infinity();
                s2azi[vIdx] =    std::numeric_limits<float>::infinity();
            }

            if ( principals[2] != std::numeric_limits<float>::infinity() )
            {
                OffshoreSphericalCoords sphCoord3(principalDirs[2]);
                s3inc[vIdx] = cvf::Math::toDegrees(sphCoord3.inc());
                s3azi[vIdx] = cvf::Math::toDegrees(sphCoord3.azi());
            }
            else
            {
                s3inc[vIdx] = std::numeric_limits<float>::infinity();
                s3azi[vIdx] = std::numeric_limits<float>::infinity();
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedPrincipal = this->findOrLoadScalarResult(partIndex, resVarAddr);

    return  requestedPrincipal;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculatePrincipalStrainValues(int partIndex, const RigFemResultAddress &resVarAddr)
{
    CVF_ASSERT(resVarAddr.componentName == "E1" || resVarAddr.componentName == "E2" || resVarAddr.componentName == "E3");

    caf::ProgressInfo frameCountProgress(this->frameCount() * 7, "");
    frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
    frameCountProgress.setNextProgressIncrement(this->frameCount());


    RigFemScalarResultFrames * s11Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E11"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s22Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E22"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s33Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E33"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s12Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E12"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s13Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E13"));
    frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
    RigFemScalarResultFrames * s23Frames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E23"));

    RigFemScalarResultFrames * s1Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E1"));
    RigFemScalarResultFrames * s2Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E2"));
    RigFemScalarResultFrames * s3Frames =  m_femPartResults[partIndex]->createScalarResult(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, "E3"));

    frameCountProgress.incrementProgress();

    int frameCount = s11Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& s11 = s11Frames->frameData(fIdx);
        const std::vector<float>& s22 = s22Frames->frameData(fIdx);
        const std::vector<float>& s33 = s33Frames->frameData(fIdx);
        const std::vector<float>& s12 = s12Frames->frameData(fIdx);
        const std::vector<float>& s13 = s13Frames->frameData(fIdx);
        const std::vector<float>& s23 = s23Frames->frameData(fIdx);

        std::vector<float>& s1 = s1Frames->frameData(fIdx);
        std::vector<float>& s2 = s2Frames->frameData(fIdx);
        std::vector<float>& s3 = s3Frames->frameData(fIdx);

        size_t valCount = s11.size();

        s1.resize(valCount);
        s2.resize(valCount);
        s3.resize(valCount);

        for ( size_t vIdx = 0; vIdx < valCount; ++vIdx )
        {
            caf::Ten3f T(s11[vIdx], s22[vIdx], s33[vIdx], s12[vIdx], s23[vIdx], s13[vIdx]);
            cvf::Vec3f principalDirs[3];
            cvf::Vec3f principals = T.calculatePrincipals(principalDirs);
            s1[vIdx] = principals[0];
            s2[vIdx] = principals[1];
            s3[vIdx] = principals[2];
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedPrincipal = this->findOrLoadScalarResult(partIndex, resVarAddr);

    return  requestedPrincipal;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDerivedResult(int partIndex, const RigFemResultAddress& resVarAddr)
{
    if (resVarAddr.isTimeLapse())
    {
        return calculateTimeLapseResult(partIndex, resVarAddr);    
    }

    if(resVarAddr.resultPosType == RIG_ELEMENT_NODAL_FACE )
    {
        if (resVarAddr.componentName == "Pazi" || resVarAddr.componentName == "Pinc" )
            return calculateSurfaceAngles(partIndex, resVarAddr);
        else
            return calculateSurfaceAlignedStress(partIndex, resVarAddr);
    }

    if (resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SFI")
    {
        return calculateSFI(partIndex, resVarAddr);    
    }

    if ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "DSM" )
    {
        return calculateDSM(partIndex, resVarAddr);
    }

    if ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "FOS" )
    {
        return calculateFOS(partIndex, resVarAddr);
    }

    if(resVarAddr.fieldName == "NE" && resVarAddr.componentName == "EV")
    {
        return calculateVolumetricStrain(partIndex, resVarAddr);
    }

    if ( resVarAddr.fieldName == "NE" && resVarAddr.componentName == "ED" )
    {
        return calculateDeviatoricStrain(partIndex, resVarAddr);
    }

    if(resVarAddr.fieldName == "ST" && resVarAddr.componentName == "Q" )
    {
        return calculateDeviatoricStress(partIndex, resVarAddr);
    }

    if(resVarAddr.fieldName == "ST"  && resVarAddr.componentName == "STM") 
    {
        return calculateMeanStressSTM(partIndex, resVarAddr);
    }

    if(resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SEM")
    {
        return calculateMeanStressSEM(partIndex, resVarAddr);
    }

    if (resVarAddr.fieldName == "S-Bar")
    {
        return calculateBarConvertedResult(partIndex, resVarAddr, "S");
    }

    if (resVarAddr.fieldName == "POR-Bar")
    {
        if (resVarAddr.resultPosType == RIG_NODAL)
            return calculateBarConvertedResult(partIndex, resVarAddr, "POR");
        else
            return calculateEnIpPorBarResult(partIndex, resVarAddr);
    }

    if((resVarAddr.fieldName == "NE")
       && (   resVarAddr.componentName == "E11"
           || resVarAddr.componentName == "E22"
           || resVarAddr.componentName == "E33"
           || resVarAddr.componentName == "E12"
           || resVarAddr.componentName == "E13"
           || resVarAddr.componentName == "E23"))
    {
        caf::ProgressInfo frameCountProgress(this->frameCount() * 2, "");
        frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemScalarResultFrames * srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "E", resVarAddr.componentName));
        RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);
        
        frameCountProgress.incrementProgress();

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

            frameCountProgress.incrementProgress();
        }

        return dstDataFrames;
    }

    if ( (resVarAddr.fieldName == "NE")
        && (   resVarAddr.componentName == "E1"
            || resVarAddr.componentName == "E2"
            || resVarAddr.componentName == "E3") )
    {
        return calculatePrincipalStrainValues(partIndex, resVarAddr);
    }

    if ((resVarAddr.fieldName == "SE") 
        && (   resVarAddr.componentName == "S11" 
            || resVarAddr.componentName == "S22" 
            || resVarAddr.componentName == "S33" 
            || resVarAddr.componentName == "S12"
            || resVarAddr.componentName == "S13" 
            || resVarAddr.componentName == "S23" ))
    {
        caf::ProgressInfo frameCountProgress(this->frameCount() * 3, "");
        frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemScalarResultFrames * srcDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "S-Bar", resVarAddr.componentName));
        frameCountProgress.incrementProgress(); frameCountProgress.setNextProgressIncrement(this->frameCount());
        RigFemScalarResultFrames * srcPORDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_NODAL, "POR-Bar", ""));
        RigFemScalarResultFrames * dstDataFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);

        frameCountProgress.incrementProgress();

        const RigFemPart * femPart = m_femParts->part(partIndex);
        float inf = std::numeric_limits<float>::infinity();

        int frameCount = srcDataFrames->frameCount();
        for (int fIdx = 0; fIdx < frameCount; ++fIdx)
        {
            const std::vector<float>& srcSFrameData = srcDataFrames->frameData(fIdx);
            std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);
            size_t valCount = srcSFrameData.size();
            dstFrameData.resize(valCount);

            const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData(fIdx);

           int elementCount = femPart->elementCount();
            for (int elmIdx = 0; elmIdx < elementCount; ++elmIdx)
            {
                RigElementType elmType = femPart->elementType(elmIdx);

                int elmNodeCount = RigFemTypes::elmentNodeCount(femPart->elementType(elmIdx));

                if (elmType == HEX8P)
                {
                    for (int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx)
                    {
                        size_t elmNodResIdx = femPart->elementNodeResultIdx(elmIdx, elmNodIdx);
                        dstFrameData[elmNodResIdx] = -srcSFrameData[elmNodResIdx];
                    }
                }
                else 
                {
                    for (int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx)
                    {
                        size_t elmNodResIdx = femPart->elementNodeResultIdx(elmIdx, elmNodIdx);

                        dstFrameData[elmNodResIdx] = inf;
                    }
                }
            }

            frameCountProgress.incrementProgress();
        }

        return dstDataFrames;
    }

    if (   (resVarAddr.fieldName == "SE" || resVarAddr.fieldName == "ST" )
        && (   resVarAddr.componentName == "S1" 
            || resVarAddr.componentName == "S2" 
            || resVarAddr.componentName == "S3" 
            || resVarAddr.componentName == "S1inc" 
            || resVarAddr.componentName == "S1azi" 
            || resVarAddr.componentName == "S2inc"
            || resVarAddr.componentName == "S2azi"
            || resVarAddr.componentName == "S3inc" 
            || resVarAddr.componentName == "S3azi" )
        )
    {
        return calculatePrincipalStressValues(partIndex, resVarAddr);
    }

    if ( resVarAddr.fieldName == "ST" 
        &&  (   resVarAddr.componentName == "S11" 
            ||  resVarAddr.componentName == "S22"  
            ||  resVarAddr.componentName == "S33" ))
    {
        caf::ProgressInfo frameCountProgress(this->frameCount() * 3, "");
        frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemScalarResultFrames * srcSDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "S-Bar", resVarAddr.componentName));
        frameCountProgress.incrementProgress();
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemScalarResultFrames * srcPORDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_NODAL, "POR-Bar", ""));

        RigFemScalarResultFrames * dstDataFrames =  m_femPartResults[partIndex]->createScalarResult(resVarAddr);
        const RigFemPart * femPart = m_femParts->part(partIndex);
        int frameCount = srcSDataFrames->frameCount();
        
        frameCountProgress.incrementProgress();

        const float inf = std::numeric_limits<float>::infinity();

        for (int fIdx = 0; fIdx < frameCount; ++fIdx)
        {
            const std::vector<float>& srcSFrameData = srcSDataFrames->frameData(fIdx);
            const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData(fIdx);
            
            std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);

            size_t valCount = srcSFrameData.size();
            dstFrameData.resize(valCount);

            int elementCount = femPart->elementCount();
            for (int elmIdx = 0; elmIdx < elementCount; ++elmIdx)
            {
                RigElementType elmType = femPart->elementType(elmIdx);

                int elmNodeCount = RigFemTypes::elmentNodeCount(femPart->elementType(elmIdx));

                if (elmType == HEX8P)
                {
                    for (int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx)
                    {
                        size_t elmNodResIdx = femPart->elementNodeResultIdx(elmIdx, elmNodIdx);
                        int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx(elmNodResIdx);

                        float por = srcPORFrameData[nodeIdx];
                        if (por == inf)  por = 0.0f;

                        dstFrameData[elmNodResIdx] = -srcSFrameData[elmNodResIdx] + por;
                    }
                }
                else 
                {
                    for (int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx)
                    {
                        size_t elmNodResIdx = femPart->elementNodeResultIdx(elmIdx, elmNodIdx);
                        dstFrameData[elmNodResIdx] = -srcSFrameData[elmNodResIdx];
                    }
                }
            }

            frameCountProgress.incrementProgress();
        }
        return dstDataFrames; 
    }

    if ( resVarAddr.fieldName == "ST" 
        &&  (   resVarAddr.componentName == "S12" 
            ||  resVarAddr.componentName == "S13"  
            ||  resVarAddr.componentName == "S23" ))
    {
        caf::ProgressInfo frameCountProgress(this->frameCount() * 2, "");
        frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemScalarResultFrames * srcSDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(resVarAddr.resultPosType, "S-Bar", resVarAddr.componentName));
        RigFemScalarResultFrames * dstDataFrames =  m_femPartResults[partIndex]->createScalarResult(resVarAddr);

        frameCountProgress.incrementProgress();

        int frameCount = srcSDataFrames->frameCount();
        for (int fIdx = 0; fIdx < frameCount; ++fIdx)
        {
            const std::vector<float>& srcSFrameData = srcSDataFrames->frameData(fIdx);
            std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);

            size_t valCount = srcSFrameData.size();
            dstFrameData.resize(valCount);
            for (size_t vIdx = 0; vIdx < valCount; ++vIdx)
            {
                dstFrameData[vIdx] = -srcSFrameData[vIdx];
            }

            frameCountProgress.incrementProgress();
        }
        return dstDataFrames; 
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
        caf::ProgressInfo frameCountProgress(this->frameCount() * 3, "");
        frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemResultAddress totStressCompAddr(resVarAddr.resultPosType, "ST", "");
        {
            std::string scomp;
            std::string gcomp = resVarAddr.componentName;
            if ( gcomp == "Gamma1" )       scomp =  "S1";
            else if ( gcomp == "Gamma2" )  scomp =  "S2";
            else if ( gcomp == "Gamma3" )  scomp =  "S3";
            else if ( gcomp == "Gamma11" ) scomp =  "S11";
            else if ( gcomp == "Gamma22" ) scomp =  "S22";
            else if ( gcomp == "Gamma33" ) scomp =  "S33";
            totStressCompAddr.componentName = scomp;
        }

        RigFemScalarResultFrames * srcDataFrames = this->findOrLoadScalarResult(partIndex, totStressCompAddr);
        
        frameCountProgress.incrementProgress();
        frameCountProgress.setNextProgressIncrement(this->frameCount());

        RigFemScalarResultFrames * srcPORDataFrames = this->findOrLoadScalarResult(partIndex, RigFemResultAddress(RIG_NODAL, "POR-Bar", ""));
        RigFemScalarResultFrames * dstDataFrames =  m_femPartResults[partIndex]->createScalarResult(resVarAddr);

        frameCountProgress.incrementProgress();

        calculateGammaFromFrames(partIndex, srcDataFrames, srcPORDataFrames, dstDataFrames, &frameCountProgress);

        return dstDataFrames; 
    }

    if (resVarAddr.fieldName == "Gamma" && resVarAddr.componentName == "")
    {
        // Create and return an empty result
        return m_femPartResults[partIndex]->createScalarResult(resVarAddr);
    }

    if (resVarAddr.resultPosType == RIG_FORMATION_NAMES)
    {
        caf::ProgressInfo frameCountProgress(2, "");
        frameCountProgress.setProgressDescription("Calculating " + QString::fromStdString(resVarAddr.fieldName + ": " + resVarAddr.componentName));

        RigFemScalarResultFrames* resFrames = m_femPartResults[partIndex]->createScalarResult(resVarAddr);
        resFrames->enableAsSingleFrameResult();

        const RigFemPart * femPart = m_femParts->part(partIndex);
        std::vector<float>& dstFrameData = resFrames->frameData(0);

        size_t valCount = femPart->elementNodeResultCount();
        float inf       = std::numeric_limits<float>::infinity();
        dstFrameData.resize(valCount, inf);

        RigFormationNames* activeFormNames = m_activeFormationNamesData.p();

        frameCountProgress.incrementProgress();

        int elementCount = femPart->elementCount();
        for(int elmIdx = 0; elmIdx < elementCount; ++elmIdx)
        {
            RigElementType elmType = femPart->elementType(elmIdx);
            int elmNodeCount = RigFemTypes::elmentNodeCount(elmType);

            size_t i, j, k;
            femPart->structGrid()->ijkFromCellIndex(elmIdx, &i, &j, &k);
            int formNameIdx = -1;
            if (activeFormNames)
            {
                formNameIdx = activeFormNames->formationIndexFromKLayerIdx(k);
            }

            for(int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx)
            {
                size_t elmNodResIdx = femPart->elementNodeResultIdx(elmIdx, elmNodIdx);

                if (formNameIdx != -1)
                {
                    dstFrameData[elmNodResIdx] = formNameIdx;
                }
                else
                {
                    dstFrameData[elmNodResIdx] = HUGE_VAL;
                }
            }
        }

        return resFrames;
    }
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::calculateGammaFromFrames(int partIndex, 
                                                           const RigFemScalarResultFrames * totalStressComponentDataFrames, 
                                                           const RigFemScalarResultFrames * srcPORDataFrames, 
                                                           RigFemScalarResultFrames * dstDataFrames,
                                                           caf::ProgressInfo* frameCountProgress)
{
    const RigFemPart * femPart = m_femParts->part(partIndex);
    int frameCount = totalStressComponentDataFrames->frameCount();
    float inf = std::numeric_limits<float>::infinity();

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcSTFrameData = totalStressComponentDataFrames->frameData(fIdx);
        const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData(fIdx);

        std::vector<float>& dstFrameData = dstDataFrames->frameData(fIdx);

        size_t valCount = srcSTFrameData.size();
        dstFrameData.resize(valCount);

        int elementCount = femPart->elementCount();
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType(elmIdx);

            int elmNodeCount = RigFemTypes::elmentNodeCount(femPart->elementType(elmIdx));

            if ( elmType == HEX8P )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx(elmIdx, elmNodIdx);
                    int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx(elmNodResIdx);

                    float por = srcPORFrameData[nodeIdx];

                    if ( por == inf || fabs(por) < 0.01e6*1.0e-5 )
                        dstFrameData[elmNodResIdx] = inf;
                    else
                        dstFrameData[elmNodResIdx] = srcSTFrameData[elmNodResIdx]/por;
                }
            }
            else
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx(elmIdx, elmNodIdx);
                    dstFrameData[elmNodResIdx] = inf;
                }
            }
        }

        frameCountProgress->incrementProgress();
    }
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
        if (resVarAddr.componentName != "") // If we did not request a particular component, do not add the components
        {
            for (size_t cIdx = 0; cIdx < compNames.size(); ++cIdx)
            {
                resAddressToComponents.push_back(RigFemResultAddress(resVarAddr.resultPosType, resVarAddr.fieldName, compNames[cIdx]));
            }
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
std::vector<std::string> RigFemPartResultsCollection::stepNames() const
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
void RigFemPartResultsCollection::deleteResult(const RigFemResultAddress& resVarAddr)
{
    CVF_ASSERT ( resVarAddr.isValid() );

    for ( int pIdx = 0; pIdx < static_cast<int>(m_femPartResults.size()); ++pIdx )
    {
        if ( m_femPartResults[pIdx].notNull() )
        {
            m_femPartResults[pIdx]->deleteScalarResult(resVarAddr);
        }
    }

    m_resultStatistics.erase(resVarAddr);
    
    if ( resVarAddr.representsAllTimeLapses() )
    {
        std::vector<RigFemResultAddress> addressesToDelete;
        for ( auto it : m_resultStatistics )
        {
            if ( it.first.resultPosType == resVarAddr.resultPosType
                && it.first.fieldName == resVarAddr.fieldName
                &&  it.first.componentName == resVarAddr.componentName )
            {
                addressesToDelete.push_back(it.first);
            }
        }

        for ( RigFemResultAddress& addr: addressesToDelete )
        {
            m_resultStatistics.erase(addr);
        }
    }
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
void RigFemPartResultsCollection::meanScalarValue(const RigFemResultAddress& resVarAddr, int frameIndex, double* meanValue)
{
   this->statistics(resVarAddr)->meanCellScalarValues(frameIndex, *meanValue);
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
void RigFemPartResultsCollection::p10p90ScalarValues(const RigFemResultAddress& resVarAddr, int frameIndex, double* p10, double* p90)
{
    this->statistics(resVarAddr)->p10p90CellScalarValues(frameIndex, *p10, *p90);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::sumScalarValue(const RigFemResultAddress& resVarAddr, double* sum)
{
    CVF_ASSERT(sum);

    this->statistics(resVarAddr)->sumCellScalarValues(*sum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::sumScalarValue(const RigFemResultAddress& resVarAddr, int frameIndex, double* sum)
{
    CVF_ASSERT(sum);

    this->statistics(resVarAddr)->sumCellScalarValues(frameIndex, *sum);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFemPartResultsCollection::scalarValuesHistogram(const RigFemResultAddress& resVarAddr)
{
    return this->statistics(resVarAddr)->cellScalarValuesHistogram();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFemPartResultsCollection::scalarValuesHistogram(const RigFemResultAddress& resVarAddr, int frameIndex)
{
    return this->statistics(resVarAddr)->cellScalarValuesHistogram(frameIndex);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RigFemPartResultsCollection::partCount() const
{
    return m_femParts->partCount();
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RigFemClosestResultIndexCalculator::RigFemClosestResultIndexCalculator(RigFemPart* femPart,
                                                                       RigFemResultPosEnum resultPosition,
                                                                       int elementIndex,
                                                                       int m_face,
                                                                       const cvf::Vec3d& m_intersectionPoint)
{
    m_resultIndexToClosestResult = -1;
    m_closestNodeId = -1;
    m_closestElementNodeResIdx = -1;

    if ( resultPosition != RIG_ELEMENT_NODAL_FACE  ||  m_face == -1 )
    {
        RigElementType elmType =  femPart->elementType(elementIndex);
        const int* elmentConn = femPart->connectivities(elementIndex);
        int elmNodeCount = RigFemTypes::elmentNodeCount(elmType);

        // Find the closest node
        int closestLocalNode = -1;
        float minDist = std::numeric_limits<float>::infinity();
        for ( int lNodeIdx = 0; lNodeIdx < elmNodeCount; ++lNodeIdx )
        {
            int nodeIdx = elmentConn[lNodeIdx];
            cvf::Vec3f nodePos = femPart->nodes().coordinates[nodeIdx];
            float dist = (nodePos - cvf::Vec3f(m_intersectionPoint)).lengthSquared();
            if ( dist < minDist )
            {
                closestLocalNode = lNodeIdx;
                minDist = dist;
            }
        }

        if ( closestLocalNode >= 0 )
        {
            int nodeIdx = elmentConn[closestLocalNode];
            m_closestElementNodeResIdx = static_cast<int>(femPart->elementNodeResultIdx(elementIndex, closestLocalNode));

            if ( resultPosition == RIG_NODAL )
            {
                m_resultIndexToClosestResult = nodeIdx;
            }
            else if (resultPosition == RIG_ELEMENT_NODAL_FACE)
            {
                m_resultIndexToClosestResult = -1;   
            }
            else
            {
                m_resultIndexToClosestResult = m_closestElementNodeResIdx;
            }

            m_closestNodeId = femPart->nodes().nodeIds[nodeIdx];
        }
    }
    else if ( m_face != -1 )
    {
        int elmNodFaceResIdx = -1;
        int closestNodeIdx = -1;
        {
            int closestLocFaceNode = -1;
            int closestLocalElmNode = -1;
            {
                RigElementType elmType =  femPart->elementType(elementIndex);
                const int* elmNodeIndices =  femPart->connectivities(elementIndex);
                int faceNodeCount = 0;
                const int*  localElmNodeIndicesForFace = RigFemTypes::localElmNodeIndicesForFace(elmType, m_face, &faceNodeCount);

                float minDist = std::numeric_limits<float>::infinity();
                for ( int faceNodIdx = 0; faceNodIdx < faceNodeCount; ++faceNodIdx )
                {
                    int nodeIdx = elmNodeIndices[localElmNodeIndicesForFace[faceNodIdx]];
                    cvf::Vec3f nodePos = femPart->nodes().coordinates[nodeIdx];
                    float dist = (nodePos - cvf::Vec3f(m_intersectionPoint)).lengthSquared();
                    if ( dist < minDist )
                    {
                        closestLocFaceNode = faceNodIdx;
                        closestNodeIdx = nodeIdx;
                        closestLocalElmNode = localElmNodeIndicesForFace[faceNodIdx];
                        minDist = dist;
                    }
                }
            }

            int elmNodFaceResIdxElmStart = elementIndex * 24; // HACK should get from part
            int elmNodFaceResIdxFaceStart = elmNodFaceResIdxElmStart + 4*m_face;

            if ( closestLocFaceNode >= 0 )
            {
                elmNodFaceResIdx = elmNodFaceResIdxFaceStart + closestLocFaceNode;
                m_closestElementNodeResIdx = static_cast<int>(femPart->elementNodeResultIdx(elementIndex, closestLocalElmNode));
            }
        }

        m_resultIndexToClosestResult = elmNodFaceResIdx;
        m_closestNodeId = femPart->nodes().nodeIds[closestNodeIdx];
    }
}