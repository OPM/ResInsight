/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimSummaryCaseSumo.h"

#include "RimSummaryEnsembleSumo.h"

#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimSummaryCaseSumo, "SummaryCaseSumo", "RimSummaryCaseSumo" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCaseSumo::RimSummaryCaseSumo()
    : m_realizationNumber( -1 )
{
    CAF_PDM_InitScriptableObject( "Sumo Realization", ":/SummaryCase.svg" );

    m_ensemble = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseSumo::createSummaryReaderInterface()
{
    // Nothing to do here
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimSummaryCaseSumo::summaryReader()
{
    return this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<time_t> RimSummaryCaseSumo::timeSteps( const RifEclipseSummaryAddress& resultAddress ) const
{
    return m_timeSteps;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<bool, std::vector<double>> RimSummaryCaseSumo::values( const RifEclipseSummaryAddress& resultAddress ) const
{
    auto it = m_values.find( resultAddress );
    if ( it == m_values.end() && m_ensemble )
    {
        // We can be asked for a result that is not available in the cache. In this case we need to load the data from the ensemble.

        m_ensemble->loadSummaryData( resultAddress );
        it = m_values.find( resultAddress );
    }

    if ( it != m_values.end() )
    {
        std::vector<double> doubleValues;
        doubleValues.reserve( it->second.size() );
        for ( auto value : it->second )
        {
            doubleValues.push_back( value );
        }
        return { true, doubleValues };
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RimSummaryCaseSumo::unitName( const RifEclipseSummaryAddress& resultAddress ) const
{
    if ( m_ensemble ) return m_ensemble->unitName( resultAddress );

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::EclipseUnitSystem RimSummaryCaseSumo::unitSystem() const
{
    if ( m_ensemble ) return m_ensemble->unitSystem();

    return RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseSumo::setEnsemble( RimSummaryEnsembleSumo* ensemble )
{
    m_ensemble = ensemble;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseSumo::setValues( const std::vector<time_t>&      timeSteps,
                                    const RifEclipseSummaryAddress& resultAddress,
                                    const std::vector<float>&       values )
{
    m_timeSteps             = timeSteps;
    m_values[resultAddress] = values;

    increaseSerialNumber();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int32_t RimSummaryCaseSumo::realizationNumber() const
{
    return m_realizationNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseSumo::setRealizationNumber( int32_t realizationNumber )
{
    m_realizationNumber = realizationNumber;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseSumo::realizationName() const
{
    return m_realizationName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseSumo::setRealizationName( const QString& realizationName )
{
    m_realizationName = realizationName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimSummaryCaseSumo::caseName() const
{
    return realizationName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryCaseSumo::createAndSetAddresses()
{
    if ( m_ensemble )
    {
        auto addresses       = m_ensemble->allResultAddresses();
        m_allResultAddresses = addresses;
    }
}
