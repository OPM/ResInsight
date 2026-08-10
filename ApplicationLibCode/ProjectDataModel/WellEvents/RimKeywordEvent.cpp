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
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
//  A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimKeywordEvent.h"

#include "RimWellEventKeywordItem.h"

#include "FileInterface/RifEventKeywordFormatter.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimKeywordEvent, "KeywordEvent" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordEvent::RimKeywordEvent()
{
    CAF_PDM_InitScriptableObject( "Keyword Event", "", "", "KeywordEvent" );

    CAF_PDM_InitScriptableField( &m_keywordName, "KeywordName", QString(), "Keyword Name" );
    CAF_PDM_InitFieldNoDefault( &m_items, "Items", "Items" );
    m_items.uiCapability()->setUiEditorTypeName( caf::PdmUiTableViewEditor::uiEditorTypeName() );
    m_items.uiCapability()->setUiLabelPosition( caf::PdmUiItemInfo::LabelPosition::TOP );
    m_items.uiCapability()->setAttribute( caf::PdmUiTableViewEditor::Keys::RESIZE_POLICY,
                                          static_cast<int>( caf::PdmUiTableViewEditorAttribute::RESIZE_TO_FILL_CONTAINER ) );
    m_items.uiCapability()->setAttribute( caf::PdmUiTableViewEditor::Keys::ALWAYS_ENFORCE_RESIZE_POLICY, true );
    m_items.uiCapability()->setAttribute( caf::PdmUiTableViewEditor::Keys::MINIMUM_HEIGHT, 200 );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimKeywordEvent::~RimKeywordEvent()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimKeywordEvent::keywordName() const
{
    return m_keywordName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordEvent::setKeywordName( const QString& keyword )
{
    m_keywordName = keyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordEvent::addStringItem( const QString& name, const QString& value )
{
    auto* item = new RimWellEventKeywordItem();
    item->setItemName( name );
    item->setStringValue( value );
    m_items.push_back( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordEvent::addIntItem( const QString& name, int value )
{
    auto* item = new RimWellEventKeywordItem();
    item->setItemName( name );
    item->setIntValue( value );
    m_items.push_back( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordEvent::addDoubleItem( const QString& name, double value )
{
    auto* item = new RimWellEventKeywordItem();
    item->setItemName( name );
    item->setDoubleValue( value );
    m_items.push_back( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordEvent::addFlagItem( const QString& name )
{
    auto* item = new RimWellEventKeywordItem();
    item->setItemName( name );
    item->setFlag();
    m_items.push_back( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellEventKeywordItem*> RimKeywordEvent::items() const
{
    std::vector<RimWellEventKeywordItem*> result;
    for ( const auto& item : m_items )
    {
        if ( item.notNull() )
        {
            result.push_back( item.p() );
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimKeywordEvent::generateScheduleKeyword( const QString& wellName ) const
{
    return RifEventKeywordFormatter::formatKeyword( m_keywordName(), items() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Opm::DeckKeyword> RimKeywordEvent::generateDeckKeyword( const QString& wellName ) const
{
    return RifEventKeywordFormatter::buildKeyword( m_keywordName(), items() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordEvent::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    // Schedule-level keyword events are not tied to a well, so the inherited well path field is not shown
    uiOrdering.add( &m_eventDate );
    uiOrdering.add( &m_keywordName );
    uiOrdering.add( &m_items );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordEvent::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    setUiName( QString( "%1 %2" ).arg( m_keywordName() ).arg( m_eventDate().toString( "yyyy-MM-dd" ) ) );

    uiTreeOrdering.skipRemainingChildren( true );
}
