/*
  Copyright 2020 Statoil ASA.

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


#define BOOST_TEST_MODULE PAvgTests

#include <exception>
#include <boost/test/unit_test.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Schedule/Well/PAvg.hpp>
#include <opm/input/eclipse/Schedule/Well/PAvgCalculatorCollection.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/common/utility/Serializer.hpp>

#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>

using namespace Opm;

BOOST_AUTO_TEST_CASE(DEFAULT_PAVG) {
    PAvg pavg;
    BOOST_CHECK_EQUAL(pavg.inner_weight(), 0.50);
    BOOST_CHECK_EQUAL(pavg.conn_weight(), 1.00);
    BOOST_CHECK_EQUAL(pavg.use_porv(), false);
    BOOST_CHECK(pavg.depth_correction() == PAvg::DepthCorrection::WELL);
    BOOST_CHECK(pavg.open_connections());
}


void invalid_deck(const std::string& deck_string, const std::string& kw) {
    Parser parser;
    auto deck = parser.parseString(deck_string);
    BOOST_CHECK_THROW( PAvg(deck[kw].back().getRecord(0)), std::exception );
}

void valid_deck(const std::string& deck_string, const std::string& kw) {
    Parser parser;
    auto deck = parser.parseString(deck_string);
    BOOST_CHECK_NO_THROW( PAvg(deck[kw].back().getRecord(0)));
}


BOOST_AUTO_TEST_CASE(PAVG_FROM_DECK) {
    std::string invalid_deck1 = R"(
WPAVE
   2*  Well /

WWPAVE
   W 2*  Well /
/
)";

    std::string invalid_deck2 = R"(
WPAVE
   2*  WELL all /

WWPAVE
   W 2*  WELL all /
/
)";

    std::string valid_input = R"(
WPAVE
   0.25 0.50  WELL ALL /

WWPAVE
   W 2*  WELL ALL /
/
)";
    invalid_deck(invalid_deck1, "WPAVE");
    invalid_deck(invalid_deck1, "WWPAVE");

    invalid_deck(invalid_deck2, "WPAVE");
    invalid_deck(invalid_deck2, "WWPAVE");

    valid_deck(valid_input, "WPAVE");
    valid_deck(valid_input, "WWPAVE");

    Parser parser;
    PAvg pavg( parser.parseString(valid_input)["WPAVE"].back().getRecord(0) );
    BOOST_CHECK_EQUAL( pavg.inner_weight(), 0.25);
    BOOST_CHECK_EQUAL( pavg.conn_weight(), 0.5);
    BOOST_CHECK( pavg.use_porv() );
}



bool contains(const std::vector<std::size_t>& index_list, std::size_t global_index) {
    auto find_iter = std::find(index_list.begin(), index_list.end(), global_index);
    return (find_iter != index_list.end());
}



BOOST_AUTO_TEST_CASE(WPAVE_CALCULATOR) {
    const std::string deck_string = R"(
START
7 OCT 2020 /

DIMENS
  10 10 3 /

GRID
DXV
  10*100.0 /
DYV
  10*100.0 /
DZV
  3*10.0 /

DEPTHZ
  121*2000.0 /

PERMX
  300*100.0 /
PERMY
  300*100.0 /
PERMZ
  300*10.0 /
PORO
  300*0.3 /

SCHEDULE
WELSPECS -- 0
  'P1' 'G' 5  5 2005 'LIQ' /
  'P2' 'G' 2  5 2005 'LIQ' /
  'P3' 'G' 3  5 2005 'LIQ' /
  'P4' 'G' 4  5 2005 'LIQ' /
  'P5' 'G' 1  1 2005 'LIQ' /    -- P5 is in the corner and will only have three neighbours
/

COMPDAT
  'P1' 0 0 1 3 OPEN 1 100 /
  'P5' 0 0 1 3 OPEN 1 100 /
/

TSTEP -- 1
  10
/


WPAVE   -- PAVG1
  0.75 0.25 NONE /


TSTEP -- 2
  10
/

WWPAVE
  P1 0.30 0.60 NONE /   -- PAVG2
  P3 0.40 0.70 NONE /   -- PAVG3
/


TSTEP -- 3
  10
/

WPAVE   -- PAVG4
  0.10 0.10 NONE /


TSTEP -- 4
  10
/

TSTEP -- 5
  10
/

END
)";

    const auto deck = Parser{}.parseString(deck_string);
    const auto es    = EclipseState{ deck };
    const auto grid  = es.getInputGrid();
    auto       sched = Schedule{ deck, es };
    auto       summary_config = SummaryConfig{deck, sched, es.fieldProps(), es.aquifer()};
    const auto& w1 = sched.getWell("P1", 0);
    const auto& porv = es.globalFieldProps().porv(true);
    auto calc = w1.pavg_calculator(grid, porv);

    {
        const auto& index_list = calc.index_list();
        for (std::size_t k = 0; k < 3; k++) {
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(4, 4, k)));

            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(5, 4, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(3, 4, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(4, 3, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(4, 5, k)));

            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(5, 5, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(3, 3, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(5, 3, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(3, 5, k)));
        }
    }
    BOOST_CHECK( !calc.add_pressure(grid.getGlobalIndex(6, 7, 8), 100));
    BOOST_CHECK_THROW(calc.wbp(), std::exception);

    for (std::size_t k = 0; k < 3; k++) {
        calc.add_pressure(grid.getGlobalIndex(4,4,k), 1);
    }
    BOOST_CHECK_EQUAL(calc.wbp(), 1);

    BOOST_CHECK_THROW(calc.wbp4(), std::exception);
    for (std::size_t k=0; k < 3; k++) {
        calc.add_pressure(grid.getGlobalIndex(5,4,k), 1);
        calc.add_pressure(grid.getGlobalIndex(3,4,k), 1);
        calc.add_pressure(grid.getGlobalIndex(4,5,k), 1);
        calc.add_pressure(grid.getGlobalIndex(4,3,k), 1);
    }
    BOOST_CHECK_EQUAL(calc.wbp4(), 1);
    BOOST_CHECK_EQUAL(calc.wbp5(), 1);

    //----------------------------------------------------

    const auto& w5 = sched.getWell("P5", 0);
    auto calc5 = w5.pavg_calculator(grid, porv);

    {
        const auto& index_list = calc5.index_list();
        BOOST_CHECK_EQUAL(index_list.size(), 12);
        for (std::size_t k = 0; k < 3; k++) {
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(0, 0, k)));

            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(1,0, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(0,1, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(1,1, k)));

            calc5.add_pressure(grid.getGlobalIndex(0,0,k), 1);
            calc5.add_pressure(grid.getGlobalIndex(1,0,k), 2.0);
            calc5.add_pressure(grid.getGlobalIndex(0,1,k), 2.0);
            calc5.add_pressure(grid.getGlobalIndex(1,1,k), 4.0);
        }

        BOOST_CHECK_EQUAL( calc5.wbp(), 1.0 );
        BOOST_CHECK_EQUAL( calc5.wbp4(), 2.0 );
        double inner_weight = 0.50;
        BOOST_CHECK_EQUAL( calc5.wbp5(), inner_weight * 1 + (1 - inner_weight) * (2 + 2) / 2 );
        BOOST_CHECK_EQUAL( calc5.wbp9(), inner_weight * 1 + (1 - inner_weight) * (2 + 2 + 4) / 3);
    }


    // We emulate MPI and calc1 and calc2 are on two different processors
    {
        auto calc1 = w5.pavg_calculator(grid, porv);
        auto calc2 = w5.pavg_calculator(grid, porv);
        for (std::size_t k = 0; k < 3; k++) {
            calc1.add_pressure(grid.getGlobalIndex(0,0,k), 1);
            calc2.add_pressure(grid.getGlobalIndex(1,0,k), 2.0);
            calc2.add_pressure(grid.getGlobalIndex(0,1,k), 2.0);
            calc2.add_pressure(grid.getGlobalIndex(1,1,k), 4.0);
        }
        BOOST_CHECK_THROW(calc1.wbp9(), std::exception);

        Serializer ser1;
        calc2.serialize(ser1);

        Serializer ser2(ser1.buffer);

        calc1.update(ser2);
        double inner_weight = 0.50;
        BOOST_CHECK_EQUAL( calc1.wbp5(), inner_weight * 1 + (1 - inner_weight) * 2 );
        BOOST_CHECK_EQUAL( calc1.wbp9(), inner_weight * 1 + (1 - inner_weight) * (2 * 2 + 4) / 3);
    }



    PAvgCalculatorCollection calculators;
    calculators.add(w1.pavg_calculator(grid, porv));
    calculators.add(w5.pavg_calculator(grid, porv));


    BOOST_CHECK( calculators.has("P1"));
    BOOST_CHECK( calculators.has("P5"));
    BOOST_CHECK( !calculators.has("P100"));

    {
        const auto& index_list = calculators.index_list();
        for (std::size_t k = 0; k < 3; k++) {
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(4, 4, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(5, 4, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(3, 4, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(4, 3, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(4, 5, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(5, 5, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(3, 3, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(5, 3, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(3, 5, k)));

            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(0, 0, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(1,0, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(0,1, k)));
            BOOST_CHECK(contains(index_list, grid.getGlobalIndex(1,1, k)));
        }
    }

    BOOST_CHECK_NO_THROW( calculators.add_pressure(100000000, 1) );
    for (std::size_t k = 0; k < 3; k++) {
        calculators.add_pressure(grid.getGlobalIndex(0,0,k), 1);
        calculators.add_pressure(grid.getGlobalIndex(1,0,k), 2.0);
        calculators.add_pressure(grid.getGlobalIndex(0,1,k), 2.0);
        calculators.add_pressure(grid.getGlobalIndex(1,1,k), 4.0);

        calculators.add_pressure(grid.getGlobalIndex(4,4,k), 1);
        calculators.add_pressure(grid.getGlobalIndex(5,4,k), 1);
        calculators.add_pressure(grid.getGlobalIndex(3,4,k), 1);
        calculators.add_pressure(grid.getGlobalIndex(4,5,k), 1);
        calculators.add_pressure(grid.getGlobalIndex(4,3,k), 1);
    }


    {
        const auto& c5 = calculators.get("P5");
        double inner_weight = 0.50;
        BOOST_CHECK_EQUAL( c5.wbp5(), inner_weight * 1 + (1 - inner_weight) * 2 );
        BOOST_CHECK_EQUAL( c5.wbp9(), inner_weight * 1 + (1 - inner_weight) * (2 * 2 + 4) / 3);
    }
}


BOOST_AUTO_TEST_CASE(CalcultorCollection) {
    PAvgCalculatorCollection calc_list;

    BOOST_CHECK(calc_list.empty());
}

