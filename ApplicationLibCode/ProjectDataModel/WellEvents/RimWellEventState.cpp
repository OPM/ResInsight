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

#include "RimWellEventState.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

namespace caf
{
template <>
void caf::AppEnum<RimWellEventState::WellState>::setUp()
{
    addItem( RimWellEventState::WellState::OPEN, "OPEN", "Open" );
    addItem( RimWellEventState::WellState::SHUT, "SHUT", "Shut" );
    addItem( RimWellEventState::WellState::STOP, "STOP", "Stop" );
    setDefault( RimWellEventState::WellState::OPEN );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellEventState, "WellEventState" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventState::RimWellEventState()
{
    CAF_PDM_InitScriptableObject( "Well State Event", "", "", "WellEventState" );

    CAF_PDM_InitScriptableField( &m_wellState, "WellState", caf::AppEnum<WellState>( WellState::OPEN ), "Well State" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventState::~RimWellEventState()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventState::WellState RimWellEventState::wellState() const
{
    return m_wellState();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventState::setWellState( WellState state )
{
    m_wellState = state;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventState::eventType() const
{
    return EventType::WSTATE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventState::generateScheduleKeyword( const QString& wellName ) const
{
    // Generate WELOPEN keyword
    QString line = QString( "WELOPEN\n   %1   %2 /\n/\n\n" ).arg( wellName, -8 ).arg( m_wellState().text() );

    return line;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventState::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_wellState );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventState::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    setUiName( QString( "WSTATE %1: %2" ).arg( m_eventDate().toString( "yyyy-MM-dd" ) ).arg( m_wellState().text() ) );
}
