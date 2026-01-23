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

#include "cafAppEnum.h"

#include "opm/input/eclipse/Deck/DeckKeyword.hpp"
#include "opm/input/eclipse/Deck/DeckOutput.hpp"
#include "opm/input/eclipse/Deck/DeckRecord.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeyword.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

#include <sstream>

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

        // Build deck items from stored data
        std::vector<Opm::DeckItem> deckItems;
        for ( const auto* item : items )
        {
            switch ( item->itemType() )
            {
                case RimWellEventKeywordItem::ItemType::STRING:
                    deckItems.push_back( RifOpmDeckTools::item( item->itemName().toStdString(), item->stringValue().toStdString() ) );
                    break;
                case RimWellEventKeywordItem::ItemType::INTEGER:
                    deckItems.push_back( RifOpmDeckTools::item( item->itemName().toStdString(), item->intValue() ) );
                    break;
                case RimWellEventKeywordItem::ItemType::DOUBLE:
                    deckItems.push_back( RifOpmDeckTools::item( item->itemName().toStdString(), item->doubleValue() ) );
                    break;
            }
        }

        if ( !deckItems.empty() )
        {
            kw.addRecord( Opm::DeckRecord{ std::move( deckItems ) } );
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
        RiaLogging::error( QString( "Failed to create keyword '%1': %2" ).arg( keyword ).arg( e.what() ) );
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
