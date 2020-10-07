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

#include "RiaLogging.h"
#include "RiaStdStringTools.h"

#include "cafAppEnum.h"

#include <cmath>

namespace caf
{
template <>
void caf::AppEnum<ObjectiveFunction::FunctionType>::setUp()
{
    addItem( ObjectiveFunction::FunctionType::M1, "M1", "M1" );
    addItem( ObjectiveFunction::FunctionType::M2, "M2", "M2" );
    setDefault( ObjectiveFunction::FunctionType::M1 );
}

} // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectiveFunction::setRange( size_t startIndex, size_t endIndex )
{
    m_startIndex = startIndex;
    m_endIndex   = endIndex;
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ObjectiveFunction::ObjectiveFunction( const RimSummaryCaseCollection* summaryCaseCollection, FunctionType type )
{
    m_summaryCaseCollection = summaryCaseCollection;
    functionType            = type;
    m_startIndex            = 0;
    m_endIndex              = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double ObjectiveFunction::value( size_t caseIndex, const RifEclipseSummaryAddress& vectorSummaryAddress, bool* hasWarning ) const
{
    auto summaryCases = m_summaryCaseCollection->allSummaryCases();

    if ( caseIndex < summaryCases.size() )
    {
        return value( summaryCases[caseIndex], vectorSummaryAddress, hasWarning );
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double ObjectiveFunction::value( RimSummaryCase*                 summaryCase,
                                 const RifEclipseSummaryAddress& vectorSummaryAddress,
                                 bool*                           hasWarning ) const
{
    RifSummaryReaderInterface* readerInterface = summaryCase->summaryReader();
    if ( readerInterface )
    {
        if ( functionType == FunctionType::M1 )
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
                        double N          = static_cast<double>( values.size() );
                        size_t startIndex = m_startIndex;
                        size_t endIndex   = m_endIndex;
                        if ( m_startIndex < m_endIndex )
                        {
                            N = static_cast<double>( m_endIndex - m_startIndex );
                        }
                        else
                        {
                            startIndex = 0;
                            endIndex   = values.size();
                        }
                        if ( N > 1 )
                        {
                            double sumValues        = 0.0;
                            double sumValuesSquared = 0.0;
                            for ( size_t index = startIndex; index < endIndex; index++ )
                            {
                                const double& value = values[index];
                                sumValues += value;
                                sumValuesSquared += value * value;
                            }
                            if ( sumValues != 0 )
                            {
                                return sumValues / std::sqrt( ( N * sumValuesSquared - sumValues * sumValues ) /
                                                              ( N * ( N - 1.0 ) ) );
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
        else if ( functionType == FunctionType::M2 )
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
                        size_t startIndex = m_startIndex;
                        size_t endIndex   = m_endIndex;
                        if ( m_startIndex == m_endIndex )
                        {
                            return values[m_startIndex];
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
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<double, double> ObjectiveFunction::minMaxValues( const RifEclipseSummaryAddress& vectorSummaryAddress ) const
{
    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();

    for ( auto value : values( vectorSummaryAddress ) )
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
std::pair<double, double> ObjectiveFunction::range() const
{
    return std::make_pair( m_startIndex, m_endIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> ObjectiveFunction::values( const RifEclipseSummaryAddress& vectorSummaryAddress ) const
{
    std::vector<double> values;
    auto                summaryCases = m_summaryCaseCollection->allSummaryCases();

    bool hasWarning = false;

    for ( size_t index = 0; index < summaryCases.size(); index++ )
    {
        values.push_back( value( index, vectorSummaryAddress, &hasWarning ) );
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
bool ObjectiveFunction::isValid( const RifEclipseSummaryAddress& vectorSummaryAddress ) const
{
    bool hasWarning = false;
    if ( m_summaryCaseCollection && m_summaryCaseCollection->allSummaryCases().size() > 0 &&
         m_summaryCaseCollection->allSummaryCases().front() )
    {
        value( m_summaryCaseCollection->allSummaryCases().front(), vectorSummaryAddress, &hasWarning );
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
bool ObjectiveFunction::operator<( const ObjectiveFunction& other ) const
{
    return this->name < other.name;
}
