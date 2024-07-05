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

#include "RimSummaryCaseCollection.h"

#include "RiaEnsembleNameTools.h"
#include "RiaFieldHandleTools.h"
#include "RiaLogging.h"
#include "RiaStatisticsTools.h"
#include "RiaSummaryAddressAnalyzer.h"

#include "RifSummaryReaderInterface.h"

#include "RimDerivedEnsembleCaseCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimProject.h"
#include "RimSummaryAddressCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsembleTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

#include <QFileInfo>

#include <cmath>

CAF_PDM_SOURCE_INIT( RimSummaryCaseCollection, "SummaryCaseSubCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::RimSummaryCaseCollection()
    : caseNameChanged( this )
    , caseRemoved( this )
{
    CAF_PDM_InitScriptableObject( "Summary Case Group", ":/SummaryGroup16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "SummaryCases", "" );

    CAF_PDM_InitScriptableField( &m_name, "SummaryCollectionName", QString( "Group" ), "Name" );
    CAF_PDM_InitScriptableField( &m_autoName, "CreateAutoName", true, "Auto Name" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_nameAndItemCount, "NameCount", "Name" );
    m_nameAndItemCount.registerGetMethod( this, &RimSummaryCaseCollection::nameAndItemCount );
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

    m_commonAddressCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::~RimSummaryCaseCollection()
{
    m_cases.deleteChildrenAsync();
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::removeCase( RimSummaryCase* summaryCase, bool notifyChange )
{
    size_t caseCountBeforeRemove = m_cases.size();

    m_cases.removeChild( summaryCase );

    m_cachedSortedEnsembleParameters.clear();
    m_analyzer.reset();

    caseRemoved.send( summaryCase );

    if ( notifyChange )
    {
        updateReferringCurveSets();
    }

    if ( m_isEnsemble && m_cases.size() != caseCountBeforeRemove )
    {
        if ( dynamic_cast<RimDerivedSummaryCase*>( summaryCase ) == nullptr ) calculateEnsembleParametersIntersectionHash();
    }

    clearChildNodes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::addCase( RimSummaryCase* summaryCase )
{
    summaryCase->nameChanged.connect( this, &RimSummaryCaseCollection::onCaseNameChanged );

    summaryCase->setShowVectorItemsInProjectTree( m_cases.empty() );

    m_cases.push_back( summaryCase );
    m_cachedSortedEnsembleParameters.clear();
    m_analyzer.reset();

    // Update derived ensemble cases (if any)
    std::vector<RimDerivedEnsembleCaseCollection*> referringObjects = objectsWithReferringPtrFieldsOfType<RimDerivedEnsembleCaseCollection>();
    for ( auto derivedEnsemble : referringObjects )
    {
        if ( !derivedEnsemble ) continue;

        derivedEnsemble->createDerivedEnsembleCases();
        derivedEnsemble->updateReferringCurveSets();
    }

    if ( m_isEnsemble )
    {
        validateEnsembleCases( { summaryCase } );
        calculateEnsembleParametersIntersectionHash();
    }

    updateReferringCurveSets();

    clearChildNodes();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseCollection::allSummaryCases() const
{
    return m_cases.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimSummaryCaseCollection::firstSummaryCase() const
{
    if ( !m_cases.empty() ) return m_cases[0];

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setName( const QString& name )
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseCollection::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::ensureNameIsUpdated()
{
    if ( m_autoName )
    {
        QStringList fileNames;
        for ( const auto& summaryCase : m_cases )
        {
            fileNames.push_back( summaryCase->summaryHeaderFilename() );
        }

        RiaEnsembleNameTools::EnsembleGroupingMode groupingMode = RiaEnsembleNameTools::EnsembleGroupingMode::FMU_FOLDER_STRUCTURE;

        QString ensembleName = RiaEnsembleNameTools::findSuitableEnsembleName( fileNames, groupingMode );
        if ( m_name == ensembleName ) return;

        m_name = ensembleName;
        caseNameChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCaseCollection::isEnsemble() const
{
    return m_isEnsemble();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setAsEnsemble( bool isEnsemble )
{
    if ( isEnsemble != m_isEnsemble )
    {
        m_isEnsemble = isEnsemble;
        updateIcon();

        if ( m_isEnsemble && dynamic_cast<RimDerivedEnsembleCaseCollection*>( this ) == nullptr )
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
std::set<RifEclipseSummaryAddress> RimSummaryCaseCollection::ensembleSummaryAddresses() const
{
    std::set<RifEclipseSummaryAddress> addresses;
    size_t                             maxAddrCount = 0;
    int                                maxAddrIndex = -1;

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
        const std::set<RifEclipseSummaryAddress>& addrs = m_cases[maxAddrIndex]->summaryReader()->allResultAddresses();
        addresses.insert( addrs.begin(), addrs.end() );
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<time_t> RimSummaryCaseCollection::ensembleTimeSteps() const
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
std::vector<RigEnsembleParameter> RimSummaryCaseCollection::variationSortedEnsembleParameters( bool excludeNoVariation ) const
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
    RimSummaryCaseCollection::correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address ) const
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
    RimSummaryCaseCollection::correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address, time_t selectedTimeStep ) const
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
    RimSummaryCaseCollection::parameterCorrelations( const RifEclipseSummaryAddress&  address,
                                                     time_t                           timeStep,
                                                     const std::vector<QString>&      selectedParameters,
                                                     const std::set<RimSummaryCase*>& selectedCases ) const
{
    auto parameters = variationSortedEnsembleParameters( true );

    if ( !selectedParameters.empty() )
    {
        parameters.erase( std::remove_if( parameters.begin(),
                                          parameters.end(),
                                          [&selectedParameters]( const RigEnsembleParameter& parameter ) {
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
    RimSummaryCaseCollection::parameterCorrelationsAllTimeSteps( const RifEclipseSummaryAddress& address,
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
RigEnsembleParameter RimSummaryCaseCollection::ensembleParameter( const QString& paramName ) const
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
void RimSummaryCaseCollection::calculateEnsembleParametersIntersectionHash()
{
    m_commonAddressCount = RimSummaryEnsembleTools::calculateEnsembleParametersIntersectionHash( allSummaryCases() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::loadDataAndUpdate()
{
    onLoadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCaseCollection::validateEnsembleCases( const std::vector<RimSummaryCase*> cases )
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
bool RimSummaryCaseCollection::operator<( const RimSummaryCaseCollection& rhs ) const
{
    return name() < rhs.name();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RimSummaryCaseCollection::unitSystem() const
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
caf::PdmFieldHandle* RimSummaryCaseCollection::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::onLoadDataAndUpdate()
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
void RimSummaryCaseCollection::updateReferringCurveSets()
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
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaSummaryAddressAnalyzer* RimSummaryCaseCollection::addressAnalyzer()
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
void RimSummaryCaseCollection::computeMinMax( const RifEclipseSummaryAddress& address )
{
    if ( m_minMaxValues.count( address ) > 0 ) return;

    double minimumValue( std::numeric_limits<double>::infinity() );
    double maximumValue( -std::numeric_limits<double>::infinity() );

    for ( const auto& s : m_cases() )
    {
        if ( !s->summaryReader() ) continue;

        auto [isOk, values] = s->summaryReader()->values( address );
        if ( values.empty() ) continue;

        const auto [min, max] = std::minmax_element( values.begin(), values.end() );

        minimumValue = std::min( *min, minimumValue );
        maximumValue = std::max( *max, maximumValue );
    }

    if ( minimumValue != std::numeric_limits<double>::infinity() && maximumValue != -std::numeric_limits<double>::infinity() )
    {
        setMinMax( address, minimumValue, maximumValue );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setMinMax( const RifEclipseSummaryAddress& address, double min, double max )
{
    m_minMaxValues[address] = std::pair( min, max );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> RimSummaryCaseCollection::minMax( const RifEclipseSummaryAddress& address )
{
    computeMinMax( address );

    return m_minMaxValues[address];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseCollection::nameAndItemCount() const
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
void RimSummaryCaseCollection::updateIcon()
{
    if ( m_isEnsemble )
        setUiIconFromResourceString( ":/SummaryEnsemble.svg" );
    else
        setUiIconFromResourceString( ":/SummaryGroup16x16.png" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::initAfterRead()
{
    if ( m_ensembleId() == -1 )
    {
        RimProject* project = RimProject::current();
        project->assignIdToEnsemble( this );
    }

    updateIcon();

    for ( const auto& summaryCase : m_cases )
    {
        summaryCase->nameChanged.connect( this, &RimSummaryCaseCollection::onCaseNameChanged );
    }

    if ( RimProject::current()->isProjectFileVersionEqualOrOlderThan( "2022.06.2" ) ) m_autoName = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_isEnsemble )
    {
        updateIcon();
    }
    if ( changedField == &m_autoName )
    {
        ensureNameIsUpdated();
    }
    if ( changedField == &m_name )
    {
        m_autoName = false;
        caseNameChanged.send();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::onCaseNameChanged( const SignalEmitter* emitter )
{
    caseNameChanged.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_autoName );
    uiOrdering.add( &m_name );
    if ( m_isEnsemble() )
    {
        uiOrdering.add( &m_ensembleId );
    }
    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
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

        uiTreeOrdering.skipRemainingChildren( true );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::setEnsembleId( int ensembleId )
{
    m_ensembleId = ensembleId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimSummaryCaseCollection::ensembleId() const
{
    return m_ensembleId();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCaseCollection::hasEnsembleParameters() const
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
void RimSummaryCaseCollection::buildChildNodes()
{
    if ( m_dataVectorFolders->isEmpty() )
    {
        m_dataVectorFolders->updateFolderStructure( ensembleSummaryAddresses(), -1, m_ensembleId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::buildMetaData()
{
    clearChildNodes();
    buildChildNodes();
    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::onCalculationUpdated()
{
    m_dataVectorFolders->deleteCalculatedAddresses();
    m_dataVectorFolders->updateFolderStructure( ensembleSummaryAddresses(), -1, m_ensembleId );

    m_analyzer.reset();

    updateConnectedEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::clearChildNodes()
{
    m_dataVectorFolders->deleteChildren();
}
