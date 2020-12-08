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

#include <cmath>
#include <limits.h>

namespace caf
{
template <>
void caf::AppEnum<RimObjectiveFunction::FunctionType>::setUp()
{
    addItem( RimObjectiveFunction::FunctionType::M1, "M1", "M1" );
    addItem( RimObjectiveFunction::FunctionType::M2, "M2", "M2" );
    setDefault( RimObjectiveFunction::FunctionType::M1 );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimObjectiveFunction::uiName() const
{
    if ( m_functionType == FunctionType::M1 )
    {
        return QString( "M1" );
    }
    else if ( m_functionType == FunctionType::M2 )
    {
        return QString( "M2" );
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObjectiveFunction::FunctionType RimObjectiveFunction::functionType()
{
    return m_functionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObjectiveFunction::setTimeStepRange( time_t startTimeStep, time_t endTimeStep )
{
    m_startTimeStep = startTimeStep;
    m_endTimeStep   = endTimeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimObjectiveFunction::setTimeStepList( std::vector<time_t> timeSteps )
{
    m_timeSteps = timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimObjectiveFunction::RimObjectiveFunction( const RimSummaryCaseCollection* summaryCaseCollection, FunctionType type )
{
    m_summaryCaseCollection = summaryCaseCollection;
    m_functionType          = type;
    m_startTimeStep         = 0;
    m_endTimeStep           = INT_MAX;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimObjectiveFunction::value( size_t                                caseIndex,
                                    std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses,
                                    bool*                                 hasWarning ) const
{
    auto summaryCases = m_summaryCaseCollection->allSummaryCases();

    if ( caseIndex < summaryCases.size() )
    {
        return value( summaryCases[caseIndex], vectorSummaryAddresses, hasWarning );
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimObjectiveFunction::value( RimSummaryCase*                       summaryCase,
                                    std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses,
                                    bool*                                 hasWarning ) const
{
    RifSummaryReaderInterface* readerInterface = summaryCase->summaryReader();
    if ( readerInterface )
    {
        if ( m_functionType == FunctionType::M1 )
        {
            double sumValues        = 0.0;
            double sumValuesSquared = 0.0;
            double N                = 0.0;
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

                    if ( readerInterface->allResultAddresses().count( vectorSummaryAddressDiff ) )
                    {
                        std::vector<double> values;
                        if ( readerInterface->values( vectorSummaryAddressDiff, &values ) )
                        {
                            const std::vector<time_t>& timeSteps = readerInterface->timeSteps( vectorSummaryAddressDiff );

                            size_t index = 0;

                            N += static_cast<double>( values.size() );
                            if ( values.size() > 1 )
                            {
                                for ( time_t t : timeSteps )
                                {
                                    if ( t >= m_startTimeStep && t <= m_endTimeStep )
                                    {
                                        const double& value = values[index];
                                        sumValues += std::abs( value );
                                        sumValuesSquared += value * value;
                                    }
                                    index++;
                                }
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
                if ( sumValues != 0 )
                {
                    return sumValues / std::sqrt( ( N * sumValuesSquared - sumValues * sumValues ) / ( N * ( N - 1.0 ) ) );
                }
            }
        }
        else if ( m_functionType == FunctionType::M2 )
        {
            double value = 0;
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

                    if ( readerInterface->allResultAddresses().count( vectorSummaryAddressDiff ) )
                    {
                        std::vector<double> values;
                        if ( readerInterface->values( vectorSummaryAddressDiff, &values ) )
                        {
                            const std::vector<time_t>& timeSteps = readerInterface->timeSteps( vectorSummaryAddressDiff );

                            size_t index = 0;

                            std::vector<time_t> xValues( 2, 0 );
                            std::vector<double> yValues( 2, 0.0 );
                            for ( time_t t : timeSteps )
                            {
                                if ( t >= m_startTimeStep && t <= m_endTimeStep )
                                {
                                    if ( xValues.front() == 0 )
                                    {
                                        xValues[0] = t;
                                        yValues[0] = values[index];
                                    }
                                    else if ( xValues.back() == 0 )
                                    {
                                        xValues[1] = t;
                                        yValues[1] = values[index];
                                    }
                                    else
                                    {
                                        xValues[0] = xValues[1];
                                        xValues[1] = t;
                                        yValues[0] = yValues[1];
                                        yValues[1] = values[index];
                                    }
                                    if ( xValues.back() != 0 )
                                    {
                                        for ( time_t timeStep : m_timeSteps )
                                        {
                                            if ( xValues[0] <= timeStep && xValues[1] >= timeStep )
                                            {
                                                double interpValue = std::abs(
                                                    RiaCurveMerger<time_t>::interpolatedYValue( timeStep, xValues, yValues ) );
                                                if ( interpValue != HUGE_VAL )
                                                {
                                                    value += interpValue;
                                                }
                                            }
                                        }
                                    }
                                    index++;
                                }
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
            }
            return value;
        }
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double>
    RimObjectiveFunction::minMaxValues( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses ) const
{
    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();

    for ( auto value : values( vectorSummaryAddresses ) )
    {
        if ( value != std::numeric_limits<double>::infinity() )
        {
            if ( value < minValue ) minValue = value;
            if ( value > maxValue ) maxValue = value;
        }
    }
    return std::make_pair( minValue, maxValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<time_t, time_t> RimObjectiveFunction::range() const
{
    return std::make_pair( m_startTimeStep, m_endTimeStep );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> RimObjectiveFunction::values( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses ) const
{
    std::vector<double> values;
    auto                summaryCases = m_summaryCaseCollection->allSummaryCases();

    bool hasWarning = false;

    for ( size_t index = 0; index < summaryCases.size(); index++ )
    {
        values.push_back( value( index, vectorSummaryAddresses, &hasWarning ) );
        if ( hasWarning )
        {
            return std::vector<double>();
        }
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimObjectiveFunction::isValid( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses ) const
{
    bool hasWarning = false;
    if ( m_summaryCaseCollection && m_summaryCaseCollection->allSummaryCases().size() > 0 &&
         m_summaryCaseCollection->allSummaryCases().front() )
    {
        value( m_summaryCaseCollection->allSummaryCases().front(), vectorSummaryAddresses, &hasWarning );
        if ( hasWarning )
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimObjectiveFunction::formulaString( std::vector<RifEclipseSummaryAddress> vectorSummaryAddresses )
{
    QString formula;
    if ( m_functionType == FunctionType::M1 )
    {
        formula += "(" + QString::fromWCharArray( L"\u03A3" ) + "(|";
        QStringList addresses;
        for ( RifEclipseSummaryAddress address : vectorSummaryAddresses )
        {
            addresses << QString::fromStdString( address.uiText() );
        }
        formula += addresses.join( "| + |" );
        formula += "|))/(stdv)";
    }
    else if ( m_functionType == FunctionType::M2 )
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
    return formula;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimObjectiveFunction::operator<( const RimObjectiveFunction& other ) const
{
    return this->uiName() < other.uiName();
}
