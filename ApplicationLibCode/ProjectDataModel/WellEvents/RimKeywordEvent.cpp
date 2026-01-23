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

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiOrdering.h"
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
void RimKeywordEvent::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_keywordName );
    uiOrdering.add( &m_items );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimKeywordEvent::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.add( &m_items );
}
