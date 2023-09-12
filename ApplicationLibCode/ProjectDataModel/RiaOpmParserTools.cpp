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
#include "RiaLogging.h"
#include "RiaTextStringTools.h"

#include "RifEclipseInputFileTools.h"

#include "cafPdmUiItem.h"
#include "cafUtils.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/I.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/P.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/V.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

#include <QFile>

#include <set>

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

    Opm::Parser parser( false );

    const Opm::ParserKeywords::WSEGLINK kw1;
    const Opm::ParserKeywords::INCLUDE  kw2;
    const Opm::ParserKeywords::PATHS    kw3;

    parser.addParserKeyword( kw1 );
    parser.addParserKeyword( kw2 );
    parser.addParserKeyword( kw3 );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaOpmParserTools::AicdTemplateValues> RiaOpmParserTools::extractWsegAicd( const std::string& filename )
{
    if ( !std::filesystem::exists( filename ) ) return {};

    try
    {
        Opm::Parser parser( false );

        const Opm::ParserKeywords::WSEGAICD kw1;
        const Opm::ParserKeywords::INCLUDE  kw2;
        const Opm::ParserKeywords::PATHS    kw3;

        parser.addParserKeyword( kw1 );
        parser.addParserKeyword( kw2 );
        parser.addParserKeyword( kw3 );

        std::stringstream ss;
        Opm::ParseContext parseContext( Opm::InputError::Action::WARN );
        auto              deck = parser.parseFile( filename, parseContext );

        const std::string keyword     = "WSEGAICD";
        auto              keywordList = deck.getKeywordList( keyword );
        if ( keywordList.empty() ) return {};

        using namespace Opm::ParserKeywords;
        std::set<std::string> keywordsToExtract = { WSEGAICD::STRENGTH::itemName,
                                                    WSEGAICD::DENSITY_CALI::itemName,
                                                    WSEGAICD::VISCOSITY_CALI::itemName,
                                                    WSEGAICD::FLOW_RATE_EXPONENT::itemName,
                                                    WSEGAICD::VISC_EXPONENT::itemName,
                                                    WSEGAICD::CRITICAL_VALUE::itemName,
                                                    WSEGAICD::MAX_ABS_RATE::itemName,
                                                    WSEGAICD::OIL_FLOW_FRACTION::itemName,
                                                    WSEGAICD::WATER_FLOW_FRACTION::itemName,
                                                    WSEGAICD::GAS_FLOW_FRACTION::itemName,
                                                    WSEGAICD::OIL_VISC_FRACTION::itemName,
                                                    WSEGAICD::WATER_VISC_FRACTION::itemName,
                                                    WSEGAICD::GAS_VISC_FRACTION::itemName };

        std::vector<RiaOpmParserTools::AicdTemplateValues> aicdData;
        for ( const auto& kw : keywordList )
        {
            auto name = kw->name();

            for ( size_t kwIndex = 0; kwIndex < kw->size(); kwIndex++ )
            {
                RiaOpmParserTools::AicdTemplateValues aicdTemplate;

                auto deckRecord = kw->getRecord( kwIndex );
                auto numItems   = deckRecord.size();
                for ( size_t deckIndex = 0; deckIndex < numItems; deckIndex++ )
                {
                    auto deckItem = deckRecord.getItem( deckIndex );
                    if ( !deckItem.hasValue( 0 ) ) continue;
                    if ( !keywordsToExtract.contains( deckItem.name() ) ) continue;

                    auto typeTag = deckItem.getType();
                    if ( typeTag == Opm::type_tag::fdouble )
                    {
                        double doubleValue            = deckItem.get<double>( 0 );
                        aicdTemplate[deckItem.name()] = doubleValue;
                    }
                }

                aicdData.push_back( aicdTemplate );
            }
        }

        return aicdData;
    }
    catch ( std::exception& e )
    {
        RiaLogging::error( e.what() );
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaOpmParserTools::AicdTemplateValues> RiaOpmParserTools::extractWsegAicdCompletor( const std::string& filename )
{
    QFile file( QString::fromStdString( filename ) );
    if ( !file.open( QFile::ReadOnly ) ) return {};

    std::vector<RiaOpmParserTools::AicdTemplateValues> aicdTemplates;

    const QString keyword( "WSEGAICD" );
    const QString keywordToStopParsing;

    auto keywordContent = RifEclipseInputFileTools::readKeywordContentFromFile( keyword, keywordToStopParsing, file );
    for ( const auto& s : keywordContent )
    {
        auto                wordsInLine = RiaTextStringTools::splitSkipEmptyParts( s );
        std::vector<double> values;
        for ( const auto& word : wordsInLine )
        {
            bool ok          = false;
            auto doubleValue = word.toDouble( &ok );
            if ( ok ) values.push_back( doubleValue );
        }

        // Completor exports the values in the following format, 12 values per row
        // WSEGAICD
        // --Number   Alpha           x        y        a     b     c     d        e        f        rhocal   viscal
        // 1          0.000017253     3.05     0.67     1     1     1     2.43     1.18     10.0     1000     1
        // 2          0.000027253     5.05     0.53     1     1     1     2.43     1.28     7.02     1000     1
        //
        if ( values.size() == 12 )
        {
            RiaOpmParserTools::AicdTemplateValues aicdValues;
            aicdValues[aicdTemplateId()] = values[0];

            aicdValues[Opm::ParserKeywords::WSEGAICD::STRENGTH::itemName]            = values[1]; // Alpha
            aicdValues[Opm::ParserKeywords::WSEGAICD::FLOW_RATE_EXPONENT::itemName]  = values[2]; // x
            aicdValues[Opm::ParserKeywords::WSEGAICD::VISC_EXPONENT::itemName]       = values[3]; // y
            aicdValues[Opm::ParserKeywords::WSEGAICD::OIL_FLOW_FRACTION::itemName]   = values[4]; // a
            aicdValues[Opm::ParserKeywords::WSEGAICD::WATER_FLOW_FRACTION::itemName] = values[5]; // b
            aicdValues[Opm::ParserKeywords::WSEGAICD::GAS_FLOW_FRACTION::itemName]   = values[6]; // c
            aicdValues[Opm::ParserKeywords::WSEGAICD::OIL_VISC_FRACTION::itemName]   = values[7]; // d
            aicdValues[Opm::ParserKeywords::WSEGAICD::WATER_VISC_FRACTION::itemName] = values[8]; // e
            aicdValues[Opm::ParserKeywords::WSEGAICD::GAS_VISC_FRACTION::itemName]   = values[9]; // f
            aicdValues[Opm::ParserKeywords::WSEGAICD::DENSITY_CALI::itemName]        = values[10]; // rhocal
            aicdValues[Opm::ParserKeywords::WSEGAICD::VISCOSITY_CALI::itemName]      = values[11]; // viscal

            aicdTemplates.push_back( aicdValues );
        }
    }

    return aicdTemplates;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RiaOpmParserTools::aicdTemplateId()
{
    return "ID_NUMBER";
}
