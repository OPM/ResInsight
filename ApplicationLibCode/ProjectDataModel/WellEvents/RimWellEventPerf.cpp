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

#include "RimWellEventPerf.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleSliderEditor.h"

namespace caf
{
template <>
void caf::AppEnum<RimWellEventPerf::State>::setUp()
{
    addItem( RimWellEventPerf::State::OPEN, "OPEN", "Open" );
    addItem( RimWellEventPerf::State::SHUT, "SHUT", "Shut" );
    setDefault( RimWellEventPerf::State::OPEN );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellEventPerf, "WellEventPerf" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventPerf::RimWellEventPerf()
{
    CAF_PDM_InitScriptableObject( "Perforation Event", ":/PerforationInterval16x16.png", "", "WellEventPerf" );

    CAF_PDM_InitScriptableField( &m_startMD, "StartMd", 0.0, "Start MD" );
    CAF_PDM_InitScriptableField( &m_endMD, "EndMd", 0.0, "End MD" );
    CAF_PDM_InitScriptableField( &m_diameter, "Diameter", 0.216, "Diameter" );
    CAF_PDM_InitScriptableField( &m_skinFactor, "SkinFactor", 0.0, "Skin Factor" );
    CAF_PDM_InitScriptableField( &m_state, "State", caf::AppEnum<State>( State::OPEN ), "State" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventPerf::~RimWellEventPerf()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventPerf::startMD() const
{
    return m_startMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventPerf::endMD() const
{
    return m_endMD();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventPerf::diameter() const
{
    return m_diameter();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimWellEventPerf::skinFactor() const
{
    return m_skinFactor();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventPerf::State RimWellEventPerf::state() const
{
    return m_state();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventPerf::setStartMD( double md )
{
    m_startMD = md;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventPerf::setEndMD( double md )
{
    m_endMD = md;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventPerf::setDiameter( double diameter )
{
    m_diameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventPerf::setSkinFactor( double skinFactor )
{
    m_skinFactor = skinFactor;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventPerf::setState( State state )
{
    m_state = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventPerf::eventType() const
{
    return EventType::PERF;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventPerf::generateScheduleKeyword( const QString& wellName ) const
{
    // Generate COMPDAT keyword line
    // COMPDAT format: Well I J K1 K2 Status SAT Trans Diam Kh Skin Dir
    // Note: Grid indices (I, J, K1, K2) require grid intersection calculation
    // This method generates a template that needs grid data to be complete

    QString status = ( m_state() == State::OPEN ) ? "OPEN" : "SHUT";

    QString line = QString( "-- %1 MD: %2 - %3, Diam: %4, Skin: %5, %6\n" )
                       .arg( wellName )
                       .arg( m_startMD(), 0, 'f', 2 )
                       .arg( m_endMD(), 0, 'f', 2 )
                       .arg( m_diameter(), 0, 'f', 4 )
                       .arg( m_skinFactor(), 0, 'f', 2 )
                       .arg( status );

    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventPerf::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_startMD );
    uiOrdering.add( &m_endMD );
    uiOrdering.add( &m_diameter );
    uiOrdering.add( &m_skinFactor );
    uiOrdering.add( &m_state );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventPerf::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    QString stateStr = ( m_state() == State::OPEN ) ? "OPEN" : "SHUT";
    setUiName( QString( "PERF %1: %2 - %3 [%4]" )
                   .arg( m_eventDate().toString( "yyyy-MM-dd" ) )
                   .arg( m_startMD(), 0, 'f', 1 )
                   .arg( m_endMD(), 0, 'f', 1 )
                   .arg( stateStr ) );
}
