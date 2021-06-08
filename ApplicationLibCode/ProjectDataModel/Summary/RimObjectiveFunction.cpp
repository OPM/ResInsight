/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RimObjectiveFunction.h"

#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"

#include "RifReaderEclipseSummary.h"

#include "RiaCurveMerger.h"
#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "cafAppEnum.h"

#include <climits>
#include <cmath>

namespace caf
{
template <>
void caf::AppEnum<RimObjectiveFunction::FunctionType>::setUp()
{
    addItem( RimObjectiveFunction::FunctionType::F1, "F1", "Time Range (F1)" );
    addItem( RimObjectiveFunction::FunctionType::F2, "F2", "Selected Time Steps (F2)" );
    setDefault( RimObjectiveFunction::FunctionType::F1 );
}

} // namespace caf

CAF_PDM_SOURCE_INIT( RimObjectiveFunction, "RimObjectiveFunction" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObjectiveFunction::RimObjectiveFunction()
    : changed( this )

{
    CAF_PDM_InitObject( "Objective Function", "", "", "" );

    CAF_PDM_InitFieldNoDefault( &m_functionType, "FunctionType", "Function Type", "", "", "" );

    CAF_PDM_InitField( &m_divideByNumberOfObservations,
                       "DivideByNumberOfObservations",
                       true,
                       "Nomalize by Number of Observations",
                       "",
                       "",
                       "" );

    CAF_PDM_InitField( &m_errorEstimatePercentage, "ErrorEstimatePercentage", 100.0, "Error Estimate [0..100 %]", "", "", "" );

    CAF_PDM_InitField( &m_useSquaredError, "UseSquaredError", true, "Use Squared Error Term", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObjectiveFunction::setFunctionType( FunctionType functionType )
{
    m_functionType = functionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimObjectiveFunction::shortName() const
{
    return caf::AppEnum<FunctionType>::text( m_functionType() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObjectiveFunction::FunctionType RimObjectiveFunction::functionType() const
{
    return m_functionType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimObjectiveFunction::value( RimSummaryCase*                              summaryCase,
                                    const std::vector<RifEclipseSummaryAddress>& vectorSummaryAddresses,
                                    const ObjectiveFunctionTimeConfig&           timeConfig,
                                    bool*                                        hasWarning ) const
{
    RifSummaryReaderInterface* readerInterface = summaryCase->summaryReader();
    if ( readerInterface )
    {
        double aggregatedObjectiveFunctionValue = 0.0;

        for ( auto vectorSummaryAddress : vectorSummaryAddresses )
        {
            std::string s = vectorSummaryAddress.quantityName() + RifReaderEclipseSummary::differenceIdentifier();
            if ( !vectorSummaryAddress.quantityName().empty() )
            {
                if ( vectorSummaryAddress.quantityName().find( RifReaderEclipseSummary::differenceIdentifier() ) !=
                     std::string::npos )
                {
                    s = vectorSummaryAddress.quantityName();
                }
                RifEclipseSummaryAddress vectorSummaryAddressDiff = vectorSummaryAddress;
                vectorSummaryAddressDiff.setQuantityName( s );

                RifEclipseSummaryAddress vectorSummaryAddressHistory = vectorSummaryAddress;
                vectorSummaryAddressDiff.setQuantityName( vectorSummaryAddress.quantityName() +
                                                          RifReaderEclipseSummary::differenceIdentifier() );

                if ( readerInterface->allResultAddresses().count( vectorSummaryAddressDiff ) )
                {
                    const std::vector<time_t>& allTimeSteps    = readerInterface->timeSteps( vectorSummaryAddressDiff );
                    std::vector<size_t> timeStepsForEvaluation = timeStepIndicesForEvaluation( allTimeSteps, timeConfig );

                    std::vector<double> summaryDiffValues;
                    std::vector<double> summaryHistoryValues;

                    if ( readerInterface->values( vectorSummaryAddressDiff, &summaryDiffValues ) &&
                         readerInterface->values( vectorSummaryAddressHistory, &summaryHistoryValues ) )
                    {
                        const double functionValue =
                            computeFunctionValue( summaryDiffValues, summaryHistoryValues, timeStepsForEvaluation );

                        if ( functionValue != std::numeric_limits<double>::infinity() )
                        {
                            aggregatedObjectiveFunctionValue += functionValue;
                        }
                    }
                }
                else
                {
                    RiaLogging::info( "The selected summary address does not have a related difference address." );
                    if ( hasWarning )
                    {
                        *hasWarning = true;
                    }
                }
            }
            else
            {
                RiaLogging::info( "Invalid summary address." );
                if ( hasWarning )
                {
                    *hasWarning = true;
                }
            }

            return aggregatedObjectiveFunctionValue;
        }
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double>
    RimObjectiveFunction::minMaxValues( const std::vector<RimSummaryCase*>&          summaryCases,
                                        const std::vector<RifEclipseSummaryAddress>& vectorSummaryAddresses,
                                        const ObjectiveFunctionTimeConfig&           timeConfig ) const
{
    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();

    for ( auto sumCase : summaryCases )
    {
        auto objValue = value( sumCase, vectorSummaryAddresses, timeConfig );
        {
            if ( objValue != std::numeric_limits<double>::infinity() )
            {
                if ( objValue < minValue ) minValue = objValue;
                if ( objValue > maxValue ) maxValue = objValue;
            }
        }
    }
    return std::make_pair( minValue, maxValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimObjectiveFunction::formulaString( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses )
{
    QString formula;

    for ( RifEclipseSummaryAddress address : vectorSummaryAddresses )
    {
        QString text = QString::fromStdString( address.uiText() );
        formula += text + "\n";
    }

    /*
        if ( m_functionType == FunctionType::F1 )
        {
            if ( m_divideByNumberOfObservations ) formula += "1/N * ";

            formula += "(" + QString::fromWCharArray( L"\u03A3" ) + "(|";
            QStringList addresses;
            for ( RifEclipseSummaryAddress address : vectorSummaryAddresses )
            {
                QString text = QString::fromStdString( address.uiText() +
       RifReaderEclipseSummary::differenceIdentifier() ); addresses << text;
            }
            formula += addresses.join( "| + |" );

            QString nominatorText;
            if ( !vectorSummaryAddresses.empty() )
            {
                QString text = QString::fromStdString( vectorSummaryAddresses.front().uiText() +
                                                       RifReaderEclipseSummary::historyIdentifier() );

                nominatorText = QString::fromWCharArray( L"\u03B5" ) + " * " + text;
            }

            formula += QString( "|))/(%1)" ).arg( nominatorText );
        }
        else if ( m_functionType == FunctionType::F2 )
        {
            formula += QString::fromWCharArray( L"\u03A3" ) + "(|";
            QStringList addresses;
            for ( RifEclipseSummaryAddress address : vectorSummaryAddresses )
            {
                addresses << QString::fromStdString( address.uiText() );
            }
            formula += addresses.join( "| + |" );
            formula += "|)";
        }
    */
    return formula;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimObjectiveFunction::operator<( const RimObjectiveFunction& other ) const
{
    return this->shortName() < other.shortName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObjectiveFunction::hideFunctionSelection()
{
    m_functionType.uiCapability()->setUiHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObjectiveFunction::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_functionType );
    uiOrdering.add( &m_divideByNumberOfObservations );
    uiOrdering.add( &m_errorEstimatePercentage );
    uiOrdering.add( &m_useSquaredError );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObjectiveFunction::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                             const QVariant&            oldValue,
                                             const QVariant&            newValue )
{
    changed.send();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimObjectiveFunction::errorEstimate() const
{
    return std::clamp( ( m_errorEstimatePercentage / 100.0 ), 0.0, 1.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<size_t> RimObjectiveFunction::timeStepIndicesForEvaluation( const std::vector<time_t>&         allTimeSteps,
                                                                        const ObjectiveFunctionTimeConfig& timeConfig ) const
{
    std::vector<size_t> timeStepIndices;
    if ( functionType() == FunctionType::F1 )
    {
        for ( size_t i = 0; i < allTimeSteps.size(); i++ )
        {
            const auto& t = allTimeSteps[i];
            if ( t >= timeConfig.m_startTimeStep && t <= timeConfig.m_endTimeStep )
            {
                timeStepIndices.push_back( i );
            }
        }
    }
    else if ( functionType() == FunctionType::F2 )
    {
        for ( const auto& t : timeConfig.m_timeSteps )
        {
            for ( size_t i = 0; i < allTimeSteps.size(); i++ )
            {
                const auto& candidateTime = allTimeSteps[i];
                if ( t == candidateTime )
                {
                    timeStepIndices.push_back( i );
                }
            }
        }
    }

    return timeStepIndices;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimObjectiveFunction::computeFunctionValue( const std::vector<double>& summaryDiffValues,
                                                   const std::vector<double>& summaryHistoryValues,
                                                   const std::vector<size_t>& evaluationIndices ) const
{
    if ( summaryHistoryValues.size() != summaryDiffValues.size() ) return std::numeric_limits<double>::infinity();

    const double epsilonErrorEstimate = errorEstimate();

    double sumValues        = 0.0;
    double sumValuesSquared = 0.0;
    size_t valueCount       = 0;

    double averageHistoryValue = 0.0;
    if ( !summaryHistoryValues.empty() )
    {
        for ( auto val : summaryHistoryValues )
        {
            averageHistoryValue += val;
        }

        averageHistoryValue /= summaryHistoryValues.size();
    }

    valueCount = summaryDiffValues.size();

    //
    //         1       ( |ti - tHi|  ) n
    // value = - * SUM ( ----------  )
    //         N       (  eps * tHi  )
    //
    // N   : observation count
    // ti  : simulated value at time step i
    // Hti : observed (history) value at time step i
    // eps : error estimate (0..1)
    // n   : 2 - squared error term
    //
    // https://github.com/OPM/ResInsight/issues/7761
    //
    //
    //

    const double epsilon = 1e-67;

    for ( size_t timeStepIndex : evaluationIndices )
    {
        const double diffValue = std::abs( summaryDiffValues[timeStepIndex] );

        double historyValue = summaryHistoryValues[timeStepIndex];
        double nominator    = std::abs( epsilonErrorEstimate * historyValue );
        if ( nominator < epsilon )
        {
            if ( averageHistoryValue > epsilon )
                nominator = averageHistoryValue;
            else if ( diffValue > epsilon )
                nominator = diffValue;
            else
                nominator = 1.0;
        }

        const double normalizedDiff = diffValue / nominator;

        sumValues += std::abs( normalizedDiff );
        sumValuesSquared += normalizedDiff * normalizedDiff;
    }

    if ( valueCount > 0 )
    {
        double functionValue = 0.0;
        if ( m_useSquaredError )
            functionValue = sumValuesSquared;
        else
            functionValue = sumValues;

        if ( m_divideByNumberOfObservations ) functionValue /= valueCount;

        return functionValue;
    }

    return std::numeric_limits<double>::infinity();
}
