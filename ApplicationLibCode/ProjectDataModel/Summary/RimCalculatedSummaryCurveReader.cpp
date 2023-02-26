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

#include "RimCalculatedSummaryCurveReader.h"

#include "RifEclipseSummaryAddress.h"
#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"
#include "RimUserDefinedCalculation.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCalculatedSummaryCurveReader::RifCalculatedSummaryCurveReader( RimSummaryCalculationCollection* calculationCollection,
                                                                  RimSummaryCase*                  summaryCase )
    : m_calculationCollection( calculationCollection )
    , m_summaryCase( summaryCase )
{
    CAF_ASSERT( summaryCase );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RifCalculatedSummaryCurveReader::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    RimSummaryCalculation* calc = findCalculationByName( resultAddress );
    if ( calc && m_summaryCase )
    {
        RimSummaryCalculationAddress address( resultAddress );
        return calc->timeSteps( m_summaryCase, address );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifCalculatedSummaryCurveReader::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    RimSummaryCalculation* calc = findCalculationByName( resultAddress );
    if ( calc && m_summaryCase )
    {
        RimSummaryCalculationAddress address( resultAddress );
        *values = calc->values( m_summaryCase, address );

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifCalculatedSummaryCurveReader::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    RimSummaryCalculation* calculation = findCalculationByName( resultAddress );
    if ( calculation != nullptr && !calculation->unitName().isEmpty() )
    {
        return calculation->unitName().toStdString();
    }

    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCalculatedSummaryCurveReader::buildMetaData()
{
    m_allResultAddresses.clear();

    for ( RimUserDefinedCalculation* calc : m_calculationCollection->calculations() )
    {
        RimSummaryCalculation* sumCalc = dynamic_cast<RimSummaryCalculation*>( calc );
        CAF_ASSERT( sumCalc );

        const auto& allAddresses = sumCalc->allAddressesForSummaryCase( m_summaryCase );
        for ( auto calculationAddress : allAddresses )
        {
            if ( calculationAddress.address().isValid() )
            {
                m_allResultAddresses.insert( calculationAddress.address() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation* RifCalculatedSummaryCurveReader::findCalculationByName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( !m_calculationCollection || !resultAddress.isCalculated() ) return nullptr;

    return dynamic_cast<RimSummaryCalculation*>( m_calculationCollection->findCalculationById( resultAddress.id() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RifCalculatedSummaryCurveReader::unitSystem() const
{
    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}
