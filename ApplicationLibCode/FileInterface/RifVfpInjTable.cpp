/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

//==================================================================================================
//
//  RifVfpInjTable - VFP Injection Table Reader Implementation
//
//  Based on: ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/Schedule/VFPInjTable.cpp
//
//  This implementation preserves the original Eclipse deck keyword values without unit conversions,
//  unlike the OPM version which automatically converts all values to SI units.
//
//==================================================================================================

#include "RifVfpInjTable.h"

#include "cafAssert.h"

#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace
{
template <typename T>
inline const Opm::DeckItem& getNonEmptyItem( const Opm::DeckRecord& record )
{
    const auto& retval = record.getItem<T>();
    if ( !retval.hasValue( 0 ) )
    {
        throw std::invalid_argument( "Missing data" );
    }
    return retval;
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpInjTable::FLO_TYPE getFloType( const std::string& floString )
{
    if ( floString == "OIL" ) return RifVfpInjTable::FLO_TYPE::FLO_OIL;
    if ( floString == "WAT" ) return RifVfpInjTable::FLO_TYPE::FLO_WAT;
    if ( floString == "GAS" ) return RifVfpInjTable::FLO_TYPE::FLO_GAS;

    throw std::invalid_argument( "Invalid RATE_TYPE string" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVfpInjTable::checkAxis( const std::vector<double>& axis )
{
    if ( axis.size() == 0 ) throw std::invalid_argument( "Empty axis" );

    if ( !std::is_sorted( axis.begin(), axis.end() ) ) throw std::invalid_argument( "Axis is not sorted" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifVfpInjTable::tableNum() const
{
    return m_tableNum;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifVfpInjTable::datumDepth() const
{
    return m_datumDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpInjTable::FLO_TYPE RifVfpInjTable::floType() const
{
    return m_floType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RifVfpInjTable::units() const
{
    return m_units;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RifVfpInjTable::bodyDef() const
{
    return m_bodyDef;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpInjTable::floAxis() const
{
    return m_floData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpInjTable::thpAxis() const
{
    return m_thpData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpInjTable::table() const
{
    return m_data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpInjTable::RifVfpInjTable()
{
    m_tableNum   = -1;
    m_datumDepth = 0.0;
    m_floType    = FLO_TYPE::FLO_OIL;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpInjTable::RifVfpInjTable( const Opm::DeckKeyword& table )
{
    using Opm::ParserKeywords::VFPINJ;

    if ( table.size() < 4 )
    {
        throw std::invalid_argument( "VFPINJ table does not appear to have enough records to be valid" );
    }

    const auto& header = table.getRecord( 0 );

    m_tableNum   = getNonEmptyItem<VFPINJ::TABLE>( header ).get<int>( 0 );
    m_datumDepth = getNonEmptyItem<VFPINJ::DATUM_DEPTH>( header ).get<double>( 0 );

    m_floType = ::getFloType( getNonEmptyItem<VFPINJ::RATE_TYPE>( header ).get<std::string>( 0 ) );

    std::string quantity_string = getNonEmptyItem<VFPINJ::PRESSURE_DEF>( header ).get<std::string>( 0 );
    if ( quantity_string != "THP" )
    {
        throw std::invalid_argument( "PRESSURE_DEF is required to be THP" );
    }

    if ( header.getItem<VFPINJ::UNITS>().hasValue( 0 ) )
    {
        m_units = header.getItem<VFPINJ::UNITS>().get<std::string>( 0 );
    }

    m_bodyDef = getNonEmptyItem<VFPINJ::BODY_DEF>( header ).get<std::string>( 0 );
    if ( m_bodyDef != "BHP" )
    {
        throw std::invalid_argument( "Invalid BODY_DEF string" );
    }

    m_floData = getNonEmptyItem<VFPINJ::FLOW_VALUES>( table.getRecord( 1 ) ).getData<double>();
    m_thpData = getNonEmptyItem<VFPINJ::THP_VALUES>( table.getRecord( 2 ) ).getData<double>();

    size_t nt = m_thpData.size();
    size_t nf = m_floData.size();
    m_data.resize( nt * nf );
    std::fill_n( m_data.data(), m_data.size(), std::nan( "0" ) );

    if ( table.size() != nt + 3 )
    {
        throw std::invalid_argument( "VFPINJ table does not contain enough records." );
    }

    for ( size_t i = 3; i < table.size(); ++i )
    {
        const auto& record = table.getRecord( i );
        int         t      = getNonEmptyItem<VFPINJ::THP_INDEX>( record ).get<int>( 0 ) - 1;

        const std::vector<double>& bhp_values = getNonEmptyItem<VFPINJ::VALUES>( record ).getData<double>();

        if ( bhp_values.size() != nf )
        {
            throw std::invalid_argument( "VFPINJ table does not contain enough FLO values." );
        }

        for ( size_t f = 0; f < bhp_values.size(); ++f )
        {
            ( *this )( t, f ) = bhp_values[f];
        }
    }

    check();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<size_t, 2> RifVfpInjTable::shape() const
{
    return { m_thpData.size(), m_floData.size() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifVfpInjTable::operator()( size_t thpIdx, size_t floIdx ) const
{
    size_t nt = m_thpData.size();
    size_t nf = m_floData.size();

    CAF_ASSERT( thpIdx < nt );
    CAF_ASSERT( floIdx < nf );

    size_t index = thpIdx + nt * floIdx;
    return m_data[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double& RifVfpInjTable::operator()( size_t thpIdx, size_t floIdx )
{
    size_t nt = m_thpData.size();
    size_t nf = m_floData.size();

    CAF_ASSERT( thpIdx < nt );
    CAF_ASSERT( floIdx < nf );

    size_t index = thpIdx + nt * floIdx;
    return m_data[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVfpInjTable::check()
{
    if ( m_tableNum <= 0 ) throw std::invalid_argument( "Invalid table number" );

    checkAxis( m_floData );
    checkAxis( m_thpData );

    if ( m_data.size() != m_thpData.size() * m_floData.size() ) throw std::invalid_argument( "Wrong data size" );

    for ( size_t t = 0; t < m_thpData.size(); ++t )
    {
        for ( size_t f = 0; f < m_floData.size(); ++f )
        {
            if ( std::isnan( ( *this )( t, f ) ) )
            {
                throw std::invalid_argument( "VFPINJ table element not initialized" );
            }
        }
    }
}