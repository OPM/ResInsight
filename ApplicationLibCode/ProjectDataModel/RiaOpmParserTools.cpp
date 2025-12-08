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

#include "FileInterface/RifVfpInjTable.h"
#include "FileInterface/RifVfpProdTable.h"
#include "RifEclipseInputFileTools.h"

#include "cafPdmUiItem.h"
#include "cafUtils.h"

#include "opm/common/utility/OpmInputError.hpp"
#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/Parser/InputErrorAction.hpp"
#include "opm/input/eclipse/Parser/ParseContext.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/E.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/G.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/I.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/P.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/S.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/T.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/V.hpp"
#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

#include <QFile>

#include <set>

namespace RiaOpmParserTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<Opm::UnitSystem, RifVfpInjTable> createInjectionTable( const Opm::DeckKeyword& keyword )
{
    Opm::UnitSystem unitSystem;
    {
        const auto& header = keyword.getRecord( 0 );

        if ( header.getItem<Opm::ParserKeywords::VFPINJ::UNITS>().hasValue( 0 ) )
        {
            std::string unitString;
            unitString = header.getItem<Opm::ParserKeywords::VFPINJ::UNITS>().get<std::string>( 0 );
            unitSystem = Opm::UnitSystem( unitString );
        }
    }

    return { unitSystem, RifVfpInjTable( keyword ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<Opm::UnitSystem, RifVfpProdTable> createProductionTable( const Opm::DeckKeyword& keyword )
{
    Opm::UnitSystem unitSystem;
    {
        const auto& header = keyword.getRecord( 0 );

        if ( header.getItem<Opm::ParserKeywords::VFPPROD::UNITS>().hasValue( 0 ) )
        {
            std::string unitString;
            unitString = header.getItem<Opm::ParserKeywords::VFPPROD::UNITS>().get<std::string>( 0 );
            unitSystem = Opm::UnitSystem( unitString );
        }
    }

    bool gaslift_opt_active = false;

    return { unitSystem, RifVfpProdTable( keyword, gaslift_opt_active ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::tuple<Opm::UnitSystem, std::vector<RifVfpProdTable>, std::vector<RifVfpInjTable>>
    extractVfpTablesFromDataFile( const std::string& dataDeckFilename )
{
    if ( !std::filesystem::exists( dataDeckFilename ) )
        return std::tuple<Opm::UnitSystem, std::vector<RifVfpProdTable>, std::vector<RifVfpInjTable>>{};

    std::vector<RifVfpProdTable> prodTables;
    std::vector<RifVfpInjTable>  injTables;

    std::optional<Opm::UnitSystem> unitSystemFromTables;

    try
    {
        // The parser is not robust when looking for a small number of keywords. Several keywords is added as parsing error occurs.
        Opm::Parser parser( false );

        // Required to include the some keywords not related or required for VFP data to avoid paring errors causing data to be skipped.
        // TUNING caused error in a Norne model
        // GRUPTREE, WELSPECS caused error in an unknown models
        // ECHO caused error in a Verdande model
        std::vector<Opm::ParserKeyword> parserKeywords = { Opm::ParserKeywords::VFPPROD(),
                                                           Opm::ParserKeywords::VFPINJ(),
                                                           Opm::ParserKeywords::INCLUDE(),
                                                           Opm::ParserKeywords::TUNING(),
                                                           Opm::ParserKeywords::GRUPTREE(),
                                                           Opm::ParserKeywords::WELSPECS(),
                                                           Opm::ParserKeywords::SLAVES(),
                                                           Opm::ParserKeywords::ECHO() };
        for ( const auto& kw : parserKeywords )
        {
            parser.addParserKeyword( kw );
        }

        Opm::ParseContext parseContext( Opm::InputErrorAction::WARN );
        auto              deck = parser.parseFile( dataDeckFilename, parseContext );

        {
            std::string prodKeyword = "VFPPROD";
            auto        keywordList = deck.getKeywordList( prodKeyword );

            for ( auto kw : keywordList )
            {
                const auto& [tableUnitSystem, table] = createProductionTable( *kw );

                if ( !unitSystemFromTables )
                {
                    unitSystemFromTables = tableUnitSystem;
                }
                prodTables.push_back( table );
            }
        }
        {
            std::string injKeyword  = "VFPINJ";
            auto        keywordList = deck.getKeywordList( injKeyword );
            for ( auto kw : keywordList )
            {
                const auto& [tableUnitSystem, table] = createInjectionTable( *kw );

                if ( !unitSystemFromTables )
                {
                    unitSystemFromTables = tableUnitSystem;
                }
                injTables.push_back( table );
            }
        }
    }
    catch ( Opm::OpmInputError& e )
    {
        QString text = QString( "Error detected when parsing '%1'. Imported data might be missing or incomplete.\n%2" )
                           .arg( QString::fromStdString( dataDeckFilename ) )
                           .arg( QString::fromStdString( e.what() ) );

        RiaLogging::warning( text );
    }
    catch ( ... )
    {
        QString text = QString( "Error detected when parsing '%1'. Imported data might be missing or incomplete." )
                           .arg( QString::fromStdString( dataDeckFilename ) );

        RiaLogging::warning( text );
    }

    Opm::UnitSystem unitSystemValue;
    if ( unitSystemFromTables )
    {
        unitSystemValue = *unitSystemFromTables;
    }

    return std::tuple<Opm::UnitSystem, std::vector<RifVfpProdTable>, std::vector<RifVfpInjTable>>{ unitSystemValue, prodTables, injTables };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<std::string, std::vector<std::pair<int, int>>> extractWseglink( const std::string& filename )
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
    Opm::ParseContext parseContext( Opm::InputErrorAction::WARN );
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
std::vector<RiaOpmParserTools::AicdTemplateValues> extractWsegAicd( const std::string& filename )
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
        Opm::ParseContext parseContext( Opm::InputErrorAction::WARN );
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
                        // Only read out explicitly set values. If the value is defaulted, do not read out the value to make sure the string
                        // "1*" is displayed in the GUI
                        if ( !deckItem.defaultApplied( 0 ) )
                        {
                            double doubleValue            = deckItem.get<double>( 0 );
                            aicdTemplate[deckItem.name()] = doubleValue;
                        }
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
std::vector<RiaOpmParserTools::AicdTemplateValues> extractWsegAicdCompletor( const std::string& filename )
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
std::string aicdTemplateId()
{
    return "ID_NUMBER";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::PhaseType> phasesFromInteheadValue( int phaseIndicator )
{
    std::set<RiaDefines::PhaseType> phases;

    if ( phaseIndicator < 0 ) return phases;

    // Phase indicator from OPM Common specification:
    // ThirdParty\custom-opm-common\opm-common\opm\output\eclipse\VectorItems\intehead.hpp
    // PHASE = 14, Phase indicator:
    //   1: oil, 2: water, 3: O/W, 4: gas, 5: G/O, 6: G/W, 7: O/G/W
    // Use bitwise logic: oil=bit 0, water=bit 1, gas=bit 2

    if ( phaseIndicator & 1 ) // oil
    {
        phases.insert( RiaDefines::PhaseType::OIL_PHASE );
    }

    if ( phaseIndicator & 2 ) // water
    {
        phases.insert( RiaDefines::PhaseType::WATER_PHASE );
    }

    if ( phaseIndicator & 4 ) // gas
    {
        phases.insert( RiaDefines::PhaseType::GAS_PHASE );
    }

    return phases;
}

} // namespace RiaOpmParserTools
