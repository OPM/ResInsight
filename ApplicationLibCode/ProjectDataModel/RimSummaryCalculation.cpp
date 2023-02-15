/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimSummaryCalculation.h"

#include "RifEclipseSummaryAddress.h"

#include "RiaCurveMerger.h"
#include "RiaLogging.h"
#include "RiaSummaryCurveDefinition.h"
#include "RiaSummaryTools.h"

#include "RifSummaryReaderInterface.h"
#include "RimDataSourceSteppingTools.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"
#include "RimCalculatedSummaryCase.h"

#include "RiuExpressionContextMenuManager.h"

#include "cafPdmUiLineEditor.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTextEditor.h"

#include "expressionparser/ExpressionParser.h"

#include <algorithm>

CAF_PDM_SOURCE_INIT( RimSummaryCalculation, "RimSummaryCalculation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation::RimSummaryCalculation()
{
    CAF_PDM_InitObject( "RimSummaryCalculation", ":/octave.png", "Calculation", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationVariable* RimSummaryCalculation::createVariable()
{
    return new RimSummaryCalculationVariable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::vector<RimSummaryCalculationVariable*>> RimSummaryCalculation::getVariables( bool showError ) const
{
    std::vector<RimSummaryCalculationVariable*> variables;

    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = dynamic_cast<RimSummaryCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        if ( !v->summaryCase() )
        {
            if ( showError )
            {
                RiaLogging::errorInMessageBox( nullptr,
                                               "Expression Parser",
                                               QString( "No summary case defined for variable : %1" ).arg( v->name() ) );
            }
            return {};
        }

        if ( !v->summaryAddress() )
        {
            if ( showError )
            {
                RiaLogging::errorInMessageBox( nullptr,
                                               "Expression Parser",
                                               QString( "No summary address defined for variable : %1" ).arg( v->name() ) );
            }
            return {};
        }

        variables.push_back( v->clone() );
    }

    return variables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::calculate()
{
    auto variables = getVariables();
    if ( !variables ) return false;

    // Not much to do for calculate: values and timesteps are generate when needed later.
    m_isDirty = false;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::substituteVariables( std::vector<RimSummaryCalculationVariable*>& vars,
                                                 const RifEclipseSummaryAddress&              address )
{
    auto firstVariable = vars.front();

    auto category = firstVariable->summaryAddress()->address().category();

    QVariant oldValue;
    QVariant newValue;
    bool     isHandledBySteppingTools = false;
    if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        oldValue                 = QString::fromStdString( firstVariable->summaryAddress()->address().wellName() );
        newValue                 = QString::fromStdString( address.wellName() );
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        oldValue                 = firstVariable->summaryAddress()->address().regionNumber();
        newValue                 = address.regionNumber();
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_GROUP )
    {
        oldValue                 = QString::fromStdString( firstVariable->summaryAddress()->address().groupName() );
        newValue                 = QString::fromStdString( address.groupName() );
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        oldValue                 = firstVariable->summaryAddress()->address().aquiferNumber();
        newValue                 = address.aquiferNumber();
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION ||
              category == RifEclipseSummaryAddress::SUMMARY_BLOCK )
    {
        oldValue                 = QString::fromStdString( firstVariable->summaryAddress()->address().blockAsString() );
        newValue                 = QString::fromStdString( address.blockAsString() );
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_MISC || category == RifEclipseSummaryAddress::SUMMARY_FIELD )
    {
        // No need to do anything for these types
        return;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION )
    {
        oldValue = QString::fromStdString( firstVariable->summaryAddress()->address().formatUiTextRegionToRegion() );
        newValue = QString::fromStdString( address.formatUiTextRegionToRegion() );
        isHandledBySteppingTools = true;
    }
    else
    {
        RiaLogging::error( QString( "Unhandled subst for category: %1" ).arg( address.uiText().c_str() ) );
    }

    if ( isHandledBySteppingTools )
    {
        for ( auto v : vars )
        {
            if ( v->summaryAddress()->address().category() == address.category() )
            {
                std::string oldVectorName = v->summaryAddress()->address().uiText();

                auto copyOfAddress = v->summaryAddress()->address();
                RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, address.category(), &copyOfAddress );

                RimSummaryAddress summaryAddress;
                summaryAddress.setAddress( copyOfAddress );
                if ( v->summaryCase() )
                {
                    summaryAddress.setCaseId( v->summaryCase()->caseId() );
                }
                
                v->setSummaryAddress( summaryAddress );
                std::string newVectorName = v->summaryAddress()->address().uiText();
                RiaLogging::info(
                    QString( "Substitution: %1 ==> %2" ).arg( oldVectorName.c_str() ).arg( newVectorName.c_str() ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<std::vector<double>, std::vector<time_t>>>
    RimSummaryCalculation::calculateWithSubstitutions( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& addr )
{
    auto variables = getVariables();
    if ( !variables ) return {};

    auto vars = variables.value();
    substituteVariables( vars, addr );

    return calculateResult( m_expression, vars, summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<std::vector<double>, std::vector<time_t>>>
    RimSummaryCalculation::calculateResult( const QString&                                     expression,
                                            const std::vector<RimSummaryCalculationVariable*>& variables,
                                            RimSummaryCase*                                    summaryCase )

{
    QString leftHandSideVariableName = RimSummaryCalculation::findLeftHandSide( expression );

    RiaTimeHistoryCurveMerger timeHistoryCurveMerger;

    for ( size_t i = 0; i < variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = variables[i];
        CAF_ASSERT( v != nullptr );

        RimSummaryCase* caseForVariable = summaryCase;
        if ( v->summaryAddress()->address().isCalculated() )
        {
            if ( !v->summaryCase() )
            {
                RiaLogging::error( QString( "Missing summary case: %1 when generating: %2" )
                                       .arg( QString::fromStdString( v->summaryAddress()->address().uiText() ) ).arg(leftHandSideVariableName) );
                RimSummaryCalculationCollection* calcColl       = RimProject::current()->calculationCollection();
                RimCalculatedSummaryCase*        calculatedCase = calcColl->calculationSummaryCase( summaryCase );
                caseForVariable                                 = calculatedCase;
            }
            else
            {
                RiaLogging::debug( QString( "Calculated address. case: '%1' " ).arg( v->summaryCase()->displayCaseName() ) );
                caseForVariable = v->summaryCase();
            }
            
        }

        RiaLogging::debug( QString( "Using case: '%1'" ).arg(caseForVariable->displayCaseName()) );
        // v->summaryCase()
        RiaSummaryCurveDefinition curveDef( caseForVariable, v->summaryAddress()->address(), false );

        std::vector<double> curveValues;
        RiaSummaryCurveDefinition::resultValues( curveDef, &curveValues );

        std::vector<time_t> curveTimeSteps = RiaSummaryCurveDefinition::timeSteps( curveDef );

        if ( !curveTimeSteps.empty() && !curveValues.empty() )
        {
            timeHistoryCurveMerger.addCurveData( curveTimeSteps, curveValues );
        }
        else
        {
            // One variable is missing: not possible to complete the calculation.
            // Can happen when stepping and substituting variables.
            return {};
        }
    }

    timeHistoryCurveMerger.computeInterpolatedValues();

    ExpressionParser parser;
    for ( size_t i = 0; i < variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = variables[i];
        CAF_ASSERT( v != nullptr );

        parser.assignVector( v->name(), timeHistoryCurveMerger.interpolatedYValuesForAllXValues( i ) );
    }

    std::vector<double> resultValues;
    resultValues.resize( timeHistoryCurveMerger.allXValues().size() );

    parser.assignVector( leftHandSideVariableName, resultValues );

    QString errorText;
    bool    evaluatedOk = parser.expandIfStatementsAndEvaluate( expression, &errorText );

    if ( evaluatedOk )
    {
        if ( timeHistoryCurveMerger.validIntervalsForAllXValues().size() > 0 )
        {
            size_t firstValidTimeStep = timeHistoryCurveMerger.validIntervalsForAllXValues().front().first;
            size_t lastValidTimeStep  = timeHistoryCurveMerger.validIntervalsForAllXValues().back().second + 1;

            if ( lastValidTimeStep > firstValidTimeStep && lastValidTimeStep <= timeHistoryCurveMerger.allXValues().size() )
            {
                std::vector<time_t> validTimeSteps( timeHistoryCurveMerger.allXValues().begin() + firstValidTimeStep,
                                                    timeHistoryCurveMerger.allXValues().begin() + lastValidTimeStep );

                std::vector<double> validValues( resultValues.begin() + firstValidTimeStep,
                                                 resultValues.begin() + lastValidTimeStep );

                return std::make_pair( validValues, validTimeSteps );
            }
        }
    }
    else
    {
        QString s = "The following error message was received from the parser library : \n\n";
        s += errorText;

        RiaLogging::errorInMessageBox( nullptr, "Expression Parser", s );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::updateDependentObjects()
{
    RimSummaryCalculationCollection* calcColl = nullptr;
    this->firstAncestorOrThisOfTypeAsserted( calcColl );
    calcColl->rebuildCaseMetaData();

    // Refresh data sources tree.
    // TODO: refresh too much: would be enough to only refresh calculated resutls.
    RimSummaryCaseMainCollection* summaryCaseCollection = RiaSummaryTools::summaryCaseMainCollection();
    auto                          summaryCases          = summaryCaseCollection->allSummaryCases();
    for ( RimSummaryCase* summaryCase : summaryCases )
    {
        summaryCase->createSummaryReaderInterface();
        summaryCase->createRftReaderInterface();
        summaryCase->refreshMetaData();
    }

    RimSummaryMultiPlotCollection* summaryPlotCollection = RiaSummaryTools::summaryMultiPlotCollection();
    for ( auto multiPlot : summaryPlotCollection->multiPlots() )
    {
        for ( RimSummaryPlot* sumPlot : multiPlot->summaryPlots() )
        {
            bool plotContainsCalculatedCurves = false;

            for ( RimSummaryCurve* sumCurve : sumPlot->summaryCurves() )
            {
                if ( sumCurve->summaryAddressY().isCalculated() )
                {
                    sumCurve->updateConnectedEditors();

                    plotContainsCalculatedCurves = true;
                }
            }

            if ( plotContainsCalculatedCurves )
            {
                sumPlot->loadDataAndUpdate();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::removeDependentObjects()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCalculationAddress> RimSummaryCalculation::allAddressesForSummaryCase( RimSummaryCase* summaryCase ) const
{
    auto variables = getVariables( false );
    if ( variables && !variables.value().empty() )
    {
        // The first variable is the substituable one. Use its category to
        // provide all available addresses.
        auto firstVariable      = variables.value().front();
        auto allResultAddresses = summaryCase->summaryReader()->allResultAddresses();
        return allAddressesForCategory( firstVariable->summaryAddress()->address().category(), allResultAddresses );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCalculationAddress>
    RimSummaryCalculation::allAddressesForCategory( RifEclipseSummaryAddress::SummaryVarCategory category,
                                                    const std::set<RifEclipseSummaryAddress>& allResultAddresses ) const
{
    std::vector<RimSummaryCalculationAddress> addresses;

    if ( category == RifEclipseSummaryAddress::SUMMARY_FIELD )
    {
        addresses.push_back(
            RimSummaryCalculationAddress( RifEclipseSummaryAddress::fieldAddress( description().toStdString(), m_id ) ) );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        std::set<int> uniqueNumbers;
        std::for_each( allResultAddresses.begin(), allResultAddresses.end(), [&]( const auto& addr ) {
            uniqueNumbers.insert( addr.aquiferNumber() );
        } );

        for ( auto num : uniqueNumbers )
        {
            addresses.push_back( RimSummaryCalculationAddress(
                RifEclipseSummaryAddress::aquiferAddress( description().toStdString(), num, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_MISC )
    {
        addresses.push_back(
            RimSummaryCalculationAddress( RifEclipseSummaryAddress::miscAddress( description().toStdString(), m_id ) ) );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_NETWORK )
    {
        addresses.push_back( RimSummaryCalculationAddress(
            RifEclipseSummaryAddress::networkAddress( description().toStdString(), m_id ) ) );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        std::set<std::string> uniqueWellNames;
        std::for_each( allResultAddresses.begin(), allResultAddresses.end(), [&]( const auto& addr ) {
            uniqueWellNames.insert( addr.wellName() );
        } );

        for ( auto wellName : uniqueWellNames )
        {
            addresses.push_back( RimSummaryCalculationAddress(
                RifEclipseSummaryAddress::wellAddress( description().toStdString(), wellName, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_GROUP )
    {
        std::set<std::string> uniqueGroupNames;
        std::for_each( allResultAddresses.begin(), allResultAddresses.end(), [&]( const auto& addr ) {
            uniqueGroupNames.insert( addr.groupName() );
        } );

        for ( auto groupName : uniqueGroupNames )
        {
            addresses.push_back( RimSummaryCalculationAddress(
                RifEclipseSummaryAddress::groupAddress( description().toStdString(), groupName, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        std::set<int> uniqueRegionNumbers;
        std::for_each( allResultAddresses.begin(), allResultAddresses.end(), [&]( const auto& addr ) {
            uniqueRegionNumbers.insert( addr.regionNumber() );
        } );

        for ( auto regionNumber : uniqueRegionNumbers )
        {
            addresses.push_back( RimSummaryCalculationAddress(
                RifEclipseSummaryAddress::regionAddress( description().toStdString(), regionNumber, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION )
    {
        std::set<std::pair<int, int>> uniqueRegionNumbers;
        std::for_each( allResultAddresses.begin(), allResultAddresses.end(), [&]( const auto& addr ) {
            uniqueRegionNumbers.insert( std::make_pair( addr.regionNumber(), addr.regionNumber2() ) );
        } );

        for ( auto regionNumber : uniqueRegionNumbers )
        {
            auto [r1, r2] = regionNumber;
            addresses.push_back( RimSummaryCalculationAddress(
                RifEclipseSummaryAddress::regionToRegionAddress( description().toStdString(), r1, r2, m_id ) ) );
        }
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCalculation::values( RimSummaryCase* summaryCase, const RimSummaryCalculationAddress& address )

{
    CAF_ASSERT( summaryCase );

    auto result = calculateWithSubstitutions( summaryCase, address.address() );
    if ( result )
    {
        auto [validValues, validTimeSteps] = result.value();
        return validValues;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryCalculation::timeSteps( RimSummaryCase*                     summaryCase,
                                                      const RimSummaryCalculationAddress& address )
{
    CAF_ASSERT( summaryCase );

    auto result = calculateWithSubstitutions( summaryCase, address.address() );
    if ( result )
    {
        auto [validValues, validTimeSteps] = result.value();
        return validTimeSteps;
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCalculation::buildCalculationName() const
{
    QString name = "Default Calculation Name";

    if ( !m_expression.v().isEmpty() )
    {
        name = m_expression;
    }

    return name;
}
