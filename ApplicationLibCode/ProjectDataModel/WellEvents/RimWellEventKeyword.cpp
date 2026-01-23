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

#include "RiaLogging.h"
#include "RimWellEventKeywordItem.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiTreeOrdering.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckOutput.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"

#include <sstream>

CAF_PDM_SOURCE_INIT( RimWellEventKeyword, "WellEventKeyword" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellEventKeyword::RimWellEventKeyword()
{
    CAF_PDM_InitScriptableObject( "Well Keyword Event", "", "", "WellEventKeyword" );

    CAF_PDM_InitScriptableField( &m_keywordName, "KeywordName", QString(), "Keyword Name" );
    CAF_PDM_InitFieldNoDefault( &m_items, "Items", "Items" );

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
Opm::DeckKeyword RimWellEventKeyword::toDeckKeyword() const
{
    QString     keyword     = m_keywordName().toUpper();
    std::string keywordName = keyword.toStdString();

    // Generic approach - works for any OPM keyword
    try
    {
        Opm::Parser               parser; // Has all builtin keywords registered
        const Opm::ParserKeyword& parserKw = parser.getKeyword( keywordName );
        Opm::DeckKeyword          kw( parserKw );

        // Build items from stored data
        std::vector<Opm::DeckItem> deckItems;
        for ( const auto& item : m_items )
        {
            if ( item.notNull() )
            {
                deckItems.push_back( item->toDeckItem() );
            }
        }

        if ( !deckItems.empty() )
        {
            kw.addRecord( Opm::DeckRecord{ std::move( deckItems ) } );
        }

        return kw;
    }
    catch ( const std::exception& e )
    {
        // Unknown keyword or error
        RiaLogging::error( QString( "Failed to create keyword '%1': %2" ).arg( keyword ).arg( e.what() ) );
        return Opm::DeckKeyword();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellEventKeyword::generateScheduleKeyword( const QString& wellName ) const
{
    Opm::DeckKeyword keyword = toDeckKeyword();

    // Return empty string if keyword creation failed
    if ( keyword.name().empty() )
    {
        return "";
    }

    // Use OPM DeckOutput to format the keyword
    std::ostringstream oss;
    Opm::DeckOutput    out( oss, 10 ); // 10 = precision
    keyword.write( out );
    return QString::fromStdString( oss.str() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_keywordName );
    uiOrdering.add( &m_items );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellEventKeyword::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName )
{
    uiTreeOrdering.add( &m_items );
}
