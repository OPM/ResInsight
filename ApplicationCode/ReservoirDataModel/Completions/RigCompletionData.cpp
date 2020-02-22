/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RigCompletionData.h"

#include "RiaLogging.h"

#include "cvfAssert.h"

#include <QString>

#include <limits>

//==================================================================================================
///
//==================================================================================================
RigCompletionData::RigCompletionData( const QString& wellName, const RigCompletionDataGridCell& cellIndex, double orderingValue )
    : m_wellName( wellName )
    , m_cellIndex( cellIndex )
    , m_saturation( std::numeric_limits<double>::infinity() )
    , m_transmissibility( std::numeric_limits<double>::infinity() )
    , m_diameter( std::numeric_limits<double>::infinity() )
    , m_kh( std::numeric_limits<double>::infinity() )
    , m_skinFactor( std::numeric_limits<double>::infinity() )
    , m_dFactor( std::numeric_limits<double>::infinity() )
    , m_direction( DIR_UNDEF )
    , m_connectionState( OPEN )
    , m_count( 1 )
    , m_wpimult( std::numeric_limits<double>::infinity() )
    , m_isMainBore( false )
    , m_completionType( CT_UNDEFINED )
    , m_firstOrderingValue( orderingValue )
    , m_secondOrderingValue( std::numeric_limits<double>::infinity() )
{
}

//==================================================================================================
///
//==================================================================================================
RigCompletionData::~RigCompletionData()
{
}

//==================================================================================================
///
//==================================================================================================
RigCompletionData::RigCompletionData( const RigCompletionData& other )
{
    copy( *this, other );
}

//==================================================================================================
///
//==================================================================================================
bool RigCompletionData::operator<( const RigCompletionData& other ) const
{
    if ( m_wellName != other.m_wellName )
    {
        return ( m_wellName < other.m_wellName );
    }

    if ( m_completionType != other.m_completionType )
    {
        return ( m_completionType < other.m_completionType );
    }

    if ( m_firstOrderingValue != other.m_firstOrderingValue )
    {
        return ( m_firstOrderingValue < other.m_firstOrderingValue );
    }

    if ( m_secondOrderingValue != other.m_secondOrderingValue )
    {
        return ( m_secondOrderingValue < other.m_secondOrderingValue );
    }

    return m_cellIndex < other.m_cellIndex;
}

//==================================================================================================
///
//==================================================================================================
RigCompletionData& RigCompletionData::operator=( const RigCompletionData& other )
{
    if ( this != &other )
    {
        copy( *this, other );
    }
    return *this;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionData::isPerforationValve( CompletionType type )
{
    return type == PERFORATION_AICD || type == PERFORATION_ICD || type == PERFORATION_ICV;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionData::isValve( CompletionType type )
{
    return isPerforationValve( type ) || type == FISHBONES_ICD;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionData::isWsegValveTypes( CompletionType type )
{
    return type == FISHBONES_ICD || type == PERFORATION_ICD || type == PERFORATION_ICV;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setFromFracture( double transmissibility, double skinFactor, double diameter )
{
    m_completionType   = FRACTURE;
    m_transmissibility = transmissibility;
    m_skinFactor       = skinFactor;
    m_diameter         = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setSecondOrderingValue( double orderingValue )
{
    m_secondOrderingValue = orderingValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setDiameter( double diameter )
{
    m_diameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setTransmissibility( double transmissibility )
{
    m_transmissibility = transmissibility;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setTransAndWPImultBackgroundDataFromFishbone( double        transmissibility,
                                                                      double        skinFactor,
                                                                      double        diameter,
                                                                      CellDirection direction,
                                                                      bool          isMainBore )
{
    m_completionType   = FISHBONES;
    m_transmissibility = transmissibility;
    m_skinFactor       = skinFactor;
    m_diameter         = diameter;
    m_direction        = direction;
    m_isMainBore       = isMainBore;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setTransAndWPImultBackgroundDataFromPerforation( double        transmissibility,
                                                                         double        skinFactor,
                                                                         double        diameter,
                                                                         double        dFactor,
                                                                         double        kh,
                                                                         CellDirection direction )
{
    m_completionType   = PERFORATION;
    m_transmissibility = transmissibility;
    m_skinFactor       = skinFactor;
    m_diameter         = diameter;
    m_dFactor          = dFactor;
    m_direction        = direction;
    m_kh               = kh;
    m_isMainBore       = true;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setCombinedValuesExplicitTrans( double         transmissibility,
                                                        double         kh,
                                                        double         dFactor,
                                                        double         skinFactor,
                                                        double         diameter,
                                                        CellDirection  celldirection,
                                                        CompletionType completionType )
{
    m_transmissibility = transmissibility;
    m_kh               = kh;
    m_dFactor          = dFactor;
    m_skinFactor       = skinFactor;
    m_diameter         = diameter;
    m_direction        = celldirection;
    m_completionType   = completionType;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::setCombinedValuesImplicitTransWPImult( double         wpimult,
                                                               double         kh,
                                                               double         dFactor,
                                                               double         skinFactor,
                                                               double         diameter,
                                                               CellDirection  celldirection,
                                                               CompletionType completionType )
{
    m_wpimult        = wpimult;
    m_kh             = kh;
    m_dFactor        = dFactor;
    m_direction      = celldirection;
    m_completionType = completionType;
    m_skinFactor     = skinFactor;
    m_diameter       = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionData::isNonDarcyFlow() const
{
    if ( !isDefaultValue( m_kh ) ) return true;
    if ( !isDefaultValue( m_dFactor ) ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setDFactor( double dFactor )
{
    m_dFactor = dFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setKh( double kh )
{
    m_kh = kh;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::addMetadata( const QString& name, const QString& comment )
{
    m_metadata.push_back( RigCompletionMetaData( name, comment ) );
}

//==================================================================================================
///
//==================================================================================================
double RigCompletionData::defaultValue()
{
    return std::numeric_limits<double>::infinity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionData::isDefaultValue( double num )
{
    return num == defaultValue();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigCompletionMetaData>& RigCompletionData::metadata() const
{
    return m_metadata;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QString& RigCompletionData::wellName() const
{
    return m_wellName;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigCompletionDataGridCell& RigCompletionData::completionDataGridCell() const
{
    return m_cellIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellConnectionState RigCompletionData::connectionState() const
{
    return m_connectionState;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::saturation() const
{
    return m_saturation;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::transmissibility() const
{
    return m_transmissibility;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::diameter() const
{
    return m_diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::kh() const
{
    return m_kh;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::skinFactor() const
{
    return m_skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::dFactor() const
{
    return m_dFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
CellDirection RigCompletionData::direction() const
{
    return m_direction;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RigCompletionData::count() const
{
    return m_count;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::wpimult() const
{
    return m_wpimult;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigCompletionData::CompletionType RigCompletionData::completionType() const
{
    return m_completionType;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RigCompletionData::isMainBore() const
{
    return m_isMainBore;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::firstOrderingValue() const
{
    return m_firstOrderingValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RigCompletionData::secondOrderingValue() const
{
    return m_secondOrderingValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RigCompletionData::setSourcePdmObject( const caf::PdmObject* object )
{
    m_sourcePdmObject = const_cast<caf::PdmObject*>( object );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const caf::PdmObject* RigCompletionData::sourcePdmObject() const
{
    return m_sourcePdmObject;
}

//==================================================================================================
///
//==================================================================================================
void RigCompletionData::copy( RigCompletionData& target, const RigCompletionData& from )
{
    target.m_metadata            = from.m_metadata;
    target.m_wellName            = from.m_wellName;
    target.m_cellIndex           = from.m_cellIndex;
    target.m_connectionState     = from.m_connectionState;
    target.m_saturation          = from.m_saturation;
    target.m_transmissibility    = from.m_transmissibility;
    target.m_diameter            = from.m_diameter;
    target.m_kh                  = from.m_kh;
    target.m_skinFactor          = from.m_skinFactor;
    target.m_dFactor             = from.m_dFactor;
    target.m_direction           = from.m_direction;
    target.m_isMainBore          = from.m_isMainBore;
    target.m_count               = from.m_count;
    target.m_wpimult             = from.m_wpimult;
    target.m_completionType      = from.m_completionType;
    target.m_firstOrderingValue  = from.m_firstOrderingValue;
    target.m_secondOrderingValue = from.m_secondOrderingValue;
    target.m_sourcePdmObject     = from.m_sourcePdmObject;
}
