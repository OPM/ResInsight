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

#include "RiaFieldHandleTools.h"
#include "RiaStatisticsTools.h"
#include "RiaWeightedMeanCalculator.h"

#include "RicfCommandObject.h"

#include "RimAnalysisPlotDataEntry.h"
#include "RimDerivedEnsembleCaseCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimGridSummaryCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"

#include "RifReaderEclipseRft.h"
#include "RifReaderEnsembleStatisticsRft.h"
#include "RifSummaryReaderInterface.h"

#include "cafPdmFieldScriptingCapability.h"

#include <QFileInfo>
#include <QMessageBox>

#include <algorithm>
#include <cmath>

CAF_PDM_SOURCE_INIT( RimSummaryCaseCollection, "SummaryCaseSubCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double EnsembleParameter::stdDeviation() const
{
    double N = static_cast<double>( values.size() );
    if ( N > 1 && isNumeric() )
    {
        double sumValues        = 0.0;
        double sumValuesSquared = 0.0;
        for ( const QVariant& variant : values )
        {
            double value = variant.toDouble();
            sumValues += value;
            sumValuesSquared += value * value;
        }

        return std::sqrt( ( N * sumValuesSquared - sumValues * sumValues ) / ( N * ( N - 1.0 ) ) );
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
/// Standard deviation normalized by max absolute value of min/max values.
/// Produces values between 0.0 and sqrt(2.0).
//--------------------------------------------------------------------------------------------------
double EnsembleParameter::normalizedStdDeviation() const
{
    const double eps = 1.0e-4;

    double maxAbs = std::max( std::fabs( maxValue ), std::fabs( minValue ) );
    if ( maxAbs < eps )
    {
        return 0.0;
    }

    double normalisedStdDev = stdDeviation() / maxAbs;
    if ( normalisedStdDev < eps )
    {
        return 0.0;
    }
    return normalisedStdDev;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool EnsembleParameter::operator<( const EnsembleParameter& other ) const
{
    if ( this->variationBin != other.variationBin )
    {
        return this->variationBin > other.variationBin; // Larger first
    }

    return this->name < other.name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::sortByBinnedVariation( std::vector<EnsembleParameter>& parameterVector )
{
    double minStdDev = std::numeric_limits<double>::infinity();
    double maxStdDev = 0.0;
    for ( const auto& paramPair : parameterVector )
    {
        double stdDev = paramPair.normalizedStdDeviation();
        if ( stdDev != 0.0 )
        {
            minStdDev = std::min( minStdDev, stdDev );
            maxStdDev = std::max( maxStdDev, stdDev );
        }
    }
    if ( ( maxStdDev - minStdDev ) <= 0.0 )
    {
        return;
    }

    double delta = ( maxStdDev - minStdDev ) / EnsembleParameter::NR_OF_VARIATION_BINS;

    std::vector<double> bins;
    bins.push_back( 0.0 );
    for ( int i = 0; i < EnsembleParameter::NR_OF_VARIATION_BINS - 1; ++i )
    {
        bins.push_back( minStdDev + ( i + 1 ) * delta );
    }

    for ( EnsembleParameter& nameParamPair : parameterVector )
    {
        int binNumber = -1;
        for ( double bin : bins )
        {
            if ( nameParamPair.normalizedStdDeviation() > bin )
            {
                binNumber++;
            }
        }
        nameParamPair.variationBin = binNumber;
    }

    // Sort by variation bin (highest first) but keep name as sorting parameter when parameters have the same variation
    // index
    std::stable_sort( parameterVector.begin(),
                      parameterVector.end(),
                      [&bins]( const EnsembleParameter& lhs, const EnsembleParameter& rhs ) {
                          return lhs.variationBin > rhs.variationBin;
                      } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString EnsembleParameter::uiName() const
{
    QString stem = name;
    QString variationString;
    if ( isNumeric() )
    {
        switch ( variationBin )
        {
            case NO_VARIATION:
                variationString = QString( " (No variation)" );
                break;
            case LOW_VARIATION:
                variationString = QString( " (Low variation)" );
                break;
            case MEDIUM_VARIATION:
                variationString = QString( " (Medium variation)" );
                break;
            case HIGH_VARIATION:
                variationString = QString( " (High variation)" );
                break;
        }
    }

    return QString( "%1%2" ).arg( stem ).arg( variationString );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::RimSummaryCaseCollection()
    : caseNameChanged( this )
    , caseRemoved( this )
{
    CAF_PDM_InitScriptableObject( "Summary Case Group", ":/SummaryGroup16x16.png", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_cases, "SummaryCases", "", "", "", "" );
    m_cases.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableField( &m_name, "SummaryCollectionName", QString( "Group" ), "Name", "", "", "" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_nameAndItemCount, "NameCount", "Name", "", "", "" );
    m_nameAndItemCount.registerGetMethod( this, &RimSummaryCaseCollection::nameAndItemCount );
    RiaFieldhandleTools::disableWriteAndSetFieldHidden( &m_nameAndItemCount );

    CAF_PDM_InitScriptableField( &m_isEnsemble, "IsEnsemble", false, "Is Ensemble", "", "", "" );
    m_isEnsemble.uiCapability()->setUiHidden( true );

    CAF_PDM_InitScriptableField( &m_ensembleId, "Id", -1, "Ensemble ID", "", "", "" );
    m_ensembleId.registerKeywordAlias( "EnsembleId" );
    m_ensembleId.uiCapability()->setUiReadOnly( true );
    m_ensembleId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    m_statisticsEclipseRftReader = new RifReaderEnsembleStatisticsRft( this );

    m_commonAddressCount = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseCollection::~RimSummaryCaseCollection()
{
    m_cases.deleteAllChildObjectsAsync();
    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::removeCase( RimSummaryCase* summaryCase )
{
    size_t caseCountBeforeRemove = m_cases.size();

    m_cases.removeChildObject( summaryCase );

    m_cachedSortedEnsembleParameters.clear();

    caseRemoved.send( summaryCase );

    updateReferringCurveSets();

    if ( m_isEnsemble && m_cases.size() != caseCountBeforeRemove )
    {
        if ( dynamic_cast<RimDerivedSummaryCase*>( summaryCase ) == nullptr )
            calculateEnsembleParametersIntersectionHash();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::addCase( RimSummaryCase* summaryCase )
{
    summaryCase->nameChanged.connect( this, &RimSummaryCaseCollection::onCaseNameChanged );

    m_cases.push_back( summaryCase );
    m_cachedSortedEnsembleParameters.clear();

    // Update derived ensemble cases (if any)
    std::vector<RimDerivedEnsembleCaseCollection*> referringObjects;
    objectsWithReferringPtrFieldsOfType( referringObjects );
    for ( auto derivedEnsemble : referringObjects )
    {
        if ( !derivedEnsemble ) continue;

        derivedEnsemble->createDerivedEnsembleCases();
        derivedEnsemble->updateReferringCurveSets();
    }

    if ( m_isEnsemble )
    {
        validateEnsembleCases( {summaryCase} );
        calculateEnsembleParametersIntersectionHash();
    }

    updateReferringCurveSets();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCase*> RimSummaryCaseCollection::allSummaryCases() const
{
    return m_cases.childObjects();
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
        const std::set<RifEclipseSummaryAddress>& addrs = m_cases[maxAddrIndex]->summaryReader()->allResultAddresses();
        for ( RifEclipseSummaryAddress addr : addrs )
        {
            std::vector<time_t> timeSteps = m_cases[maxAddrIndex]->summaryReader()->timeSteps( addr );
            allTimeSteps.insert( timeSteps.begin(), timeSteps.end() );
        }
    }
    return allTimeSteps;
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RimSummaryCaseCollection::wellsWithRftData() const
{
    std::set<QString> allWellNames;
    for ( RimSummaryCase* summaryCase : m_cases )
    {
        RifReaderRftInterface* reader = summaryCase->rftReader();
        if ( reader )
        {
            std::set<QString> wellNames = reader->wellNames();
            allWellNames.insert( wellNames.begin(), wellNames.end() );
        }
    }
    return allWellNames;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QDateTime> RimSummaryCaseCollection::rftTimeStepsForWell( const QString& wellName ) const
{
    std::set<QDateTime> allTimeSteps;
    for ( RimSummaryCase* summaryCase : m_cases )
    {
        RifReaderRftInterface* reader = summaryCase->rftReader();
        if ( reader )
        {
            std::set<QDateTime> timeStep = reader->availableTimeSteps( wellName );
            allTimeSteps.insert( timeStep.begin(), timeStep.end() );
        }
    }
    return allTimeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderRftInterface* RimSummaryCaseCollection::rftStatisticsReader()
{
    return m_statisticsEclipseRftReader.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<EnsembleParameter>&
    RimSummaryCaseCollection::variationSortedEnsembleParameters( bool excludeNoVariation ) const
{
    if ( m_cachedSortedEnsembleParameters.size() ) return m_cachedSortedEnsembleParameters;

    std::set<QString> paramSet;
    for ( RimSummaryCase* rimCase : this->allSummaryCases() )
    {
        if ( rimCase->caseRealizationParameters() != nullptr )
        {
            auto ps = rimCase->caseRealizationParameters()->parameters();
            for ( auto p : ps )
            {
                paramSet.insert( p.first );
            }
        }
    }

    m_cachedSortedEnsembleParameters.reserve( paramSet.size() );
    for ( const QString& parameterName : paramSet )
    {
        auto ensembleParameter = this->createEnsembleParameter( parameterName );
        if ( !excludeNoVariation || ensembleParameter.normalizedStdDeviation() != 0.0 )
        {
            m_cachedSortedEnsembleParameters.push_back( ensembleParameter );
        }
    }
    RimSummaryCaseCollection::sortByBinnedVariation( m_cachedSortedEnsembleParameters );

    return m_cachedSortedEnsembleParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<EnsembleParameter, double>>
    RimSummaryCaseCollection::correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address ) const
{
    auto parameters = parameterCorrelationsAllTimeSteps( address );
    std::sort( parameters.begin(),
               parameters.end(),
               []( const std::pair<EnsembleParameter, double>& lhs, const std::pair<EnsembleParameter, double>& rhs ) {
                   return std::abs( lhs.second ) > std::abs( rhs.second );
               } );
    return parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<EnsembleParameter, double>>
    RimSummaryCaseCollection::correlationSortedEnsembleParameters( const RifEclipseSummaryAddress& address,
                                                                   time_t selectedTimeStep ) const
{
    auto parameters = parameterCorrelations( address, selectedTimeStep );
    std::sort( parameters.begin(),
               parameters.end(),
               []( const std::pair<EnsembleParameter, double>& lhs, const std::pair<EnsembleParameter, double>& rhs ) {
                   return std::abs( lhs.second ) > std::abs( rhs.second );
               } );
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
std::vector<std::pair<EnsembleParameter, double>>
    RimSummaryCaseCollection::parameterCorrelations( const RifEclipseSummaryAddress& address,
                                                     time_t                          timeStep,
                                                     const std::vector<QString>&     selectedParameters ) const
{
    auto parameters = variationSortedEnsembleParameters( true );

    if ( !selectedParameters.empty() )
    {
        parameters.erase( std::remove_if( parameters.begin(),
                                          parameters.end(),
                                          [&selectedParameters]( const EnsembleParameter& parameter ) {
                                              return std::find( selectedParameters.begin(),
                                                                selectedParameters.end(),
                                                                parameter.name ) == selectedParameters.end();
                                          } ),
                          parameters.end() );
    }

    std::vector<double>                              caseValuesAtTimestep;
    std::map<EnsembleParameter, std::vector<double>> parameterValues;

    for ( size_t caseIdx = 0u; caseIdx < m_cases.size(); ++caseIdx )
    {
        RimSummaryCase*            summaryCase = m_cases[caseIdx];
        RifSummaryReaderInterface* reader      = summaryCase->summaryReader();
        if ( !reader ) continue;

        if ( !summaryCase->caseRealizationParameters() ) continue;

        std::vector<double> values;

        double closestValue    = std::numeric_limits<double>::infinity();
        time_t closestTimeStep = 0;
        if ( reader->values( address, &values ) )
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

    std::vector<std::pair<EnsembleParameter, double>> correlationResults;
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
std::vector<std::pair<EnsembleParameter, double>>
    RimSummaryCaseCollection::parameterCorrelationsAllTimeSteps( const RifEclipseSummaryAddress& address,
                                                                 const std::vector<QString>& selectedParameters ) const
{
    const size_t     maxTimeStepCount = 10;
    std::set<time_t> timeSteps        = ensembleTimeSteps();
    if ( timeSteps.empty() ) return {};

    std::vector<time_t> timeStepsVector( timeSteps.begin(), timeSteps.end() );
    size_t              stride = std::max( (size_t)1, timeStepsVector.size() / maxTimeStepCount );

    std::vector<std::vector<std::pair<EnsembleParameter, double>>> correlationsForChosenTimeSteps;

    for ( size_t i = stride; i < timeStepsVector.size(); i += stride )
    {
        std::vector<std::pair<EnsembleParameter, double>> correlationsForTimeStep =
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
std::vector<EnsembleParameter> RimSummaryCaseCollection::alphabeticEnsembleParameters() const
{
    std::set<QString> paramSet;
    for ( RimSummaryCase* rimCase : this->allSummaryCases() )
    {
        if ( rimCase->caseRealizationParameters() != nullptr )
        {
            auto ps = rimCase->caseRealizationParameters()->parameters();
            for ( auto p : ps )
            {
                paramSet.insert( p.first );
            }
        }
    }

    std::vector<EnsembleParameter> sortedEnsembleParameters;
    sortedEnsembleParameters.reserve( paramSet.size() );
    for ( const QString& parameterName : paramSet )
    {
        sortedEnsembleParameters.push_back( this->createEnsembleParameter( parameterName ) );
    }
    return sortedEnsembleParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
EnsembleParameter RimSummaryCaseCollection::ensembleParameter( const QString& paramName ) const
{
    if ( !isEnsemble() || paramName.isEmpty() ) return EnsembleParameter();

    const std::vector<EnsembleParameter>& ensembleParams = variationSortedEnsembleParameters();

    for ( const EnsembleParameter& ensParam : ensembleParams )
    {
        if ( ensParam.name == paramName ) return ensParam;
    }

    return EnsembleParameter();
}

EnsembleParameter RimSummaryCaseCollection::createEnsembleParameter( const QString& paramName ) const
{
    EnsembleParameter eParam;
    eParam.name = paramName;

    size_t numericValuesCount = 0;
    size_t textValuesCount    = 0;

    auto summaryCases = allSummaryCases();
    // Make sure the values list exactly matches the case count
    // And use an invalid value (infinity) for invalid cases.
    eParam.values.resize( summaryCases.size(), std::numeric_limits<double>::infinity() );

    // Prepare case realization params, and check types
    for ( size_t caseIdx = 0; caseIdx < summaryCases.size(); ++caseIdx )
    {
        auto rimCase = summaryCases[caseIdx];

        auto crp = rimCase->caseRealizationParameters();
        if ( !crp ) continue;

        auto value = crp->parameterValue( paramName );
        if ( !value.isValid() ) continue;

        if ( value.isNumeric() )
        {
            double numVal          = value.numericValue();
            eParam.values[caseIdx] = QVariant( numVal );
            if ( numVal < eParam.minValue ) eParam.minValue = numVal;
            if ( numVal > eParam.maxValue ) eParam.maxValue = numVal;
            numericValuesCount++;
        }
        else if ( value.isText() )
        {
            eParam.values[caseIdx] = QVariant( value.textValue() );
            textValuesCount++;
        }
    }

    if ( numericValuesCount && !textValuesCount )
    {
        eParam.type = EnsembleParameter::TYPE_NUMERIC;
    }
    else if ( textValuesCount && !numericValuesCount )
    {
        eParam.type = EnsembleParameter::TYPE_TEXT;
    }
    if ( numericValuesCount && textValuesCount )
    {
        // A mix of types have been added to parameter values
        if ( numericValuesCount > textValuesCount )
        {
            // Use numeric type
            for ( auto& val : eParam.values )
            {
                if ( val.type() == QVariant::String )
                {
                    val.setValue( std::numeric_limits<double>::infinity() );
                }
            }
            eParam.type = EnsembleParameter::TYPE_NUMERIC;
        }
        else
        {
            // Use text type
            for ( auto& val : eParam.values )
            {
                if ( val.type() == QVariant::Double )
                {
                    val.setValue( QString::number( val.value<double>() ) );
                }
            }
            eParam.type     = EnsembleParameter::TYPE_TEXT;
            eParam.minValue = std::numeric_limits<double>::infinity();
            eParam.maxValue = -std::numeric_limits<double>::infinity();
        }
    }

    if ( eParam.isText() )
    {
        // Remove duplicate texts
        std::set<QString> valueSet;
        for ( const auto& val : eParam.values )
        {
            valueSet.insert( val.toString() );
        }
        eParam.values.clear();
        for ( const auto& val : valueSet )
        {
            eParam.values.push_back( QVariant( val ) );
        }
    }

    return eParam;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::calculateEnsembleParametersIntersectionHash()
{
    clearEnsembleParametersHashes();

    // Find ensemble parameters intersection
    std::set<QString> paramNames;
    auto              sumCases = allSummaryCases();

    for ( size_t i = 0; i < sumCases.size(); i++ )
    {
        auto crp = sumCases[i]->caseRealizationParameters();
        if ( !crp ) continue;

        auto caseParamNames = crp->parameterNames();

        if ( i == 0 )
            paramNames = caseParamNames;
        else
        {
            std::set<QString> newIntersection;
            std::set_intersection( paramNames.begin(),
                                   paramNames.end(),
                                   caseParamNames.begin(),
                                   caseParamNames.end(),
                                   std::inserter( newIntersection, newIntersection.end() ) );

            if ( paramNames.size() != newIntersection.size() ) paramNames = newIntersection;
        }
    }

    for ( auto sumCase : sumCases )
    {
        auto crp = sumCase->caseRealizationParameters();
        if ( crp ) crp->calculateParametersHash( paramNames );
    }

    // Find common addess count
    for ( const auto sumCase : sumCases )
    {
        const auto reader = sumCase->summaryReader();
        if ( !reader ) continue;
        auto currAddrCount = reader->allResultAddresses().size();

        if ( m_commonAddressCount == 0 )
        {
            m_commonAddressCount = currAddrCount;
        }
        else
        {
            if ( currAddrCount != m_commonAddressCount )
            {
                m_commonAddressCount = 0;
                break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::clearEnsembleParametersHashes()
{
    for ( auto sumCase : allSummaryCases() )
    {
        auto crp = sumCase->caseRealizationParameters();
        if ( crp ) crp->clearParametersHash();
    }
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
    try
    {
        QString                errors;
        std::hash<std::string> paramsHasher;
        size_t                 paramsHash = 0;

        for ( RimSummaryCase* rimCase : cases )
        {
            if ( rimCase->caseRealizationParameters() == nullptr ||
                 rimCase->caseRealizationParameters()->parameters().empty() )
            {
                errors.append( QString( "The case %1 has no ensemble parameters\n" )
                                   .arg( QFileInfo( rimCase->summaryHeaderFilename() ).fileName() ) );
            }
            else
            {
                QString paramNames;
                for ( std::pair<QString, RigCaseRealizationParameters::Value> paramPair :
                      rimCase->caseRealizationParameters()->parameters() )
                {
                    paramNames.append( paramPair.first );
                }

                size_t currHash = paramsHasher( paramNames.toStdString() );
                if ( paramsHash == 0 )
                {
                    paramsHash = currHash;
                }
                else if ( paramsHash != currHash )
                {
                    throw QString( "Ensemble parameters differ between cases" );
                }
            }
        }

        if ( !errors.isEmpty() )
        {
            QString textToDisplay = errors.left( 500 );

            textToDisplay.prepend( "Missing ensemble parameters\n\n" );

            textToDisplay.append( "\n" );
            textToDisplay.append( "No parameters file (parameters.txt or runspecification.xml) was found in \n" );
            textToDisplay.append( "the searched folders. ResInsight searches the home folder of the summary \n" );
            textToDisplay.append( "case file and the three folder levels above that.\n" );

            throw textToDisplay;
        }
        return true;
    }
    catch ( QString errorMessage )
    {
        QMessageBox mbox;
        mbox.setIcon( QMessageBox::Icon::Warning );
        mbox.setText( errorMessage );
        mbox.exec();
        return false;
    }
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
RiaEclipseUnitTools::UnitSystem RimSummaryCaseCollection::unitSystem() const
{
    if ( m_cases.empty() )
    {
        return RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN;
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
    if ( m_isEnsemble ) calculateEnsembleParametersIntersectionHash();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::updateReferringCurveSets()
{
    // Update curve set referring to this group
    std::vector<caf::PdmObject*> referringObjects;
    objectsWithReferringPtrFieldsOfType( referringObjects );

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
QString RimSummaryCaseCollection::nameAndItemCount() const
{
    size_t itemCount = m_cases.size();
    if ( itemCount > 20 )
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
        setUiIconFromResourceString( ":/SummaryEnsemble16x16.png" );
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
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseCollection::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                 const QVariant&            oldValue,
                                                 const QVariant&            newValue )
{
    if ( changedField == &m_isEnsemble )
    {
        updateIcon();
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
void RimSummaryCaseCollection::setNameAsReadOnly()
{
    m_name.uiCapability()->setUiReadOnly( true );
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
