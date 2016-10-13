/*
  Copyright (C) 2013 by Andreas Lauser

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_TEST_MODULE PvtxTableTests

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>

// generic table classes
#include <opm/parser/eclipse/EclipseState/Tables/SimpleTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PvtxTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/TableManager.hpp>

// keyword specific table classes
//#include <opm/parser/eclipse/EclipseState/Tables/PvtoTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SwofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/SgofTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/PlyadsTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/VFPProdTable.hpp>
#include <opm/parser/eclipse/EclipseState/Tables/VFPInjTable.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include <stdexcept>
#include <iostream>

using namespace Opm;


BOOST_AUTO_TEST_CASE( PvtxNumTables1 ) {
    ParserPtr parser(new Parser());
    boost::filesystem::path deckFile("testdata/integration_tests/TABLES/PVTX1.DATA");
    ParseContext parseContext;
    DeckPtr deck =  parser->parseFile(deckFile.string(), parseContext);
    BOOST_CHECK_EQUAL( PvtxTable::numTables( deck->getKeyword<ParserKeywords::PVTO>()) , 1);

    auto ranges = PvtxTable::recordRanges( deck->getKeyword<ParserKeywords::PVTO>() );
    auto range = ranges[0];
    BOOST_CHECK_EQUAL( range.first , 0 );
    BOOST_CHECK_EQUAL( range.second , 2 );
}


BOOST_AUTO_TEST_CASE( PvtxNumTables2 ) {
    ParserPtr parser(new Parser());
    boost::filesystem::path deckFile("testdata/integration_tests/TABLES/PVTO2.DATA");
    ParseContext parseContext;
    DeckPtr deck =  parser->parseFile(deckFile.string(), parseContext);
    BOOST_CHECK_EQUAL( PvtxTable::numTables( deck->getKeyword<ParserKeywords::PVTO>()) , 3);

    auto ranges = PvtxTable::recordRanges( deck->getKeyword<ParserKeywords::PVTO>() );
    auto range1 = ranges[0];
    BOOST_CHECK_EQUAL( range1.first , 0 );
    BOOST_CHECK_EQUAL( range1.second , 41 );

    auto range2 = ranges[1];
    BOOST_CHECK_EQUAL( range2.first , 42 );
    BOOST_CHECK_EQUAL( range2.second , 43 );

    auto range3 = ranges[2];
    BOOST_CHECK_EQUAL( range3.first , 44 );
    BOOST_CHECK_EQUAL( range3.second , 46 );
}

BOOST_AUTO_TEST_CASE( PvtxNumTables3 ) {
    const char *deckData =
        "TABDIMS\n"
        "1 2 /\n"
        "\n"
        "PVTO\n"
        " 1 2 3 4"
        "   5 6 7/\n"
        " 8 9 10 11 /\n"
        "/\n"
        "12 13 14 15\n"
        "   16 17 18/\n"
        "19 20 21 22/\n"
        "/\n";

    Opm::ParserPtr parser(new Opm::Parser);
    Opm::DeckConstPtr deck(parser->parseString(deckData, Opm::ParseContext()));

    auto ranges = PvtxTable::recordRanges( deck->getKeyword<ParserKeywords::PVTO>() );
    BOOST_CHECK_EQUAL( 2 ,ranges.size() );

    auto range1 = ranges[0];
    BOOST_CHECK_EQUAL( range1.first , 0 );
    BOOST_CHECK_EQUAL( range1.second , 2 );

    auto range2 = ranges[1];
    BOOST_CHECK_EQUAL( range2.first , 3 );
    BOOST_CHECK_EQUAL( range2.second , 5 );
}



BOOST_AUTO_TEST_CASE( PVTOSaturatedTable ) {
    ParserPtr parser(new Parser());
    boost::filesystem::path deckFile("testdata/integration_tests/TABLES/PVTX1.DATA");
    ParseContext parseContext;
    DeckPtr deck =  parser->parseFile(deckFile.string(), parseContext);
    Opm::TableManager tables(*deck);
    const auto& pvtoTables = tables.getPvtoTables( );
    const auto& pvtoTable = pvtoTables[0];

    const auto& saturatedTable = pvtoTable.getSaturatedTable( );
    BOOST_CHECK_EQUAL( saturatedTable.numColumns( ) , 4 );
    BOOST_CHECK_EQUAL( saturatedTable.numRows( ) , 2 );

    BOOST_CHECK_EQUAL( saturatedTable.get(0 , 0) , 20.59 );
    BOOST_CHECK_EQUAL( saturatedTable.get(0 , 1) , 28.19 );
}


BOOST_AUTO_TEST_CASE( PVTGSaturatedTable ) {
    ParserPtr parser(new Parser());
    boost::filesystem::path deckFile("testdata/integration_tests/TABLES/PVTX1.DATA");
    ParseContext parseContext;
    DeckPtr deck =  parser->parseFile(deckFile.string(), parseContext);
    Opm::TableManager tables(*deck);
    const auto& pvtgTables = tables.getPvtgTables( );
    const auto& pvtgTable = pvtgTables[0];

    const auto& saturatedTable = pvtgTable.getSaturatedTable( );
    BOOST_CHECK_EQUAL( saturatedTable.numColumns( ) , 4 );
    BOOST_CHECK_EQUAL( saturatedTable.numRows( ) , 2 );

    BOOST_CHECK_EQUAL( saturatedTable.get(1 , 0) , 0.00002448 );
    BOOST_CHECK_EQUAL( saturatedTable.get(1 , 1) , 0.00000628 );
}

