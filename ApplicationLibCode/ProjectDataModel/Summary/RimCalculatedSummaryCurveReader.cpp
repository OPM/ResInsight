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
        RimSummaryCalculationAddress address( resultAddress );
        return calc->timeSteps( address );
    }

    static std::vector<time_t> dummy;

    return dummy;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifCalculatedSummaryCurveReader::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    RimSummaryCalculation* calc = findCalculationByName( resultAddress );
    if ( calc )
    {
        RimSummaryCalculationAddress address( resultAddress );
        *values = calc->values( address );

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

#include "RiaLogging.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifCalculatedSummaryCurveReader::buildMetaData( RimSummaryCase* summaryCase )
{
    m_allResultAddresses.clear();

    for ( RimUserDefinedCalculation* calc : m_calculationCollection->calculations() )
    {
        RimSummaryCalculation* sumCalc = dynamic_cast<RimSummaryCalculation*>( calc );

        // TODO: should probably avoid calling this with summaryCase == null...
        auto allAddresses = summaryCase ? sumCalc->allAddressesForSummaryCase( summaryCase ) : sumCalc->allAddresses();

        for ( auto addr : allAddresses )
        {
            RimSummaryCalculationAddress* calculationAddress = dynamic_cast<RimSummaryCalculationAddress*>( addr );
            if ( calculationAddress->address().isValid() )
            {
                RiaLogging::info( QString( "Adding result for %1" ).arg( calculationAddress->address().uiText().c_str() ) );
                m_allResultAddresses.insert( calculationAddress->address() );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCalculation*
    RifCalculatedSummaryCurveReader::findCalculationByName( const RifEclipseSummaryAddress& resultAddress ) const
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
