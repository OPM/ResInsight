/*
  Copyright 2015 IRIS

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

#define BOOST_TEST_MODULE WellSolventTests

#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/DeckItem.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>

using namespace Opm;

static Deck createDeckWithOutSolvent() {
    Opm::Parser parser;
    std::string input =
            "GRID\n"
            "PERMX\n"
            "   1000*0.25/\n"
            "COPY\n"
            "  PERMX PERMY /\n"
            "  PERMX PERMZ /\n"
            "/\n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     'W_1'        'OP'   2   2  1*       \'OIL\'  7* /   \n"
            "/\n"
            "COMPDAT\n"
            " 'W_1'  2*  1   1 'OPEN' / \n"
            "/\n"
            "WCONINJE\n"
            "     'W_1' 'WATER' 'OPEN' 'BHP' 1 2 3/\n/\n";

    return parser.parseString(input);
}

static Deck createDeckWithGasInjector() {
    Opm::Parser parser;
    std::string input =
            "GRID\n"
            "PERMX\n"
            "   1000*0.25/\n"
            "COPY\n"
            "  PERMX PERMY /\n"
            "  PERMX PERMZ /\n"
            "/\n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     'W_1'        'OP'   1   1  1*       \'GAS\'  7* /   \n"
            "/\n"
            "COMPDAT\n"
            " 'W_1'  2*  1   1 'OPEN' / \n"
            "/\n"
            "WCONINJE\n"
            "     'W_1' 'GAS' 'OPEN' 'BHP' 1 2 3/\n/\n"
            "WSOLVENT\n"
            "     'W_1'        1 / \n "
            "/\n";

    return parser.parseString(input);
}

static Deck createDeckWithDynamicWSOLVENT() {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "1 JAN 2000 / \n"
            "GRID\n"
            "PERMX\n"
            "   1000*0.25/\n"
            "COPY\n"
            "  PERMX PERMY /\n"
            "  PERMX PERMZ /\n"
            "/\n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     'W_1'        'OP'   1   1  1*       \'GAS\'  7* /   \n"
            "/\n"
            "COMPDAT\n"
            " 'W_1'  2*  1   1 'OPEN' / \n"
            "/\n"
            "WCONINJE\n"
            "     'W_1' 'GAS' 'OPEN' 'BHP' 1 2 3/\n/\n"
            "DATES             -- 2\n"
            " 1  MAY 2000 / \n"
            "/\n"
            "WSOLVENT\n"
            "     'W_1'        1 / \n "
            "/\n"
            "DATES             -- 3,4\n"
            " 1  JUL 2000 / \n"
            " 1  AUG 2000 / \n"
            "/\n"
            "WSOLVENT\n"
            "     'W_1'        0 / \n "
            "/\n";

    return parser.parseString(input);
}

static Deck createDeckWithOilInjector() {
    Opm::Parser parser;
    std::string input =
            "GRID\n"
            "PERMX\n"
            "   1000*0.25/\n"
            "COPY\n"
            "  PERMX PERMY /\n"
            "  PERMX PERMZ /\n"
            "/\n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     'W_1'        'OP'   2   2  1*       \'OIL\'  7* /   \n"
            "/\n"
            "COMPDAT\n"
            " 'W_1'  2*  1   1 'OPEN' / \n"
            "/\n"
            "WCONINJE\n"
            "     'W_1' 'OIL' 'OPEN' 'BHP' 1 2 3/\n/\n"
            "WSOLVENT\n"
            "     'W_1'        1 / \n "
            "/\n";

    return parser.parseString(input);
}

static Deck createDeckWithWaterInjector() {
    Opm::Parser parser;
    std::string input =
            "GRID\n"
            "PERMX\n"
            "   1000*0.25/\n"
            "COPY\n"
            "  PERMX PERMY /\n"
            "  PERMX PERMZ /\n"
            "/\n"
            "SCHEDULE\n"
            "WELSPECS\n"
            "     'W_1'        'OP'   2   2  1*       \'OIL\'  7* /   \n"
            "/\n"
            "COMPDAT\n"
            " 'W_1'  2*  1   1 'OPEN' / \n"
            "/\n"
            "WCONINJE\n"
            "     'W_1' 'WATER' 'OPEN' 'BHP' 1 2 3/\n/\n"
            "WSOLVENT\n"
            "     'W_1'        1 / \n "
            "/\n";

    return parser.parseString(input);
}
BOOST_AUTO_TEST_CASE(TestNoSolvent) {
    auto deck = createDeckWithOutSolvent();
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec(deck);
    Schedule schedule(deck, grid , fp, runspec, python);
    BOOST_CHECK(!deck.hasKeyword("WSOLVENT"));
}

BOOST_AUTO_TEST_CASE(TestGasInjector) {
    auto deck = createDeckWithGasInjector();
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec(deck);
    Schedule schedule(deck, grid , fp, runspec, python);
    BOOST_CHECK(deck.hasKeyword("WSOLVENT"));
}

BOOST_AUTO_TEST_CASE(TestDynamicWSOLVENT) {
    auto deck = createDeckWithDynamicWSOLVENT();
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec(deck);
    Schedule schedule(deck, grid , fp, runspec, python);
    BOOST_CHECK(deck.hasKeyword("WSOLVENT"));
    const auto& keyword = deck["WSOLVENT"].back();
    BOOST_CHECK_EQUAL(keyword.size(),1U);
    const auto& record = keyword.getRecord(0);
    const std::string& well_name = record.getItem("WELL").getTrimmedString(0);
    BOOST_CHECK_EQUAL(well_name, "W_1");
    BOOST_CHECK_EQUAL(schedule.getWell("W_1", 0).getSolventFraction(),0); //default 0
    BOOST_CHECK_EQUAL(schedule.getWell("W_1", 1).getSolventFraction(),1);
    BOOST_CHECK_EQUAL(schedule.getWell("W_1", 2).getSolventFraction(),1);
    BOOST_CHECK_EQUAL(schedule.getWell("W_1", 3).getSolventFraction(),0);
}

BOOST_AUTO_TEST_CASE(TestOilInjector) {
    auto deck = createDeckWithOilInjector();
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec(deck);
    BOOST_CHECK_THROW (Schedule(deck , grid , fp, runspec, python), std::exception);
}

BOOST_AUTO_TEST_CASE(TestWaterInjector) {
    auto deck = createDeckWithWaterInjector();
    auto python = std::make_shared<Python>();
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp(deck, Phases{true, true, true}, grid, table);
    Runspec runspec(deck);
    BOOST_CHECK_THROW (Schedule(deck, grid , fp, runspec, python), std::exception);
}
