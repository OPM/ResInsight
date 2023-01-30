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
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

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
std::optional<std::vector<RimSummaryCalculationVariable*>> RimSummaryCalculation::getVariables() const
{
    std::vector<RimSummaryCalculationVariable*> variables;

    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = dynamic_cast<RimSummaryCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        if ( !v->summaryCase() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No summary case defined for variable : %1" ).arg( v->name() ) );
            return {};
        }

        if ( !v->summaryAddress() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No summary address defined for variable : %1" ).arg( v->name() ) );
            return {};
        }

        variables.push_back( v );
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

    RimSummaryCalculationVariable* v = variables.value()[0];

    auto well = v->summaryAddress()->address().wellName();
    auto addr = RifEclipseSummaryAddress::calculatedWellAddress( description().toStdString(), well, m_id );

    // Clear existing values
    m_cachedResults.erase( addr );
    m_cachedTimesteps.erase( addr );

    auto result = calculateResult( m_expression, variables.value() );
    if ( result )
    {
        auto [validValues, validTimeSteps] = result.value();
        m_cachedResults[addr]              = validValues;
        m_cachedTimesteps[addr]            = validTimeSteps;

        m_isDirty = false;
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::substituteVariables( std::vector<RimSummaryCalculationVariable*>& vars,
                                                 const RifEclipseSummaryAddress&              address )
{
    auto firstVariable = vars.front();

    QVariant oldValue;
    QVariant newValue;
    if ( firstVariable->summaryAddress()->address().category() == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        oldValue = QString::fromStdString( firstVariable->summaryAddress()->address().wellName() );
        newValue = QString::fromStdString( address.wellName() );
    }
    else if ( firstVariable->summaryAddress()->address().category() == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        oldValue = firstVariable->summaryAddress()->address().regionNumber();
        newValue = address.regionNumber();
    }
    else if ( firstVariable->summaryAddress()->address().category() == RifEclipseSummaryAddress::SUMMARY_GROUP )
    {
        oldValue = QString::fromStdString( firstVariable->summaryAddress()->address().groupName() );
        newValue = QString::fromStdString( address.groupName() );
    }

    for ( auto v : vars )
    {
        if ( v->summaryAddress()->address().category() == address.category() )
        {
            std::string oldVectorName = v->summaryAddress()->address().uiText();

            auto copyOfAddress = v->summaryAddress()->address();
            RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, address.category(), &copyOfAddress );

            RimSummaryAddress summaryAddress;
            summaryAddress.setAddress( copyOfAddress );
            v->setSummaryAddress( summaryAddress );
            std::string newVectorName = v->summaryAddress()->address().uiText();
            RiaLogging::info(
                QString( "Substitution: %1 ==> %2" ).arg( oldVectorName.c_str() ).arg( newVectorName.c_str() ) );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<std::vector<double>, std::vector<time_t>>>
    RimSummaryCalculation::calculateWithSubstitutions( const RifEclipseSummaryAddress& addr )
{
    auto variables = getVariables();
    if ( !variables ) return {};

    auto vars = variables.value();
    substituteVariables( vars, addr );

    return calculateResult( m_expression, vars );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<std::pair<std::vector<double>, std::vector<time_t>>>
    RimSummaryCalculation::calculateResult( const QString&                                     expression,
                                            const std::vector<RimSummaryCalculationVariable*>& variables )

{
    QString leftHandSideVariableName = RimSummaryCalculation::findLeftHandSide( expression );

    RiaTimeHistoryCurveMerger timeHistoryCurveMerger;

    for ( size_t i = 0; i < variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = variables[i];
        CAF_ASSERT( v != nullptr );

        RiaSummaryCurveDefinition curveDef( v->summaryCase(), v->summaryAddress()->address(), false );

        std::vector<double> curveValues;
        RiaSummaryCurveDefinition::resultValues( curveDef, &curveValues );

        std::vector<time_t> curveTimeSteps = RiaSummaryCurveDefinition::timeSteps( curveDef );

        if ( !curveTimeSteps.empty() && !curveValues.empty() )
        {
            timeHistoryCurveMerger.addCurveData( curveTimeSteps, curveValues );
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
std::vector<RimUserDefinedCalculationAddress*> RimSummaryCalculation::allAddresses() const
{
    std::vector<RimUserDefinedCalculationAddress*> addresses;

    auto variables = getVariables();
    if ( variables && !variables.value().empty() )
    {
        // The first variable is the substituable one. Use its category to
        // provide all available addresses.

        auto firstVariable      = variables.value().front();
        auto allResultAddresses = firstVariable->summaryCase()->summaryReader()->allResultAddresses();

        if ( firstVariable->summaryAddress()->address().category() == RifEclipseSummaryAddress::SUMMARY_WELL )
        {
            auto wells = RimProject::current()->simulationWellNames();

            for ( auto well : wells )
            {
                addresses.push_back( new RimSummaryCalculationAddress(
                    RifEclipseSummaryAddress::calculatedWellAddress( description().toStdString(), well.toStdString(), m_id ) ) );
            }
        }
        else if ( firstVariable->summaryAddress()->address().category() == RifEclipseSummaryAddress::SUMMARY_GROUP )
        {
            std::set<std::string> uniqueGroupNames;
            std::for_each( allResultAddresses.begin(), allResultAddresses.end(), [&]( const auto& addr ) {
                uniqueGroupNames.insert( addr.groupName() );
            } );

            for ( auto groupName : uniqueGroupNames )
            {
                addresses.push_back( new RimSummaryCalculationAddress(
                    RifEclipseSummaryAddress::calculatedGroupAddress( description().toStdString(), groupName, m_id ) ) );
            }
        }
        else if ( firstVariable->summaryAddress()->address().category() == RifEclipseSummaryAddress::SUMMARY_REGION )
        {
            std::set<int> uniqueRegionNumbers;
            std::for_each( allResultAddresses.begin(), allResultAddresses.end(), [&]( const auto& addr ) {
                uniqueRegionNumbers.insert( addr.regionNumber() );
            } );

            for ( auto regionNumber : uniqueRegionNumbers )
            {
                addresses.push_back( new RimSummaryCalculationAddress(
                    RifEclipseSummaryAddress::calculatedRegionAddress( description().toStdString(), regionNumber, m_id ) ) );
            }
        }
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCalculation::values( const RimUserDefinedCalculationAddress& addr )
{
    const RimSummaryCalculationAddress* address = dynamic_cast<const RimSummaryCalculationAddress*>( &addr );
    if ( !address ) return {};

    if ( auto it = m_cachedResults.find( address->address() ); it != m_cachedResults.end() )
    {
        return it->second;
    }
    else
    {
        auto result = calculateWithSubstitutions( address->address() );
        if ( result )
        {
            auto [validValues, validTimeSteps]    = result.value();
            m_cachedResults[address->address()]   = validValues;
            m_cachedTimesteps[address->address()] = validTimeSteps;
            return validValues;
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimSummaryCalculation::timeSteps( const RimUserDefinedCalculationAddress& addr )
{
    static std::vector<time_t> dummy;

    const RimSummaryCalculationAddress* address = dynamic_cast<const RimSummaryCalculationAddress*>( &addr );
    if ( !address ) return dummy;

    if ( auto it = m_cachedTimesteps.find( address->address() ); it != m_cachedTimesteps.end() )
    {
        return it->second;
    }

    return dummy;
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
