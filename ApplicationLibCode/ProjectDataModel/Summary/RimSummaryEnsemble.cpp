/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RimSummaryEnsemble.h"

#include "RiaEnsembleNameTools.h"
#include "RiaFieldHandleTools.h"
#include "RiaFilePathTools.h"
#include "RiaLogging.h"
#include "RiaStatisticsTools.h"
#include "RiaStdStringTools.h"
#include "RiaTextStringTools.h"
#include "Summary/Ensemble/RimSummaryEnsembleParameterCollection.h"
#include "Summary/RiaSummaryAddressAnalyzer.h"
#include "Summary/RiaSummaryTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimDeltaSummaryEnsemble.h"
#include "RimEnsembleCurveSet.h"
#include "RimProject.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsembleTools.h"
#include "RimSummaryPlot.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>

#include <cmath>

CAF_PDM_SOURCE_INIT( RimSummaryEnsemble, "SummaryCaseSubCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble::RimSummaryEnsemble()
    : caseNameChanged( this )
    , caseRemoved( this )
{
    CAF_PDM_InitScriptableObject( "Summary Case Group", ":/SummaryGroup16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "SummaryCases", "" );

    CAF_PDM_InitScriptableField( &m_name, "SummaryCollectionName", QString( "Group" ), "Name" );
    CAF_PDM_InitScriptableField( &m_autoName, "CreateAutoName", true, "Auto Name" );
    CAF_PDM_InitScriptableField( &m_useKey1, "UseKey1", false, "Use First Path Part" );
    CAF_PDM_InitScriptableField( &m_useKey2, "UseKey2", false, "Use Second Path Part" );

    QString defaultText = RiaDefines::key1VariableName() + "-" + RiaDefines::key2VariableName();
    QString tooltipText = QString( "Variables in template is supported, and will be replaced to create name. Example '%1'" ).arg( defaultText );
    CAF_PDM_InitField( &m_nameTemplateString, "NameTemplateString", defaultText, "Name Template", "", tooltipText );

    CAF_PDM_InitFieldNoDefault( &m_groupingMode, "GroupingMode", "Grouping Mode" );

    CAF_PDM_InitScriptableField( &m_includeInAutoReload, "IncludeInAutoReload", false, "Include in Automatic Case Reloads" );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_includeInAutoReload );

    CAF_PDM_InitScriptableFieldNoDefault( &m_nameAndItemCount, "NameCount", "Name" );
    m_nameAndItemCount.registerGetMethod( this, &RimSummaryEnsemble::nameAndItemCount );
    RiaFieldHandleTools::disableWriteAndSetFieldHidden( &m_nameAndItemCount );

    CAF_PDM_InitScriptableField( &m_isEnsemble, "IsEnsemble", false, "Is Ensemble" );
    m_isEnsemble.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableField( &m_ensembleId, "Id", -1, "Ensemble ID" );
    m_ensembleId.registerKeywordAlias( "EnsembleId" );
    m_ensembleId.uiCapability()->setUiReadOnly( true );
    m_ensembleId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    CAF_PDM_InitFieldNoDefault( &m_dataVectorFolders, "DataVectorFolders", "Data Folders" );
    m_dataVectorFolders = new RimSummaryAddressCollection();
    m_dataVectorFolders.uiCapability()->setUiHidden( true );
    m_dataVectorFolders->uiCapability()->setUiTreeHidden( true );
    m_dataVectorFolders.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_ensembleParameters, "EnsembleParameters", "Ensemble Parameters" );
    m_ensembleParameters = new RimSummaryEnsembleParameterCollection();
    m_ensembleParameters.uiCapability()->setUiHidden( true );
    m_ensembleParameters->uiCapability()->setUiTreeHidden( true );
    m_ensembleParameters.xmlCapability()->disableIO();

    CAF_PDM_InitFieldNoDefault( &m_ensembleDescription, "Description", "Description" );
    m_ensembleDescription.registerGetMethod( this, &RimSummaryEnsemble::ensembleDescription );
    m_ensembleDescription.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::TOP );
    m_ensembleDescription.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    m_ensembleDescription.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryEnsemble::~RimSummaryEnsemble()
{
    m_cases.deleteChildrenAsync();
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::removeCase( RimSummaryCase* summaryCase, bool notifyChange )
{
    size_t caseCountBeforeRemove = m_cases.size();

    m_cases.removeChild( summaryCase );

    m_cachedSortedEnsembleParameters.clear();
    m_analyzer.reset();

    caseRemoved.send( summaryCase );

    if ( notifyChange )
    {
        updateReferringCurveSetsZoomAll();
    }

    if ( m_isEnsemble && m_cases.size() != caseCountBeforeRemove )
    {
        if ( dynamic_cast<RimDeltaSummaryCase*>( summaryCase ) == nullptr ) calculateEnsembleParametersIntersectionHash();
    }

    clearChildNodes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::addCase( RimSummaryCase* summaryCase, bool notifyChange )
{
    summaryCase->nameChanged.connect( this, &RimSummaryEnsemble::onCaseNameChanged );

    summaryCase->setShowVectorItemsInProjectTree( m_cases.empty() );

    m_cases.push_back( summaryCase );
    m_cachedSortedEnsembleParameters.clear();
    m_analyzer.reset();

    // Update derived ensemble cases (if any)
    std::vector<RimDeltaSummaryEnsemble*> referringObjects = objectsWithReferringPtrFieldsOfType<RimDeltaSummaryEnsemble>();
    for ( auto derivedEnsemble : referringObjects )
    {
        if ( !derivedEnsemble ) continue;

        derivedEnsemble->createDerivedEnsembleCases();
        if ( notifyChange ) derivedEnsemble->updateReferringCurveSetsZoomAll();
    }

    if ( m_isEnsemble )
    {
        validateEnsembleCases( { summaryCase } );
        calculateEnsembleParametersIntersectionHash();
    }

    if ( notifyChange ) updateReferringCurveSetsZoomAll();

    clearChildNodes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryEnsemble::allSummaryCases() const
{
    return m_cases.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryEnsemble::firstSummaryCase() const
{
    if ( !m_cases.empty() ) return m_cases[0];

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::replaceCases( const std::vector<RimSummaryCase*>& summaryCases, bool notifyChange )
{
    m_cases.deleteChildrenAsync();

    if ( summaryCases.empty() ) return;

    auto lastCase = summaryCases.back();

    for ( auto summaryCase : summaryCases )
    {
        if ( summaryCase == lastCase ) continue;

        // Do what is required to add the case, avoid updates until all cases are added
        summaryCase->nameChanged.connect( this, &RimSummaryEnsemble::onCaseNameChanged );
        if ( m_cases.empty() )
        {
            summaryCase->setShowVectorItemsInProjectTree( true );
        }
        m_cases.push_back( summaryCase );
    }

    if ( lastCase )
    {
        // Add the last case, and update connected plots
        addCase( lastCase, notifyChange );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::reloadCases()
{
    for ( auto summaryCase : allSummaryCases() )
    {
        bool buildAddressObjects = false;
        RiaSummaryTools::reloadSummaryCaseAndUpdateConnectedPlots( summaryCase, buildAddressObjects );

        RiaLogging::info( QString( "Reloaded data for %1" ).arg( summaryCase->summaryHeaderFilename() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::setNameTemplate( const QString& name )
{
    m_nameTemplateString = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryEnsemble::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::updateName( const std::set<QString>& existingEnsembleNames )
{
    const auto [key1, key2] = nameKeys();

    QString templateText;
    if ( m_autoName )
    {
        templateText = nameTemplateText();
    }
    else
    {
        templateText = m_nameTemplateString();
    }

    std::map<QString, QString> keyValues = {
        { RiaDefines::key1VariableName(), QString::fromStdString( key1 ) },
        { RiaDefines::key2VariableName(), QString::fromStdString( key2 ) },
    };

    auto candidateName = RiaTextStringTools::replaceTemplateTextWithValues( templateText, keyValues );

    if ( m_autoName )
    {
        candidateName = candidateName.trimmed();

        // When using auto name, remove leading and trailing commas that may occur if key1 or key2 is empty
        if ( candidateName.startsWith( "," ) )
        {
            candidateName = candidateName.mid( 1 );
        }
        if ( candidateName.endsWith( "," ) )
        {
            candidateName = candidateName.left( candidateName.length() - 1 );
        }

        // Avoid identical ensemble names by appending a number
        if ( existingEnsembleNames.contains( candidateName ) )
        {
            int     counter = 1;
            QString uniqueName;
            do
            {
                uniqueName = QString( "%1 (subset-%2)" ).arg( candidateName ).arg( counter++ );
            } while ( existingEnsembleNames.contains( uniqueName ) );

            candidateName = uniqueName;
        }
    }

    if ( m_name == candidateName ) return;

    m_name = candidateName;
    caseNameChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::setUsePathKey1( bool useKey1 )
{
    m_useKey1 = useKey1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::setUsePathKey2( bool useKey2 )
{
    m_useKey2 = useKey2;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::setGroupingMode( RiaDefines::EnsembleGroupingMode groupingMode )
{
    m_groupingMode = groupingMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EnsembleGroupingMode RimSummaryEnsemble::groupingMode() const
{
    return m_groupingMode();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsemble::isEnsemble() const
{
    return m_isEnsemble();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::setAsEnsemble( bool isEnsemble )
{
    if ( isEnsemble != m_isEnsemble )
    {
        m_isEnsemble = isEnsemble;
        updateIcon();

        if ( m_isEnsemble && dynamic_cast<RimDeltaSummaryEnsemble*>( this ) == nullptr )
        {
            validateEnsembleCases( allSummaryCases() );
            calculateEnsembleParametersIntersectionHash();
        }

        buildMetaData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> RimSummaryEnsemble::ensembleSummaryAddresses() const
{
    // For performance reasons, we only return the addresses from the case with the most keywords.
    // Calling buildMetaData() on all realizations is expensive
    if ( auto caseWithMostKeywords = RimSummaryEnsembleTools::caseWithMostDataObjects( m_cases.childrenByType() ) )
    {
        if ( auto reader = caseWithMostKeywords->summaryReader() )
        {
            // If the reader has no addresses, we need to build the metadata to populate them
            // This is typically the case for newly created cases or cases that have not been read yet
            reader->createAddressesIfRequired();

            return reader->allResultAddresses();
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<time_t> RimSummaryEnsemble::ensembleTimeSteps() const
{
    std::set<time_t> allTimeSteps;
    size_t           maxAddrCount = 0;
    int              maxAddrIndex = -1;

    for ( int i = 0; i < (int)m_cases.size(); i++ )
    {
        RimSummaryCase* currCase = m_cases[i];
        if ( !currCase ) continue;

        RifSummaryReaderInterface* reader = currCase->summaryReader();
        if ( !reader ) continue;

        size_t addrCount = reader->allResultAddresses().size();
        if ( addrCount > maxAddrCount )
        {
            maxAddrCount = addrCount;
            maxAddrIndex = (int)i;
        }
    }

    if ( maxAddrIndex >= 0 && m_cases[maxAddrIndex]->summaryReader() )
    {
        RifSummaryReaderInterface* reader = m_cases[maxAddrIndex]->summaryReader();

        const std::set<RifEclipseSummaryAddress>& addrs = reader->allResultAddresses();
        for ( RifEclipseSummaryAddress addr : addrs )
        {
            std::vector<time_t> timeSteps = reader->timeSteps( addr );
            if ( !timeSteps.empty() )
            {
                allTimeSteps.insert( timeSteps.begin(), timeSteps.end() );
                break;
            }
        }
    }
    return allTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEnsembleParameter> RimSummaryEnsemble::variationSortedEnsembleParameters( bool excludeNoVariation ) const
{
    if ( m_cachedSortedEnsembleParameters.empty() )
    {
        m_cachedSortedEnsembleParameters = RimSummaryEnsembleTools::createVariationSortedEnsembleParameters( allSummaryCases() );
    }

    if ( !excludeNoVariation )
    {
        return m_cachedSortedEnsembleParameters;
    }

    const double                      epsilon = 1e-9;
    std::vector<RigEnsembleParameter> parametersWithVariation;
    for ( const auto& p : m_cachedSortedEnsembleParameters )
    {
        if ( std::abs( p.normalizedStdDeviation() ) > epsilon )
        {
            parametersWithVariation.push_back( p );
        }
    }
    return parametersWithVariation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<RigEnsembleParameter, double>>
    RimSummaryEnsemble::correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address ) const
{
    auto parameters = parameterCorrelationsAllTimeSteps( address );
    std::sort( parameters.begin(),
               parameters.end(),
               []( const std::pair<RigEnsembleParameter, double>& lhs, const std::pair<RigEnsembleParameter, double>& rhs )
               { return std::abs( lhs.second ) > std::abs( rhs.second ); } );
    return parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<RigEnsembleParameter, double>>
    RimSummaryEnsemble::correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address, time_t selectedTimeStep ) const
{
    auto parameters = parameterCorrelations( address, selectedTimeStep );
    std::sort( parameters.begin(),
               parameters.end(),
               []( const std::pair<RigEnsembleParameter, double>& lhs, const std::pair<RigEnsembleParameter, double>& rhs )
               { return std::abs( lhs.second ) > std::abs( rhs.second ); } );
    return parameters;
}

time_t timeDiff( time_t lhs, time_t rhs )
{
    if ( lhs >= rhs )
    {
        return lhs - rhs;
    }
    return rhs - lhs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<RigEnsembleParameter, double>>
    RimSummaryEnsemble::parameterCorrelations( const RifEclipseSummaryAddress&  address,
                                               time_t                           timeStep,
                                               const std::vector<QString>&      selectedParameters,
                                               const std::set<RimSummaryCase*>& selectedCases ) const
{
    auto parameters = variationSortedEnsembleParameters( true );

    if ( !selectedParameters.empty() )
    {
        parameters.erase( std::remove_if( parameters.begin(),
                                          parameters.end(),
                                          [&selectedParameters]( const RigEnsembleParameter& parameter )
                                          {
                                              return std::find( selectedParameters.begin(), selectedParameters.end(), parameter.name ) ==
                                                     selectedParameters.end();
                                          } ),
                          parameters.end() );
    }

    std::vector<double>                                 caseValuesAtTimestep;
    std::map<RigEnsembleParameter, std::vector<double>> parameterValues;

    for ( size_t caseIdx = 0u; caseIdx < m_cases.size(); ++caseIdx )
    {
        RimSummaryCase*            summaryCase = m_cases[caseIdx];
        RifSummaryReaderInterface* reader      = summaryCase->summaryReader();
        if ( !reader ) continue;

        if ( !selectedCases.empty() && selectedCases.count( summaryCase ) == 0 ) continue;

        if ( !summaryCase->caseRealizationParameters() ) continue;

        double closestValue    = std::numeric_limits<double>::infinity();
        time_t closestTimeStep = 0;
        auto [isOk, values]    = reader->values( address );
        if ( isOk )
        {
            const std::vector<time_t>& timeSteps = reader->timeSteps( address );
            for ( size_t i = 0; i < timeSteps.size(); ++i )
            {
                if ( timeDiff( timeSteps[i], timeStep ) < timeDiff( timeStep, closestTimeStep ) )
                {
                    closestValue    = values[i];
                    closestTimeStep = timeSteps[i];
                }
            }
        }
        if ( closestValue != std::numeric_limits<double>::infinity() )
        {
            caseValuesAtTimestep.push_back( closestValue );

            for ( auto parameter : parameters )
            {
                if ( parameter.isNumeric() && parameter.isValid() )
                {
                    double paramValue = parameter.values[caseIdx].toDouble();
                    parameterValues[parameter].push_back( paramValue );
                }
            }
        }
    }

    std::vector<std::pair<RigEnsembleParameter, double>> correlationResults;
    for ( auto parameterValuesPair : parameterValues )
    {
        double correlation = 0.0;
        double pearson     = RiaStatisticsTools::pearsonCorrelation( parameterValuesPair.second, caseValuesAtTimestep );
        if ( pearson != std::numeric_limits<double>::infinity() ) correlation = pearson;
        correlationResults.push_back( std::make_pair( parameterValuesPair.first, correlation ) );
    }
    return correlationResults;
}

//--------------------------------------------------------------------------------------------------
/// Returns a vector of the parameters and the average absolute values of correlations per time step
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<RigEnsembleParameter, double>>
    RimSummaryEnsemble::parameterCorrelationsAllTimeSteps( const RifEclipseSummaryAddress& address,
                                                           const std::vector<QString>&     selectedParameters ) const
{
    const size_t     maxTimeStepCount = 10;
    std::set<time_t> timeSteps        = ensembleTimeSteps();
    if ( timeSteps.empty() ) return {};

    std::vector<time_t> timeStepsVector( timeSteps.begin(), timeSteps.end() );
    size_t              stride = std::max( (size_t)1, timeStepsVector.size() / maxTimeStepCount );

    std::vector<std::vector<std::pair<RigEnsembleParameter, double>>> correlationsForChosenTimeSteps;

    for ( size_t i = stride; i < timeStepsVector.size(); i += stride )
    {
        std::vector<std::pair<RigEnsembleParameter, double>> correlationsForTimeStep =
            parameterCorrelations( address, timeStepsVector[i], selectedParameters );
        correlationsForChosenTimeSteps.push_back( correlationsForTimeStep );
    }

    for ( size_t i = 1; i < correlationsForChosenTimeSteps.size(); ++i )
    {
        for ( size_t j = 0; j < correlationsForChosenTimeSteps[0].size(); ++j )
        {
            correlationsForChosenTimeSteps[0][j].second += correlationsForChosenTimeSteps[i][j].second;
        }
    }
    for ( size_t j = 0; j < correlationsForChosenTimeSteps[0].size(); ++j )
    {
        correlationsForChosenTimeSteps[0][j].second /= correlationsForChosenTimeSteps.size();
    }

    return correlationsForChosenTimeSteps[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigEnsembleParameter RimSummaryEnsemble::ensembleParameter( const QString& paramName ) const
{
    if ( !isEnsemble() || paramName.isEmpty() ) return {};

    for ( const RigEnsembleParameter& ensParam : variationSortedEnsembleParameters() )
    {
        if ( ensParam.name == paramName ) return ensParam;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::calculateEnsembleParametersIntersectionHash()
{
    RimSummaryEnsembleTools::calculateEnsembleParametersIntersectionHash( allSummaryCases() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::loadDataAndUpdate()
{
    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsemble::validateEnsembleCases( const std::vector<RimSummaryCase*> cases )
{
    // Validate ensemble parameters
    QString                errors;
    std::hash<std::string> paramsHasher;
    size_t                 paramsHash        = 0;
    RimSummaryCase*        parameterBaseCase = nullptr;

    for ( RimSummaryCase* rimCase : cases )
    {
        if ( rimCase->caseRealizationParameters() == nullptr || rimCase->caseRealizationParameters()->parameters().empty() )
        {
            errors.append(
                QString( "The case %1 has no ensemble parameters\n" ).arg( QFileInfo( rimCase->summaryHeaderFilename() ).fileName() ) );
        }
        else
        {
            QString paramNames;
            for ( std::pair<QString, RigCaseRealizationParameters::Value> paramPair : rimCase->caseRealizationParameters()->parameters() )
            {
                paramNames.append( paramPair.first );
            }

            size_t currHash = paramsHasher( paramNames.toStdString() );
            if ( paramsHash == 0 )
            {
                paramsHash        = currHash;
                parameterBaseCase = rimCase;
            }
            else if ( paramsHash != currHash )
            {
                errors.append( QString( "The parameters in case %1 is not matching base parameters in %2\n" )
                                   .arg( QFileInfo( rimCase->summaryHeaderFilename() ).fileName() )
                                   .arg( QFileInfo( parameterBaseCase->summaryHeaderFilename() ).fileName() ) );
            }
        }
    }

    if ( !errors.isEmpty() )
    {
        const int maxNumberOfCharactersToDisplaye = 1000;
        QString   textToDisplay                   = errors.left( maxNumberOfCharactersToDisplaye );

        RiaLogging::errorInMessageBox( nullptr, "", textToDisplay );

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Sorting operator for sets and maps. Sorts by name.
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsemble::operator<( const RimSummaryEnsemble& rhs ) const
{
    return name() < rhs.name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RimSummaryEnsemble::unitSystem() const
{
    if ( m_cases.empty() )
    {
        return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
    }
    return m_cases[0]->unitsSystem();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryEnsemble::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::onLoadDataAndUpdate()
{
    if ( m_isEnsemble )
    {
        calculateEnsembleParametersIntersectionHash();
        clearChildNodes();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::updateReferringCurveSets( bool doZoomAll )
{
    // Update curve set referring to this group
    std::vector<caf::PdmObject*> referringObjects = objectsWithReferringPtrFieldsOfType<PdmObject>();

    for ( auto object : referringObjects )
    {
        RimEnsembleCurveSet* curveSet = dynamic_cast<RimEnsembleCurveSet*>( object );

        bool updateParentPlot = true;
        if ( curveSet )
        {
            curveSet->loadDataAndUpdate( updateParentPlot );

            if ( doZoomAll )
            {
                if ( auto parentPlot = curveSet->firstAncestorOrThisOfType<RimSummaryPlot>() )
                {
                    // If the ensemble name has changed, make sure the name in the project tree is updated
                    parentPlot->updateConnectedEditors();
                    parentPlot->zoomAll();
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::updateReferringCurveSets()
{
    updateReferringCurveSets( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::updateReferringCurveSetsZoomAll()
{
    updateReferringCurveSets( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryAddressAnalyzer* RimSummaryEnsemble::addressAnalyzer()
{
    if ( !m_analyzer )
    {
        m_analyzer = std::make_unique<RiaSummaryAddressAnalyzer>();

        m_analyzer->appendAddresses( ensembleSummaryAddresses() );
    }

    return m_analyzer.get();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::computeMinMax( const RifEclipseSummaryAddress& address )
{
    if ( m_minMaxValues.count( address ) > 0 ) return;

    double minimumValue( std::numeric_limits<double>::infinity() );
    double maximumValue( -std::numeric_limits<double>::infinity() );

    for ( const auto& s : m_cases() )
    {
        if ( auto reader = s->summaryReader() )
        {
            auto [isOk, values] = reader->values( address );
            if ( values.empty() ) continue;

            const auto [min, max] = std::minmax_element( values.begin(), values.end() );

            minimumValue = std::min( *min, minimumValue );
            maximumValue = std::max( *max, maximumValue );
        }
    }

    if ( minimumValue != std::numeric_limits<double>::infinity() && maximumValue != -std::numeric_limits<double>::infinity() )
    {
        setMinMax( address, minimumValue, maximumValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::setMinMax( const RifEclipseSummaryAddress& address, double min, double max )
{
    m_minMaxValues[address] = std::pair( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimSummaryEnsemble::minMax( const RifEclipseSummaryAddress& address )
{
    computeMinMax( address );

    return m_minMaxValues[address];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::cleanupBeforeDelete()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryEnsemble::nameAndItemCount() const
{
    size_t       itemCount          = m_cases.size();
    const size_t itemCountThreshold = 20;
    if ( itemCount > itemCountThreshold )
    {
        return QString( "%1 (%2)" ).arg( m_name() ).arg( itemCount );
    }

    return m_name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::updateIcon()
{
    if ( m_isEnsemble )
        setUiIconFromResourceString( ":/SummaryEnsemble.svg" );
    else
        setUiIconFromResourceString( ":/SummaryGroup16x16.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::initAfterRead()
{
    if ( m_ensembleId() == -1 )
    {
        RimProject* project = RimProject::current();
        project->assignIdToEnsemble( this );
    }

    updateIcon();

    for ( const auto& summaryCase : m_cases )
    {
        summaryCase->nameChanged.connect( this, &RimSummaryEnsemble::onCaseNameChanged );
    }

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2022.06.2" ) ) m_autoName = false;

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2024.12.2" ) )
    {
        m_nameTemplateString = m_name;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_isEnsemble )
    {
        updateIcon();
    }
    if ( changedField == &m_autoName || changedField == &m_nameTemplateString )
    {
        RiaSummaryTools::updateSummaryEnsembleNames();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::onCaseNameChanged( const SignalEmitter* emitter )
{
    caseNameChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const
{
    menuBuilder.subMenuStart( "Import" );
    menuBuilder << "RicImportSummaryCaseFeature";
    menuBuilder << "RicImportSummaryCasesFeature";
    menuBuilder << "RicImportSummaryGroupFeature";
    menuBuilder << "RicImportEnsembleFeature";
    menuBuilder.subMenuEnd();
    menuBuilder << "RicNewSummaryMultiPlotFeature";
    menuBuilder << "RicNewDerivedEnsembleFeature";
    menuBuilder << "RicOpenSummaryPlotEditorFeature";
    menuBuilder << "RicAppendSummaryCurvesForSummaryCasesFeature";
    menuBuilder << "RicAppendSummaryPlotsForSummaryCasesFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicAppendSummaryCurvesForSummaryCasesFeature";
    menuBuilder << "RicAppendSummaryPlotsForSummaryCasesFeature";
    menuBuilder << "RicCreateMultiPlotFromSelectionFeature";
    menuBuilder << "RicCreatePlotFromTemplateByShortcutFeature";
    menuBuilder.addSeparator();
    menuBuilder << "RicReloadSummaryCaseFeature";
    menuBuilder << "RicReplaceSummaryEnsembleFeature";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsemble::isAutoNameChecked() const
{
    return m_autoName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_autoName );

    if ( !m_autoName() )
    {
        uiOrdering.add( &m_nameTemplateString );
    }

    uiOrdering.add( &m_name );
    m_name.uiCapability()->setUiReadOnly( true );

    if ( m_isEnsemble() ) uiOrdering.add( &m_ensembleId );

    uiOrdering.add( &m_ensembleDescription );

    uiOrdering.add( &m_includeInAutoReload );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    if ( m_isEnsemble() )
    {
        buildChildNodes();

        m_dataVectorFolders->updateUiTreeOrdering( uiTreeOrdering );

        if ( !m_cases.empty() )
        {
            auto subnode = uiTreeOrdering.add( "Realizations", ":/Folder.png" );
            for ( auto& smcase : m_cases )
            {
                subnode->add( smcase );
            }
        }

        m_ensembleParameters->updateUiTreeOrdering( uiTreeOrdering );

        uiTreeOrdering.skipRemainingChildren( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_nameTemplateString )
    {
        if ( auto attr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute ) )
        {
            attr->placeholderText        = RiaDefines::key1VariableName() + "-" + RiaDefines::key2VariableName();
            attr->notifyWhenTextIsEdited = true;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( auto* treeItemAttribute = dynamic_cast<caf::PdmUiTreeViewItemAttribute*>( attribute ) )
    {
        if ( m_includeInAutoReload() )
        {
            auto tag  = caf::PdmUiTreeViewItemAttribute::createTag();
            tag->icon = caf::IconProvider( ":/TimedRefresh.png" );
            treeItemAttribute->tags.push_back( std::move( tag ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryEnsemble::ensembleDescription() const
{
    QString txt;

    if ( !m_cases.empty() )
    {
        txt += QString( "First summary file of %1 files :\n  " ).arg( m_cases.size() );
        auto firstCase = m_cases[0]->summaryHeaderFilename();
        txt += firstCase;
        txt += "\n";
    }

    const auto [key1, key2] = nameKeys();
    txt += "\nDetected variables that can be used when defining the text in the 'Name Template' field:\n";
    txt += QString( "  %1: %2\n" ).arg( RiaDefines::key1VariableName() ).arg( QString::fromStdString( key1 ) );
    txt += QString( "  %1: %2\n" ).arg( RiaDefines::key2VariableName() ).arg( QString::fromStdString( key2 ) );

    txt += "\n";
    if ( m_groupingMode() == RiaDefines::EnsembleGroupingMode::EVEREST_FOLDER_STRUCTURE )
    {
        txt += "The ensemble is grouped by the file folder structure.\n";
    }
    else if ( m_groupingMode() == RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE )
    {
        txt += "The ensemble is grouped by the FMU file folder structure.\n";
    }
    else if ( m_groupingMode() == RiaDefines::EnsembleGroupingMode::RESINSIGHT_OPMFLOW_STRUCTURE )
    {
        txt += "The ensemble is grouped by the ResInsight Opm Flow file folder structure.\n";
    }

    return txt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<std::string, std::string> RimSummaryEnsemble::nameKeys() const
{
    std::string key1 = "Undefined KEY1";
    std::string key2 = "Undefined KEY2";

    if ( m_groupingMode() == RiaDefines::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE )
    {
        if ( !m_cases.empty() )
        {
            auto fileNames = RiaEnsembleNameTools::groupFilePathsFmu( { m_cases[0]->summaryHeaderFilename().toStdString() } );
            if ( !fileNames.empty() )
            {
                key1 = fileNames.begin()->first.first;
                key2 = fileNames.begin()->first.second;
            }
        }
    }
    else if ( m_groupingMode() == RiaDefines::EnsembleGroupingMode::EVEREST_FOLDER_STRUCTURE )
    {
        if ( m_cases.size() > 1 )
        {
            auto name1 = RiaFilePathTools::toInternalSeparator( m_cases[0]->summaryHeaderFilename() ).toStdString();
            auto name2 = RiaFilePathTools::toInternalSeparator( m_cases[1]->summaryHeaderFilename() ).toStdString();

            auto parts1 = RiaStdStringTools::splitString( name1, '/' );
            auto parts2 = RiaStdStringTools::splitString( name2, '/' );

            size_t commonParts = 0;
            for ( size_t i = 0; i < std::min( parts1.size(), parts2.size() ); i++ )
            {
                if ( parts1[i] == parts2[i] )
                {
                    commonParts++;
                }
                else
                {
                    break;
                }
            }

            if ( commonParts == 1 )
            {
                key2 = parts1[commonParts - 1];
            }
            else if ( commonParts > 1 )
            {
                key1 = parts1[commonParts - 2];
                key2 = parts1[commonParts - 1];
            }
        }
    }
    else if ( m_groupingMode() == RiaDefines::EnsembleGroupingMode::RESINSIGHT_OPMFLOW_STRUCTURE )
    {
        if ( !m_cases.empty() )
        {
            auto fileNames = RiaEnsembleNameTools::groupFilePathsOpm( { m_cases[0]->summaryHeaderFilename().toStdString() } );
            if ( !fileNames.empty() )
            {
                key1 = fileNames.begin()->first.first;
                key2 = fileNames.begin()->first.second;
            }
        }
    }

    return { key1, key2 };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryEnsemble::nameTemplateText() const
{
    QString text;
    if ( m_useKey1() ) text += RiaDefines::key1VariableName();
    if ( m_useKey2() )
    {
        if ( !text.isEmpty() ) text += ", ";
        text += RiaDefines::key2VariableName();
    }

    return text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::setEnsembleId( int ensembleId )
{
    m_ensembleId = ensembleId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryEnsemble::ensembleId() const
{
    return m_ensembleId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsemble::hasEnsembleParameters() const
{
    for ( RimSummaryCase* rimCase : allSummaryCases() )
    {
        if ( rimCase->caseRealizationParameters() != nullptr )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::buildChildNodes()
{
    if ( m_dataVectorFolders->isEmpty() )
    {
        m_dataVectorFolders->updateFolderStructure( ensembleSummaryAddresses(), -1, m_ensembleId );
    }
    if ( m_ensembleParameters->isEmpty() )
    {
        m_ensembleParameters->setEnsembleId( ensembleId() );
        for ( auto& ensPar : variationSortedEnsembleParameters() )
        {
            m_ensembleParameters->addParameter( ensPar.name, false /*check duplicate*/ );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::buildMetaData()
{
    clearChildNodes();
    buildChildNodes();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::onCalculationUpdated()
{
    m_dataVectorFolders->deleteCalculatedAddresses();
    m_dataVectorFolders->updateFolderStructure( ensembleSummaryAddresses(), -1, m_ensembleId );

    m_analyzer.reset();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryEnsemble::clearChildNodes()
{
    m_dataVectorFolders->deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryEnsemble::includeInAutoReload() const
{
    return m_includeInAutoReload();
}
