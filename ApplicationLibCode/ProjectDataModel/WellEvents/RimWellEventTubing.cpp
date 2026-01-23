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

#include "RimWellEventTubing.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimWellEventTubing, "WellEventTubing" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventTubing::RimWellEventTubing()
{
    CAF_PDM_InitScriptableObject( "Tubing Event", "", "", "WellEventTubing" );

    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "Start MD" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 0.0, "End MD" );
    CAF_PDM_InitScriptableField( &m_innerDiameter, "InnerDiameter", 0.15, "Inner Diameter [m]" );
    CAF_PDM_InitScriptableField( &m_roughness, "Roughness", 1.0e-5, "Roughness [m]" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventTubing::~RimWellEventTubing()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventTubing::startMD() const
{
    return m_startMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventTubing::endMD() const
{
    return m_endMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventTubing::innerDiameter() const
{
    return m_innerDiameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventTubing::roughness() const
{
    return m_roughness();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTubing::setStartMD( double md )
{
    m_startMD = md;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTubing::setEndMD( double md )
{
    m_endMD = md;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTubing::setInnerDiameter( double diameter )
{
    m_innerDiameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTubing::setRoughness( double roughness )
{
    m_roughness = roughness;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventTubing::eventType() const
{
    return EventType::TUBING;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventTubing::generateScheduleKeyword( const QString& wellName ) const
{
    // Generate a comment line describing the tubing change
    // Actual WELSEGS segment data requires grid calculations
    QString line = QString( "-- %1 TUBING: MD %2 - %3, Diam: %4 m, Rough: %5 m\n" )
                       .arg( wellName )
                       .arg( m_startMD(), 0, 'f', 2 )
                       .arg( m_endMD(), 0, 'f', 2 )
                       .arg( m_innerDiameter(), 0, 'f', 4 )
                       .arg( m_roughness(), 0, 'e', 3 );

    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTubing::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_startMD );
    uiOrdering.add( &m_endMD );
    uiOrdering.add( &m_innerDiameter );
    uiOrdering.add( &m_roughness );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventTubing::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    setUiName( QString( "TUBING %1: %2 - %3 (D: %4 m)" )
                   .arg( m_eventDate().toString( "yyyy-MM-dd" ) )
                   .arg( m_startMD(), 0, 'f', 1 )
                   .arg( m_endMD(), 0, 'f', 1 )
                   .arg( m_innerDiameter(), 0, 'f', 3 ) );
}
