/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Ceetron Solutions AS
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
#include "RigWbsParameter.h"

#include "cafAssert.h"

namespace caf
{
template <>
void RigWbsParameter::SourceEnum::setUp()
{
    addItem( RigWbsParameter::GRID, "GRID", "Grid" );
    addItem( RigWbsParameter::LAS_FILE, "LAS_FILE", "LAS File" );
    addItem( RigWbsParameter::ELEMENT_PROPERTY_TABLE, "ELEMENT_PROPERTY_TABLE", "Property Table" );
    addItem( RigWbsParameter::USER_DEFINED, "USER_DEFINED", "User Defined" );
    addItem( RigWbsParameter::HYDROSTATIC, "HYDROSTATIC", "Hydrostatic" );
    addItem( RigWbsParameter::DERIVED_FROM_K0FG, "DERIVED_FROM_K0FG", "FG derived from K0_FG" );
    addItem( RigWbsParameter::PROPORTIONAL_TO_SH, "PROPORTIONAL_TO_SH", "Proportional to SH" );
    addItem( RigWbsParameter::UNDEFINED, "UNDEFINED", "Undefined" );
    setDefault( RigWbsParameter::UNDEFINED );
}
} // End namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter::RigWbsParameter( const QString& name, bool normalizeByHydroStaticPP, const SourceVector& sources )
    : m_name( name )
    , m_sources( sources )
    , m_normalizeByHydroStaticPP( normalizeByHydroStaticPP )
    , m_exclusiveOptions( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigWbsParameter::name() const
{
    return m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigWbsParameter::Source> RigWbsParameter::sources() const
{
    std::vector<RigWbsParameter::Source> allValidSources;
    for ( auto sourceEntryPair : m_sources )
    {
        allValidSources.push_back( sourceEntryPair.first );
    }
    return allValidSources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWbsParameter::addressString( Source source ) const
{
    return address( source ).primary;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigFemResultAddress RigWbsParameter::femAddress( Source source ) const
{
    RigFemResultPosEnum posType = RIG_WELLPATH_DERIVED;
    if ( source == GRID )
        posType = RIG_ELEMENT_NODAL;
    else if ( source == ELEMENT_PROPERTY_TABLE )
        posType = RIG_ELEMENT;

    auto addr = address( source );
    return RigFemResultAddress( posType, addr.primary.toStdString(), addr.secondary.toStdString() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWbsParameter::normalizeByHydrostaticPP() const
{
    return m_normalizeByHydroStaticPP;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWbsParameter::exclusiveOptions() const
{
    return m_exclusiveOptions;
}

//--------------------------------------------------------------------------------------------------
/// Set options to be exclusive rather than an order of preference
//--------------------------------------------------------------------------------------------------
void RigWbsParameter::setOptionsExclusive( bool exclusive )
{
    m_exclusiveOptions = exclusive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString>
    RigWbsParameter::allSourceLabels( const QString& delimiter /*= " "*/,
                                      double         userDefinedValue /*= std::numeric_limits<double>::infinity() */ )
{
    std::vector<QString> allLabels;
    for ( size_t i = 0; i < SourceEnum::size(); ++i )
    {
        allLabels.push_back( sourceLabel( SourceEnum::fromIndex( i ), delimiter, userDefinedValue ) );
    }
    return allLabels;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RigWbsParameter::sourceLabel( Source         currentSource,
                                      const QString& delimiter,
                                      double         userDefinedValue /*= std::numeric_limits<double>::infinity() */ )
{
    QString sourceLabel;
    if ( currentSource == USER_DEFINED )
    {
        sourceLabel = QString( "%1 = %2" ).arg( m_name ).arg( userDefinedValue );
    }
    else
    {
        sourceLabel = QString( "%1 = %2" ).arg( m_name ).arg( SourceEnum::uiText( currentSource ) );
    }
    sourceLabel.replace( " ", delimiter );
    return sourceLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWbsParameter::operator==( const RigWbsParameter& rhs ) const
{
    return m_name == rhs.m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWbsParameter::operator<( const RigWbsParameter& rhs ) const
{
    return m_name < rhs.m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::PP_Reservoir()
{
    SourceVector sources = {{GRID, SourceAddress( "POR-Bar" )},
                            {LAS_FILE, SourceAddress( "POR" )},
                            {ELEMENT_PROPERTY_TABLE, SourceAddress( "POR" )}};
    return RigWbsParameter( "PP", true, sources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::PP_NonReservoir()
{
    return RigWbsParameter( "PP Non-Reservoir",
                            true,
                            {{LAS_FILE, SourceAddress( "POR_NonReservoir" )},
                             {HYDROSTATIC, SourceAddress( "Hydrostatic" )},
                             {USER_DEFINED, SourceAddress()}} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::poissonRatio()
{
    return RigWbsParameter( "Poisson Ratio",
                            false,
                            {{LAS_FILE, SourceAddress( "POISSON_RATIO" )},
                             {ELEMENT_PROPERTY_TABLE, SourceAddress( "RATIO" )},
                             {USER_DEFINED, SourceAddress()}} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::UCS()
{
    return RigWbsParameter( "UCS",
                            false,
                            {{LAS_FILE, SourceAddress( "UCS" )},
                             {ELEMENT_PROPERTY_TABLE, SourceAddress( "UCS" )},
                             {USER_DEFINED, SourceAddress()}} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::OBG()
{
    std::vector<std::pair<Source, SourceAddress>> sources = {{GRID, SourceAddress( "ST", "S33" )}};
    return RigWbsParameter( "OBG", true, sources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::OBG0()
{
    std::vector<std::pair<Source, SourceAddress>> sources = {{GRID, SourceAddress( "ST", "S33" )},
                                                             {LAS_FILE, SourceAddress( "POR" )}};
    return RigWbsParameter( "OBG0", true, sources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::SH()
{
    std::vector<std::pair<Source, SourceAddress>> sources = {{GRID, SourceAddress( "ST", "S3" )}};
    return RigWbsParameter( "SH", true, sources );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::DF()
{
    return RigWbsParameter( "DF",
                            false,
                            {{LAS_FILE, SourceAddress( "DF" )},
                             {ELEMENT_PROPERTY_TABLE, SourceAddress( "DF" )},
                             {USER_DEFINED, SourceAddress()}} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::K0_FG()
{
    return RigWbsParameter( "K0_FG", false, {{LAS_FILE, SourceAddress( "K0_FG" )}, {USER_DEFINED, SourceAddress()}} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::K0_SH()
{
    return RigWbsParameter( "K0_SH", false, {{LAS_FILE, SourceAddress( "K0_SH" )}, {USER_DEFINED, SourceAddress()}} );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter RigWbsParameter::FG_Shale()
{
    RigWbsParameter param( "FG Shale",
                           true,
                           {{DERIVED_FROM_K0FG, SourceAddress()}, {PROPORTIONAL_TO_SH, SourceAddress()}} );
    param.setOptionsExclusive( true );
    return param;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigWbsParameter> RigWbsParameter::allParameters()
{
    return {PP_Reservoir(), PP_NonReservoir(), poissonRatio(), UCS(), OBG(), OBG0(), SH(), DF(), K0_FG(), K0_SH(), FG_Shale()};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigWbsParameter::findParameter( QString parameterName, RigWbsParameter* foundParam /* = nullptr*/ )
{
    auto params = allParameters();
    auto it     = params.find( parameterName );
    if ( it != params.end() )
    {
        if ( foundParam ) *foundParam = *it;
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWbsParameter::SourceAddress RigWbsParameter::address( Source source ) const
{
    for ( auto sourceEntryPair : m_sources )
    {
        if ( sourceEntryPair.first == source ) return sourceEntryPair.second;
    }
    return SourceAddress();
}
