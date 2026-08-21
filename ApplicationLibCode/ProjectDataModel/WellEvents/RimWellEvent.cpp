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

#include "RimTools.h"
#include "RimWellPath.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDateEditor.h"

namespace caf
{
template <>
void AppEnum<RimWellEvent::EventType>::setUp()
{
    addItem( RimWellEvent::EventType::PERF, "PERF", "Perforation" );
    addItem( RimWellEvent::EventType::VALVE, "VALVE", "Valve" );
    addItem( RimWellEvent::EventType::TUBING, "TUBING", "Tubing" );
    addItem( RimWellEvent::EventType::WSTATE, "WSTATE", "Well State" );
    addItem( RimWellEvent::EventType::WTYPE, "WTYPE", "Well Type" );
    addItem( RimWellEvent::EventType::WELLSPEC, "WELLSPEC", "Well Specification" );
    addItem( RimWellEvent::EventType::WCONTROL, "WCONTROL", "Well Control" );
    addItem( RimWellEvent::EventType::KEYWORD, "KEYWORD", "Well Keyword" );
    addItem( RimWellEvent::EventType::SCHEDULE_KEYWORD, "SCHEDULE_KEYWORD", "Schedule Keyword" );
    addItem( RimWellEvent::EventType::RAW_TEXT, "RAW_TEXT", "Raw Text" );
    setDefault( RimWellEvent::EventType::PERF );
}
} // namespace caf

CAF_PDM_ABSTRACT_SOURCE_INIT( RimWellEvent, "WellEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::RimWellEvent()
{
    CAF_PDM_InitScriptableObject( "Well Event", "", "", "WellEvent" );

    CAF_PDM_InitField( &m_eventDate, "EventDate", QDateTime(), "Event Date" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "Well Path" );
    CAF_PDM_InitScriptableField( &m_comment, "Comment", QString(), "Comment" );
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
QString RimWellEvent::comment() const
{
    return m_comment();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEvent::setComment( const QString& comment )
{
    m_comment = comment;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEvent::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_wellPath );
    uiOrdering.add( &m_eventDate );
    uiOrdering.add( &m_comment );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimWellEvent::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_wellPath )
    {
        RimTools::wellPathOptionItems( &options );
    }

    return options;
}
