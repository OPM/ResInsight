/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020- Equinor ASA
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

#include "RiaOpmParserTools.h"

#include "cafPdmUiItem.h"
#include "cafUtils.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/I.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/V.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::VFPInjTable> RiaOpmParserTools::extractVfpInjectionTables( const std::string& filename )
{
    std::vector<Opm::VFPInjTable> tables;

    try
    {
        Opm::Parser                           parser( false );
        const ::Opm::ParserKeywords::VFPINJ   kw1;
        const ::Opm::ParserKeywords::VFPIDIMS kw2;

        parser.addParserKeyword( kw1 );
        parser.addParserKeyword( kw2 );

        auto deck = parser.parseFile( filename );

        std::string myKeyword   = "VFPINJ";
        auto        keywordList = deck.getKeywordList( myKeyword );

        for ( auto kw : keywordList )
        {
            auto name = kw->name();

            Opm::UnitSystem unitSystem;
            {
                const auto& header = kw->getRecord( 0 );

                if ( header.getItem<Opm::ParserKeywords::VFPINJ::UNITS>().hasValue( 0 ) )
                {
                    std::string units_string;
                    units_string = header.getItem<Opm::ParserKeywords::VFPINJ::UNITS>().get<std::string>( 0 );
                    unitSystem   = Opm::UnitSystem( units_string );
                }
            }

            Opm::VFPInjTable table( *kw, unitSystem );
            tables.push_back( table );
        }
    }
    catch ( ... )
    {
    }

    return tables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<Opm::VFPProdTable> RiaOpmParserTools::extractVfpProductionTables( const std::string& filename )
{
    std::vector<Opm::VFPProdTable> tables;

    try
    {
        Opm::Parser                          parser( false );
        const ::Opm::ParserKeywords::VFPPROD kw1;

        parser.addParserKeyword( kw1 );

        auto deck = parser.parseFile( filename );

        std::string myKeyword   = "VFPPROD";
        auto        keywordList = deck.getKeywordList( myKeyword );

        for ( auto kw : keywordList )
        {
            auto name = kw->name();

            Opm::UnitSystem unitSystem;
            {
                const auto& header = kw->getRecord( 0 );

                if ( header.getItem<Opm::ParserKeywords::VFPPROD::UNITS>().hasValue( 0 ) )
                {
                    std::string units_string;
                    units_string = header.getItem<Opm::ParserKeywords::VFPPROD::UNITS>().get<std::string>( 0 );
                    unitSystem   = Opm::UnitSystem( units_string );
                }
            }

            bool              gaslift_opt_active = false;
            Opm::VFPProdTable table( *kw, gaslift_opt_active, unitSystem );
            tables.push_back( table );
        }
    }
    catch ( ... )
    {
    }

    return tables;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::pair<int, int>>> RiaOpmParserTools::extractWseglink( const std::string& filename )
{
    if ( !std::filesystem::exists( filename ) ) return {};

    Opm::Parser                         parser( false );
    const Opm::ParserKeywords::WSEGLINK kw1;
    const Opm::ParserKeywords::INCLUDE  kw2;

    parser.addParserKeyword( kw1 );
    parser.addParserKeyword( kw2 );

    std::stringstream ss;
    Opm::ParseContext parseContext( Opm::InputError::Action::WARN );
    auto              deck = parser.parseFile( filename, parseContext );

    std::string keyword     = "WSEGLINK";
    auto        keywordList = deck.getKeywordList( keyword );
    if ( keywordList.empty() ) return {};

    std::map<std::string, std::vector<std::pair<int, int>>> wseglink;
    for ( auto kw : keywordList )
    {
        auto name = kw->name();

        for ( size_t i = 0; i < kw->size(); i++ )
        {
            auto deckRecord = kw->getRecord( i );

            std::string wellName;
            int         segment1 = -1;
            int         segment2 = -1;

            {
                auto itemName = ::Opm::ParserKeywords::WSEGLINK::WELL::itemName;
                if ( deckRecord.hasItem( itemName ) && deckRecord.getItem( itemName ).hasValue( 0 ) )
                {
                    wellName = deckRecord.getItem( itemName ).getTrimmedString( 0 );
                }
            }
            {
                auto itemName = ::Opm::ParserKeywords::WSEGLINK::SEGMENT1::itemName;
                if ( deckRecord.hasItem( itemName ) && deckRecord.getItem( itemName ).hasValue( 0 ) )
                {
                    segment1 = deckRecord.getItem( itemName ).get<int>( 0 );
                }
            }
            {
                auto itemName = ::Opm::ParserKeywords::WSEGLINK::SEGMENT2::itemName;
                if ( deckRecord.hasItem( itemName ) && deckRecord.getItem( itemName ).hasValue( 0 ) )
                {
                    segment2 = deckRecord.getItem( itemName ).get<int>( 0 );
                }
            }

            if ( segment1 != -1 && segment2 != -1 )
            {
                wseglink[wellName].push_back( std::make_pair( segment1, segment2 ) );
            }
        }
    }

    return wseglink;
}
