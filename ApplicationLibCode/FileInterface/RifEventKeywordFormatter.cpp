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

#include <iterator>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace
{
QString keywordToString( const Opm::DeckKeyword& kw )
{
    if ( kw.name().empty() ) return {};
    std::ostringstream oss;
    Opm::DeckOutput    out( oss, 10 );
    kw.write( out );
    return QString::fromStdString( oss.str() );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifEventKeywordFormatter::isRecordlessKeyword( const Opm::DeckKeyword& keyword )
{
    try
    {
        static const Opm::Parser parser;
        const auto&              parserKeyword = parser.getKeyword( keyword.name() );
        return parserKeyword.begin() == parserKeyword.end();
    }
    catch ( const std::exception& )
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Opm::DeckKeyword> RifEventKeywordFormatter::buildKeyword( const QString&                               keywordName,
                                                                        const std::vector<RimWellEventKeywordItem*>& items )
{
    QString     keyword = keywordName.toUpper();
    std::string kwName  = keyword.toStdString();

    try
    {
        Opm::Parser               parser;
        const Opm::ParserKeyword& parserKw = parser.getKeyword( kwName );
        Opm::DeckKeyword          kw( parserKw );

        const size_t numRecords = static_cast<size_t>( std::distance( parserKw.begin(), parserKw.end() ) );
        if ( numRecords == 0 )
        {
            if ( !items.empty() )
            {
                RiaLogging::error( std::format( "Recordless keyword '{}' does not accept items.", keyword ) );
                return std::nullopt;
            }
            return kw;
        }

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
        const bool isMnemonicList = numRecords == 1 && parserRecord.size() == 1 &&
                                    parserRecord.get( 0 ).sizeType() == Opm::ParserItem::item_size::ALL;

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
            // Positional keyword (WCONHIST, WELTARG, TUNING, ...): emit items in the schema-defined
            // order regardless of caller-supplied order (e.g. Python dict insertion order).
            std::unordered_map<std::string, const RimWellEventKeywordItem*> userItemsByName;
            userItemsByName.reserve( items.size() );
            for ( const auto* item : items )
            {
                userItemsByName.emplace( item->itemName().toStdString(), item );
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

            // Build one record's items: emit schema items up to the last one the user provided,
            // turning intermediate gaps into default markers ("1*") and dropping trailing
            // unspecified items.
            auto buildRecordItems = [&]( const Opm::ParserRecord& record )
            {
                std::optional<size_t> lastProvidedIdx;
                for ( size_t i = 0; i < record.size(); ++i )
                {
                    if ( userItemsByName.contains( record.get( i ).name() ) ) lastProvidedIdx = i;
                }

                std::vector<Opm::DeckItem> deckItems;
                if ( lastProvidedIdx.has_value() )
                {
                    deckItems.reserve( *lastProvidedIdx + 1 );
                    for ( size_t i = 0; i <= *lastProvidedIdx; ++i )
                    {
                        const std::string& name = record.get( i ).name();
                        auto               it   = userItemsByName.find( name );
                        if ( it == userItemsByName.end() )
                            deckItems.push_back( RifOpmDeckTools::defaultItem( name ) );
                        else
                            appendDeckItem( deckItems, name, it->second );
                    }
                }
                return deckItems;
            };

            // Item names known to the schema across all of the keyword's records.
            std::unordered_set<std::string> canonicalNames;
            for ( const auto& record : parserKw )
            {
                for ( size_t i = 0; i < record.size(); ++i )
                {
                    canonicalNames.insert( record.get( i ).name() );
                }
            }

            if ( numRecords > 1 )
            {
                // Multi-record keyword (e.g. TUNING): emit every record, each terminated by its own
                // '/'. Records the user did not populate are still written (as a bare '/') so the
                // record structure required by the schema is preserved.
                for ( const auto& record : parserKw )
                {
                    kw.addRecord( Opm::DeckRecord{ buildRecordItems( record ) } );
                }

                for ( const auto* item : items )
                {
                    const std::string name = item->itemName().toStdString();
                    if ( !canonicalNames.contains( name ) )
                    {
                        RiaLogging::warning(
                            std::format( "Keyword '{}': item '{}' is not part of the keyword schema and was ignored.", kwName, name ) );
                    }
                }
            }
            else
            {
                // Single-record keyword: emit the canonical block, then any non-schema items in
                // caller-supplied order.
                std::vector<Opm::DeckItem> deckItems = buildRecordItems( parserRecord );

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
        }

        if ( kw.name().empty() ) return std::nullopt;
        return kw;
    }
    catch ( const std::exception& e )
    {
        RiaLogging::error( std::format( "Failed to create keyword '{}': {}", keyword, e.what() ) );
        return std::nullopt;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Opm::DeckKeyword> RifEventKeywordFormatter::buildWconprod( const RimWellEventControl* controlEvent, const QString& wellName )
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
    return kw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Opm::DeckKeyword> RifEventKeywordFormatter::buildWconinje( const RimWellEventControl* controlEvent, const QString& wellName )
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
    return kw;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::optional<Opm::DeckKeyword> RifEventKeywordFormatter::buildWellEvent( const RimWellEvent* event, const QString& wellName )
{
    if ( event->eventType() == RimWellEvent::EventType::WCONTROL )
    {
        const auto* controlEvent = dynamic_cast<const RimWellEventControl*>( event );
        if ( controlEvent )
        {
            return controlEvent->isProducer() ? buildWconprod( controlEvent, wellName ) : buildWconinje( controlEvent, wellName );
        }
    }
    else if ( event->eventType() == RimWellEvent::EventType::KEYWORD )
    {
        const auto* keywordEvent = dynamic_cast<const RimWellEventKeyword*>( event );
        if ( keywordEvent )
        {
            return buildKeyword( keywordEvent->keywordName(), keywordEvent->items() );
        }
    }

    return std::nullopt;
}

//--------------------------------------------------------------------------------------------------
/// QString-returning wrappers: build the keyword and serialize once.
//--------------------------------------------------------------------------------------------------
QString RifEventKeywordFormatter::formatKeyword( const QString& keywordName, const std::vector<RimWellEventKeywordItem*>& items )
{
    auto kw = buildKeyword( keywordName, items );
    return kw ? keywordToString( *kw ) : QString();
}

QString RifEventKeywordFormatter::formatWconprod( const RimWellEventControl* controlEvent, const QString& wellName )
{
    auto kw = buildWconprod( controlEvent, wellName );
    return kw ? keywordToString( *kw ) : QString();
}

QString RifEventKeywordFormatter::formatWconinje( const RimWellEventControl* controlEvent, const QString& wellName )
{
    auto kw = buildWconinje( controlEvent, wellName );
    return kw ? keywordToString( *kw ) : QString();
}

QString RifEventKeywordFormatter::formatWellEvent( const RimWellEvent* event, const QString& wellName )
{
    auto kw = buildWellEvent( event, wellName );
    return kw ? keywordToString( *kw ) : QString();
}
