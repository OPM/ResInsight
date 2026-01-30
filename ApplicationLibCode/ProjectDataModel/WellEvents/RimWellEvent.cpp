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

#include "RimWellEvent.h"

#include "RimWellPath.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDateEditor.h"

CAF_PDM_ABSTRACT_SOURCE_INIT( RimWellEvent, "WellEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::RimWellEvent()
{
    CAF_PDM_InitScriptableObject( "Well Event", "", "", "WellEvent" );

    CAF_PDM_InitField( &m_eventDate, "EventDate", QDateTime(), "Event Date" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "Well Path" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::~RimWellEvent()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RimWellEvent::eventDate() const
{
    return m_eventDate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEvent::setEventDate( const QDateTime& date )
{
    m_eventDate = date;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RimWellEvent::wellPath() const
{
    return m_wellPath();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEvent::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEvent::wellName() const
{
    if ( m_wellPath() )
    {
        return m_wellPath()->name();
    }
    return QString();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEvent::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_wellPath );
    uiOrdering.add( &m_eventDate );
}
