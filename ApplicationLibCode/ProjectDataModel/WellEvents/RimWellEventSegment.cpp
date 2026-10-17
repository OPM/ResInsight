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

#include "RimWellEventSegment.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiCheckBoxAndTextEditor.h"

CAF_PDM_SOURCE_INIT( RimWellEventSegment, "WellEventSegment" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventSegment::RimWellEventSegment()
{
    CAF_PDM_InitScriptableObject( "Segment Event", "", "", "WellEventSegment" );

    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "Start MD" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 0.0, "End MD" );
    CAF_PDM_InitScriptableField( &m_innerDiameter, "InnerDiameter", 0.15, "Inner Diameter [m]" );
    CAF_PDM_InitScriptableField( &m_roughness, "Roughness", 1.0e-5, "Roughness [m]" );

    CAF_PDM_InitScriptableField( &m_fixedSegmentLength, "FixedSegmentLength", std::make_pair( false, 50.0 ), "Fixed Segment Length" );
    m_fixedSegmentLength.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    CAF_PDM_InitScriptableField( &m_minSegmentLength, "MinSegmentLength", std::make_pair( false, 10.0 ), "Min Segment Length" );
    m_minSegmentLength.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    CAF_PDM_InitScriptableField( &m_maxSegmentLength, "MaxSegmentLength", std::make_pair( false, 100.0 ), "Max Segment Length" );
    m_maxSegmentLength.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventSegment::~RimWellEventSegment()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventSegment::startMD() const
{
    return m_startMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventSegment::endMD() const
{
    return m_endMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventSegment::innerDiameter() const
{
    return m_innerDiameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventSegment::roughness() const
{
    return m_roughness();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::setStartMD( double md )
{
    m_startMD = md;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::setEndMD( double md )
{
    m_endMD = md;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::setInnerDiameter( double diameter )
{
    m_innerDiameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::setRoughness( double roughness )
{
    m_roughness = roughness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RimWellEventSegment::fixedSegmentLength() const
{
    if ( m_fixedSegmentLength().first ) return m_fixedSegmentLength().second;
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RimWellEventSegment::minSegmentLength() const
{
    if ( m_minSegmentLength().first ) return m_minSegmentLength().second;
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<double> RimWellEventSegment::maxSegmentLength() const
{
    if ( m_maxSegmentLength().first ) return m_maxSegmentLength().second;
    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::setFixedSegmentLength( std::optional<double> length )
{
    m_fixedSegmentLength = std::make_pair( length.has_value(), length.value_or( m_fixedSegmentLength().second ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::setMinSegmentLength( std::optional<double> length )
{
    m_minSegmentLength = std::make_pair( length.has_value(), length.value_or( m_minSegmentLength().second ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::setMaxSegmentLength( std::optional<double> length )
{
    m_maxSegmentLength = std::make_pair( length.has_value(), length.value_or( m_maxSegmentLength().second ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventSegment::eventType() const
{
    return EventType::SEGMENT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventSegment::generateScheduleKeyword( const QString& wellName ) const
{
    // Generate a comment line describing the segment change
    // Actual WELSEGS segment data requires grid calculations
    QString line = QString( "-- %1 SEGMENT: MD %2 - %3, Diam: %4 m, Rough: %5 m\n" )
                       .arg( wellName )
                       .arg( m_startMD(), 0, 'f', 2 )
                       .arg( m_endMD(), 0, 'f', 2 )
                       .arg( m_innerDiameter(), 0, 'f', 4 )
                       .arg( m_roughness(), 0, 'e', 3 );

    QStringList lengths;
    if ( auto length = fixedSegmentLength() ) lengths.push_back( QString( "Fixed seg len: %1 m" ).arg( *length, 0, 'f', 2 ) );
    if ( auto length = minSegmentLength() ) lengths.push_back( QString( "Min seg len: %1 m" ).arg( *length, 0, 'f', 2 ) );
    if ( auto length = maxSegmentLength() ) lengths.push_back( QString( "Max seg len: %1 m" ).arg( *length, 0, 'f', 2 ) );
    if ( !lengths.isEmpty() )
    {
        line.chop( 1 );
        line += ", " + lengths.join( ", " ) + "\n";
    }

    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_startMD );
    uiOrdering.add( &m_endMD );
    uiOrdering.add( &m_innerDiameter );
    uiOrdering.add( &m_roughness );
    uiOrdering.add( &m_fixedSegmentLength );
    uiOrdering.add( &m_minSegmentLength );
    uiOrdering.add( &m_maxSegmentLength );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventSegment::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    setUiName( QString( "SEGMENT %1: %2 - %3 (D: %4 m)" )
                   .arg( m_eventDate().toString( "yyyy-MM-dd" ) )
                   .arg( m_startMD(), 0, 'f', 1 )
                   .arg( m_endMD(), 0, 'f', 1 )
                   .arg( m_innerDiameter(), 0, 'f', 3 ) );
}
