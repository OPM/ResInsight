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
//  RifVfpProdTable - VFP Production Table Reader Implementation
//
//  Based on: ThirdParty/custom-opm-common/opm-common/opm/input/eclipse/Schedule/VFPProdTable.cpp
//
//  This implementation preserves the original Eclipse deck keyword values without unit conversions,
//  unlike the OPM version which automatically converts all values to SI units.
//
//==================================================================================================

#include "RifVfpProdTable.h"
#include "RifVfpInjTable.h"

#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/V.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::FLO_TYPE getFloType( const Opm::DeckItem& item )
{
    const std::string& flo_string = item.getTrimmedString( 0 );

    if ( flo_string == "OIL" ) return RifVfpProdTable::FLO_TYPE::FLO_OIL;
    if ( flo_string == "LIQ" ) return RifVfpProdTable::FLO_TYPE::FLO_LIQ;
    if ( flo_string == "GAS" ) return RifVfpProdTable::FLO_TYPE::FLO_GAS;

    throw std::invalid_argument( "Invalid RATE_TYPE string: " + flo_string );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::WFR_TYPE getWFRType( const Opm::DeckItem& item )
{
    const std::string& wfr_string = item.getTrimmedString( 0 );

    if ( wfr_string == "WOR" ) return RifVfpProdTable::WFR_TYPE::WFR_WOR;
    if ( wfr_string == "WCT" ) return RifVfpProdTable::WFR_TYPE::WFR_WCT;
    if ( wfr_string == "WGR" ) return RifVfpProdTable::WFR_TYPE::WFR_WGR;

    throw std::invalid_argument( "Invalid WFR string" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::GFR_TYPE getGFRType( const Opm::DeckItem& item )
{
    const std::string& gfr_string = item.getTrimmedString( 0 );

    if ( gfr_string == "GOR" ) return RifVfpProdTable::GFR_TYPE::GFR_GOR;
    if ( gfr_string == "GLR" ) return RifVfpProdTable::GFR_TYPE::GFR_GLR;
    if ( gfr_string == "OGR" ) return RifVfpProdTable::GFR_TYPE::GFR_OGR;

    throw std::invalid_argument( "Invalid GFR string" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::ALQ_TYPE getALQType( const Opm::DeckItem& item, bool gasliftOptActive )
{
    if ( item.defaultApplied( 0 ) )
    {
        if ( gasliftOptActive ) return RifVfpProdTable::ALQ_TYPE::ALQ_GRAT;
        return RifVfpProdTable::ALQ_TYPE::ALQ_UNDEF;
    }
    else
    {
        const std::string& alqString = item.getTrimmedString( 0 );

        if ( alqString == "GRAT" ) return RifVfpProdTable::ALQ_TYPE::ALQ_GRAT;
        if ( alqString == "IGLR" ) return RifVfpProdTable::ALQ_TYPE::ALQ_IGLR;
        if ( alqString == "TGLR" ) return RifVfpProdTable::ALQ_TYPE::ALQ_TGLR;
        if ( alqString == "PUMP" ) return RifVfpProdTable::ALQ_TYPE::ALQ_PUMP;
        if ( alqString == "COMP" ) return RifVfpProdTable::ALQ_TYPE::ALQ_COMP;
        if ( alqString == "BEAN" ) return RifVfpProdTable::ALQ_TYPE::ALQ_BEAN;
        if ( alqString == "" )
        {
            if ( gasliftOptActive ) return RifVfpProdTable::ALQ_TYPE::ALQ_GRAT;
            return RifVfpProdTable::ALQ_TYPE::ALQ_UNDEF;
        }

        throw std::invalid_argument( "Invalid ALQ_DEF string: " + alqString );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::RifVfpProdTable()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::RifVfpProdTable( const Opm::DeckKeyword& table, bool gasliftOptActive )
{
    using Opm::ParserKeywords::VFPPROD;

    if ( table.size() < 7 )
    {
        throw std::invalid_argument( "VFPPROD table does not appear to have enough records to be valid" );
    }

    const auto& header = table.getRecord( 0 );

    m_tableNum   = header.getItem<VFPPROD::TABLE>().get<int>( 0 );
    m_datumDepth = header.getItem<VFPPROD::DATUM_DEPTH>().get<double>( 0 );

    m_floType = ::getFloType( header.getItem<VFPPROD::RATE_TYPE>() );
    m_wfrType = ::getWFRType( header.getItem<VFPPROD::WFR>() );
    m_gfrType = ::getGFRType( header.getItem<VFPPROD::GFR>() );

    std::string quantity_string = header.getItem<VFPPROD::PRESSURE_DEF>().get<std::string>( 0 );
    if ( quantity_string != "THP" )
    {
        throw std::invalid_argument( "PRESSURE_DEF is required to be THP" );
    }

    m_alqType = ::getALQType( header.getItem<VFPPROD::ALQ_DEF>(), gasliftOptActive );

    if ( header.getItem<VFPPROD::UNITS>().hasValue( 0 ) )
    {
        m_units = header.getItem<VFPPROD::UNITS>().get<std::string>( 0 );
    }

    m_bodyDef = header.getItem<VFPPROD::BODY_DEF>().get<std::string>( 0 );
    if ( m_bodyDef == "TEMP" )
    {
        throw std::invalid_argument( "Invalid BODY_DEF string: TEMP not supported" );
    }
    else if ( m_bodyDef != "BHP" )
    {
        throw std::invalid_argument( "Invalid BODY_DEF string" );
    }

    m_floData = table.getRecord( 1 ).getItem<VFPPROD::FLOW_VALUES>().getData<double>();
    m_thpData = table.getRecord( 2 ).getItem<VFPPROD::THP_VALUES>().getData<double>();
    m_wfrData = table.getRecord( 3 ).getItem<VFPPROD::WFR_VALUES>().getData<double>();
    m_gfrData = table.getRecord( 4 ).getItem<VFPPROD::GFR_VALUES>().getData<double>();
    m_alqData = table.getRecord( 5 ).getItem<VFPPROD::ALQ_VALUES>().getData<double>();

    size_t nt = m_thpData.size();
    size_t nw = m_wfrData.size();
    size_t ng = m_gfrData.size();
    size_t na = m_alqData.size();
    size_t nf = m_floData.size();
    m_data.resize( nt * nw * ng * na * nf );
    std::fill_n( m_data.data(), m_data.size(), std::nan( "0" ) );

    if ( table.size() != nt * nw * ng * na + 6 )
    {
        throw std::invalid_argument( "VFPPROD table does not contain enough records." );
    }

    for ( size_t i = 6; i < table.size(); ++i )
    {
        const auto& record = table.getRecord( i );
        int         t      = record.getItem<VFPPROD::THP_INDEX>().get<int>( 0 ) - 1;
        int         w      = record.getItem<VFPPROD::WFR_INDEX>().get<int>( 0 ) - 1;
        int         g      = record.getItem<VFPPROD::GFR_INDEX>().get<int>( 0 ) - 1;
        int         a      = record.getItem<VFPPROD::ALQ_INDEX>().get<int>( 0 ) - 1;

        const std::vector<double>& bhp_values = record.getItem<VFPPROD::VALUES>().getData<double>();

        if ( bhp_values.size() != nf )
        {
            throw std::invalid_argument( "VFPPROD table does not contain enough FLO values." );
        }

        for ( size_t f = 0; f < bhp_values.size(); ++f )
        {
            ( *this )( t, w, g, a, f ) = bhp_values[f];
        }
    }

    check();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifVfpProdTable::tableNum() const
{
    return m_tableNum;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifVfpProdTable::datumDepth() const
{
    return m_datumDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::FLO_TYPE RifVfpProdTable::floType() const
{
    return m_floType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::WFR_TYPE RifVfpProdTable::wfrType() const
{
    return m_wfrType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::GFR_TYPE RifVfpProdTable::gfrType() const
{
    return m_gfrType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifVfpProdTable::ALQ_TYPE RifVfpProdTable::alqType() const
{
    return m_alqType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RifVfpProdTable::units() const
{
    return m_units;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::string& RifVfpProdTable::bodyDef() const
{
    return m_bodyDef;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpProdTable::floAxis() const
{
    return m_floData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpProdTable::thpAxis() const
{
    return m_thpData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpProdTable::wfrAxis() const
{
    return m_wfrData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpProdTable::gfrAxis() const
{
    return m_gfrData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpProdTable::alqAxis() const
{
    return m_alqData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>& RifVfpProdTable::table() const
{
    return m_data;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<size_t, 5> RifVfpProdTable::shape() const
{
    return { m_thpData.size(), m_wfrData.size(), m_gfrData.size(), m_alqData.size(), m_floData.size() };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RifVfpProdTable::operator()( size_t thpIdx, size_t wfrIdx, size_t gfrIdx, size_t alqIdx, size_t floIdx ) const
{
    size_t nt = m_thpData.size();
    size_t nw = m_wfrData.size();
    size_t ng = m_gfrData.size();
    size_t na = m_alqData.size();
    size_t nf = m_floData.size();

    assert( thpIdx < nt );
    assert( wfrIdx < nw );
    assert( gfrIdx < ng );
    assert( alqIdx < na );
    assert( floIdx < nf );

    size_t index = thpIdx + nt * ( wfrIdx + nw * ( gfrIdx + ng * ( alqIdx + na * floIdx ) ) );
    return m_data[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double& RifVfpProdTable::operator()( size_t thpIdx, size_t wfrIdx, size_t gfrIdx, size_t alqIdx, size_t floIdx )
{
    size_t nt = m_thpData.size();
    size_t nw = m_wfrData.size();
    size_t ng = m_gfrData.size();
    size_t na = m_alqData.size();
    size_t nf = m_floData.size();

    assert( thpIdx < nt );
    assert( wfrIdx < nw );
    assert( gfrIdx < ng );
    assert( alqIdx < na );
    assert( floIdx < nf );

    size_t index = thpIdx + nt * ( wfrIdx + nw * ( gfrIdx + ng * ( alqIdx + na * floIdx ) ) );
    return m_data[index];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifVfpProdTable::check()
{
    if ( m_tableNum <= 0 ) throw std::invalid_argument( "Invalid table number" );

    RifVfpInjTable::checkAxis( m_floData );
    RifVfpInjTable::checkAxis( m_thpData );
    RifVfpInjTable::checkAxis( m_wfrData );
    RifVfpInjTable::checkAxis( m_gfrData );
    RifVfpInjTable::checkAxis( m_alqData );

    size_t nt = m_thpData.size();
    size_t nw = m_wfrData.size();
    size_t ng = m_gfrData.size();
    size_t na = m_alqData.size();
    size_t nf = m_floData.size();

    for ( size_t t = 0; t < nt; ++t )
    {
        for ( size_t w = 0; w < nw; ++w )
        {
            for ( size_t g = 0; g < ng; ++g )
            {
                for ( size_t a = 0; a < na; ++a )
                {
                    for ( size_t f = 0; f < nf; ++f )
                    {
                        if ( std::isnan( ( *this )( t, w, g, a, f ) ) )
                        {
                            throw std::invalid_argument( "VFPPROD table element not initialized" );
                        }
                    }
                }
            }
        }
    }
}