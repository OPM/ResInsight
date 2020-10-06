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

#include "RiaLogging.h"

#include "RifElementPropertyReader.h"
#include "RifGeoMechReaderInterface.h"

#include "RigFemNativeStatCalc.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultCalculatorBarConverted.h"
#include "RigFemPartResultCalculatorCompaction.h"
#include "RigFemPartResultCalculatorDSM.h"
#include "RigFemPartResultCalculatorED.h"
#include "RigFemPartResultCalculatorEV.h"
#include "RigFemPartResultCalculatorEnIpPorBar.h"
#include "RigFemPartResultCalculatorFOS.h"
#include "RigFemPartResultCalculatorFormationIndices.h"
#include "RigFemPartResultCalculatorGamma.h"
#include "RigFemPartResultCalculatorInitialPorosity.h"
#include "RigFemPartResultCalculatorMudWeightWindow.h"
#include "RigFemPartResultCalculatorNE.h"
#include "RigFemPartResultCalculatorNodalGradients.h"
#include "RigFemPartResultCalculatorNormalSE.h"
#include "RigFemPartResultCalculatorNormalST.h"
#include "RigFemPartResultCalculatorNormalized.h"
#include "RigFemPartResultCalculatorPoreCompressibility.h"
#include "RigFemPartResultCalculatorPorosityPermeability.h"
#include "RigFemPartResultCalculatorPrincipalStrain.h"
#include "RigFemPartResultCalculatorPrincipalStress.h"
#include "RigFemPartResultCalculatorQ.h"
#include "RigFemPartResultCalculatorSFI.h"
#include "RigFemPartResultCalculatorSM.h"
#include "RigFemPartResultCalculatorShearSE.h"
#include "RigFemPartResultCalculatorShearST.h"
#include "RigFemPartResultCalculatorShearSlipIndicator.h"
#include "RigFemPartResultCalculatorStressAnisotropy.h"
#include "RigFemPartResultCalculatorStressGradients.h"
#include "RigFemPartResultCalculatorSurfaceAlignedStress.h"
#include "RigFemPartResultCalculatorSurfaceAngles.h"
#include "RigFemPartResultCalculatorTimeLapse.h"
#include "RigFemPartResults.h"
#include "RigFemScalarResultFrames.h"
#include "RigFormationNames.h"
#include "RigStatisticsDataCache.h"
#include "RigWbsParameter.h"

#include "RimMainPlotCollection.h"
#include "RimMudWeightWindowParameters.h"
#include "RimProject.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"

#include "Riu3DMainWindowTools.h"

#ifdef USE_ODB_API
#include "RifOdbReader.h"
#endif

#include "cafProgressInfo.h"
#include "cafTensor3.h"

#include "cvfMath.h"

#include <QString>

#include <cmath>
#include <cstdlib>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string RigFemPartResultsCollection::FIELD_NAME_COMPACTION = "COMPACTION";

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

    m_biotFixedFactor   = 1.0;
    m_biotResultAddress = "";

    m_referenceTimeStep = 0;

    m_initialPermeabilityFixed         = 1.0;
    m_initialPermeabilityResultAddress = "";
    m_permeabilityExponent             = 1.0;

    m_airGapMudWeightWindow                          = 0.0;
    m_referenceLayerMudWeightWindow                  = 0;
    m_shMultiplierMudWeightWindow                    = 1.05;
    m_nonReservoirPorePressureAddressMudWeightWindow = "";
    m_hydrostaticMultiplierPPNonResMudWeightWindow   = 1.0;

    m_waterDensityShearSlipIndicator = 1.03;

    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorTimeLapse( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorSurfaceAngles( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorSurfaceAlignedStress( *this ) ) );
    m_resultCalculators.push_back( std::unique_ptr<RigFemPartResultCalculator>(
        new RigFemPartResultCalculatorBarConverted( *this, "S-Bar", "S" ) ) );
    m_resultCalculators.push_back( std::unique_ptr<RigFemPartResultCalculator>(
        new RigFemPartResultCalculatorBarConverted( *this, "POR-Bar", "POR" ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorEnIpPorBar( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorNodalGradients( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorCompaction( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorSFI( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorDSM( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorFOS( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorED( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorEV( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorSM( *this ) ) );
    m_resultCalculators.push_back( std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorQ( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorStressGradients( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorNormalized( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorNormalST( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorShearST( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorNormalSE( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorShearSE( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorNE( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorGamma( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorPrincipalStrain( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorPrincipalStress( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorStressAnisotropy( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorPoreCompressibility( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorPorosityPermeability( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorInitialPorosity( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorMudWeightWindow( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorShearSlipIndicator( *this ) ) );
    m_resultCalculators.push_back(
        std::unique_ptr<RigFemPartResultCalculator>( new RigFemPartResultCalculatorFormationIndices( *this ) ) );
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

    RimProject* project = RimProject::current();
    if ( project )
    {
        if ( project->mainPlotCollection() )
        {
            RimWellLogPlotCollection* plotCollection = project->mainPlotCollection()->wellLogPlotCollection();
            if ( plotCollection )
            {
                for ( auto wellLogPlot : plotCollection->wellLogPlots() )
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
    std::vector<RigFemResultAddress> newAddresses;
    for ( const QString& filename : filenames )
    {
        m_elementPropertyReader->addFile( filename.toStdString() );

        // Collect all addresses which was added in this file
        std::vector<std::string> fields = m_elementPropertyReader->fieldsInFile( filename.toStdString() );
        for ( const std::string& field : fields )
        {
            newAddresses.push_back( RigFemResultAddress( RIG_ELEMENT, field, "" ) );
        }
    }

    // Invalidate previous result if already in cache
    for ( const RigFemResultAddress& address : newAddresses )
    {
        this->deleteResult( address );
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
std::map<std::string, QString>
    RigFemPartResultsCollection::addressesInElementPropertyFiles( const std::vector<QString>& filenames )
{
    std::map<std::string, QString> fieldsInFile;
    for ( const QString& filename : filenames )
    {
        std::vector<std::string> fields = m_elementPropertyReader->fieldsInFile( filename.toStdString() );
        for ( const std::string& field : fields )
        {
            fieldsInFile[field] = filename;
        }
    }

    return fieldsInFile;
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
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setBiotCoefficientParameters( double biotFixedFactor, const QString& biotResultAddress )
{
    m_biotFixedFactor   = biotFixedFactor;
    m_biotResultAddress = biotResultAddress;

    // Invalidate all results which depends on biot coefficient (directly or indirectly)
    for ( auto elementType : {RIG_ELEMENT_NODAL, RIG_INTEGRATION_POINT} )
    {
        deleteResult(
            RigFemResultAddress( elementType, "COMPRESSIBILITY", "PORE", RigFemResultAddress::allTimeLapsesValue() ) );
        deleteResult(
            RigFemResultAddress( elementType, "COMPRESSIBILITY", "VERTICAL", RigFemResultAddress::allTimeLapsesValue() ) );
        deleteResult( RigFemResultAddress( elementType,
                                           "COMPRESSIBILITY",
                                           "VERTICAL-RATIO",
                                           RigFemResultAddress::allTimeLapsesValue() ) );
    }

    // Depends on COMRESSIBILITY.PORE which depends on biot coefficient
    std::set<RigFemResultAddress> initPermResults = initialPermeabilityDependentResults();
    for ( auto result : initPermResults )
    {
        deleteResult( result );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setReferenceTimeStep( int referenceTimeStep )
{
    m_referenceTimeStep = referenceTimeStep;

    std::set<RigFemResultAddress> results = referenceCaseDependentResults();
    for ( auto result : results )
    {
        deleteResult( result );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RigFemPartResultsCollection::referenceTimeStep() const
{
    return m_referenceTimeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setPermeabilityParameters( double         fixedInitalPermeability,
                                                             const QString& initialPermeabilityAddress,
                                                             double         permeabilityExponent )
{
    m_initialPermeabilityFixed         = fixedInitalPermeability;
    m_initialPermeabilityResultAddress = initialPermeabilityAddress;
    m_permeabilityExponent             = permeabilityExponent;

    std::set<RigFemResultAddress> results = initialPermeabilityDependentResults();
    for ( auto result : results )
    {
        deleteResult( result );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFemPartResultsCollection::initialPermeabilityFixed() const
{
    return m_initialPermeabilityFixed;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigFemPartResultsCollection::initialPermeabilityAddress() const
{
    return m_initialPermeabilityResultAddress;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFemPartResultsCollection::permeabilityExponent() const
{
    return m_permeabilityExponent;
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

        for ( auto [addrString, values] : elementProperties )
        {
            RigFemResultAddress       addressForElement( RIG_ELEMENT, addrString, "" );
            RigFemScalarResultFrames* currentFrames = m_femPartResults[partIndex]->createScalarResult( addressForElement );
            currentFrames->enableAsSingleFrameResult();
            currentFrames->frameData( 0 ).swap( values );
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
        frames = m_femPartResults[partIndex]->createScalarResult( resVarAddr ); // Create a dummy empty result, if
                                                                                // the request did not specify the
                                                                                // component.
    }

    return frames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemScalarResultFrames* RigFemPartResultsCollection::createScalarResult( int                        partIndex,
                                                                           const RigFemResultAddress& resVarAddr )
{
    CVF_ASSERT( partIndex < static_cast<int>( m_femPartResults.size() ) );
    CVF_ASSERT( resVarAddr.isValid() );
    return m_femPartResults[partIndex]->createScalarResult( resVarAddr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::deleteAllScalarResults()
{
    for ( cvf::ref<RigFemPartResults> results : m_femPartResults )
    {
        results->deleteAllScalarResults();
    }
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

    const std::vector<std::string> stressComponentNames           = getStressComponentNames();
    const std::vector<std::string> stressGradientComponentNames   = getStressGradientComponentNames();
    const std::vector<std::string> stressAnisotropyComponentNames = getStressAnisotropyComponentNames();

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

            fieldCompNames["SE"].push_back( "SM" );
            fieldCompNames["SE"].push_back( "SFI" );
            fieldCompNames["SE"].push_back( "DSM" );
            fieldCompNames["SE"].push_back( "FOS" );

            for ( auto& s : stressComponentNames )
            {
                fieldCompNames["SE"].push_back( s );
            }

            for ( auto& s : stressAnisotropyComponentNames )
            {
                fieldCompNames["SE"].push_back( s );
            }

            fieldCompNames["SE"].push_back( "S1inc" );
            fieldCompNames["SE"].push_back( "S1azi" );
            fieldCompNames["SE"].push_back( "S2inc" );
            fieldCompNames["SE"].push_back( "S2azi" );
            fieldCompNames["SE"].push_back( "S3inc" );
            fieldCompNames["SE"].push_back( "S3azi" );

            fieldCompNames["ST"].push_back( "SM" );
            fieldCompNames["ST"].push_back( "Q" );
            fieldCompNames["ST"].push_back( "DPN" );

            for ( auto& s : stressComponentNames )
            {
                fieldCompNames["ST"].push_back( s );
            }

            for ( auto& s : stressAnisotropyComponentNames )
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

            fieldCompNames["COMPRESSIBILITY"].push_back( "PORE" );
            fieldCompNames["COMPRESSIBILITY"].push_back( "VERTICAL" );
            fieldCompNames["COMPRESSIBILITY"].push_back( "VERTICAL-RATIO" );

            fieldCompNames["PORO-PERM"].push_back( "PHI0" );
            fieldCompNames["PORO-PERM"].push_back( "PHI" );
            fieldCompNames["PORO-PERM"].push_back( "DPHI" );
            fieldCompNames["PORO-PERM"].push_back( "PERM" );

            fieldCompNames["MUD-WEIGHT"].push_back( "MWW" );
            fieldCompNames["MUD-WEIGHT"].push_back( "MWM" );
            fieldCompNames["MUD-WEIGHT"].push_back( "UMWL" );
            fieldCompNames["MUD-WEIGHT"].push_back( "LMWL" );
        }
        else if ( resPos == RIG_INTEGRATION_POINT )
        {
            fieldCompNames = m_readerInterface->scalarIntegrationPointFieldAndComponentNames();

            fieldCompNames["SE"].push_back( "SM" );
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

            for ( auto& s : stressAnisotropyComponentNames )
            {
                fieldCompNames["SE"].push_back( s );
            }

            fieldCompNames["SE"].push_back( "S1inc" );
            fieldCompNames["SE"].push_back( "S1azi" );
            fieldCompNames["SE"].push_back( "S2inc" );
            fieldCompNames["SE"].push_back( "S2azi" );
            fieldCompNames["SE"].push_back( "S3inc" );
            fieldCompNames["SE"].push_back( "S3azi" );

            fieldCompNames["ST"].push_back( "SM" );
            fieldCompNames["ST"].push_back( "Q" );
            fieldCompNames["ST"].push_back( "DPN" );

            fieldCompNames["ST"].push_back( "S11" );
            fieldCompNames["ST"].push_back( "S22" );
            fieldCompNames["ST"].push_back( "S33" );
            fieldCompNames["ST"].push_back( "S12" );
            fieldCompNames["ST"].push_back( "S13" );
            fieldCompNames["ST"].push_back( "S23" );
            fieldCompNames["ST"].push_back( "S1" );
            fieldCompNames["ST"].push_back( "S2" );
            fieldCompNames["ST"].push_back( "S3" );

            for ( auto& s : stressAnisotropyComponentNames )
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

            fieldCompNames["COMPRESSIBILITY"].push_back( "PORE" );
            fieldCompNames["COMPRESSIBILITY"].push_back( "VERTICAL" );
            fieldCompNames["COMPRESSIBILITY"].push_back( "VERTICAL-RATIO" );

            fieldCompNames["PORO-PERM"].push_back( "PHI0" );
            fieldCompNames["PORO-PERM"].push_back( "PHI" );
            fieldCompNames["PORO-PERM"].push_back( "DPHI" );
            fieldCompNames["PORO-PERM"].push_back( "PERM" );

            fieldCompNames["MUD-WEIGHT"].push_back( "MWW" );
            fieldCompNames["MUD-WEIGHT"].push_back( "MWM" );
            fieldCompNames["MUD-WEIGHT"].push_back( "UMWL" );
            fieldCompNames["MUD-WEIGHT"].push_back( "LMWL" );
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
RigFemScalarResultFrames* RigFemPartResultsCollection::calculateDerivedResult( int                        partIndex,
                                                                               const RigFemResultAddress& resVarAddr )
{
    for ( const auto& calculator : m_resultCalculators )
    {
        if ( calculator->isMatching( resVarAddr ) ) return calculator->calculate( partIndex, resVarAddr );
    }

    if ( resVarAddr.fieldName == "ST" && resVarAddr.componentName.empty() )
    {
        // Create and return an empty result
        return m_femPartResults[partIndex]->createScalarResult( resVarAddr );
    }

    if ( resVarAddr.fieldName == "Gamma" && resVarAddr.componentName.empty() )
    {
        // Create and return an empty result
        return m_femPartResults[partIndex]->createScalarResult( resVarAddr );
    }

    return nullptr;
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
        if ( !resVarAddr.componentName.empty() ) // If we did not request a particular component, do not add the
                                                 // components
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
bool RigFemPartResultsCollection::isResultInSet( const RigFemResultAddress&           result,
                                                 const std::set<RigFemResultAddress>& results )
{
    for ( auto res : results )
    {
        if ( res.resultPosType == result.resultPosType && res.fieldName == result.fieldName &&
             res.componentName == result.componentName )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigFemResultAddress> RigFemPartResultsCollection::normalizedResults()
{
    std::set<std::string> validFields     = {"SE", "ST"};
    std::set<std::string> validComponents = {"S11", "S22", "S33", "S12", "S13", "S23", "S1", "S2", "S3", "SM"};

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
    return isResultInSet( result, normalizedResults() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigFemResultAddress> RigFemPartResultsCollection::referenceCaseDependentResults()
{
    std::set<RigFemResultAddress> results;
    for ( auto elementType : {RIG_ELEMENT_NODAL, RIG_INTEGRATION_POINT} )
    {
        results.insert(
            RigFemResultAddress( elementType, "COMPRESSIBILITY", "PORE", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert(
            RigFemResultAddress( elementType, "COMPRESSIBILITY", "VERTICAL", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elementType,
                                             "COMPRESSIBILITY",
                                             "VERTICAL-RATIO",
                                             RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elementType, "PORO-PERM", "PHI", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elementType, "PORO-PERM", "DPHI", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elementType, "PORO-PERM", "PERM", RigFemResultAddress::allTimeLapsesValue() ) );
    }

    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigFemResultAddress> RigFemPartResultsCollection::mudWeightWindowResults()
{
    std::set<RigFemResultAddress> results;
    for ( auto elmType : {RIG_ELEMENT_NODAL, RIG_INTEGRATION_POINT} )
    {
        results.insert( RigFemResultAddress( elmType, "MUD-WEIGHT", "MWW", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elmType, "MUD-WEIGHT", "MWM", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elmType, "MUD-WEIGHT", "UMWL", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elmType, "MUD-WEIGHT", "LMWL", RigFemResultAddress::allTimeLapsesValue() ) );
    }

    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigFemResultAddress> RigFemPartResultsCollection::initialPermeabilityDependentResults()
{
    std::set<RigFemResultAddress> results;
    for ( auto elementType : {RIG_ELEMENT_NODAL, RIG_INTEGRATION_POINT} )
    {
        results.insert( RigFemResultAddress( elementType, "PORO-PERM", "PHI", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elementType, "PORO-PERM", "DPHI", RigFemResultAddress::allTimeLapsesValue() ) );
        results.insert( RigFemResultAddress( elementType, "PORO-PERM", "PERM", RigFemResultAddress::allTimeLapsesValue() ) );
    }
    return results;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultsCollection::isReferenceCaseDependentResult( const RigFemResultAddress& result )
{
    return isResultInSet( result, referenceCaseDependentResults() );
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
double RigFemPartResultsCollection::normalizationAirGap() const
{
    return m_normalizationAirGap;
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
const RigFemPartCollection* RigFemPartResultsCollection::parts() const
{
    return m_femParts.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigFemPartResultsCollection::getStressComponentNames( bool includeShear )
{
    std::vector<std::string> componentNames = {"S11", "S22", "S33", "S1", "S2", "S3"};
    if ( includeShear )
    {
        componentNames.push_back( "S12" );
        componentNames.push_back( "S13" );
        componentNames.push_back( "S23" );
    }

    return componentNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigFemPartResultsCollection::getStressAnisotropyComponentNames()
{
    return {"SA12", "SA13", "SA23"};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::string> RigFemPartResultsCollection::getStressGradientComponentNames( bool includeShear )
{
    std::vector<std::string> directions           = {"X", "Y", "Z"};
    std::vector<std::string> stressComponentNames = getStressComponentNames( includeShear );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigFemPartResultsCollection::isValidBiotData( const std::vector<float>& biotData, size_t elementCount ) const
{
    if ( biotData.size() != elementCount )
    {
        QString txt = QString( "Unexpected size of biot coefficient element properties: %1 (expected: %2)" )
                          .arg( biotData.size() )
                          .arg( elementCount );

        Riu3DMainWindowTools::reportAndShowWarning( "Wrong size of biot data", txt );

        return false;
    }

    for ( float b : biotData )
    {
        if ( !std::isinf( b ) && ( b < 0.0 || b > 1.0 ) )
        {
            RiaLogging::error(
                QString( "Found unexpected biot coefficient. The value must be in the [0, 1] interval." ) );
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setCalculationParameters( RimMudWeightWindowParameters::ParameterType parameterType,
                                                            const QString&                              address,
                                                            double                                      value )
{
    parameterAddresses[parameterType] = address;
    parameterValues[parameterType]    = value;

    // Invalidate dependent results
    for ( auto result : mudWeightWindowResults() )
    {
        this->deleteResult( result );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFemPartResultsCollection::getCalculationParameterValue( RimMudWeightWindowParameters::ParameterType parameterType ) const
{
    auto it = parameterValues.find( parameterType );
    if ( it != parameterValues.end() )
        return it->second;
    else
    {
        // TODO: log error maybe?
        return 1.0;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigFemPartResultsCollection::getCalculationParameterAddress( RimMudWeightWindowParameters::ParameterType parameterType ) const
{
    auto it = parameterAddresses.find( parameterType );
    if ( it != parameterAddresses.end() )
        return it->second;
    else
    {
        // TODO: log error maybe?
        return QString();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFemPartResultsCollection::airGapMudWeightWindow() const
{
    return m_airGapMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFemPartResultsCollection::shMultiplierMudWeightWindow() const
{
    return m_shMultiplierMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFemPartResultsCollection::hydrostaticMultiplierPPNonRes() const
{
    return m_hydrostaticMultiplierPPNonResMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::NonReservoirPorePressureType
    RigFemPartResultsCollection::nonReservoirPorePressureTypeMudWeightWindow() const
{
    return m_nonReservoirPorePressureTypeMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigFemPartResultsCollection::nonReservoirPorePressureAddressMudWeightWindow() const
{
    return m_nonReservoirPorePressureAddressMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::LowerLimitType RigFemPartResultsCollection::lowerLimitParameterMudWeightWindow() const
{
    return m_lowerLimitParameterMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::UpperLimitType RigFemPartResultsCollection::upperLimitParameterMudWeightWindow() const
{
    return m_upperLimitParameterMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigFemPartResultsCollection::referenceLayerMudWeightWindow() const
{
    return m_referenceLayerMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setMudWeightWindowParameters(
    double                                                        airGap,
    RimMudWeightWindowParameters::UpperLimitType                  upperLimit,
    RimMudWeightWindowParameters::LowerLimitType                  lowerLimit,
    int                                                           referenceLayer,
    RimMudWeightWindowParameters::FractureGradientCalculationType fgCalculationType,
    double                                                        shMultiplier,
    RimMudWeightWindowParameters::NonReservoirPorePressureType    nonReservoirPorePressureType,
    double                                                        hydrostaticMultiplierPPNonRes,
    const QString&                                                nonReservoirPorePressureAddress )
{
    m_airGapMudWeightWindow                          = airGap;
    m_upperLimitParameterMudWeightWindow             = upperLimit;
    m_lowerLimitParameterMudWeightWindow             = lowerLimit;
    m_referenceLayerMudWeightWindow                  = referenceLayer;
    m_fractureGradientCalculationTypeMudWeightWindow = fgCalculationType;
    m_shMultiplierMudWeightWindow                    = shMultiplier;
    m_nonReservoirPorePressureTypeMudWeightWindow    = nonReservoirPorePressureType;
    m_hydrostaticMultiplierPPNonResMudWeightWindow   = hydrostaticMultiplierPPNonRes;
    m_nonReservoirPorePressureAddressMudWeightWindow = nonReservoirPorePressureAddress;

    // Invalidate dependent results
    for ( auto result : mudWeightWindowResults() )
    {
        this->deleteResult( result );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMudWeightWindowParameters::FractureGradientCalculationType
    RigFemPartResultsCollection::fractureGradientCalculationTypeMudWeightWindow() const
{
    return m_fractureGradientCalculationTypeMudWeightWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigFemPartResultsCollection::waterDensityShearSlipIndicator() const
{
    return m_waterDensityShearSlipIndicator;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigFemPartResultsCollection::setWaterDensityShearSlipIndicator( double waterDensity )
{
    m_waterDensityShearSlipIndicator = waterDensity;

    for ( auto elementType : {RIG_ELEMENT_NODAL, RIG_INTEGRATION_POINT} )
    {
        deleteResult( RigFemResultAddress( elementType, "ST", "DPN", RigFemResultAddress::allTimeLapsesValue() ) );
    }
}
