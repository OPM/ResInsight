/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimWellEventRawText.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTextEditor.h"
#include "cafPdmUiTreeOrdering.h"

namespace caf
{
template <>
void AppEnum<RimWellEventRawText::Placement>::setUp()
{
    addItem( RimWellEventRawText::Placement::AFTER_DATE, "AFTER_DATE", "After Date" );
    addItem( RimWellEventRawText::Placement::BEFORE_KEYWORD, "BEFORE_KEYWORD", "Before Keyword" );
    addItem( RimWellEventRawText::Placement::AFTER_KEYWORD, "AFTER_KEYWORD", "After Keyword" );
    addItem( RimWellEventRawText::Placement::END_OF_DATE, "END_OF_DATE", "End of Date" );
    setDefault( RimWellEventRawText::Placement::AFTER_DATE );
}
} // namespace caf

CAF_PDM_SOURCE_INIT( RimWellEventRawText, "RawTextEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventRawText::RimWellEventRawText()
{
    CAF_PDM_InitScriptableObject( "Raw Text Event", "", "", "RawTextEvent" );

    CAF_PDM_InitScriptableField( &m_text, "Text", QString(), "Text" );
    m_text.uiCapability()->setUiEditorTypeName( caf::PdmUiTextEditor::uiEditorTypeName() );
    CAF_PDM_InitScriptableField( &m_placement, "Placement", Placement::AFTER_DATE, "Placement" );
    CAF_PDM_InitScriptableField( &m_anchorKeyword, "AnchorKeyword", QString(), "Anchor Keyword" );
    CAF_PDM_InitScriptableField( &m_priority, "Priority", 0, "Priority" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventRawText::~RimWellEventRawText()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventRawText::text() const
{
    return m_text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventRawText::setText( const QString& text )
{
    m_text = text;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventRawText::Placement RimWellEventRawText::placement() const
{
    return m_placement();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventRawText::setPlacement( Placement placement )
{
    m_placement = placement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventRawText::anchorKeyword() const
{
    return m_anchorKeyword();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventRawText::setAnchorKeyword( const QString& keyword )
{
    m_anchorKeyword = keyword.toUpper();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimWellEventRawText::priority() const
{
    return m_priority();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventRawText::setPriority( int priority )
{
    m_priority = priority;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEvent::EventType RimWellEventRawText::eventType() const
{
    return EventType::RAW_TEXT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventRawText::generateScheduleKeyword( const QString& wellName ) const
{
    return m_text();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventRawText::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_eventDate );
    uiOrdering.add( &m_placement );
    uiOrdering.add( &m_anchorKeyword );
    uiOrdering.add( &m_priority );
    uiOrdering.add( &m_text );
    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventRawText::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    setUiName( QString( "Raw Text %1" ).arg( m_eventDate().toString( "yyyy-MM-dd" ) ) );
    uiTreeOrdering.skipRemainingChildren( true );
}
