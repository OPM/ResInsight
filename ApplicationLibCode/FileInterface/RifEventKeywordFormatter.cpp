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

#include "RifEventKeywordFormatter.h"

#include "RifOpmDeckTools.h"
#include "RimWellEvent.h"
#include "RimWellEventControl.h"
#include "RimWellEventKeyword.h"
#include "RimWellEventKeywordItem.h"

#include "RiaLogging.h"
#include "RiaQStringFormatter.h"

#include "cafAppEnum.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckOutput.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserItem.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"
#include "opm/input/eclipse/Parser/ParserRecord.hpp"

#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEventKeywordFormatter::formatKeyword( const QString& keywordName, const std::vector<RimWellEventKeywordItem*>& items )
{
    QString     keyword = keywordName.toUpper();
    std::string kwName  = keyword.toStdString();

    try
    {
        Opm::Parser               parser;
        const Opm::ParserKeyword& parserKw = parser.getKeyword( kwName );
        Opm::DeckKeyword          kw( parserKw );

        const Opm::ParserRecord& parserRecord = parserKw.getRecord( 0 );

        auto stringValue = []( const RimWellEventKeywordItem* item ) -> std::string
        {
            switch ( item->itemType() )
            {
                case RimWellEventKeywordItem::ItemType::STRING:
                    return item->stringValue().toStdString();
                case RimWellEventKeywordItem::ItemType::INTEGER:
                    return std::to_string( item->intValue() );
                case RimWellEventKeywordItem::ItemType::DOUBLE:
                    return std::to_string( item->doubleValue() );
                case RimWellEventKeywordItem::ItemType::FLAG:
                    return "";
            }
            return "";
        };

        // RPTRST / RPTSCHED-style keywords have a single ALL-sized item that holds a free-form
        // list of mnemonics ("BASIC=2 DEN ROCKC ..."). Emit each user-supplied key either as
        // "KEY=VALUE" (typed value) or bare "KEY" (FLAG), all packed into one DeckItem.
        const bool isMnemonicList = parserRecord.size() == 1 && parserRecord.get( 0 ).sizeType() == Opm::ParserItem::item_size::ALL;

        if ( isMnemonicList )
        {
            std::vector<std::string> tokens;
            tokens.reserve( items.size() );
            for ( const auto* item : items )
            {
                const std::string name = item->itemName().toStdString();
                if ( item->itemType() == RimWellEventKeywordItem::ItemType::FLAG )
                {
                    tokens.push_back( name );
                }
                else
                {
                    tokens.push_back( name + "=" + stringValue( item ) );
                }
            }

            if ( !tokens.empty() )
            {
                std::vector<Opm::DeckItem> deckItems;
                deckItems.push_back( RifOpmDeckTools::rawStringItem( parserRecord.get( 0 ).name(), std::move( tokens ) ) );
                kw.addRecord( Opm::DeckRecord{ std::move( deckItems ) } );
            }
        }
        else
        {
            // Positional keyword (WCONHIST, WELTARG, ...): emit items in the schema-defined order
            // regardless of caller-supplied order (e.g. Python dict insertion order).
            std::unordered_map<std::string, const RimWellEventKeywordItem*> userItemsByName;
            userItemsByName.reserve( items.size() );
            for ( const auto* item : items )
            {
                userItemsByName.emplace( item->itemName().toStdString(), item );
            }

            // Highest canonical index actually provided by the user. Trailing unspecified items are
            // dropped, intermediate gaps become default markers ("1*").
            std::optional<size_t>           lastProvidedIdx;
            std::unordered_set<std::string> canonicalNames;
            canonicalNames.reserve( parserRecord.size() );
            for ( size_t i = 0; i < parserRecord.size(); ++i )
            {
                const std::string& name = parserRecord.get( i ).name();
                canonicalNames.insert( name );
                if ( userItemsByName.contains( name ) )
                {
                    lastProvidedIdx = i;
                }
            }

            auto appendDeckItem = [&]( std::vector<Opm::DeckItem>& out, const std::string& name, const RimWellEventKeywordItem* item )
            {
                switch ( item->itemType() )
                {
                    case RimWellEventKeywordItem::ItemType::STRING:
                        out.push_back( RifOpmDeckTools::item( name, item->stringValue().toStdString() ) );
                        break;
                    case RimWellEventKeywordItem::ItemType::INTEGER:
                        out.push_back( RifOpmDeckTools::item( name, item->intValue() ) );
                        break;
                    case RimWellEventKeywordItem::ItemType::DOUBLE:
                        out.push_back( RifOpmDeckTools::item( name, item->doubleValue() ) );
                        break;
                    case RimWellEventKeywordItem::ItemType::FLAG:
                        // FLAG only makes sense for mnemonic-list keywords; for positional schemas
                        // there is no meaningful value to emit, so render as a default marker.
                        out.push_back( RifOpmDeckTools::defaultItem( name ) );
                        break;
                }
            };

            std::vector<Opm::DeckItem> deckItems;
            if ( lastProvidedIdx.has_value() )
            {
                deckItems.reserve( *lastProvidedIdx + 1 );
                for ( size_t i = 0; i <= *lastProvidedIdx; ++i )
                {
                    const std::string& name = parserRecord.get( i ).name();
                    auto               it   = userItemsByName.find( name );
                    if ( it == userItemsByName.end() )
                    {
                        deckItems.push_back( RifOpmDeckTools::defaultItem( name ) );
                    }
                    else
                    {
                        appendDeckItem( deckItems, name, it->second );
                    }
                }
            }

            // Items not in the keyword's schema are still emitted in caller-supplied order
            // after the canonical block.
            for ( const auto* item : items )
            {
                const std::string name = item->itemName().toStdString();
                if ( canonicalNames.contains( name ) ) continue;
                appendDeckItem( deckItems, name, item );
            }

            if ( !deckItems.empty() )
            {
                kw.addRecord( Opm::DeckRecord{ std::move( deckItems ) } );
            }
        }

        // Return empty string if keyword creation failed
        if ( kw.name().empty() )
        {
            return "";
        }

        // Format with OPM DeckOutput
        std::ostringstream oss;
        Opm::DeckOutput    out( oss, 10 );
        kw.write( out );
        return QString::fromStdString( oss.str() );
    }
    catch ( const std::exception& e )
    {
        RiaLogging::error( std::format( "Failed to create keyword '{}': {}", keyword, e.what() ) );
        return "";
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEventKeywordFormatter::formatWconprod( const RimWellEventControl* controlEvent, const QString& wellName )
{
    using W = Opm::ParserKeywords::WCONPROD;

    std::vector<Opm::DeckItem> items;

    items.push_back( RifOpmDeckTools::item( W::WELL::itemName, wellName.toStdString() ) );

    QString statusStr = caf::AppEnum<RimWellEventControl::WellStatus>( controlEvent->wellStatus() ).text();
    items.push_back( RifOpmDeckTools::item( W::STATUS::itemName, statusStr.toStdString() ) );

    QString controlModeStr = caf::AppEnum<RimWellEventControl::ControlMode>( controlEvent->controlMode() ).text();
    items.push_back( RifOpmDeckTools::item( W::CMODE::itemName, controlModeStr.toStdString() ) );

    items.push_back(
        RifOpmDeckTools::optionalItem( W::ORAT::itemName,
                                       controlEvent->oilRate() > 0.0 ? std::optional<double>( controlEvent->oilRate() ) : std::nullopt ) );
    items.push_back( RifOpmDeckTools::optionalItem( W::WRAT::itemName,
                                                    controlEvent->waterRate() > 0.0 ? std::optional<double>( controlEvent->waterRate() )
                                                                                    : std::nullopt ) );
    items.push_back(
        RifOpmDeckTools::optionalItem( W::GRAT::itemName,
                                       controlEvent->gasRate() > 0.0 ? std::optional<double>( controlEvent->gasRate() ) : std::nullopt ) );
    items.push_back( RifOpmDeckTools::optionalItem( W::LRAT::itemName,
                                                    controlEvent->liquidRate() > 0.0 ? std::optional<double>( controlEvent->liquidRate() )
                                                                                     : std::nullopt ) );
    items.push_back( RifOpmDeckTools::defaultItem( W::RESV::itemName ) );
    items.push_back( RifOpmDeckTools::optionalItem( W::BHP::itemName,
                                                    controlEvent->bhpLimit() > 0.0 ? std::optional<double>( controlEvent->bhpLimit() )
                                                                                   : std::nullopt ) );
    items.push_back( RifOpmDeckTools::optionalItem( W::THP::itemName,
                                                    controlEvent->thpLimit() > 0.0 ? std::optional<double>( controlEvent->thpLimit() )
                                                                                   : std::nullopt ) );

    Opm::DeckKeyword kw{ W() };
    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );

    std::ostringstream oss;
    Opm::DeckOutput    out( oss, 10 );
    kw.write( out );
    return QString::fromStdString( oss.str() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEventKeywordFormatter::formatWconinje( const RimWellEventControl* controlEvent, const QString& wellName )
{
    using W = Opm::ParserKeywords::WCONINJE;

    std::vector<Opm::DeckItem> items;

    items.push_back( RifOpmDeckTools::item( W::WELL::itemName, wellName.toStdString() ) );
    items.push_back( RifOpmDeckTools::item( W::TYPE::itemName, "WATER" ) );

    QString statusStr = caf::AppEnum<RimWellEventControl::WellStatus>( controlEvent->wellStatus() ).text();
    items.push_back( RifOpmDeckTools::item( W::STATUS::itemName, statusStr.toStdString() ) );

    QString controlModeStr = caf::AppEnum<RimWellEventControl::ControlMode>( controlEvent->controlMode() ).text();
    items.push_back( RifOpmDeckTools::item( W::CMODE::itemName, controlModeStr.toStdString() ) );

    items.push_back( RifOpmDeckTools::optionalItem( W::RATE::itemName,
                                                    controlEvent->controlValue() > 0.0 ? std::optional<double>( controlEvent->controlValue() )
                                                                                       : std::nullopt ) );
    items.push_back( RifOpmDeckTools::defaultItem( W::RESV::itemName ) );
    items.push_back( RifOpmDeckTools::optionalItem( W::BHP::itemName,
                                                    controlEvent->bhpLimit() > 0.0 ? std::optional<double>( controlEvent->bhpLimit() )
                                                                                   : std::nullopt ) );
    items.push_back( RifOpmDeckTools::optionalItem( W::THP::itemName,
                                                    controlEvent->thpLimit() > 0.0 ? std::optional<double>( controlEvent->thpLimit() )
                                                                                   : std::nullopt ) );

    Opm::DeckKeyword kw{ W() };
    kw.addRecord( Opm::DeckRecord{ std::move( items ) } );

    std::ostringstream oss;
    Opm::DeckOutput    out( oss, 10 );
    kw.write( out );
    return QString::fromStdString( oss.str() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RifEventKeywordFormatter::formatWellEvent( const RimWellEvent* event, const QString& wellName )
{
    if ( event->eventType() == RimWellEvent::EventType::WCONTROL )
    {
        auto* controlEvent = dynamic_cast<const RimWellEventControl*>( event );
        if ( controlEvent )
        {
            if ( controlEvent->isProducer() )
            {
                return formatWconprod( controlEvent, wellName );
            }
            else
            {
                return formatWconinje( controlEvent, wellName );
            }
        }
    }
    else if ( event->eventType() == RimWellEvent::EventType::KEYWORD )
    {
        auto* keywordEvent = dynamic_cast<const RimWellEventKeyword*>( event );
        if ( keywordEvent )
        {
            return formatKeyword( keywordEvent->keywordName(), keywordEvent->items() );
        }
    }

    return {};
}
