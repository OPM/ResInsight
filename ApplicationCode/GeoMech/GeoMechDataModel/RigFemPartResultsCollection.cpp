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

#include "RifElementPropertyReader.h"
#include "RifGeoMechReaderInterface.h"

#ifdef USE_ODB_API
#include "RifOdbReader.h"
#endif

#include "RiaApplication.h"

#include "RiaOffshoreSphericalCoords.h"

#include "RigFemNativeStatCalc.h"
#include "RigFemPartCollection.h"
#include "RigFemPartGrid.h"
#include "RigFemPartResults.h"
#include "RigFemScalarResultFrames.h"
#include "RigFormationNames.h"
#include "RigHexGradientTools.h"
#include "RigHexIntersectionTools.h"
#include "RigStatisticsDataCache.h"
#include "RigWbsParameter.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"

#include "cafProgressInfo.h"
#include "cafTensor3.h"

#include "cvfBoundingBox.h"
#include "cvfGeometryTools.h"
#include "cvfMath.h"

#include <QString>

#include <cmath>
#include <stdlib.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string RigFemPartResultsCollection::FIELD_NAME_COMPACTION = "COMPACTION";

//--------------------------------------------------------------------------------------------------
/// Internal definitions
//--------------------------------------------------------------------------------------------------
class RefElement
{
public:
    size_t              elementIdx;
    float               intersectionPointToCurrentNodeDistance;
    cvf::Vec3f          intersectionPoint;
    std::vector<size_t> elementFaceNodeIdxs;
};

static std::vector<cvf::Vec3d> coordsFromNodeIndices( const RigFemPart& part, const std::vector<size_t>& nodeIdxs );
static std::vector<size_t>     nodesForElement( const RigFemPart& part, size_t elementIdx );
static float                   horizontalDistance( const cvf::Vec3f& p1, const cvf::Vec3f& p2 );
static void findReferenceElementForNode( const RigFemPart& part, size_t nodeIdx, size_t kRefLayer, RefElement* refElement );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemPartResultsCollection::RigFemPartResultsCollection( RifGeoMechReaderInterface*  readerInterface,
                                                          RifElementPropertyReader*   elementPropertyReader,
                                                          const RigFemPartCollection* femPartCollection )
{
    CVF_ASSERT( readerInterface );
    CVF_ASSERT( elementPropertyReader );
    m_readerInterface       = readerInterface;
    m_elementPropertyReader = elementPropertyReader;
    m_femParts              = femPartCollection;

    m_femPartResults.resize( m_femParts->partCount() );
    std::vector<std::string> filteredStepNames = m_readerInterface->filteredStepNames();
    for ( auto& femPartResult : m_femPartResults )
    {
        femPartResult = new RigFemPartResults;
        femPartResult->initResultSteps( filteredStepNames );
    }

    m_cohesion            = 10.0;
    m_frictionAngleRad    = cvf::Math::toRadians( 30.0 );
    m_normalizationAirGap = 0.0;
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
void RigFemPartResultsCollection::setActiveFormationNames( RigFormationNames* activeFormationNames )
{
    m_activeFormationNamesData = activeFormationNames;

    RimProject* project = RiaApplication::instance()->project();
    if ( project )
    {
        if ( project->mainPlotCollection() )
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if ( plotCollection )
            {
                for ( RimWellLogPlot* wellLogPlot : plotCollection->wellLogPlots )
                {
                    wellLogPlot->loadDataAndUpdate();
                }
            }
        }
    }

    this->deleteResult( RigFemResultAddress( RIG_FORMATION_NAMES, "Active Formation Names", "" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RigFemPartResultsCollection::formationNames() const
{
    if ( activeFormationNames() )
    {
        return activeFormationNames()->formationNames();
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigFormationNames* RigFemPartResultsCollection::activeFormationNames() const
{
    return m_activeFormationNamesData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::addElementPropertyFiles( const std::vector<QString>& filenames )
{
    for ( const QString& filename : filenames )
    {
        m_elementPropertyReader->addFile( filename.toStdString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress>
    RigFemPartResultsCollection::removeElementPropertyFiles( const std::vector<QString>& filenames )
{
    std::vector<RigFemResultAddress> addressesToRemove;

    for ( const QString& filename : filenames )
    {
        std::vector<std::string> fields = m_elementPropertyReader->fieldsInFile( filename.toStdString() );

        for ( const std::string& field : fields )
        {
            addressesToRemove.push_back( RigFemResultAddress( RIG_ELEMENT, field, "" ) );
        }

        m_elementPropertyReader->removeFile( filename.toStdString() );
    }

    for ( const RigFemResultAddress& address : addressesToRemove )
    {
        this->deleteResult( address );
    }

    return addressesToRemove;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setCalculationParameters( double cohesion, double frictionAngleRad )
{
    m_cohesion         = cohesion;
    m_frictionAngleRad = frictionAngleRad;

    // Todo, delete all dependent results
    this->deleteResult( RigFemResultAddress( RIG_ELEMENT_NODAL, "SE", "SFI", RigFemResultAddress::allTimeLapsesValue() ) );
    this->deleteResult(
        RigFemResultAddress( RIG_INTEGRATION_POINT, "SE", "SFI", RigFemResultAddress::allTimeLapsesValue() ) );
    this->deleteResult( RigFemResultAddress( RIG_ELEMENT_NODAL, "SE", "DSM", RigFemResultAddress::allTimeLapsesValue() ) );
    this->deleteResult(
        RigFemResultAddress( RIG_INTEGRATION_POINT, "SE", "DSM", RigFemResultAddress::allTimeLapsesValue() ) );
    this->deleteResult( RigFemResultAddress( RIG_ELEMENT_NODAL, "SE", "FOS", RigFemResultAddress::allTimeLapsesValue() ) );
    this->deleteResult(
        RigFemResultAddress( RIG_INTEGRATION_POINT, "SE", "FOS", RigFemResultAddress::allTimeLapsesValue() ) );
}

//--------------------------------------------------------------------------------------------------
/// Will always return a valid object, but it can be empty
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::findOrLoadScalarResult( int                        partIndex,
                                                                               const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( partIndex < (int)( m_femPartResults.size() ) );
    CVF_ASSERT( m_readerInterface.notNull() );
    CVF_ASSERT( resVarAddr.isValid() );

    // If we have it in the cache, return it
    RigFemScalarResultFrames* frames = m_femPartResults[partIndex]->findScalarResult( resVarAddr );
    if ( frames ) return frames;

    // Check whether a derived result is requested

    frames = calculateDerivedResult( partIndex, resVarAddr );
    if ( frames ) return frames;

    if ( resVarAddr.resultPosType == RIG_ELEMENT )
    {
        std::map<std::string, std::vector<float>> elementProperties =
            m_elementPropertyReader->readAllElementPropertiesInFileContainingField( resVarAddr.fieldName );

        for ( std::pair<std::string, std::vector<float>> elem : elementProperties )
        {
            RigFemResultAddress       addressForElement( RIG_ELEMENT, elem.first, "" );
            RigFemScalarResultFrames* currentFrames = m_femPartResults[partIndex]->createScalarResult( addressForElement );
            currentFrames->enableAsSingleFrameResult();
            currentFrames->frameData( 0 ).swap( elem.second );
        }

        frames = m_femPartResults[partIndex]->findScalarResult( resVarAddr );
        if ( frames )
        {
            return frames;
        }
        else
        {
            return m_femPartResults[partIndex]->createScalarResult( resVarAddr );
        }
    }

    // We need to read the data as bulk fields, and populate the correct scalar caches

    std::vector<RigFemResultAddress> resultAddressOfComponents = this->getResAddrToComponentsToRead( resVarAddr );

    if ( !resultAddressOfComponents.empty() )
    {
        std::vector<RigFemScalarResultFrames*> resultsForEachComponent;
        for ( const auto& resultAddressOfComponent : resultAddressOfComponents )
        {
            resultsForEachComponent.push_back( m_femPartResults[partIndex]->createScalarResult( resultAddressOfComponent ) );
        }

        int               frameCount = this->frameCount();
        caf::ProgressInfo progress( frameCount, "" );
        progress.setProgressDescription(
            QString( "Loading Native Result %1 %2" ).arg( resVarAddr.fieldName.c_str(), resVarAddr.componentName.c_str() ) );

        for ( int stepIndex = 0; stepIndex < frameCount; ++stepIndex )
        {
            std::vector<double> frameTimes = m_readerInterface->frameTimes( stepIndex );

            for ( int fIdx = 1; (size_t)fIdx < frameTimes.size() && fIdx < 2; ++fIdx ) // Read only the second frame
            {
                std::vector<std::vector<float>*> componentDataVectors;
                for ( auto& componentResult : resultsForEachComponent )
                {
                    componentDataVectors.push_back( &( componentResult->frameData( stepIndex ) ) );
                }

                switch ( resVarAddr.resultPosType )
                {
                    case RIG_NODAL:
                        m_readerInterface->readNodeField( resVarAddr.fieldName, partIndex, stepIndex, fIdx, &componentDataVectors );
                        break;
                    case RIG_ELEMENT_NODAL:
                        m_readerInterface->readElementNodeField( resVarAddr.fieldName,
                                                                 partIndex,
                                                                 stepIndex,
                                                                 fIdx,
                                                                 &componentDataVectors );
                        break;
                    case RIG_INTEGRATION_POINT:
                        m_readerInterface->readIntegrationPointField( resVarAddr.fieldName,
                                                                      partIndex,
                                                                      stepIndex,
                                                                      fIdx,
                                                                      &componentDataVectors );
                        break;
                    default:
                        break;
                }
            }

            progress.incrementProgress();
        }

        // Now fetch the particular component requested, which should now exist and be read.
        frames = m_femPartResults[partIndex]->findScalarResult( resVarAddr );
    }

    if ( !frames )
    {
        frames = m_femPartResults[partIndex]->createScalarResult( resVarAddr ); // Create a dummy empty result, if the
                                                                                // request did not specify the component.
    }

    return frames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::string>>
    RigFemPartResultsCollection::scalarFieldAndComponentNames( RigFemResultPosEnum resPos )
{
    std::map<std::string, std::vector<std::string>> fieldCompNames;

    if ( resPos == RIG_FORMATION_NAMES )
    {
        if ( activeFormationNames() ) fieldCompNames["Active Formation Names"];
    }

    const std::vector<std::string> stressComponentNames         = getStressComponentNames();
    const std::vector<std::string> stressGradientComponentNames = getStressGradientComponentNames();

    if ( m_readerInterface.notNull() )
    {
        if ( resPos == RIG_NODAL )
        {
            fieldCompNames = m_readerInterface->scalarNodeFieldAndComponentNames();
            fieldCompNames["POR-Bar"];
            fieldCompNames[FIELD_NAME_COMPACTION];
        }
        else if ( resPos == RIG_ELEMENT_NODAL )
        {
            fieldCompNames = m_readerInterface->scalarElementNodeFieldAndComponentNames();

            fieldCompNames["SE"].push_back( "SEM" );
            fieldCompNames["SE"].push_back( "SFI" );
            fieldCompNames["SE"].push_back( "DSM" );
            fieldCompNames["SE"].push_back( "FOS" );

            for ( auto& s : stressComponentNames )
            {
                fieldCompNames["SE"].push_back( s );
            }

            fieldCompNames["SE"].push_back( "S1inc" );
            fieldCompNames["SE"].push_back( "S1azi" );
            fieldCompNames["SE"].push_back( "S2inc" );
            fieldCompNames["SE"].push_back( "S2azi" );
            fieldCompNames["SE"].push_back( "S3inc" );
            fieldCompNames["SE"].push_back( "S3azi" );

            fieldCompNames["ST"].push_back( "STM" );
            fieldCompNames["ST"].push_back( "Q" );

            for ( auto& s : stressComponentNames )
            {
                fieldCompNames["ST"].push_back( s );
            }

            fieldCompNames["ST"].push_back( "S1inc" );
            fieldCompNames["ST"].push_back( "S1azi" );
            fieldCompNames["ST"].push_back( "S2inc" );
            fieldCompNames["ST"].push_back( "S2azi" );
            fieldCompNames["ST"].push_back( "S3inc" );
            fieldCompNames["ST"].push_back( "S3azi" );

            fieldCompNames["Gamma"].push_back( "Gamma1" );
            fieldCompNames["Gamma"].push_back( "Gamma2" );
            fieldCompNames["Gamma"].push_back( "Gamma3" );
            fieldCompNames["Gamma"].push_back( "Gamma11" );
            fieldCompNames["Gamma"].push_back( "Gamma22" );
            fieldCompNames["Gamma"].push_back( "Gamma33" );

            fieldCompNames["NE"].push_back( "EV" );
            fieldCompNames["NE"].push_back( "ED" );
            fieldCompNames["NE"].push_back( "E11" );
            fieldCompNames["NE"].push_back( "E22" );
            fieldCompNames["NE"].push_back( "E33" );
            fieldCompNames["NE"].push_back( "E12" );
            fieldCompNames["NE"].push_back( "E13" );
            fieldCompNames["NE"].push_back( "E23" );
            fieldCompNames["NE"].push_back( "E1" );
            fieldCompNames["NE"].push_back( "E2" );
            fieldCompNames["NE"].push_back( "E3" );
        }
        else if ( resPos == RIG_INTEGRATION_POINT )
        {
            fieldCompNames = m_readerInterface->scalarIntegrationPointFieldAndComponentNames();

            fieldCompNames["SE"].push_back( "SEM" );
            fieldCompNames["SE"].push_back( "SFI" );
            fieldCompNames["SE"].push_back( "DSM" );
            fieldCompNames["SE"].push_back( "FOS" );

            fieldCompNames["SE"].push_back( "S11" );
            fieldCompNames["SE"].push_back( "S22" );
            fieldCompNames["SE"].push_back( "S33" );
            fieldCompNames["SE"].push_back( "S12" );
            fieldCompNames["SE"].push_back( "S13" );
            fieldCompNames["SE"].push_back( "S23" );
            fieldCompNames["SE"].push_back( "S1" );
            fieldCompNames["SE"].push_back( "S2" );
            fieldCompNames["SE"].push_back( "S3" );

            fieldCompNames["SE"].push_back( "S1inc" );
            fieldCompNames["SE"].push_back( "S1azi" );
            fieldCompNames["SE"].push_back( "S2inc" );
            fieldCompNames["SE"].push_back( "S2azi" );
            fieldCompNames["SE"].push_back( "S3inc" );
            fieldCompNames["SE"].push_back( "S3azi" );

            fieldCompNames["ST"].push_back( "STM" );
            fieldCompNames["ST"].push_back( "Q" );

            fieldCompNames["ST"].push_back( "S11" );
            fieldCompNames["ST"].push_back( "S22" );
            fieldCompNames["ST"].push_back( "S33" );
            fieldCompNames["ST"].push_back( "S12" );
            fieldCompNames["ST"].push_back( "S13" );
            fieldCompNames["ST"].push_back( "S23" );
            fieldCompNames["ST"].push_back( "S1" );
            fieldCompNames["ST"].push_back( "S2" );
            fieldCompNames["ST"].push_back( "S3" );

            fieldCompNames["ST"].push_back( "S1inc" );
            fieldCompNames["ST"].push_back( "S1azi" );
            fieldCompNames["ST"].push_back( "S2inc" );
            fieldCompNames["ST"].push_back( "S2azi" );
            fieldCompNames["ST"].push_back( "S3inc" );
            fieldCompNames["ST"].push_back( "S3azi" );

            fieldCompNames["Gamma"].push_back( "Gamma1" );
            fieldCompNames["Gamma"].push_back( "Gamma2" );
            fieldCompNames["Gamma"].push_back( "Gamma3" );
            fieldCompNames["Gamma"].push_back( "Gamma11" );
            fieldCompNames["Gamma"].push_back( "Gamma22" );
            fieldCompNames["Gamma"].push_back( "Gamma33" );

            fieldCompNames["NE"].push_back( "EV" );
            fieldCompNames["NE"].push_back( "ED" );
            fieldCompNames["NE"].push_back( "E11" );
            fieldCompNames["NE"].push_back( "E22" );
            fieldCompNames["NE"].push_back( "E33" );
            fieldCompNames["NE"].push_back( "E12" );
            fieldCompNames["NE"].push_back( "E13" );
            fieldCompNames["NE"].push_back( "E23" );
            fieldCompNames["NE"].push_back( "E1" );
            fieldCompNames["NE"].push_back( "E2" );
            fieldCompNames["NE"].push_back( "E3" );
        }
        else if ( resPos == RIG_ELEMENT_NODAL_FACE )
        {
            fieldCompNames["Plane"].push_back( "Pinc" );
            fieldCompNames["Plane"].push_back( "Pazi" );

            fieldCompNames["SE"].push_back( "SN" );
            fieldCompNames["SE"].push_back( "TP" );
            fieldCompNames["SE"].push_back( "TPinc" );
            fieldCompNames["SE"].push_back( "TPH" );
            fieldCompNames["SE"].push_back( "TPQV" );
            fieldCompNames["SE"].push_back( "FAULTMOB" );
            fieldCompNames["SE"].push_back( "PCRIT" );

            fieldCompNames["ST"].push_back( "SN" );
            fieldCompNames["ST"].push_back( "TP" );
            fieldCompNames["ST"].push_back( "TPinc" );
            fieldCompNames["ST"].push_back( "TPH" );
            fieldCompNames["ST"].push_back( "TPQV" );
            fieldCompNames["ST"].push_back( "FAULTMOB" );
            fieldCompNames["ST"].push_back( "PCRIT" );
        }
        else if ( resPos == RIG_ELEMENT )
        {
            for ( const std::string& field : m_elementPropertyReader->scalarElementFields() )
            {
                fieldCompNames[field];
            }
        }
        else if ( resPos == RIG_WELLPATH_DERIVED )
        {
            std::vector<QString> angles = RiaDefines::wbsAngleResultNames();
            for ( QString angle : angles )
            {
                fieldCompNames[angle.toStdString()];
            }
            std::vector<QString> derivedResults = RiaDefines::wbsDerivedResultNames();
            for ( QString result : derivedResults )
            {
                fieldCompNames[result.toStdString()];
            }
            std::set<RigWbsParameter> wbsParameters = RigWbsParameter::allParameters();
            for ( const RigWbsParameter& parameter : wbsParameters )
            {
                fieldCompNames[parameter.name().toStdString()];
            }
        }
        else if ( resPos == RIG_DIFFERENTIALS )
        {
            fieldCompNames["POR-Bar"];
            fieldCompNames["POR-Bar"].push_back( "X" );
            fieldCompNames["POR-Bar"].push_back( "Y" );
            fieldCompNames["POR-Bar"].push_back( "Z" );

            for ( auto& s : stressGradientComponentNames )
            {
                fieldCompNames["SE"].push_back( s );
            }

            for ( auto& s : stressGradientComponentNames )
            {
                fieldCompNames["ST"].push_back( s );
            }
        }
    }

    return fieldCompNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultsCollection::calculateBarConvertedResult( int                        partIndex,
                                                              const RigFemResultAddress& convertedResultAddr,
                                                              const std::string&         fieldNameToConvert )
{
    caf::ProgressInfo frameCountProgress( this->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " +
        QString::fromStdString( convertedResultAddr.fieldName + ": " + convertedResultAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemResultAddress       unconvertedResultAddr( convertedResultAddr.resultPosType,
                                               fieldNameToConvert,
                                               convertedResultAddr.componentName );
    RigFemScalarResultFrames* srcDataFrames = this->findOrLoadScalarResult( partIndex, unconvertedResultAddr );
    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( convertedResultAddr );

    frameCountProgress.incrementProgress();

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData = dstDataFrames->frameData( fIdx );
        size_t                    valCount     = srcFrameData.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = 1.0e-5 * srcFrameData[vIdx];
        }

        frameCountProgress.incrementProgress();
    }
    this->deleteResult( unconvertedResultAddr );
    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
/// Convert POR NODAL result to POR-Bar Elment Nodal result
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultsCollection::calculateEnIpPorBarResult( int partIndex, const RigFemResultAddress& convertedResultAddr )
{
    caf::ProgressInfo frameCountProgress( this->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " +
        QString::fromStdString( convertedResultAddr.fieldName + ": " + convertedResultAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemResultAddress       unconvertedResultAddr( RIG_NODAL, "POR", "" );
    RigFemScalarResultFrames* srcDataFrames = this->findOrLoadScalarResult( partIndex, unconvertedResultAddr );
    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( convertedResultAddr );

    frameCountProgress.incrementProgress();

    const RigFemPart* femPart = m_femParts->part( partIndex );
    float             inf     = std::numeric_limits<float>::infinity();

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData = dstDataFrames->frameData( fIdx );

        if ( srcFrameData.empty() ) continue; // Create empty results if we have no POR result.

        size_t valCount = femPart->elementNodeResultCount();
        dstFrameData.resize( valCount, inf );

        int elementCount = femPart->elementCount();

#pragma omp parallel for schedule( dynamic )
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType( elmIdx );

            if ( elmType == HEX8P )
            {
                int elmNodeCount = RigFemTypes::elmentNodeCount( elmType );
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx        = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    int    nodeIdx             = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );
                    dstFrameData[elmNodResIdx] = 1.0e-5 * srcFrameData[nodeIdx];
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    this->deleteResult( unconvertedResultAddr );
    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateTimeLapseResult( int                        partIndex,
                                                                                 const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.isTimeLapse() );

    if ( resVarAddr.fieldName != "Gamma" )
    {
        caf::ProgressInfo frameCountProgress( this->frameCount() * 2, "" );
        frameCountProgress.setProgressDescription(
            "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
        frameCountProgress.setNextProgressIncrement( this->frameCount() );

        RigFemResultAddress       resVarNative( resVarAddr.resultPosType,
                                          resVarAddr.fieldName,
                                          resVarAddr.componentName,
                                          RigFemResultAddress::noTimeLapseValue(),
                                          resVarAddr.refKLayerIndex );
        RigFemScalarResultFrames* srcDataFrames = this->findOrLoadScalarResult( partIndex, resVarNative );
        RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

        frameCountProgress.incrementProgress();

        int frameCount   = srcDataFrames->frameCount();
        int baseFrameIdx = resVarAddr.timeLapseBaseFrameIdx;
        if ( baseFrameIdx >= frameCount ) return dstDataFrames;
        const std::vector<float>& baseFrameData = srcDataFrames->frameData( baseFrameIdx );
        if ( baseFrameData.empty() ) return dstDataFrames;

        for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
        {
            const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
            if ( srcFrameData.empty() ) continue; // Create empty results

            std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
            size_t              valCount     = srcFrameData.size();
            dstFrameData.resize( valCount );

#pragma omp parallel for
            for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
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

        caf::ProgressInfo frameCountProgress( this->frameCount() * 3, "" );
        frameCountProgress.setProgressDescription(
            "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
        frameCountProgress.setNextProgressIncrement( this->frameCount() );

        RigFemResultAddress totStressCompAddr( resVarAddr.resultPosType, "ST", "", resVarAddr.timeLapseBaseFrameIdx );
        {
            std::string scomp;
            std::string gcomp = resVarAddr.componentName;
            if ( gcomp == "Gamma1" )
                scomp = "S1";
            else if ( gcomp == "Gamma2" )
                scomp = "S2";
            else if ( gcomp == "Gamma3" )
                scomp = "S3";
            else if ( gcomp == "Gamma11" )
                scomp = "S11";
            else if ( gcomp == "Gamma22" )
                scomp = "S22";
            else if ( gcomp == "Gamma33" )
                scomp = "S33";
            totStressCompAddr.componentName = scomp;
        }

        RigFemScalarResultFrames* srcDataFrames = this->findOrLoadScalarResult( partIndex, totStressCompAddr );
        frameCountProgress.incrementProgress();
        frameCountProgress.setNextProgressIncrement( this->frameCount() );
        RigFemScalarResultFrames* srcPORDataFrames =
            this->findOrLoadScalarResult( partIndex,
                                          RigFemResultAddress( RIG_NODAL, "POR-Bar", "", resVarAddr.timeLapseBaseFrameIdx ) );
        RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

        frameCountProgress.incrementProgress();

        calculateGammaFromFrames( partIndex, srcDataFrames, srcPORDataFrames, dstDataFrames, &frameCountProgress );
        if ( resVarAddr.normalizeByHydrostaticPressure() && isNormalizableResult( resVarAddr ) )
        {
            dstDataFrames = calculateNormalizedResult( partIndex, resVarAddr );
        }

        return dstDataFrames;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateMeanStressSEM( int                        partIndex,
                                                                               const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SEM" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 4, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* sa11 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S11" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* sa22 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S22" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* sa33 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S33" ) );

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = sa11->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& sa11Data = sa11->frameData( fIdx );
        const std::vector<float>& sa22Data = sa22->frameData( fIdx );
        const std::vector<float>& sa33Data = sa33->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = sa11Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = ( sa11Data[vIdx] + sa22Data[vIdx] + sa33Data[vIdx] ) / 3.0f;
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateSFI( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SFI" );
    caf::ProgressInfo frameCountProgress( this->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* se1Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S1" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* se3Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S3" ) );

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    float cohPrFricAngle = (float)( m_cohesion / tan( m_frictionAngleRad ) );
    float sinFricAng     = sin( m_frictionAngleRad );

    int frameCount = se1Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& se1Data = se1Frames->frameData( fIdx );
        const std::vector<float>& se3Data = se3Frames->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = se1Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            float se1        = se1Data[vIdx];
            float se3        = se3Data[vIdx];
            float se1Se3Diff = se1 - se3;

            if ( fabs( se1Se3Diff ) < 1e-7 )
            {
                dstFrameData[vIdx] = std::numeric_limits<float>::infinity();
            }
            else
            {
                dstFrameData[vIdx] = ( ( cohPrFricAngle + 0.5 * ( se1Data[vIdx] + se3Data[vIdx] ) ) * sinFricAng ) /
                                     ( 0.5 * ( se1Data[vIdx] - se3Data[vIdx] ) );
            }
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDSM( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "DSM" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* se1Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S1" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* se3Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "S3" ) );

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    float tanFricAng        = tan( m_frictionAngleRad );
    float cohPrTanFricAngle = (float)( m_cohesion / tanFricAng );
    int   frameCount        = se1Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& se1Data = se1Frames->frameData( fIdx );
        const std::vector<float>& se3Data = se3Frames->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = se1Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = dsm( se1Data[vIdx], se3Data[vIdx], tanFricAng, cohPrTanFricAngle );
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateFOS( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "FOS" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* dsmFrames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "SE", "DSM" ) );

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = dsmFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& dsmData = dsmFrames->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = dsmData.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            float dsm          = dsmData[vIdx];
            dstFrameData[vIdx] = 1.0f / dsm;
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateMeanStressSTM( int                        partIndex,
                                                                               const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "ST" && resVarAddr.componentName == "STM" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 4, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* st11 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "S11" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* st22 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "S22" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* st33 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "S33" ) );

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = st11->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& st11Data = st11->frameData( fIdx );
        const std::vector<float>& st22Data = st22->frameData( fIdx );
        const std::vector<float>& st33Data = st33->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = st11Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = ( st11Data[vIdx] + st22Data[vIdx] + st33Data[vIdx] ) / 3.0f;
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateStressGradients( int                        partIndex,
                                                                                 const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "ST" || resVarAddr.fieldName == "SE" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating gradient: " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    QString origFieldName     = QString::fromStdString( resVarAddr.fieldName );
    QString origComponentName = QString::fromStdString( resVarAddr.componentName );
    // Split out the direction of the component name: SE-X => SE
    QString componentName = origComponentName.left( origComponentName.lastIndexOf( QChar( '-' ) ) );

    RigFemScalarResultFrames* inputResultFrames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType,
                                                           resVarAddr.fieldName,
                                                           componentName.toStdString() ) );

    RigFemScalarResultFrames* dataFramesX = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, componentName.toStdString() + "-X" ) );
    RigFemScalarResultFrames* dataFramesY = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, componentName.toStdString() + "-Y" ) );
    RigFemScalarResultFrames* dataFramesZ = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, componentName.toStdString() + "-Z" ) );
    frameCountProgress.incrementProgress();

    const RigFemPart*              femPart      = m_femParts->part( partIndex );
    int                            elementCount = femPart->elementCount();
    const std::vector<cvf::Vec3f>& nodeCoords   = femPart->nodes().coordinates;

    int frameCount = inputResultFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& inputData = inputResultFrames->frameData( fIdx );

        std::vector<float>& dstFrameDataX = dataFramesX->frameData( fIdx );
        std::vector<float>& dstFrameDataY = dataFramesY->frameData( fIdx );
        std::vector<float>& dstFrameDataZ = dataFramesZ->frameData( fIdx );
        size_t              valCount      = inputData.size();
        dstFrameDataX.resize( valCount );
        dstFrameDataY.resize( valCount );
        dstFrameDataZ.resize( valCount );

#pragma omp parallel for schedule( dynamic )
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            const int*     cornerIndices = femPart->connectivities( elmIdx );
            RigElementType elmType       = femPart->elementType( elmIdx );

            if ( !( elmType == HEX8P || elmType == HEX8 ) ) continue;

            // Find the corner coordinates for element
            std::array<cvf::Vec3d, 8> hexCorners;
            for ( int i = 0; i < 8; i++ )
            {
                hexCorners[i] = cvf::Vec3d( nodeCoords[cornerIndices[i]] );
            }

            // Find the corresponding corner values for the element
            std::array<double, 8> cornerValues;

            int elmNodeCount = RigFemTypes::elmentNodeCount( elmType );
            for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
            {
                size_t elmNodResIdx     = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                cornerValues[elmNodIdx] = inputData[elmNodResIdx];
            }

            std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

            for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
            {
                size_t elmNodResIdx         = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                dstFrameDataX[elmNodResIdx] = gradients[elmNodIdx].x();
                dstFrameDataY[elmNodResIdx] = gradients[elmNodIdx].y();
                dstFrameDataZ[elmNodResIdx] = gradients[elmNodIdx].z();
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedStress = this->findOrLoadScalarResult( partIndex, resVarAddr );
    CVF_ASSERT( requestedStress );
    return requestedStress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateNodalGradients( int                        partIndex,
                                                                                const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "POR-Bar" );
    CVF_ASSERT( resVarAddr.componentName == "X" || resVarAddr.componentName == "Y" || resVarAddr.componentName == "Z" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 5, "" );
    frameCountProgress.setProgressDescription(
        "Calculating gradient: " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* dataFramesX =
        m_femPartResults[partIndex]->createScalarResult( RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "X" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* dataFramesY =
        m_femPartResults[partIndex]->createScalarResult( RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "Y" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* dataFramesZ =
        m_femPartResults[partIndex]->createScalarResult( RigFemResultAddress( RIG_NODAL, resVarAddr.fieldName, "Z" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemResultAddress       porResultAddr( RIG_NODAL, "POR-Bar", "" );
    RigFemScalarResultFrames* srcDataFrames = this->findOrLoadScalarResult( partIndex, porResultAddr );

    frameCountProgress.incrementProgress();

    const RigFemPart* femPart = m_femParts->part( partIndex );
    float             inf     = std::numeric_limits<float>::infinity();

    const std::vector<cvf::Vec3f>& nodeCoords = femPart->nodes().coordinates;
    size_t                         nodeCount  = femPart->nodes().nodeIds.size();

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData  = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameDataX = dataFramesX->frameData( fIdx );
        std::vector<float>&       dstFrameDataY = dataFramesY->frameData( fIdx );
        std::vector<float>&       dstFrameDataZ = dataFramesZ->frameData( fIdx );

        if ( srcFrameData.empty() ) continue; // Create empty results if we have no POR result.

        size_t valCount = femPart->elementNodeResultCount();
        dstFrameDataX.resize( valCount, inf );
        dstFrameDataY.resize( valCount, inf );
        dstFrameDataZ.resize( valCount, inf );

        int elementCount = femPart->elementCount();

#pragma omp parallel for schedule( dynamic )
        for ( long nodeIdx = 0; nodeIdx < static_cast<long>( nodeCount ); nodeIdx++ )
        {
            const std::vector<int> elements = femPart->elementsUsingNode( nodeIdx );

            // Compute the average of the elements contributing to this node
            cvf::Vec3d result         = cvf::Vec3d::ZERO;
            int        nValidElements = 0;
            for ( int elmIdx : elements )
            {
                RigElementType elmType = femPart->elementType( elmIdx );
                if ( elmType == HEX8P )
                {
                    // Find the corner coordinates and values for the node
                    std::array<cvf::Vec3d, 8> hexCorners;
                    std::array<double, 8>     cornerValues;

                    int elmNodeCount = RigFemTypes::elmentNodeCount( elmType );
                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        size_t elmNodResIdx   = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        size_t resultValueIdx = femPart->resultValueIdxFromResultPosType( RIG_NODAL, elmIdx, elmNodIdx );

                        cornerValues[elmNodIdx] = srcFrameData[resultValueIdx];
                        hexCorners[elmNodIdx]   = cvf::Vec3d( nodeCoords[resultValueIdx] );
                    }

                    std::array<cvf::Vec3d, 8> gradients = RigHexGradientTools::gradients( hexCorners, cornerValues );

                    for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                    {
                        size_t elmNodResIdx   = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                        size_t resultValueIdx = femPart->resultValueIdxFromResultPosType( RIG_NODAL, elmIdx, elmNodIdx );
                        // Only use the gradient for particular corner corresponding to the node
                        if ( static_cast<size_t>( nodeIdx ) == resultValueIdx )
                        {
                            result.add( gradients[elmNodIdx] );
                        }
                    }

                    nValidElements++;
                }
            }

            if ( nValidElements > 0 )
            {
                // Round very small values to zero to avoid ugly coloring when gradients
                // are dominated by floating point math artifacts.
                double epsilon = 1.0e-6;
                result /= static_cast<double>( nValidElements );
                dstFrameDataX[nodeIdx] = std::abs( result.x() ) > epsilon ? result.x() : 0.0;
                dstFrameDataY[nodeIdx] = std::abs( result.y() ) > epsilon ? result.y() : 0.0;
                dstFrameDataZ[nodeIdx] = std::abs( result.z() ) > epsilon ? result.z() : 0.0;
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedGradient = this->findOrLoadScalarResult( partIndex, resVarAddr );
    CVF_ASSERT( requestedGradient );
    return requestedGradient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateNormalizedResult( int                        partIndex,
                                                                                  const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.normalizeByHydrostaticPressure() && isNormalizableResult( resVarAddr ) );

    RigFemResultAddress unscaledResult             = resVarAddr;
    unscaledResult.normalizedByHydrostaticPressure = false;

    caf::ProgressInfo frameCountProgress( this->frameCount() * 3, "Calculating Normalized Result" );

    RigFemScalarResultFrames* srcDataFrames = nullptr;
    RigFemScalarResultFrames* dstDataFrames = nullptr;

    {
        auto task     = frameCountProgress.task( "Loading Unscaled Result", this->frameCount() );
        srcDataFrames = this->findOrLoadScalarResult( partIndex, unscaledResult );
        if ( !srcDataFrames ) return nullptr;
    }
    {
        auto task     = frameCountProgress.task( "Creating Space for Normalized Result", this->frameCount() );
        dstDataFrames = m_femPartResults[partIndex]->createScalarResult( RigFemResultAddress( resVarAddr ) );
        if ( !dstDataFrames ) return nullptr;
    }

    frameCountProgress.setProgressDescription( "Normalizing Result" );
    frameCountProgress.setNextProgressIncrement( 1u );

    const RigFemPart*              femPart      = m_femParts->part( partIndex );
    float                          inf          = std::numeric_limits<float>::infinity();
    int                            elmNodeCount = femPart->elementCount();
    const std::vector<cvf::Vec3f>& nodeCoords   = femPart->nodes().coordinates;

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData = dstDataFrames->frameData( fIdx );

        size_t resultCount = srcFrameData.size();
        dstFrameData.resize( resultCount );

        if ( resVarAddr.resultPosType == RIG_ELEMENT_NODAL )
        {
#pragma omp parallel for schedule( dynamic )
            for ( int elmNodeResIdx = 0; elmNodeResIdx < resultCount; ++elmNodeResIdx )
            {
                const int        nodeIdx             = femPart->nodeIdxFromElementNodeResultIdx( elmNodeResIdx );
                const cvf::Vec3f node                = nodeCoords[nodeIdx];
                double           tvdRKB              = std::abs( node.z() ) + m_normalizationAirGap;
                double           hydrostaticPressure = RiaWellLogUnitTools::hydrostaticPorePressureBar( tvdRKB );
                dstFrameData[elmNodeResIdx]          = srcFrameData[elmNodeResIdx] / hydrostaticPressure;
            }
        }
        else if ( resVarAddr.resultPosType == RIG_NODAL )
        {
#pragma omp parallel for schedule( dynamic )
            for ( int nodeResIdx = 0; nodeResIdx < resultCount; ++nodeResIdx )
            {
                const cvf::Vec3f node                = nodeCoords[nodeResIdx];
                double           tvdRKB              = std::abs( node.z() ) + m_normalizationAirGap;
                double           hydrostaticPressure = RiaWellLogUnitTools::hydrostaticPorePressureBar( tvdRKB );
                dstFrameData[nodeResIdx]             = srcFrameData[nodeResIdx] / hydrostaticPressure;
            }
        }
        frameCountProgress.incrementProgress();
    }
    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDeviatoricStress( int                        partIndex,
                                                                                  const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "ST" && resVarAddr.componentName == "Q" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 5, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* st11 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "S1" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* st22 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "S2" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* st33 =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "S3" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* stm =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "ST", "STM" ) );

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = st11->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& st11Data = st11->frameData( fIdx );
        const std::vector<float>& st22Data = st22->frameData( fIdx );
        const std::vector<float>& st33Data = st33->frameData( fIdx );

        const std::vector<float>& stmData = stm->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = st11Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            float stmVal   = stmData[vIdx];
            float st11Corr = st11Data[vIdx] - stmVal;
            float st22Corr = st22Data[vIdx] - stmVal;
            float st33Corr = st33Data[vIdx] - stmVal;

            dstFrameData[vIdx] = sqrt( 1.5 * ( st11Corr * st11Corr + st22Corr * st22Corr + st33Corr * st33Corr ) );
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateVolumetricStrain( int                        partIndex,
                                                                                  const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "NE" && resVarAddr.componentName == "EV" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 4, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* ea11 = nullptr;
    RigFemScalarResultFrames* ea22 = nullptr;
    RigFemScalarResultFrames* ea33 = nullptr;

    {
        ea11 = this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "NE", "E11" ) );
        frameCountProgress.incrementProgress();
        frameCountProgress.setNextProgressIncrement( this->frameCount() );
        ea22 = this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "NE", "E22" ) );
        frameCountProgress.incrementProgress();
        frameCountProgress.setNextProgressIncrement( this->frameCount() );
        ea33 = this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "NE", "E33" ) );
    }

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = ea11->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& ea11Data = ea11->frameData( fIdx );
        const std::vector<float>& ea22Data = ea22->frameData( fIdx );
        const std::vector<float>& ea33Data = ea33->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = ea11Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = ( ea11Data[vIdx] + ea22Data[vIdx] + ea33Data[vIdx] );
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDeviatoricStrain( int                        partIndex,
                                                                                  const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == "NE" && resVarAddr.componentName == "ED" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* ea11 = nullptr;
    RigFemScalarResultFrames* ea33 = nullptr;
    {
        ea11 = this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "NE", "E1" ) );
        frameCountProgress.incrementProgress();
        frameCountProgress.setNextProgressIncrement( this->frameCount() );
        ea33 = this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "NE", "E3" ) );
    }

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = ea11->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& ea11Data = ea11->frameData( fIdx );
        const std::vector<float>& ea33Data = ea33->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );
        size_t              valCount     = ea11Data.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = 0.666666666666667f * ( ea11Data[vIdx] - ea33Data[vIdx] );
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultsCollection::calculateSurfaceAlignedStress( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.componentName == "STH" || resVarAddr.componentName == "STQV" ||
                resVarAddr.componentName == "SN" || resVarAddr.componentName == "TPH" ||
                resVarAddr.componentName == "TPQV" || resVarAddr.componentName == "THQV" ||
                resVarAddr.componentName == "TP" || resVarAddr.componentName == "TPinc" ||
                resVarAddr.componentName == "FAULTMOB" || resVarAddr.componentName == "PCRIT" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 7, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* s11Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S11" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s22Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S22" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s33Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S33" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s12Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S12" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s23Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S23" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s13Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_ELEMENT_NODAL, resVarAddr.fieldName, "S13" ) );

    RigFemScalarResultFrames* SNFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "SN" ) );
    RigFemScalarResultFrames* STHFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "STH" ) );
    RigFemScalarResultFrames* STQVFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "STQV" ) );
    RigFemScalarResultFrames* TNHFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "TPH" ) );
    RigFemScalarResultFrames* TNQVFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "TPQV" ) );
    RigFemScalarResultFrames* THQVFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "THQV" ) );
    RigFemScalarResultFrames* TPFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "TP" ) );
    RigFemScalarResultFrames* TPincFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "TPinc" ) );
    RigFemScalarResultFrames* FAULTMOBFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "FAULTMOB" ) );
    RigFemScalarResultFrames* PCRITFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "PCRIT" ) );

    frameCountProgress.incrementProgress();

    const RigFemPart*              femPart         = m_femParts->part( partIndex );
    const std::vector<cvf::Vec3f>& nodeCoordinates = femPart->nodes().coordinates;

    float tanFricAng        = tan( m_frictionAngleRad );
    float cohPrTanFricAngle = (float)( m_cohesion / tanFricAng );

    int frameCount = s11Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& s11 = s11Frames->frameData( fIdx );
        const std::vector<float>& s22 = s22Frames->frameData( fIdx );
        const std::vector<float>& s33 = s33Frames->frameData( fIdx );
        const std::vector<float>& s12 = s12Frames->frameData( fIdx );
        const std::vector<float>& s23 = s23Frames->frameData( fIdx );
        const std::vector<float>& s13 = s13Frames->frameData( fIdx );

        std::vector<float>& SNDat       = SNFrames->frameData( fIdx );
        std::vector<float>& STHDat      = STHFrames->frameData( fIdx );
        std::vector<float>& STQVDat     = STQVFrames->frameData( fIdx );
        std::vector<float>& TNHDat      = TNHFrames->frameData( fIdx );
        std::vector<float>& TNQVDat     = TNQVFrames->frameData( fIdx );
        std::vector<float>& THQVDat     = THQVFrames->frameData( fIdx );
        std::vector<float>& TPDat       = TPFrames->frameData( fIdx );
        std::vector<float>& TincDat     = TPincFrames->frameData( fIdx );
        std::vector<float>& FAULTMOBDat = FAULTMOBFrames->frameData( fIdx );
        std::vector<float>& PCRITDat    = PCRITFrames->frameData( fIdx );

        // HACK ! Todo : make it robust against other elements than Hex8
        size_t valCount = s11.size() * 3; // Number of Elm Node Face results 24 = 4 * num faces = 3* numElmNodes

        SNDat.resize( valCount );
        STHDat.resize( valCount );
        STQVDat.resize( valCount );
        TNHDat.resize( valCount );
        TNQVDat.resize( valCount );
        THQVDat.resize( valCount );
        TPDat.resize( valCount );
        TincDat.resize( valCount );
        FAULTMOBDat.resize( valCount );
        PCRITDat.resize( valCount );

        int elementCount = femPart->elementCount();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType        = femPart->elementType( elmIdx );
            int            faceCount      = RigFemTypes::elmentFaceCount( elmType );
            const int*     elmNodeIndices = femPart->connectivities( elmIdx );

            int elmNodFaceResIdxElmStart = elmIdx * 24; // HACK should get from part

            for ( int lfIdx = 0; lfIdx < faceCount; ++lfIdx )
            {
                int        faceNodeCount = 0;
                const int* localElmNodeIndicesForFace =
                    RigFemTypes::localElmNodeIndicesForFace( elmType, lfIdx, &faceNodeCount );
                if ( faceNodeCount == 4 )
                {
                    int        elmNodFaceResIdxFaceStart = elmNodFaceResIdxElmStart + lfIdx * 4; // HACK
                    cvf::Vec3f quadVxs[4];

                    quadVxs[0] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[0]]] );
                    quadVxs[1] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[1]]] );
                    quadVxs[2] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[2]]] );
                    quadVxs[3] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[3]]] );

                    cvf::Mat3f rotMx = cvf::GeometryTools::computePlaneHorizontalRotationMx( quadVxs[2] - quadVxs[0],
                                                                                             quadVxs[3] - quadVxs[1] );

                    size_t qElmNodeResIdx[4];
                    qElmNodeResIdx[0] = femPart->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[0] );
                    qElmNodeResIdx[1] = femPart->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[1] );
                    qElmNodeResIdx[2] = femPart->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[2] );
                    qElmNodeResIdx[3] = femPart->elementNodeResultIdx( elmIdx, localElmNodeIndicesForFace[3] );

                    for ( int qIdx = 0; qIdx < 4; ++qIdx )
                    {
                        size_t elmNodResIdx = qElmNodeResIdx[qIdx];
                        float  t11          = s11[elmNodResIdx];
                        float  t22          = s22[elmNodResIdx];
                        float  t33          = s33[elmNodResIdx];
                        float  t12          = s12[elmNodResIdx];
                        float  t23          = s23[elmNodResIdx];
                        float  t13          = s13[elmNodResIdx];

                        caf::Ten3f tensor( t11, t22, t33, t12, t23, t13 );
                        caf::Ten3f xfTen            = tensor.rotated( rotMx );
                        int        elmNodFaceResIdx = elmNodFaceResIdxFaceStart + qIdx;

                        float szx = xfTen[caf::Ten3f::SZX];
                        float syz = xfTen[caf::Ten3f::SYZ];
                        float szz = xfTen[caf::Ten3f::SZZ];

                        STHDat[elmNodFaceResIdx]  = xfTen[caf::Ten3f::SXX];
                        STQVDat[elmNodFaceResIdx] = xfTen[caf::Ten3f::SYY];
                        SNDat[elmNodFaceResIdx]   = xfTen[caf::Ten3f::SZZ];

                        TNHDat[elmNodFaceResIdx]  = xfTen[caf::Ten3f::SZX];
                        TNQVDat[elmNodFaceResIdx] = xfTen[caf::Ten3f::SYZ];
                        THQVDat[elmNodFaceResIdx] = xfTen[caf::Ten3f::SXY];

                        float TP                = sqrt( szx * szx + syz * syz );
                        TPDat[elmNodFaceResIdx] = TP;

                        if ( TP > 1e-5 )
                        {
                            TincDat[elmNodFaceResIdx] = cvf::Math::toDegrees( acos( syz / TP ) );
                        }
                        else
                        {
                            TincDat[elmNodFaceResIdx] = std::numeric_limits<float>::infinity();
                        }

                        FAULTMOBDat[elmNodFaceResIdx] = TP / ( tanFricAng * ( szz + cohPrTanFricAngle ) );
                        PCRITDat[elmNodFaceResIdx]    = szz - TP / tanFricAng;
                    }
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedSurfStress = this->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedSurfStress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateSurfaceAngles( int                        partIndex,
                                                                               const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.componentName == "Pazi" || resVarAddr.componentName == "Pinc" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 1, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );

    RigFemScalarResultFrames* PaziFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "Pazi" ) );
    RigFemScalarResultFrames* PincFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "Pinc" ) );

    const RigFemPart*              femPart         = m_femParts->part( partIndex );
    const std::vector<cvf::Vec3f>& nodeCoordinates = femPart->nodes().coordinates;
    int                            frameCount      = this->frameCount();

    // HACK ! Todo : make it robust against other elements than Hex8
    size_t valCount = femPart->elementCount() * 24; // Number of Elm Node Face results 24 = 4 * num faces = 3* numElmNodes

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        std::vector<float>& Pazi = PaziFrames->frameData( fIdx );
        std::vector<float>& Pinc = PincFrames->frameData( fIdx );

        Pazi.resize( valCount );
        Pinc.resize( valCount );

        int elementCount = femPart->elementCount();
#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType        = femPart->elementType( elmIdx );
            int            faceCount      = RigFemTypes::elmentFaceCount( elmType );
            const int*     elmNodeIndices = femPart->connectivities( elmIdx );

            int elmNodFaceResIdxElmStart = elmIdx * 24; // HACK should get from part

            for ( int lfIdx = 0; lfIdx < faceCount; ++lfIdx )
            {
                int        faceNodeCount = 0;
                const int* localElmNodeIndicesForFace =
                    RigFemTypes::localElmNodeIndicesForFace( elmType, lfIdx, &faceNodeCount );
                if ( faceNodeCount == 4 )
                {
                    int        elmNodFaceResIdxFaceStart = elmNodFaceResIdxElmStart + lfIdx * 4; // HACK
                    cvf::Vec3f quadVxs[4];

                    quadVxs[0] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[0]]] );
                    quadVxs[1] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[1]]] );
                    quadVxs[2] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[2]]] );
                    quadVxs[3] = ( nodeCoordinates[elmNodeIndices[localElmNodeIndicesForFace[3]]] );

                    cvf::Mat3f rotMx = cvf::GeometryTools::computePlaneHorizontalRotationMx( quadVxs[2] - quadVxs[0],
                                                                                             quadVxs[3] - quadVxs[1] );
                    RiaOffshoreSphericalCoords sphCoord(
                        cvf::Vec3f( rotMx.rowCol( 0, 2 ), rotMx.rowCol( 1, 2 ), rotMx.rowCol( 2, 2 ) ) ); // Use Ez from
                                                                                                          // the matrix
                                                                                                          // as plane
                                                                                                          // normal

                    for ( int qIdx = 0; qIdx < 4; ++qIdx )
                    {
                        int elmNodFaceResIdx   = elmNodFaceResIdxFaceStart + qIdx;
                        Pazi[elmNodFaceResIdx] = cvf::Math::toDegrees( sphCoord.azi() );
                        Pinc[elmNodFaceResIdx] = cvf::Math::toDegrees( sphCoord.inc() );
                    }
                }
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedPlaneAngle = this->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedPlaneAngle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultsCollection::calculatePrincipalStressValues( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.componentName == "S1" || resVarAddr.componentName == "S2" || resVarAddr.componentName == "S3" ||
                resVarAddr.componentName == "S1inc" || resVarAddr.componentName == "S1azi" ||
                resVarAddr.componentName == "S2inc" || resVarAddr.componentName == "S2azi" ||
                resVarAddr.componentName == "S3inc" || resVarAddr.componentName == "S3azi" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 7, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* s11Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S11" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s22Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S22" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s33Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S33" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s12Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S12" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s13Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S13" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s23Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S23" ) );

    RigFemScalarResultFrames* s1Frames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S1" ) );
    RigFemScalarResultFrames* s2Frames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S2" ) );
    RigFemScalarResultFrames* s3Frames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S3" ) );

    RigFemScalarResultFrames* s1IncFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S1inc" ) );
    RigFemScalarResultFrames* s1AziFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S1azi" ) );
    RigFemScalarResultFrames* s2IncFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S2inc" ) );
    RigFemScalarResultFrames* s2AziFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S2azi" ) );
    RigFemScalarResultFrames* s3IncFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S3inc" ) );
    RigFemScalarResultFrames* s3AziFrames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "S3azi" ) );

    frameCountProgress.incrementProgress();

    int frameCount = s11Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& s11 = s11Frames->frameData( fIdx );
        const std::vector<float>& s22 = s22Frames->frameData( fIdx );
        const std::vector<float>& s33 = s33Frames->frameData( fIdx );
        const std::vector<float>& s12 = s12Frames->frameData( fIdx );
        const std::vector<float>& s13 = s13Frames->frameData( fIdx );
        const std::vector<float>& s23 = s23Frames->frameData( fIdx );

        std::vector<float>& s1 = s1Frames->frameData( fIdx );
        std::vector<float>& s2 = s2Frames->frameData( fIdx );
        std::vector<float>& s3 = s3Frames->frameData( fIdx );

        std::vector<float>& s1inc = s1IncFrames->frameData( fIdx );
        std::vector<float>& s1azi = s1AziFrames->frameData( fIdx );
        std::vector<float>& s2inc = s2IncFrames->frameData( fIdx );
        std::vector<float>& s2azi = s2AziFrames->frameData( fIdx );
        std::vector<float>& s3inc = s3IncFrames->frameData( fIdx );
        std::vector<float>& s3azi = s3AziFrames->frameData( fIdx );

        size_t valCount = s11.size();

        s1.resize( valCount );
        s2.resize( valCount );
        s3.resize( valCount );
        s1inc.resize( valCount );
        s1azi.resize( valCount );
        s2inc.resize( valCount );
        s2azi.resize( valCount );
        s3inc.resize( valCount );
        s3azi.resize( valCount );

#pragma omp parallel for schedule( dynamic )
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            caf::Ten3f T( s11[vIdx], s22[vIdx], s33[vIdx], s12[vIdx], s23[vIdx], s13[vIdx] );
            cvf::Vec3f principalDirs[3];
            cvf::Vec3f principals = T.calculatePrincipals( principalDirs );
            s1[vIdx]              = principals[0];
            s2[vIdx]              = principals[1];
            s3[vIdx]              = principals[2];

            if ( principals[0] != std::numeric_limits<float>::infinity() )
            {
                RiaOffshoreSphericalCoords sphCoord1( principalDirs[0] );
                s1inc[vIdx] = cvf::Math::toDegrees( sphCoord1.inc() );
                s1azi[vIdx] = cvf::Math::toDegrees( sphCoord1.azi() );
            }
            else
            {
                s1inc[vIdx] = std::numeric_limits<float>::infinity();
                s1azi[vIdx] = std::numeric_limits<float>::infinity();
            }

            if ( principals[1] != std::numeric_limits<float>::infinity() )
            {
                RiaOffshoreSphericalCoords sphCoord2( principalDirs[1] );
                s2inc[vIdx] = cvf::Math::toDegrees( sphCoord2.inc() );
                s2azi[vIdx] = cvf::Math::toDegrees( sphCoord2.azi() );
            }
            else
            {
                s2inc[vIdx] = std::numeric_limits<float>::infinity();
                s2azi[vIdx] = std::numeric_limits<float>::infinity();
            }

            if ( principals[2] != std::numeric_limits<float>::infinity() )
            {
                RiaOffshoreSphericalCoords sphCoord3( principalDirs[2] );
                s3inc[vIdx] = cvf::Math::toDegrees( sphCoord3.inc() );
                s3azi[vIdx] = cvf::Math::toDegrees( sphCoord3.azi() );
            }
            else
            {
                s3inc[vIdx] = std::numeric_limits<float>::infinity();
                s3azi[vIdx] = std::numeric_limits<float>::infinity();
            }
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedPrincipal = this->findOrLoadScalarResult( partIndex, resVarAddr );

    return requestedPrincipal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames*
    RigFemPartResultsCollection::calculatePrincipalStrainValues( int partIndex, const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.componentName == "E1" || resVarAddr.componentName == "E2" || resVarAddr.componentName == "E3" );

    caf::ProgressInfo frameCountProgress( this->frameCount() * 7, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* s11Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E11" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s22Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E22" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s33Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E33" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s12Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E12" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s13Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E13" ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* s23Frames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E23" ) );

    RigFemScalarResultFrames* s1Frames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E1" ) );
    RigFemScalarResultFrames* s2Frames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E2" ) );
    RigFemScalarResultFrames* s3Frames = m_femPartResults[partIndex]->createScalarResult(
        RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "E3" ) );

    frameCountProgress.incrementProgress();

    int frameCount = s11Frames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& s11 = s11Frames->frameData( fIdx );
        const std::vector<float>& s22 = s22Frames->frameData( fIdx );
        const std::vector<float>& s33 = s33Frames->frameData( fIdx );
        const std::vector<float>& s12 = s12Frames->frameData( fIdx );
        const std::vector<float>& s13 = s13Frames->frameData( fIdx );
        const std::vector<float>& s23 = s23Frames->frameData( fIdx );

        std::vector<float>& s1 = s1Frames->frameData( fIdx );
        std::vector<float>& s2 = s2Frames->frameData( fIdx );
        std::vector<float>& s3 = s3Frames->frameData( fIdx );

        size_t valCount = s11.size();

        s1.resize( valCount );
        s2.resize( valCount );
        s3.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            caf::Ten3f T( s11[vIdx], s22[vIdx], s33[vIdx], s12[vIdx], s23[vIdx], s13[vIdx] );
            cvf::Vec3f principalDirs[3];
            cvf::Vec3f principals = T.calculatePrincipals( principalDirs );
            s1[vIdx]              = principals[0];
            s2[vIdx]              = principals[1];
            s3[vIdx]              = principals[2];
        }

        frameCountProgress.incrementProgress();
    }

    RigFemScalarResultFrames* requestedPrincipal = this->findOrLoadScalarResult( partIndex, resVarAddr );

    return requestedPrincipal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateCompactionValues( int                        partIndex,
                                                                                  const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.fieldName == FIELD_NAME_COMPACTION );

    caf::ProgressInfo frameCountProgress( this->frameCount() + 1, "" );
    frameCountProgress.setProgressDescription( "Calculating " + QString::fromStdString( resVarAddr.fieldName ) );

    RigFemScalarResultFrames* u3Frames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( resVarAddr.resultPosType, "U", "U3" ) );
    frameCountProgress.incrementProgress();

    RigFemScalarResultFrames* compactionFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    const RigFemPart* part = m_femParts->part( partIndex );
    part->ensureIntersectionSearchTreeIsBuilt();

    for ( int t = 0; t < u3Frames->frameCount(); t++ )
    {
        std::vector<float>& compactionFrame = compactionFrames->frameData( t );
        size_t              nodeCount       = part->nodes().nodeIds.size();

        frameCountProgress.incrementProgress();

        compactionFrame.resize( nodeCount );

        {
            // Make sure the AABB-tree is created before using OpenMP
            cvf::BoundingBox    bb;
            std::vector<size_t> refElementCandidates;

            part->findIntersectingCells( bb, &refElementCandidates );

            // Also make sure the struct grid is created, as this is required before using OpenMP
            part->getOrCreateStructGrid();
        }

#pragma omp parallel for
        for ( long n = 0; n < static_cast<long>( nodeCount ); n++ )
        {
            RefElement refElement;
            findReferenceElementForNode( *part, n, resVarAddr.refKLayerIndex, &refElement );

            if ( refElement.elementIdx != cvf::UNDEFINED_SIZE_T )
            {
                float  shortestDist      = std::numeric_limits<float>::infinity();
                size_t closestRefNodeIdx = cvf::UNDEFINED_SIZE_T;

                for ( size_t nodeIdx : refElement.elementFaceNodeIdxs )
                {
                    float dist = horizontalDistance( refElement.intersectionPoint, part->nodes().coordinates[nodeIdx] );
                    if ( dist < shortestDist )
                    {
                        shortestDist      = dist;
                        closestRefNodeIdx = nodeIdx;
                    }
                }

                cvf::Vec3f currentNodeCoord = part->nodes().coordinates[n];
                if ( currentNodeCoord.z() >= refElement.intersectionPoint.z() )
                    compactionFrame[n] = -( u3Frames->frameData( t )[n] - u3Frames->frameData( t )[closestRefNodeIdx] );
                else
                    compactionFrame[n] = -( u3Frames->frameData( t )[closestRefNodeIdx] - u3Frames->frameData( t )[n] );
            }
            else
            {
                compactionFrame[n] = HUGE_VAL;
            }
        }
    }

    RigFemScalarResultFrames* requestedPrincipal = this->findOrLoadScalarResult( partIndex, resVarAddr );
    return requestedPrincipal;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateNE( int partIndex, const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( this->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* srcDataFrames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, "E", resVarAddr.componentName ) );
    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData = dstDataFrames->frameData( fIdx );
        size_t                    valCount     = srcFrameData.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = -srcFrameData[vIdx];
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateSE( int partIndex, const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( this->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* srcDataFrames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, "S-Bar", resVarAddr.componentName ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );
    RigFemScalarResultFrames* srcPORDataFrames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, "POR-Bar", "" ) );
    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    const RigFemPart* femPart = m_femParts->part( partIndex );
    float             inf     = std::numeric_limits<float>::infinity();

    int frameCount = srcDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcSFrameData = srcDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData  = dstDataFrames->frameData( fIdx );
        size_t                    valCount      = srcSFrameData.size();
        dstFrameData.resize( valCount );

        const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData( fIdx );

        int elementCount = femPart->elementCount();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType( elmIdx );

            int elmNodeCount = RigFemTypes::elmentNodeCount( femPart->elementType( elmIdx ) );

            if ( elmType == HEX8P )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < srcSFrameData.size() )
                    {
                        dstFrameData[elmNodResIdx] = -srcSFrameData[elmNodResIdx];
                    }
                }
            }
            else
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < dstFrameData.size() )
                    {
                        dstFrameData[elmNodResIdx] = inf;
                    }
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
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateST_11_22_33( int                        partIndex,
                                                                             const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( this->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* srcSDataFrames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, "S-Bar", resVarAddr.componentName ) );
    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* srcPORDataFrames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, "POR-Bar", "" ) );

    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );
    const RigFemPart*         femPart       = m_femParts->part( partIndex );
    int                       frameCount    = srcSDataFrames->frameCount();

    frameCountProgress.incrementProgress();

    const float inf = std::numeric_limits<float>::infinity();

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcSFrameData   = srcSDataFrames->frameData( fIdx );
        const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );

        size_t valCount = srcSFrameData.size();
        dstFrameData.resize( valCount );

        int elementCount = femPart->elementCount();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType( elmIdx );

            int elmNodeCount = RigFemTypes::elmentNodeCount( femPart->elementType( elmIdx ) );

            if ( elmType == HEX8P )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < srcSFrameData.size() )
                    {
                        int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );

                        float por = srcPORFrameData[nodeIdx];
                        if ( por == inf ) por = 0.0f;

                        dstFrameData[elmNodResIdx] = -srcSFrameData[elmNodResIdx] + por;
                    }
                }
            }
            else
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < srcSFrameData.size() )
                    {
                        dstFrameData[elmNodResIdx] = -srcSFrameData[elmNodResIdx];
                    }
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
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateST_12_13_23( int                        partIndex,
                                                                             const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( this->frameCount() * 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* srcSDataFrames =
        this->findOrLoadScalarResult( partIndex,
                                      RigFemResultAddress( resVarAddr.resultPosType, "S-Bar", resVarAddr.componentName ) );
    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    int frameCount = srcSDataFrames->frameCount();
    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcSFrameData = srcSDataFrames->frameData( fIdx );
        std::vector<float>&       dstFrameData  = dstDataFrames->frameData( fIdx );

        size_t valCount = srcSFrameData.size();
        dstFrameData.resize( valCount );

#pragma omp parallel for
        for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
        {
            dstFrameData[vIdx] = -srcSFrameData[vIdx];
        }

        frameCountProgress.incrementProgress();
    }

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateGamma( int partIndex, const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( this->frameCount() * 3, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemResultAddress totStressCompAddr( resVarAddr.resultPosType, "ST", "" );
    {
        std::string scomp;
        std::string gcomp = resVarAddr.componentName;
        if ( gcomp == "Gamma1" )
            scomp = "S1";
        else if ( gcomp == "Gamma2" )
            scomp = "S2";
        else if ( gcomp == "Gamma3" )
            scomp = "S3";
        else if ( gcomp == "Gamma11" )
            scomp = "S11";
        else if ( gcomp == "Gamma22" )
            scomp = "S22";
        else if ( gcomp == "Gamma33" )
            scomp = "S33";
        totStressCompAddr.componentName = scomp;
    }

    RigFemScalarResultFrames* srcDataFrames = this->findOrLoadScalarResult( partIndex, totStressCompAddr );

    frameCountProgress.incrementProgress();
    frameCountProgress.setNextProgressIncrement( this->frameCount() );

    RigFemScalarResultFrames* srcPORDataFrames =
        this->findOrLoadScalarResult( partIndex, RigFemResultAddress( RIG_NODAL, "POR-Bar", "" ) );
    RigFemScalarResultFrames* dstDataFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );

    frameCountProgress.incrementProgress();

    calculateGammaFromFrames( partIndex, srcDataFrames, srcPORDataFrames, dstDataFrames, &frameCountProgress );

    return dstDataFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateFormationIndices( int                        partIndex,
                                                                                  const RigFemResultAddress& resVarAddr )
{
    caf::ProgressInfo frameCountProgress( 2, "" );
    frameCountProgress.setProgressDescription(
        "Calculating " + QString::fromStdString( resVarAddr.fieldName + ": " + resVarAddr.componentName ) );

    RigFemScalarResultFrames* resFrames = m_femPartResults[partIndex]->createScalarResult( resVarAddr );
    resFrames->enableAsSingleFrameResult();

    const RigFemPart*   femPart      = m_femParts->part( partIndex );
    std::vector<float>& dstFrameData = resFrames->frameData( 0 );

    size_t valCount = femPart->elementNodeResultCount();
    float  inf      = std::numeric_limits<float>::infinity();
    dstFrameData.resize( valCount, inf );

    const RigFormationNames* activeFormNames = m_activeFormationNamesData.p();

    frameCountProgress.incrementProgress();

    if ( activeFormNames )
    {
        // Has to be done before the parallel loop because the first call allocates.
        const RigFemPartGrid* structGrid = femPart->getOrCreateStructGrid();

        int elementCount = femPart->elementCount();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType      = femPart->elementType( elmIdx );
            int            elmNodeCount = RigFemTypes::elmentNodeCount( elmType );

            size_t i, j, k;
            bool   validIndex = structGrid->ijkFromCellIndex( elmIdx, &i, &j, &k );
            if ( validIndex )
            {
                int formNameIdx = activeFormNames->formationIndexFromKLayerIdx( k );

                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );

                    if ( formNameIdx != -1 )
                    {
                        dstFrameData[elmNodResIdx] = formNameIdx;
                    }
                    else
                    {
                        dstFrameData[elmNodResIdx] = HUGE_VAL;
                    }
                }
            }
        }
    }

    return resFrames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDerivedResult( int                        partIndex,
                                                                               const RigFemResultAddress& resVarAddr )
{
    if ( resVarAddr.isTimeLapse() )
    {
        return calculateTimeLapseResult( partIndex, resVarAddr );
    }

    if ( resVarAddr.normalizeByHydrostaticPressure() && isNormalizableResult( resVarAddr ) )
    {
        return calculateNormalizedResult( partIndex, resVarAddr );
    }

    if ( resVarAddr.resultPosType == RIG_ELEMENT_NODAL_FACE )
    {
        if ( resVarAddr.componentName == "Pazi" || resVarAddr.componentName == "Pinc" )
        {
            return calculateSurfaceAngles( partIndex, resVarAddr );
        }
        else if ( resVarAddr.componentName.empty() )
        {
            return nullptr;
        }
        else
        {
            return calculateSurfaceAlignedStress( partIndex, resVarAddr );
        }
    }

    if ( resVarAddr.fieldName == FIELD_NAME_COMPACTION )
    {
        return calculateCompactionValues( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SFI" )
    {
        return calculateSFI( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "DSM" )
    {
        return calculateDSM( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "FOS" )
    {
        return calculateFOS( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "NE" && resVarAddr.componentName == "EV" )
    {
        return calculateVolumetricStrain( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "NE" && resVarAddr.componentName == "ED" )
    {
        return calculateDeviatoricStrain( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "ST" && resVarAddr.componentName == "Q" )
    {
        return calculateDeviatoricStress( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "ST" && resVarAddr.componentName == "STM" )
    {
        return calculateMeanStressSTM( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "ST" || resVarAddr.fieldName == "SE" )
    {
        const std::vector<std::string> allowedComponentNames = getStressGradientComponentNames();

        for ( auto& allowedComponentName : allowedComponentNames )
        {
            if ( resVarAddr.componentName == allowedComponentName )
            {
                return calculateStressGradients( partIndex, resVarAddr );
            }
        }
    }

    if ( resVarAddr.fieldName == "SE" && resVarAddr.componentName == "SEM" )
    {
        return calculateMeanStressSEM( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "S-Bar" )
    {
        return calculateBarConvertedResult( partIndex, resVarAddr, "S" );
    }

    if ( resVarAddr.fieldName == "POR-Bar" )
    {
        if ( resVarAddr.resultPosType == RIG_NODAL )
        {
            if ( resVarAddr.componentName == "X" || resVarAddr.componentName == "Y" || resVarAddr.componentName == "Z" )
            {
                return calculateNodalGradients( partIndex, resVarAddr );
            }
            else
            {
                return calculateBarConvertedResult( partIndex, resVarAddr, "POR" );
            }
        }
        else
            return calculateEnIpPorBarResult( partIndex, resVarAddr );
    }

    if ( ( resVarAddr.fieldName == "NE" ) && ( resVarAddr.componentName == "E11" || resVarAddr.componentName == "E22" ||
                                               resVarAddr.componentName == "E33" || resVarAddr.componentName == "E12" ||
                                               resVarAddr.componentName == "E13" || resVarAddr.componentName == "E23" ) )
    {
        return calculateNE( partIndex, resVarAddr );
    }

    if ( ( resVarAddr.fieldName == "NE" ) &&
         ( resVarAddr.componentName == "E1" || resVarAddr.componentName == "E2" || resVarAddr.componentName == "E3" ) )
    {
        return calculatePrincipalStrainValues( partIndex, resVarAddr );
    }

    if ( ( resVarAddr.fieldName == "SE" ) && ( resVarAddr.componentName == "S11" || resVarAddr.componentName == "S22" ||
                                               resVarAddr.componentName == "S33" || resVarAddr.componentName == "S12" ||
                                               resVarAddr.componentName == "S13" || resVarAddr.componentName == "S23" ) )
    {
        return calculateSE( partIndex, resVarAddr );
    }

    if ( ( resVarAddr.fieldName == "SE" || resVarAddr.fieldName == "ST" ) &&
         ( resVarAddr.componentName == "S1" || resVarAddr.componentName == "S2" || resVarAddr.componentName == "S3" ||
           resVarAddr.componentName == "S1inc" || resVarAddr.componentName == "S1azi" ||
           resVarAddr.componentName == "S2inc" || resVarAddr.componentName == "S2azi" ||
           resVarAddr.componentName == "S3inc" || resVarAddr.componentName == "S3azi" ) )
    {
        return calculatePrincipalStressValues( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "ST" &&
         ( resVarAddr.componentName == "S11" || resVarAddr.componentName == "S22" || resVarAddr.componentName == "S33" ) )
    {
        return calculateST_11_22_33( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "ST" &&
         ( resVarAddr.componentName == "S12" || resVarAddr.componentName == "S13" || resVarAddr.componentName == "S23" ) )
    {
        return calculateST_12_13_23( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "ST" && resVarAddr.componentName.empty() )
    {
        // Create and return an empty result
        return m_femPartResults[partIndex]->createScalarResult( resVarAddr );
    }

    if ( resVarAddr.fieldName == "Gamma" &&
         ( resVarAddr.componentName == "Gamma1" || resVarAddr.componentName == "Gamma2" ||
           resVarAddr.componentName == "Gamma3" || resVarAddr.componentName == "Gamma11" ||
           resVarAddr.componentName == "Gamma22" || resVarAddr.componentName == "Gamma33" ) )
    {
        return calculateGamma( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "Gamma" && resVarAddr.componentName.empty() )
    {
        // Create and return an empty result
        return m_femPartResults[partIndex]->createScalarResult( resVarAddr );
    }

    if ( resVarAddr.resultPosType == RIG_FORMATION_NAMES )
    {
        return calculateFormationIndices( partIndex, resVarAddr );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::calculateGammaFromFrames( int                             partIndex,
                                                            const RigFemScalarResultFrames* totalStressComponentDataFrames,
                                                            const RigFemScalarResultFrames* srcPORDataFrames,
                                                            RigFemScalarResultFrames*       dstDataFrames,
                                                            caf::ProgressInfo*              frameCountProgress )
{
    const RigFemPart* femPart    = m_femParts->part( partIndex );
    int               frameCount = totalStressComponentDataFrames->frameCount();
    float             inf        = std::numeric_limits<float>::infinity();

    for ( int fIdx = 0; fIdx < frameCount; ++fIdx )
    {
        const std::vector<float>& srcSTFrameData  = totalStressComponentDataFrames->frameData( fIdx );
        const std::vector<float>& srcPORFrameData = srcPORDataFrames->frameData( fIdx );

        std::vector<float>& dstFrameData = dstDataFrames->frameData( fIdx );

        size_t valCount = srcSTFrameData.size();
        dstFrameData.resize( valCount );

        int elementCount = femPart->elementCount();

#pragma omp parallel for
        for ( int elmIdx = 0; elmIdx < elementCount; ++elmIdx )
        {
            RigElementType elmType = femPart->elementType( elmIdx );

            int elmNodeCount = RigFemTypes::elmentNodeCount( femPart->elementType( elmIdx ) );

            if ( elmType == HEX8P )
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < srcSTFrameData.size() )
                    {
                        int nodeIdx = femPart->nodeIdxFromElementNodeResultIdx( elmNodResIdx );

                        float por = srcPORFrameData[nodeIdx];

                        if ( por == inf || fabs( por ) < 0.01e6 * 1.0e-5 )
                            dstFrameData[elmNodResIdx] = inf;
                        else
                            dstFrameData[elmNodResIdx] = srcSTFrameData[elmNodResIdx] / por;
                    }
                }
            }
            else
            {
                for ( int elmNodIdx = 0; elmNodIdx < elmNodeCount; ++elmNodIdx )
                {
                    size_t elmNodResIdx = femPart->elementNodeResultIdx( elmIdx, elmNodIdx );
                    if ( elmNodResIdx < dstFrameData.size() )
                    {
                        dstFrameData[elmNodResIdx] = inf;
                    }
                }
            }
        }

        frameCountProgress->incrementProgress();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress>
    RigFemPartResultsCollection::getResAddrToComponentsToRead( const RigFemResultAddress& resVarAddr )
{
    std::map<std::string, std::vector<std::string>> fieldAndComponentNames;
    switch ( resVarAddr.resultPosType )
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
        default:
            break;
    }

    std::vector<RigFemResultAddress> resAddressToComponents;

    std::map<std::string, std::vector<std::string>>::iterator fcIt = fieldAndComponentNames.find( resVarAddr.fieldName );

    if ( fcIt != fieldAndComponentNames.end() )
    {
        std::vector<std::string> compNames = fcIt->second;
        if ( !resVarAddr.componentName.empty() ) // If we did not request a particular component, do not add the components
        {
            for ( const auto& compName : compNames )
            {
                resAddressToComponents.push_back(
                    RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, compName ) );
            }
        }

        if ( compNames.empty() ) // This is a scalar field. Add one component named ""
        {
            CVF_ASSERT( resVarAddr.componentName == "" );
            resAddressToComponents.push_back( resVarAddr );
        }
    }

    return resAddressToComponents;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigFemPartResultsCollection::filteredStepNames() const
{
    CVF_ASSERT( m_readerInterface.notNull() );
    return m_readerInterface->filteredStepNames();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPartResultsCollection::frameCount()
{
    return static_cast<int>( filteredStepNames().size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float RigFemPartResultsCollection::dsm( float p1, float p3, float tanFricAng, float cohPrTanFricAngle )
{
    if ( p1 == HUGE_VAL || p3 == HUGE_VAL )
    {
        return std::nan( "" );
    }

    CVF_ASSERT( p1 > p3 );

    float pi_4 = 0.785398163397448309616f;
    float rho  = 2.0f * ( atan( sqrt( ( p1 + cohPrTanFricAngle ) / ( p3 + cohPrTanFricAngle ) ) ) - pi_4 );

    return tan( rho ) / tanFricAng;
}

//--------------------------------------------------------------------------------------------------
/// Returns whether any of the parts actually had any of the requested results
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultsCollection::assertResultsLoaded( const RigFemResultAddress& resVarAddr )
{
    if ( !resVarAddr.isValid() ) return false;

    bool foundResults = false;

    for ( int pIdx = 0; pIdx < static_cast<int>( m_femPartResults.size() ); ++pIdx )
    {
        if ( m_femPartResults[pIdx].notNull() )
        {
            RigFemScalarResultFrames* scalarResults = findOrLoadScalarResult( pIdx, resVarAddr );
            for ( int fIdx = 0; fIdx < scalarResults->frameCount(); ++fIdx )
            {
                foundResults = foundResults || !scalarResults->frameData( fIdx ).empty();
            }
        }
    }

    return foundResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::deleteResult( const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( resVarAddr.isValid() );

    for ( auto& femPartResult : m_femPartResults )
    {
        if ( femPartResult.notNull() )
        {
            femPartResult->deleteScalarResult( resVarAddr );
        }
    }

    m_resultStatistics.erase( resVarAddr );

    if ( resVarAddr.representsAllTimeLapses() )
    {
        std::vector<RigFemResultAddress> addressesToDelete;
        for ( auto it : m_resultStatistics )
        {
            if ( it.first.resultPosType == resVarAddr.resultPosType && it.first.fieldName == resVarAddr.fieldName &&
                 it.first.componentName == resVarAddr.componentName )
            {
                addressesToDelete.push_back( it.first );
            }
        }

        for ( RigFemResultAddress& addr : addressesToDelete )
        {
            m_resultStatistics.erase( addr );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::deleteResultFrame( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex )
{
    CVF_ASSERT( resVarAddr.isValid() );
    RigFemScalarResultFrames* frames = m_femPartResults[partIndex]->findScalarResult( resVarAddr );
    if ( frames )
    {
        std::vector<float>().swap( frames->frameData( frameIndex ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress> RigFemPartResultsCollection::loadedResults() const
{
    std::vector<RigFemResultAddress> currentResults;
    for ( auto& femPartResult : m_femPartResults )
    {
        std::vector<RigFemResultAddress> partResults = femPartResult->loadedResults();
        currentResults.insert( currentResults.end(), partResults.begin(), partResults.end() );
    }
    return currentResults;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<float>&
    RigFemPartResultsCollection::resultValues( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex )
{
    CVF_ASSERT( resVarAddr.isValid() );

    RigFemScalarResultFrames* scalarResults = findOrLoadScalarResult( partIndex, resVarAddr );
    return scalarResults->frameData( frameIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<caf::Ten3f>
    RigFemPartResultsCollection::tensors( const RigFemResultAddress& resVarAddr, int partIndex, int frameIndex )
{
    CVF_ASSERT( resVarAddr.resultPosType == RIG_ELEMENT_NODAL || resVarAddr.resultPosType == RIG_INTEGRATION_POINT );

    std::vector<caf::Ten3f> outputTensors;

    std::vector<RigFemResultAddress> addresses = tensorComponentAddresses( resVarAddr );

    if ( addresses.empty() )
    {
        return outputTensors;
    }

    const std::vector<float>& v11 = resultValues( addresses[caf::Ten3f::SXX], partIndex, frameIndex );
    const std::vector<float>& v22 = resultValues( addresses[caf::Ten3f::SYY], partIndex, frameIndex );
    const std::vector<float>& v33 = resultValues( addresses[caf::Ten3f::SZZ], partIndex, frameIndex );
    const std::vector<float>& v12 = resultValues( addresses[caf::Ten3f::SXY], partIndex, frameIndex );
    const std::vector<float>& v13 = resultValues( addresses[caf::Ten3f::SZX], partIndex, frameIndex );
    const std::vector<float>& v23 = resultValues( addresses[caf::Ten3f::SYZ], partIndex, frameIndex );

    size_t valCount = v11.size();
    outputTensors.resize( valCount );

#pragma omp parallel for
    for ( long vIdx = 0; vIdx < static_cast<long>( valCount ); ++vIdx )
    {
        caf::Ten3f tensor( v11[vIdx], v22[vIdx], v33[vIdx], v12[vIdx], v23[vIdx], v13[vIdx] );
        outputTensors[vIdx] = tensor;
    }

    return outputTensors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigStatisticsDataCache* RigFemPartResultsCollection::statistics( const RigFemResultAddress& resVarAddr )
{
    RigStatisticsDataCache* statCache = m_resultStatistics[resVarAddr].p();
    if ( !statCache )
    {
        RigFemNativeStatCalc* calculator = new RigFemNativeStatCalc( this, resVarAddr );
        statCache                        = new RigStatisticsDataCache( calculator );
        m_resultStatistics[resVarAddr]   = statCache;
    }

    return statCache;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::minMaxScalarValues( const RigFemResultAddress& resVarAddr,
                                                      int                        frameIndex,
                                                      double*                    localMin,
                                                      double*                    localMax )
{
    this->statistics( resVarAddr )->minMaxCellScalarValues( frameIndex, *localMin, *localMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::minMaxScalarValues( const RigFemResultAddress& resVarAddr,
                                                      double*                    globalMin,
                                                      double*                    globalMax )
{
    this->statistics( resVarAddr )->minMaxCellScalarValues( *globalMin, *globalMax );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZero( const RigFemResultAddress& resVarAddr,
                                                       int                        frameIndex,
                                                       double*                    localPosClosestToZero,
                                                       double*                    localNegClosestToZero )
{
    this->statistics( resVarAddr )->posNegClosestToZero( frameIndex, *localPosClosestToZero, *localNegClosestToZero );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZero( const RigFemResultAddress& resVarAddr,
                                                       double*                    globalPosClosestToZero,
                                                       double*                    globalNegClosestToZero )
{
    this->statistics( resVarAddr )->posNegClosestToZero( *globalPosClosestToZero, *globalNegClosestToZero );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::meanScalarValue( const RigFemResultAddress& resVarAddr, double* meanValue )
{
    CVF_ASSERT( meanValue );

    this->statistics( resVarAddr )->meanCellScalarValues( *meanValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::meanScalarValue( const RigFemResultAddress& resVarAddr, int frameIndex, double* meanValue )
{
    this->statistics( resVarAddr )->meanCellScalarValues( frameIndex, *meanValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::p10p90ScalarValues( const RigFemResultAddress& resVarAddr, double* p10, double* p90 )
{
    this->statistics( resVarAddr )->p10p90CellScalarValues( *p10, *p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::p10p90ScalarValues( const RigFemResultAddress& resVarAddr,
                                                      int                        frameIndex,
                                                      double*                    p10,
                                                      double*                    p90 )
{
    this->statistics( resVarAddr )->p10p90CellScalarValues( frameIndex, *p10, *p90 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::sumScalarValue( const RigFemResultAddress& resVarAddr, double* sum )
{
    CVF_ASSERT( sum );

    this->statistics( resVarAddr )->sumCellScalarValues( *sum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::sumScalarValue( const RigFemResultAddress& resVarAddr, int frameIndex, double* sum )
{
    CVF_ASSERT( sum );

    this->statistics( resVarAddr )->sumCellScalarValues( frameIndex, *sum );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFemPartResultsCollection::scalarValuesHistogram( const RigFemResultAddress& resVarAddr )
{
    return this->statistics( resVarAddr )->cellScalarValuesHistogram();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<size_t>& RigFemPartResultsCollection::scalarValuesHistogram( const RigFemResultAddress& resVarAddr,
                                                                               int                        frameIndex )
{
    return this->statistics( resVarAddr )->cellScalarValuesHistogram( frameIndex );
}

std::vector<RigFemResultAddress>
    RigFemPartResultsCollection::tensorPrincipalComponentAdresses( const RigFemResultAddress& resVarAddr )
{
    std::vector<RigFemResultAddress> addresses;

    for ( size_t i = 0; i < 3; ++i )
    {
        addresses.push_back( RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "" ) );
    }

    if ( resVarAddr.fieldName == "SE" || resVarAddr.fieldName == "ST" )
    {
        addresses[0].componentName = "S1";
        addresses[1].componentName = "S2";
        addresses[2].componentName = "S3";
    }
    else if ( resVarAddr.fieldName == "NE" )
    {
        addresses[0].componentName = "E1";
        addresses[1].componentName = "E2";
        addresses[2].componentName = "E3";
    }
    else
    {
        addresses.clear();
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigFemResultAddress> RigFemPartResultsCollection::normalizedResults()
{
    std::set<std::string> validFields     = {"SE", "ST"};
    std::set<std::string> validComponents = {"S11", "S22", "S33", "S12", "S13", "S23", "S1", "S2", "S3"};

    std::set<RigFemResultAddress> results;
    for ( auto field : validFields )
    {
        for ( auto component : validComponents )
        {
            results.insert(
                RigFemResultAddress( RIG_ELEMENT_NODAL, field, component, RigFemResultAddress::allTimeLapsesValue(), -1, true ) );
        }
    }
    results.insert(
        RigFemResultAddress( RIG_ELEMENT_NODAL, "SE", "SEM", RigFemResultAddress::allTimeLapsesValue(), -1, true ) );
    results.insert(
        RigFemResultAddress( RIG_ELEMENT_NODAL, "ST", "STM", RigFemResultAddress::allTimeLapsesValue(), -1, true ) );
    results.insert(
        RigFemResultAddress( RIG_ELEMENT_NODAL, "ST", "Q", RigFemResultAddress::allTimeLapsesValue(), -1, true ) );

    results.insert( RigFemResultAddress( RIG_NODAL, "POR-Bar", "", RigFemResultAddress::allTimeLapsesValue(), -1, true ) );
    results.insert(
        RigFemResultAddress( RIG_ELEMENT_NODAL, "POR-Bar", "", RigFemResultAddress::allTimeLapsesValue(), -1, true ) );

    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultsCollection::isNormalizableResult( const RigFemResultAddress& result )
{
    for ( auto normRes : normalizedResults() )
    {
        if ( normRes.resultPosType == result.resultPosType && normRes.fieldName == result.fieldName &&
             normRes.componentName == result.componentName )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setNormalizationAirGap( double normalizationAirGap )
{
    if ( std::abs( m_normalizationAirGap - normalizationAirGap ) > 1.0e-8 )
    {
        for ( auto result : normalizedResults() )
        {
            this->deleteResult( result );
        }
    }
    m_normalizationAirGap = normalizationAirGap;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::minMaxScalarValuesOverAllTensorComponents( const RigFemResultAddress& resVarAddr,
                                                                             int                        frameIndex,
                                                                             double*                    localMin,
                                                                             double*                    localMax )
{
    double currentMin = HUGE_VAL;
    double currentMax = -HUGE_VAL;

    double min, max;

    for ( const auto& address : tensorPrincipalComponentAdresses( resVarAddr ) )
    {
        this->statistics( address )->minMaxCellScalarValues( frameIndex, min, max );
        if ( min < currentMin )
        {
            currentMin = min;
        }
        if ( max > currentMax )
        {
            currentMax = max;
        }
    }

    *localMin = currentMin;
    *localMax = currentMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::minMaxScalarValuesOverAllTensorComponents( const RigFemResultAddress& resVarAddr,
                                                                             double*                    globalMin,
                                                                             double*                    globalMax )
{
    double currentMin = HUGE_VAL;
    double currentMax = -HUGE_VAL;

    double min, max;

    for ( const auto& address : tensorPrincipalComponentAdresses( resVarAddr ) )
    {
        this->statistics( address )->minMaxCellScalarValues( min, max );
        if ( min < currentMin )
        {
            currentMin = min;
        }
        if ( max > currentMax )
        {
            currentMax = max;
        }
    }

    *globalMin = currentMin;
    *globalMax = currentMax;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZeroOverAllTensorComponents( const RigFemResultAddress& resVarAddr,
                                                                              int                        frameIndex,
                                                                              double* localPosClosestToZero,
                                                                              double* localNegClosestToZero )
{
    double currentPosClosestToZero = HUGE_VAL;
    double currentNegClosestToZero = -HUGE_VAL;

    double pos, neg;

    for ( const auto& address : tensorPrincipalComponentAdresses( resVarAddr ) )
    {
        this->statistics( address )->posNegClosestToZero( frameIndex, pos, neg );
        if ( pos < currentPosClosestToZero )
        {
            currentPosClosestToZero = pos;
        }
        if ( neg > currentNegClosestToZero )
        {
            currentNegClosestToZero = neg;
        }
    }

    *localPosClosestToZero = currentPosClosestToZero;
    *localNegClosestToZero = currentNegClosestToZero;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::posNegClosestToZeroOverAllTensorComponents( const RigFemResultAddress& resVarAddr,
                                                                              double* globalPosClosestToZero,
                                                                              double* globalNegClosestToZero )
{
    double currentPosClosestToZero = HUGE_VAL;
    double currentNegClosestToZero = -HUGE_VAL;

    double pos, neg;

    for ( const auto& address : tensorPrincipalComponentAdresses( resVarAddr ) )
    {
        this->statistics( address )->posNegClosestToZero( pos, neg );
        if ( pos < currentPosClosestToZero )
        {
            currentPosClosestToZero = pos;
        }
        if ( neg > currentNegClosestToZero )
        {
            currentNegClosestToZero = neg;
        }
    }

    *globalPosClosestToZero = currentPosClosestToZero;
    *globalNegClosestToZero = currentNegClosestToZero;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigFemResultAddress> RigFemPartResultsCollection::tensorComponentAddresses( const RigFemResultAddress& resVarAddr )
{
    std::vector<RigFemResultAddress> addresses( 6, RigFemResultAddress( resVarAddr.resultPosType, resVarAddr.fieldName, "" ) );

    if ( resVarAddr.fieldName == "SE" || resVarAddr.fieldName == "ST" )
    {
        addresses[caf::Ten3f::SXX].componentName = "S11";
        addresses[caf::Ten3f::SYY].componentName = "S22";
        addresses[caf::Ten3f::SZZ].componentName = "S33";
        addresses[caf::Ten3f::SXY].componentName = "S12";
        addresses[caf::Ten3f::SZX].componentName = "S13";
        addresses[caf::Ten3f::SYZ].componentName = "S23";
    }
    else if ( resVarAddr.fieldName == "NE" )
    {
        addresses[caf::Ten3f::SXX].componentName = "E11";
        addresses[caf::Ten3f::SYY].componentName = "E22";
        addresses[caf::Ten3f::SZZ].componentName = "E33";
        addresses[caf::Ten3f::SXY].componentName = "E12";
        addresses[caf::Ten3f::SZX].componentName = "E13";
        addresses[caf::Ten3f::SYZ].componentName = "E23";
    }
    else
    {
        return std::vector<RigFemResultAddress>();
    }
    return addresses;
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
RigFemClosestResultIndexCalculator::RigFemClosestResultIndexCalculator( RigFemPart*         femPart,
                                                                        RigFemResultPosEnum resultPosition,
                                                                        int                 elementIndex,
                                                                        int                 m_face,
                                                                        const cvf::Vec3d&   intersectionPointInDomain )
{
    m_resultIndexToClosestResult = -1;
    m_closestNodeId              = -1;
    m_closestElementNodeResIdx   = -1;

    if ( resultPosition != RIG_ELEMENT_NODAL_FACE || m_face == -1 )
    {
        RigElementType elmType      = femPart->elementType( elementIndex );
        const int*     elmentConn   = femPart->connectivities( elementIndex );
        int            elmNodeCount = RigFemTypes::elmentNodeCount( elmType );

        // Find the closest node
        int   closestLocalNode = -1;
        float minDist          = std::numeric_limits<float>::infinity();
        for ( int lNodeIdx = 0; lNodeIdx < elmNodeCount; ++lNodeIdx )
        {
            int        nodeIdx         = elmentConn[lNodeIdx];
            cvf::Vec3f nodePosInDomain = femPart->nodes().coordinates[nodeIdx];
            float      dist            = ( nodePosInDomain - cvf::Vec3f( intersectionPointInDomain ) ).lengthSquared();
            if ( dist < minDist )
            {
                closestLocalNode = lNodeIdx;
                minDist          = dist;
            }
        }

        if ( closestLocalNode >= 0 )
        {
            int nodeIdx = elmentConn[closestLocalNode];
            m_closestElementNodeResIdx =
                static_cast<int>( femPart->elementNodeResultIdx( elementIndex, closestLocalNode ) );

            if ( resultPosition == RIG_NODAL )
            {
                m_resultIndexToClosestResult = nodeIdx;
            }
            else if ( resultPosition == RIG_ELEMENT_NODAL_FACE )
            {
                m_resultIndexToClosestResult = -1;
            }
            else if ( resultPosition == RIG_ELEMENT )
            {
                m_resultIndexToClosestResult = elementIndex;
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
        int closestNodeIdx   = -1;
        {
            int closestLocFaceNode  = -1;
            int closestLocalElmNode = -1;
            {
                RigElementType elmType        = femPart->elementType( elementIndex );
                const int*     elmNodeIndices = femPart->connectivities( elementIndex );
                int            faceNodeCount  = 0;
                const int*     localElmNodeIndicesForFace =
                    RigFemTypes::localElmNodeIndicesForFace( elmType, m_face, &faceNodeCount );

                float minDist = std::numeric_limits<float>::infinity();
                for ( int faceNodIdx = 0; faceNodIdx < faceNodeCount; ++faceNodIdx )
                {
                    int        nodeIdx         = elmNodeIndices[localElmNodeIndicesForFace[faceNodIdx]];
                    cvf::Vec3f nodePosInDomain = femPart->nodes().coordinates[nodeIdx];
                    float      dist = ( nodePosInDomain - cvf::Vec3f( intersectionPointInDomain ) ).lengthSquared();
                    if ( dist < minDist )
                    {
                        closestLocFaceNode  = faceNodIdx;
                        closestNodeIdx      = nodeIdx;
                        closestLocalElmNode = localElmNodeIndicesForFace[faceNodIdx];
                        minDist             = dist;
                    }
                }
            }

            int elmNodFaceResIdxElmStart  = elementIndex * 24; // HACK should get from part
            int elmNodFaceResIdxFaceStart = elmNodFaceResIdxElmStart + 4 * m_face;

            if ( closestLocFaceNode >= 0 )
            {
                elmNodFaceResIdx = elmNodFaceResIdxFaceStart + closestLocFaceNode;
                m_closestElementNodeResIdx =
                    static_cast<int>( femPart->elementNodeResultIdx( elementIndex, closestLocalElmNode ) );
            }
        }

        m_resultIndexToClosestResult = elmNodFaceResIdx;
        m_closestNodeId              = femPart->nodes().nodeIds[closestNodeIdx];
    }
}

//--------------------------------------------------------------------------------------------------
/// Internal functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3d> coordsFromNodeIndices( const RigFemPart& part, const std::vector<size_t>& nodeIdxs )
{
    std::vector<cvf::Vec3d> out;
    for ( const auto& nodeIdx : nodeIdxs )
        out.push_back( cvf::Vec3d( part.nodes().coordinates[nodeIdx] ) );
    return out;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> nodesForElement( const RigFemPart& part, size_t elementIdx )
{
    std::vector<size_t> nodeIdxs;
    const int*          nodeConn = part.connectivities( elementIdx );
    for ( int n = 0; n < 8; n++ )
        nodeIdxs.push_back( nodeConn[n] );
    return nodeIdxs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float horizontalDistance( const cvf::Vec3f& p1, const cvf::Vec3f& p2 )
{
    cvf::Vec3f p1_ = p1;
    cvf::Vec3f p2_ = p2;
    p1_.z() = p2_.z() = 0;
    return p1_.pointDistance( p2_ );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void findReferenceElementForNode( const RigFemPart& part, size_t nodeIdx, size_t kRefLayer, RefElement* refElement )
{
    static const double zMin = -1e6, zMax = 1e6;

    cvf::BoundingBox bb;
    cvf::Vec3f       currentNodeCoord = part.nodes().coordinates[nodeIdx];
    cvf::Vec3f       p1               = cvf::Vec3f( currentNodeCoord.x(), currentNodeCoord.y(), zMin );
    cvf::Vec3f       p2               = cvf::Vec3f( currentNodeCoord.x(), currentNodeCoord.y(), zMax );
    bb.add( p1 );
    bb.add( p2 );

    std::vector<size_t> refElementCandidates;
    part.findIntersectingCells( bb, &refElementCandidates );

    const RigFemPartGrid* grid = part.getOrCreateStructGrid();

    refElement->elementIdx                             = cvf::UNDEFINED_SIZE_T;
    refElement->intersectionPointToCurrentNodeDistance = std::numeric_limits<float>::infinity();
    size_t i, j, k;
    for ( const size_t elemIdx : refElementCandidates )
    {
        bool validIndex = grid->ijkFromCellIndex( elemIdx, &i, &j, &k );
        if ( validIndex && k == kRefLayer )
        {
            const std::vector<size_t> nodeIndices = nodesForElement( part, elemIdx );
            CVF_ASSERT( nodeIndices.size() == 8 );

            std::vector<HexIntersectionInfo> intersections;
            RigHexIntersectionTools::lineHexCellIntersection( cvf::Vec3d( p1 ),
                                                              cvf::Vec3d( p2 ),
                                                              coordsFromNodeIndices( part, nodeIndices ).data(),
                                                              elemIdx,
                                                              &intersections );

            for ( const auto& intersection : intersections )
            {
                cvf::Vec3f intersectionPoint = cvf::Vec3f( intersection.m_intersectionPoint );

                float nodeToIntersectionDistance = currentNodeCoord.pointDistance( intersectionPoint );
                if ( nodeToIntersectionDistance < refElement->intersectionPointToCurrentNodeDistance )
                {
                    cvf::ubyte faceNodes[4];
                    grid->cellFaceVertexIndices( intersection.m_face, faceNodes );
                    std::vector<size_t> topFaceCoords( {nodeIndices[faceNodes[0]],
                                                        nodeIndices[faceNodes[1]],
                                                        nodeIndices[faceNodes[2]],
                                                        nodeIndices[faceNodes[3]]} );

                    refElement->elementIdx                             = elemIdx;
                    refElement->intersectionPointToCurrentNodeDistance = nodeToIntersectionDistance;
                    refElement->intersectionPoint                      = intersectionPoint;
                    refElement->elementFaceNodeIdxs                    = topFaceCoords;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigFemPartResultsCollection::getStressComponentNames()
{
    return {"S11", "S22", "S33", "S12", "S13", "S23", "S1", "S2", "S3"};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigFemPartResultsCollection::getStressGradientComponentNames()
{
    std::vector<std::string> directions           = {"X", "Y", "Z"};
    std::vector<std::string> stressComponentNames = getStressComponentNames();

    std::vector<std::string> stressGradientComponentNames;
    for ( auto& s : stressComponentNames )
    {
        for ( auto& d : directions )
        {
            stressGradientComponentNames.push_back( s + "-" + d );
        }
    }

    return stressGradientComponentNames;
}
