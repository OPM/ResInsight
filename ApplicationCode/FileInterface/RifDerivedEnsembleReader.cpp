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
                                                    RifSummaryReaderInterface* sourceSummaryReader1,
                                                    RifSummaryReaderInterface* sourceSummaryReader2 )
{
    CVF_ASSERT( derivedCase );

    m_derivedCase = derivedCase;

    if ( sourceSummaryReader1 )
    {
        m_allResultAddresses = sourceSummaryReader1->allResultAddresses();
        m_allErrorAddresses  = sourceSummaryReader1->allErrorAddresses();
    }
    if ( sourceSummaryReader2 )
    {
        for ( auto a : sourceSummaryReader2->allResultAddresses() )
        {
            m_allResultAddresses.insert( a );
        }
        for ( auto a : sourceSummaryReader2->allErrorAddresses() )
        {
            m_allErrorAddresses.insert( a );
        }
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
    return RiaEclipseUnitTools::UnitSystem::UNITS_UNKNOWN;
}
