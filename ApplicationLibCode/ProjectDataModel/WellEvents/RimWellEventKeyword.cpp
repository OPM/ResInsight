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

#include "RimWellEventKeyword.h"

#include "RimWellEventKeywordItem.h"

#include "FileInterface/RifEventKeywordFormatter.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTableViewEditor.h"
#include "cafPdmUiTreeOrdering.h"

CAF_PDM_SOURCE_INIT( RimWellEventKeyword, "WellEventKeyword" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeyword::RimWellEventKeyword()
{
    CAF_PDM_InitScriptableObject( "Well Keyword Event", "", "", "WellEventKeyword" );

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
RimWellEventKeyword::~RimWellEventKeyword()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventKeyword::keywordName() const
{
    return m_keywordName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::setKeywordName( const QString& keyword )
{
    m_keywordName = keyword;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::addStringItem( const QString& name, const QString& value )
{
    auto* item = new RimWellEventKeywordItem();
    item->setItemName( name );
    item->setStringValue( value );
    m_items.push_back( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::addIntItem( const QString& name, int value )
{
    auto* item = new RimWellEventKeywordItem();
    item->setItemName( name );
    item->setIntValue( value );
    m_items.push_back( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::addDoubleItem( const QString& name, double value )
{
    auto* item = new RimWellEventKeywordItem();
    item->setItemName( name );
    item->setDoubleValue( value );
    m_items.push_back( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::addFlagItem( const QString& name )
{
    auto* item = new RimWellEventKeywordItem();
    item->setItemName( name );
    item->setFlag();
    m_items.push_back( item );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellEventKeywordItem*> RimWellEventKeyword::items() const
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
QString RimWellEventKeyword::generateScheduleKeyword( const QString& wellName ) const
{
    return RifEventKeywordFormatter::formatKeyword( m_keywordName(), items() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Opm::DeckKeyword> RimWellEventKeyword::generateDeckKeyword( const QString& wellName ) const
{
    return RifEventKeywordFormatter::buildKeyword( m_keywordName(), items() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimWellEvent::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_keywordName );
    uiOrdering.add( &m_items );

    uiOrdering.skipRemainingFields();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    setUiName( QString( "%1 %2 [%3]" ).arg( m_keywordName() ).arg( m_eventDate().toString( "yyyy-MM-dd" ) ).arg( wellName() ) );

    uiTreeOrdering.skipRemainingChildren( true );
}
