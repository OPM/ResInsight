/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017- Statoil ASA
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

#include "RifDerivedEnsembleReader.h"

#include "RimDerivedSummaryCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifDerivedEnsembleReader::RifDerivedEnsembleReader( RimDerivedSummaryCase*     derivedCase,
                                                    RifSummaryReaderInterface* sourceSummaryReader1 )
{
    CVF_ASSERT( derivedCase );

    m_derivedCase = derivedCase;

    if ( sourceSummaryReader1 )
    {
        // TODO: This is assuming that the addresses of both reader interfaces are equal
        m_allResultAddresses = sourceSummaryReader1->allResultAddresses();
        m_allErrorAddresses  = sourceSummaryReader1->allErrorAddresses();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RifDerivedEnsembleReader::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( !resultAddress.isValid() )
    {
        static std::vector<time_t> empty;
        return empty;
    }

    if ( m_derivedCase->needsCalculation( resultAddress ) )
    {
        m_derivedCase->calculate( resultAddress );
    }

    return m_derivedCase->timeSteps( resultAddress );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifDerivedEnsembleReader::values( const RifEclipseSummaryAddress& resultAddress, std::vector<double>* values ) const
{
    if ( !resultAddress.isValid() ) return false;

    if ( m_derivedCase->needsCalculation( resultAddress ) )
    {
        m_derivedCase->calculate( resultAddress );
    }
    auto dataValues = m_derivedCase->values( resultAddress );
    values->clear();
    values->reserve( dataValues.size() );
    for ( auto val : dataValues )
        values->push_back( val );
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifDerivedEnsembleReader::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    return "";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystem RifDerivedEnsembleReader::unitSystem() const
{
    return RiaEclipseUnitTools::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifDerivedEnsembleReader::updateData( RimSummaryCase* sumCase1, RimSummaryCase* sumCase2 )
{
    m_allErrorAddresses.clear();
    m_allResultAddresses.clear();

    if ( sumCase1 && sumCase1->summaryReader() )
    {
        m_allErrorAddresses  = sumCase1->summaryReader()->allErrorAddresses();
        m_allResultAddresses = sumCase1->summaryReader()->allResultAddresses();
    }
}
