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

#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckItem.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/Deck/DeckRecord.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>

using namespace Opm;

static DeckPtr createDeckWithOutSolvent() {
    Opm::Parser parser;
    std::string input =
            "SCHEDULE\n"
            "WELSPECS\n"
            "     'W_1'        'OP'   2   2  1*       \'OIL\'  7* /   \n"
            "/\n"
            "COMPDAT\n"
            " 'W_1'  2*  1   1 'OPEN' / \n"
            "/\n"
            "WCONINJE\n"
            "     'W_1' 'WATER' 'OPEN' 'BHP' 1 2 3/\n/\n";

    return parser.parseString(input, ParseContext());
}

static DeckPtr createDeckWithGasInjector() {
    Opm::Parser parser;
    std::string input =
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

    return parser.parseString(input, ParseContext());
}

static DeckPtr createDeckWithDynamicWSOLVENT() {
    Opm::Parser parser;
    std::string input =
            "START             -- 0 \n"
            "1 JAN 2000 / \n"
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

    return parser.parseString(input, ParseContext());
}

static DeckPtr createDeckWithOilInjector() {
    Opm::Parser parser;
    std::string input =
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

    return parser.parseString(input, ParseContext());
}

static DeckPtr createDeckWithWaterInjector() {
    Opm::Parser parser;
    std::string input =
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

    return parser.parseString(input, ParseContext());
}
BOOST_AUTO_TEST_CASE(TestNoSolvent) {
    DeckPtr deck = createDeckWithOutSolvent();
    std::shared_ptr<const EclipseGrid> grid = std::make_shared<const EclipseGrid>(10,10,10);
    Schedule schedule(ParseContext() , grid , deck );
    BOOST_CHECK(!deck->hasKeyword("WSOLVENT"));
}

BOOST_AUTO_TEST_CASE(TestGasInjector) {
    DeckPtr deck = createDeckWithGasInjector();
    std::shared_ptr<const EclipseGrid> grid = std::make_shared<const EclipseGrid>(10,10,10);
    Schedule schedule(ParseContext(), grid , deck );
    BOOST_CHECK(deck->hasKeyword("WSOLVENT"));

}

BOOST_AUTO_TEST_CASE(TestDynamicWSOLVENT) {
    DeckPtr deck = createDeckWithDynamicWSOLVENT();
    std::shared_ptr<const EclipseGrid> grid = std::make_shared<const EclipseGrid>(10,10,10);
    Schedule schedule(ParseContext() , grid , deck );
    BOOST_CHECK(deck->hasKeyword("WSOLVENT"));
    const auto& keyword = deck->getKeyword("WSOLVENT");
    BOOST_CHECK_EQUAL(keyword.size(),1);
    const auto& record = keyword.getRecord(0);
    const std::string& wellNamesPattern = record.getItem("WELL").getTrimmedString(0);
    std::vector<WellPtr> wells_solvent = schedule.getWells(wellNamesPattern);
    BOOST_CHECK_EQUAL(wellNamesPattern, "W_1");
    BOOST_CHECK_EQUAL(wells_solvent[0]->getSolventFraction(0),0); //default 0
    BOOST_CHECK_EQUAL(wells_solvent[0]->getSolventFraction(1),1);
    BOOST_CHECK_EQUAL(wells_solvent[0]->getSolventFraction(2),1);
    BOOST_CHECK_EQUAL(wells_solvent[0]->getSolventFraction(3),0);
}

BOOST_AUTO_TEST_CASE(TestOilInjector) {
    DeckPtr deck = createDeckWithOilInjector();
    std::shared_ptr<const EclipseGrid> grid = std::make_shared<const EclipseGrid>(10,10,10);
    BOOST_CHECK_THROW (Schedule(ParseContext() , grid , deck ), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(TestWaterInjector) {
    DeckPtr deck = createDeckWithWaterInjector();
    std::shared_ptr<const EclipseGrid> grid = std::make_shared<const EclipseGrid>(10,10,10);
    BOOST_CHECK_THROW (Schedule(ParseContext(), grid , deck ), std::invalid_argument);
}
