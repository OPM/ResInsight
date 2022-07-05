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

#include <opm/input/eclipse/Parser/ParserKeywords/P.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

// generic table classes
#include <opm/input/eclipse/EclipseState/Tables/SimpleTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PvtxTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/TableManager.hpp>

// keyword specific table classes
//#include <opm/input/eclipse/EclipseState/Tables/PvtoTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SwofTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/SgofTable.hpp>
#include <opm/input/eclipse/EclipseState/Tables/PlyadsTable.hpp>
#include <opm/input/eclipse/Schedule/VFPProdTable.hpp>
#include <opm/input/eclipse/Schedule/VFPInjTable.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <filesystem>
#include <stdexcept>
#include <iostream>

using namespace Opm;

inline std::string prefix() {
    return boost::unit_test::framework::master_test_suite().argv[1];
}

BOOST_AUTO_TEST_CASE( PvtxNumTables1 ) {
    Parser parser;
    std::filesystem::path deckFile(prefix() + "TABLES/PVTX1.DATA");
    auto deck =  parser.parseFile(deckFile.string());
    BOOST_CHECK_EQUAL( PvtxTable::numTables( deck.get<ParserKeywords::PVTO>().back()) , 1);

    auto ranges = PvtxTable::recordRanges( deck.get<ParserKeywords::PVTO>().back() );
    auto range = ranges[0];
    BOOST_CHECK_EQUAL( range.first , 0 );
    BOOST_CHECK_EQUAL( range.second , 2 );
}


BOOST_AUTO_TEST_CASE( PvtxNumTables2 ) {
    Parser parser;
    std::filesystem::path deckFile(prefix() + "TABLES/PVTO2.DATA");
    auto deck =  parser.parseFile(deckFile.string());
    BOOST_CHECK_EQUAL( PvtxTable::numTables( deck.get<ParserKeywords::PVTO>().back()) , 3);

    auto ranges = PvtxTable::recordRanges( deck.get<ParserKeywords::PVTO>().back() );
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

    Opm::Parser parser;
    auto deck = parser.parseString(deckData);

    auto ranges = PvtxTable::recordRanges( deck.get<ParserKeywords::PVTO>().back() );
    BOOST_CHECK_EQUAL( 2 ,ranges.size() );

    auto range1 = ranges[0];
    BOOST_CHECK_EQUAL( range1.first , 0 );
    BOOST_CHECK_EQUAL( range1.second , 2 );

    auto range2 = ranges[1];
    BOOST_CHECK_EQUAL( range2.first , 3 );
    BOOST_CHECK_EQUAL( range2.second , 5 );
}



BOOST_AUTO_TEST_CASE( PVTOSaturatedTable ) {
    Parser parser;
    std::filesystem::path deckFile(prefix() + "TABLES/PVTX1.DATA");
    auto deck =  parser.parseFile(deckFile.string());
    Opm::TableManager tables(deck);
    const auto& pvtoTables = tables.getPvtoTables( );
    const auto& pvtoTable = pvtoTables[0];

    const auto& saturatedTable = pvtoTable.getSaturatedTable( );
    BOOST_CHECK_EQUAL( saturatedTable.numColumns( ) , 4 );
    BOOST_CHECK_EQUAL( saturatedTable.numRows( ) , 2 );

    BOOST_CHECK_EQUAL( saturatedTable.get(0 , 0) , 20.59 );
    BOOST_CHECK_EQUAL( saturatedTable.get(0 , 1) , 28.19 );

    {
        int num = 0;
        UnitSystem units( UnitSystem::UnitType::UNIT_TYPE_METRIC );
        for (const auto& table :  pvtoTable) {
            if (num == 0) {
                {
                    const auto& col = table.getColumn(0);
                    BOOST_CHECK_EQUAL( col.size() , 5 );
                    BOOST_CHECK_CLOSE( col[0] , units.to_si( UnitSystem::measure::pressure , 50 ) , 1e-3);
                    BOOST_CHECK_CLOSE( col[4] , units.to_si( UnitSystem::measure::pressure , 150) , 1e-3);
                }
                {
                    const auto& col = table.getColumn(2);
                    BOOST_CHECK_CLOSE( col[0] , units.to_si( UnitSystem::measure::viscosity , 1.180) , 1e-3);
                    BOOST_CHECK_CLOSE( col[4] , units.to_si( UnitSystem::measure::viscosity , 1.453) , 1e-3);
                }
            }

            if (num == 1) {
                const auto& col = table.getColumn(0);
                BOOST_CHECK_EQUAL( col.size() , 5 );
                BOOST_CHECK_CLOSE( col[0] , units.to_si( UnitSystem::measure::pressure , 70  ), 1e-3);
                BOOST_CHECK_CLOSE( col[4] , units.to_si( UnitSystem::measure::pressure , 170 ), 1e-3);
            }
            num++;
        }
        BOOST_CHECK_EQUAL( num , pvtoTable.size() );
    }
}


BOOST_AUTO_TEST_CASE( PVTGSaturatedTable ) {
    Parser parser;
    std::filesystem::path deckFile(prefix() + "TABLES/PVTX1.DATA");
    auto deck =  parser.parseFile(deckFile.string());
    Opm::TableManager tables(deck);
    const auto& pvtgTables = tables.getPvtgTables( );
    const auto& pvtgTable = pvtgTables[0];

    const auto& saturatedTable = pvtgTable.getSaturatedTable( );
    BOOST_CHECK_EQUAL( saturatedTable.numColumns( ) , 4 );
    BOOST_CHECK_EQUAL( saturatedTable.numRows( ) , 2 );

    BOOST_CHECK_EQUAL( saturatedTable.get(1 , 0) , 0.00002448 );
    BOOST_CHECK_EQUAL( saturatedTable.get(1 , 1) , 0.00000628 );
}

BOOST_AUTO_TEST_CASE( PVTWTable ) {
    const std::string input = R"(
        RUNSPEC

        DIMENS
            10 10 10 /

        TABDIMS
            1 2 /

        PROPS

        PVTW
            3600.0000 1.00341 3.00E-06 0.52341 0.00E-01 /
            3900 1 2.67E-06 0.56341 1.20E-07 /
        )";

    auto deck = Parser().parseString( input );
    TableManager tables( deck );

    const auto& pvtw = tables.getPvtwTable();

    const auto& rec1 = pvtw[0];
    const auto& rec2 = pvtw.at(1);

    BOOST_CHECK_THROW( pvtw.at(2), std::out_of_range );

    BOOST_CHECK_CLOSE( 3600.00, rec1.reference_pressure / 1e5, 1e-5 );
    BOOST_CHECK_CLOSE( 1.00341, rec1.volume_factor, 1e-5 );
    BOOST_CHECK_CLOSE( 3.0e-06, rec1.compressibility * 1e5, 1e-5 );
    BOOST_CHECK_CLOSE( 0.52341, rec1.viscosity * 1e3, 1e-5 );
    BOOST_CHECK_CLOSE( 0.0e-01, rec1.viscosibility * 1e5, 1e-5 );

    BOOST_CHECK_CLOSE( 3900,     rec2.reference_pressure / 1e5, 1e-5 );
    BOOST_CHECK_CLOSE( 1.0,      rec2.volume_factor, 1e-5 );
    BOOST_CHECK_CLOSE( 2.67e-06, rec2.compressibility * 1e5, 1e-5 );
    BOOST_CHECK_CLOSE( 0.56341,  rec2.viscosity * 1e3, 1e-5 );
    BOOST_CHECK_CLOSE( 1.20e-07, rec2.viscosibility * 1e5, 1e-5 );
}
