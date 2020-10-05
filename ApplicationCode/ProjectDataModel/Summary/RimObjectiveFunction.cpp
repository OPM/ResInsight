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

#include "RifReaderEclipseSummary.h"

#include "RiaStdStringTools.h"

#include "cafAppEnum.h"

#include <cmath>

namespace caf
{
template <>
void caf::AppEnum<ObjectiveFunction::FunctionType>::setUp()
{
    addItem( ObjectiveFunction::FunctionType::M, "M", "M" );
    setDefault( ObjectiveFunction::FunctionType::M );
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
ObjectiveFunction::ObjectiveFunction( const RimSummaryCaseCollection* summaryCaseCollection )
{
    m_summaryCaseCollection = summaryCaseCollection;
    functionType            = FunctionType::M;
    m_startIndex            = 0;
    m_endIndex              = 0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double ObjectiveFunction::value( size_t caseIndex, const RifEclipseSummaryAddress& vectorSummaryAddress ) const
{
    auto summaryCases = m_summaryCaseCollection->allSummaryCases();

    if ( caseIndex < summaryCases.size() )
    {
        return value( summaryCases[caseIndex], vectorSummaryAddress );
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double ObjectiveFunction::value( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& vectorSummaryAddress ) const
{
    RifSummaryReaderInterface* readerInterface = summaryCase->summaryReader();

    if ( functionType == FunctionType::M )
    {
        std::string s = vectorSummaryAddress.quantityName() + RifReaderEclipseSummary::differenceIdentifier();
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

                    return sumValues / std::sqrt( ( N * sumValuesSquared - sumValues * sumValues ) / ( N * ( N - 1.0 ) ) );
                }
            }
        }
    }
    return 0.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<double> ObjectiveFunction::values( const RifEclipseSummaryAddress& vectorSummaryAddress ) const
{
    std::vector<double> values;
    auto                summaryCases = m_summaryCaseCollection->allSummaryCases();

    for ( size_t index = 0; index < summaryCases.size(); index++ )
    {
        values.push_back( value( index, vectorSummaryAddress ) );
    }

    return values;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool ObjectiveFunction::operator<( const ObjectiveFunction& other ) const
{
    return this->name < other.name;
}
