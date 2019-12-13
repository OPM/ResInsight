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

#include "RimSummaryCalculation.h"
#include "RimSummaryCalculationCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifCalculatedSummaryCurveReader::RifCalculatedSummaryCurveReader( RimSummaryCalculationCollection* calculationCollection )
    : m_calculationCollection( calculationCollection )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifCalculatedSummaryCurveReader::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    RimSummaryCalculation* calc = findCalculationByName( resultAddress );
    if ( calc )
    {
        return calc->timeSteps();
    }

    static std::vector<time_t> dummy;

    return dummy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifCalculatedSummaryCurveReader::values( const RifEclipseSummaryAddress& resultAddress,
                                              std::vector<double>*            values ) const
{
    RimSummaryCalculation* calc = findCalculationByName( resultAddress );
    if ( calc )
    {
        *values = calc->values();

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

    for ( RimSummaryCalculation* calc : m_calculationCollection->calculations() )
    {
        m_allResultAddresses.insert(
            RifEclipseSummaryAddress::calculatedAddress( calc->description().toStdString(), calc->id() ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation*
    RifCalculatedSummaryCurveReader::findCalculationByName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_calculationCollection && resultAddress.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED )
    {
        return m_calculationCollection->findCalculationById( resultAddress.id() );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RifCalculatedSummaryCurveReader::unitSystem() const
{
    return RiaEclipseUnitTools::UNITS_UNKNOWN;
}
