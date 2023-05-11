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
#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimProject.h"
#include "RimSummaryAddress.h"
#include "RimSummaryCalculationCollection.h"
#include "RimSummaryCalculationVariable.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RiuExpressionContextMenuManager.h"

#include "cafPdmUiCheckBoxEditor.h"
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

    CAF_PDM_InitField( &m_distributeToOtherItems, "DistributeToOtherItems", true, "Distribute to other items (wells, groups, ..)" );
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
std::optional<std::vector<SummaryCalculationVariable>> RimSummaryCalculation::getVariables() const
{
    std::vector<SummaryCalculationVariable> variables;

    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = dynamic_cast<RimSummaryCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        if ( !v->summaryCase() )
        {
            return {};
        }

        if ( !v->summaryAddress() )
        {
            return {};
        }

        if ( v->summaryAddress()->address().id() == id() )
        {
            return {};
        }

        if ( v->summaryAddress()->address().isCalculated() )
        {
            std::set<int> calcIds;
            if ( detectCyclicCalculation( v->summaryAddress()->address().id(), calcIds ) )
            {
                return {};
            }
        }

        SummaryCalculationVariable variable;
        variable.name           = v->name();
        variable.summaryCase    = v->summaryCase();
        variable.summaryAddress = v->summaryAddress()->address();
        variables.push_back( variable );
    }

    return variables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::checkVariables() const
{
    for ( size_t i = 0; i < m_variables.size(); i++ )
    {
        RimSummaryCalculationVariable* v = dynamic_cast<RimSummaryCalculationVariable*>( m_variables[i] );
        CAF_ASSERT( v != nullptr );

        if ( !v->summaryCase() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No summary case defined for variable : %1" ).arg( v->name() ) );
            return false;
        }

        if ( !v->summaryAddress() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "No summary address defined for variable : %1" ).arg( v->name() ) );
            return false;
        }

        if ( v->summaryAddress()->address().id() == id() )
        {
            RiaLogging::errorInMessageBox( nullptr,
                                           "Expression Parser",
                                           QString( "Recursive calculation detected for variable : %1" ).arg( v->name() ) );
            return false;
        }

        if ( v->summaryAddress()->address().isCalculated() )
        {
            std::set<int> calcIds;
            if ( detectCyclicCalculation( v->summaryAddress()->address().id(), calcIds ) )
            {
                RiaLogging::errorInMessageBox( nullptr,
                                               "Expression Parser",
                                               QString( "Cyclic calculation detected for variable : %1" ).arg( v->name() ) );
                return false;
            }
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::detectCyclicCalculation( int id, std::set<int>& ids ) const
{
    if ( ids.count( id ) > 0 )
        return true;
    else
    {
        ids.insert( id );

        // Get calculation for the referenced id
        RimSummaryCalculationCollection* calcColl = RimProject::current()->calculationCollection();
        auto                             calc     = dynamic_cast<RimSummaryCalculation*>( calcColl->findCalculationById( id ) );

        // Check if any of the variables references already seen calculations
        auto vars = calc->variables();
        for ( size_t i = 0; i < vars->size(); i++ )
        {
            auto variable = dynamic_cast<RimSummaryCalculationVariable*>( vars->at( i ) );
            auto addr     = variable->summaryAddress()->address();

            if ( addr.id() != -1 && detectCyclicCalculation( addr.id(), ids ) ) return true;
        }

        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    RimUserDefinedCalculation::defineEditorAttribute( field, uiConfigName, attribute );

    if ( field == &m_distributeToOtherItems )
    {
        auto myAttr = dynamic_cast<caf::PdmUiCheckBoxEditorAttribute*>( attribute );
        if ( myAttr )
        {
            myAttr->setWordWrap( true );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryCalculation::calculate()
{
    auto isOk = checkVariables();
    if ( !isOk ) return false;

    // Not much to do for calculate: values and timesteps are generate when needed later.
    m_isDirty = false;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCalculation::substituteVariables( std::vector<SummaryCalculationVariable>& vars, const RifEclipseSummaryAddress& address )
{
    auto firstVariable = vars.front();

    auto category = firstVariable.summaryAddress.category();

    QVariant oldValue;
    QVariant newValue;
    bool     isHandledBySteppingTools = false;
    if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        oldValue                 = QString::fromStdString( firstVariable.summaryAddress.wellName() );
        newValue                 = QString::fromStdString( address.wellName() );
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        oldValue                 = firstVariable.summaryAddress.regionNumber();
        newValue                 = address.regionNumber();
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_GROUP )
    {
        oldValue                 = QString::fromStdString( firstVariable.summaryAddress.groupName() );
        newValue                 = QString::fromStdString( address.groupName() );
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        oldValue                 = firstVariable.summaryAddress.aquiferNumber();
        newValue                 = address.aquiferNumber();
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL_COMPLETION || category == RifEclipseSummaryAddress::SUMMARY_BLOCK )
    {
        oldValue                 = QString::fromStdString( firstVariable.summaryAddress.blockAsString() );
        newValue                 = QString::fromStdString( address.blockAsString() );
        isHandledBySteppingTools = true;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_MISC || category == RifEclipseSummaryAddress::SUMMARY_FIELD ||
              category == RifEclipseSummaryAddress::SUMMARY_IMPORTED )
    {
        // No need to do anything for these types
        return;
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION )
    {
        oldValue                 = QString::fromStdString( firstVariable.summaryAddress.formatUiTextRegionToRegion() );
        newValue                 = QString::fromStdString( address.formatUiTextRegionToRegion() );
        isHandledBySteppingTools = true;
    }
    else
    {
        RiaLogging::error( QString( "Unhandled subst for category: %1" ).arg( address.uiText().c_str() ) );
    }

    if ( isHandledBySteppingTools )
    {
        for ( auto& v : vars )
        {
            if ( v.summaryAddress.category() == address.category() )
            {
                RimDataSourceSteppingTools::updateAddressIfMatching( oldValue, newValue, address.category(), &v.summaryAddress );
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
    RimSummaryCalculation::calculateResult( const QString&                                 expression,
                                            const std::vector<SummaryCalculationVariable>& variables,
                                            RimSummaryCase*                                summaryCase )

{
    QString leftHandSideVariableName = RimSummaryCalculation::findLeftHandSide( expression );

    RiaTimeHistoryCurveMerger timeHistoryCurveMerger;

    for ( size_t i = 0; i < variables.size(); i++ )
    {
        SummaryCalculationVariable v = variables[i];

        RiaSummaryCurveDefinition curveDef( summaryCase, v.summaryAddress, false );

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
        SummaryCalculationVariable v = variables[i];
        parser.assignVector( v.name, timeHistoryCurveMerger.interpolatedYValuesForAllXValues( i ) );
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

                std::vector<double> validValues( resultValues.begin() + firstValidTimeStep, resultValues.begin() + lastValidTimeStep );

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
    RimSummaryCalculationCollection* calcColl = firstAncestorOrThisOfTypeAsserted<RimSummaryCalculationCollection>();
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

    RimObservedDataCollection* observedDataCollection = RiaSummaryTools::observedDataCollection();
    auto                       observedData           = observedDataCollection->allObservedSummaryData();
    for ( auto obs : observedData )
    {
        obs->createSummaryReaderInterface();
        obs->createRftReaderInterface();
        obs->refreshMetaData();
    }

    auto summaryCaseCollections = summaryCaseCollection->summaryCaseCollections();
    for ( RimSummaryCaseCollection* summaryCaseCollection : summaryCaseCollections )
    {
        summaryCaseCollection->refreshMetaData();
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
    auto variables = getVariables();
    if ( variables && !variables.value().empty() )
    {
        // The first variable is the substituable one. Use its category to
        // provide all available addresses.
        auto firstVariable = variables.value().front();
        if ( m_distributeToOtherItems )
        {
            auto allResultAddresses = summaryCase->summaryReader()->allResultAddresses();
            return allAddressesForCategory( firstVariable.summaryAddress.category(), allResultAddresses );
        }
        else
        {
            // Generate the result only for the first variable
            return { RimSummaryCalculationAddress( singleAddressesForCategory( firstVariable.summaryAddress ) ) };
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCalculationAddress>
    RimSummaryCalculation::allAddressesForCategory( RifEclipseSummaryAddress::SummaryVarCategory category,
                                                    const std::set<RifEclipseSummaryAddress>&    allResultAddresses ) const
{
    std::vector<RimSummaryCalculationAddress> addresses;

    std::string name = shortName().toStdString();
    if ( category == RifEclipseSummaryAddress::SUMMARY_FIELD )
    {
        addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::fieldAddress( name, m_id ) ) );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        std::set<int> uniqueNumbers;
        std::for_each( allResultAddresses.begin(),
                       allResultAddresses.end(),
                       [&]( const auto& addr ) { uniqueNumbers.insert( addr.aquiferNumber() ); } );

        for ( auto num : uniqueNumbers )
        {
            addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::aquiferAddress( name, num, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_MISC )
    {
        addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::miscAddress( name, m_id ) ) );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_NETWORK )
    {
        addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::networkAddress( name, m_id ) ) );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        std::set<std::string> uniqueWellNames;
        std::for_each( allResultAddresses.begin(),
                       allResultAddresses.end(),
                       [&]( const auto& addr ) { uniqueWellNames.insert( addr.wellName() ); } );

        for ( auto wellName : uniqueWellNames )
        {
            addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::wellAddress( name, wellName, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_GROUP )
    {
        std::set<std::string> uniqueGroupNames;
        std::for_each( allResultAddresses.begin(),
                       allResultAddresses.end(),
                       [&]( const auto& addr ) { uniqueGroupNames.insert( addr.groupName() ); } );

        for ( auto groupName : uniqueGroupNames )
        {
            addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::groupAddress( name, groupName, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        std::set<int> uniqueRegionNumbers;
        std::for_each( allResultAddresses.begin(),
                       allResultAddresses.end(),
                       [&]( const auto& addr ) { uniqueRegionNumbers.insert( addr.regionNumber() ); } );

        for ( auto regionNumber : uniqueRegionNumbers )
        {
            addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::regionAddress( name, regionNumber, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION )
    {
        std::set<std::pair<int, int>> uniqueRegionNumbers;
        std::for_each( allResultAddresses.begin(),
                       allResultAddresses.end(),
                       [&]( const auto& addr ) { uniqueRegionNumbers.insert( std::make_pair( addr.regionNumber(), addr.regionNumber2() ) ); } );

        for ( auto regionNumber : uniqueRegionNumbers )
        {
            auto [r1, r2] = regionNumber;
            addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::regionToRegionAddress( name, r1, r2, m_id ) ) );
        }
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_IMPORTED )
    {
        addresses.push_back( RimSummaryCalculationAddress( RifEclipseSummaryAddress::importedAddress( name, m_id ) ) );
    }

    return addresses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculationAddress RimSummaryCalculation::singleAddressesForCategory( const RifEclipseSummaryAddress& address ) const
{
    std::string name = shortName().toStdString();

    RifEclipseSummaryAddress::SummaryVarCategory category = address.category();
    if ( category == RifEclipseSummaryAddress::SUMMARY_FIELD )
    {
        return RifEclipseSummaryAddress::fieldAddress( name, m_id );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_AQUIFER )
    {
        return RifEclipseSummaryAddress::aquiferAddress( name, address.aquiferNumber(), m_id );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_MISC )
    {
        return RifEclipseSummaryAddress::miscAddress( name, m_id );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_NETWORK )
    {
        return RifEclipseSummaryAddress::networkAddress( name, m_id );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_WELL )
    {
        return RifEclipseSummaryAddress::wellAddress( name, address.wellName(), m_id );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_GROUP )
    {
        return RifEclipseSummaryAddress::groupAddress( name, address.groupName(), m_id );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION )
    {
        return RifEclipseSummaryAddress::regionAddress( name, address.regionNumber(), m_id );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_REGION_2_REGION )
    {
        return RifEclipseSummaryAddress::regionToRegionAddress( name, address.regionNumber(), address.regionNumber2(), m_id );
    }
    else if ( category == RifEclipseSummaryAddress::SUMMARY_IMPORTED )
    {
        return RifEclipseSummaryAddress::importedAddress( name, m_id );
    }

    return RifEclipseSummaryAddress();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimSummaryCalculation::values( RimSummaryCase* summaryCase, const RimSummaryCalculationAddress& address )

{
    CAF_ASSERT( summaryCase );

    const auto& result = calculateWithSubstitutions( summaryCase, address.address() );
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
std::vector<time_t> RimSummaryCalculation::timeSteps( RimSummaryCase* summaryCase, const RimSummaryCalculationAddress& address )
{
    CAF_ASSERT( summaryCase );

    const auto& result = calculateWithSubstitutions( summaryCase, address.address() );
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
