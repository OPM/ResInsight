/*
  Copyright 2020 Equinor ASA.

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

#define BOOST_TEST_MODULE NetworkTests

#include <boost/test/unit_test.hpp>

#include <algorithm>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Schedule/Network/ExtNetwork.hpp>
#include <opm/input/eclipse/Schedule/Network/Node.hpp>
#include <opm/input/eclipse/Schedule/Network/Branch.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

using namespace Opm;

namespace {
Schedule make_schedule(const std::string& schedule_string) {
    Parser parser;
    auto python = std::make_shared<Python>();
    Deck deck = parser.parseString(schedule_string);
    EclipseGrid grid(10,10,10);
    TableManager table ( deck );
    FieldPropsManager fp( deck, Phases{true, true, true}, grid, table);
    Runspec runspec (deck);
    Schedule schedule(deck, grid , fp, runspec, python);
    return schedule;
}
}


BOOST_AUTO_TEST_CASE(CreateNetwork) {
    Network::ExtNetwork network;
    BOOST_CHECK( !network.active() );
    auto schedule = make_schedule("SCHEDULE\n");
    auto network2 = schedule[0].network.get();
    BOOST_CHECK( !network2.active() );
}



BOOST_AUTO_TEST_CASE(Branch) {
    BOOST_CHECK_THROW( Network::Branch("down", "up", 100, Network::Branch::AlqEQ::ALQ_INPUT), std::logic_error);
}


BOOST_AUTO_TEST_CASE(INVALID_DOWNTREE_NODE) {
    std::string deck_string = R"(
SCHEDULE

GRUPTREE
 'PROD'    'FIELD' /

 'M5S'    'PLAT-A'  /
 'M5N'    'PLAT-A'  /

 'C1'     'M5N'  /
 'F1'     'M5N'  /
 'B1'     'M5S'  /
 'G1'     'M5S'  /
/

BRANPROP
--  Downtree  Uptree   #VFP    ALQ
    B1X        PLAT-A    5      1*      /
    C1         PLAT-A    4      1*      /
/

NODEPROP
--  Node_name Pr    autoChock?      addGasLift?     Group_name
     PLAT-A 21.0   NO     NO    1*  /
     B1    1*  NO     NO    1*  /
     C1    1*  NO     NO    1*  /
/
)";

    BOOST_CHECK_THROW( make_schedule(deck_string), std::exception);
}


BOOST_AUTO_TEST_CASE(INVALID_UPTREE_NODE) {
    std::string deck_string = R"(
SCHEDULE

GRUPTREE
 'PROD'    'FIELD' /

 'M5S'    'PLAT-A'  /
 'M5N'    'PLAT-A'  /

 'C1'     'M5N'  /
 'F1'     'M5N'  /
 'B1'     'M5S'  /
 'G1'     'M5S'  /
/

BRANPROP
--  Downtree  Uptree   #VFP    ALQ
    B1         PLAT-AX    5      1*      /
    C1         PLAT-AX    4      1*      /
/

NODEPROP
--  Node_name Pr    autoChock?      addGasLift?     Group_name
     PLAT-A 21.0   NO     NO    1*  /
     B1    1*  NO     NO    1*  /
     C1    1*  NO     NO    1*  /
/
)";

    BOOST_CHECK_THROW( make_schedule(deck_string), std::exception);
}

BOOST_AUTO_TEST_CASE(INVALID_VFP_NODE) {
    std::string deck_string = R"(
SCHEDULE

GRUPTREE
 'PROD'    'FIELD' /

 'M5S'    'PLAT-A'  /
 'M5N'    'PLAT-A'  /

 'C1'     'M5N'  /
 'F1'     'M5N'  /
 'B1'     'M5S'  /
 'G1'     'M5S'  /
/

BRANPROP
--  Downtree  Uptree   #VFP    ALQ
    B1         PLAT-A    5      1*      /
    C1         PLAT-A    4      1*      /  --This is a choke branch - must have VFP=9999
/

NODEPROP
--  Node_name Pr    autoChock?      addGasLift?     Group_name
     PLAT-A 21.0   NO     NO    1*  /
     B1    1*  NO     NO    1*  /
     C1    1*  YES    NO    1*  /
/
)";

    BOOST_CHECK_THROW( make_schedule(deck_string), std::exception);
}

BOOST_AUTO_TEST_CASE(OK) {
    std::string deck_string = R"(
SCHEDULE

GRUPTREE
 'PROD'    'FIELD' /

 'M5S'    'PLAT-A'  /
 'M5N'    'PLAT-A'  /

 'C1'     'M5N'  /
 'F1'     'M5N'  /
 'B1'     'M5S'  /
 'G1'     'M5S'  /
/

BRANPROP
--  Downtree  Uptree   #VFP    ALQ
    B1         PLAT-A    9999      1*      /
    C1         PLAT-A    9999      1*      /
/

NODEPROP
--  Node_name Pr    autoChock?      addGasLift?     Group_name
     PLAT-A 21.0   NO     NO    1*  /
     B1    1*  YES      NO    1*  /
     C1    1*  YES     NO     'GROUP' /
/

TSTEP
  10 /

BRANPROP
--  Downtree  Uptree   #VFP    ALQ
    C1         PLAT-A    0 1*      /
/


)";

    auto sched = make_schedule(deck_string);
    {
        const auto& network = sched[0].network.get();
        const auto& b1 = network.node("B1");
        BOOST_CHECK(b1.as_choke());
        BOOST_CHECK(!b1.add_gas_lift_gas());
        BOOST_CHECK(b1.name() == b1.target_group());
        BOOST_CHECK(!b1.terminal_pressure());

        const auto& p = network.node("PLAT-A");
        BOOST_CHECK(p.terminal_pressure());
        BOOST_CHECK_EQUAL(p.terminal_pressure().value(), 21 * 100000);
        BOOST_CHECK(p == network.root());

        BOOST_CHECK_THROW(network.node("NO_SUCH_NODE"), std::out_of_range);



        BOOST_CHECK_EQUAL(network.downtree_branches("PLAT-A").size(), 2U);
        for (const auto& b : network.downtree_branches("PLAT-A")) {
            BOOST_CHECK_EQUAL(b.uptree_node(), "PLAT-A");
            BOOST_CHECK(b.downtree_node() == "B1" || b.downtree_node() == "C1");
        }


        const auto& platform_uptree = network.uptree_branch("PLAT-A");
        BOOST_CHECK(!platform_uptree.has_value());

        const auto& B1_uptree = network.uptree_branch("B1");
        BOOST_CHECK(B1_uptree.has_value());
        BOOST_CHECK_EQUAL(B1_uptree->downtree_node(), "B1");
        BOOST_CHECK_EQUAL(B1_uptree->uptree_node(), "PLAT-A");

        BOOST_CHECK(network.active());
    }
    {
        const auto& network = sched[1].network.get();
        const auto& b1 = network.node("B1");
        BOOST_CHECK(b1.as_choke());
        BOOST_CHECK(!b1.add_gas_lift_gas());
        BOOST_CHECK(b1.name() == b1.target_group());
        BOOST_CHECK(!b1.terminal_pressure());

        BOOST_CHECK_EQUAL(network.downtree_branches("PLAT-A").size(), 1U);
        for (const auto& b : network.downtree_branches("PLAT-A")) {
            BOOST_CHECK_EQUAL(b.uptree_node(), "PLAT-A");
            BOOST_CHECK(b.downtree_node() == "B1");
        }


        const auto& platform_uptree = network.uptree_branch("PLAT-A");
        BOOST_CHECK(!platform_uptree.has_value());

        const auto& B1_uptree = network.uptree_branch("B1");
        BOOST_CHECK(B1_uptree.has_value());
        BOOST_CHECK_EQUAL(B1_uptree->downtree_node(), "B1");
        BOOST_CHECK_EQUAL(B1_uptree->uptree_node(), "PLAT-A");

        BOOST_CHECK_THROW( network.uptree_branch("C1"), std::out_of_range);
        BOOST_CHECK_THROW( network.node("C1"), std::out_of_range);

        BOOST_CHECK(network.active());
    }
}

BOOST_AUTO_TEST_CASE(NodeNames) {
    const auto sched = make_schedule(R"(
SCHEDULE

GRUPTREE
 'PROD'    'FIELD' /

 'M5S'    'PLAT-A'  /
 'M5N'    'PLAT-A'  /

 'C1'     'M5N'  /
 'F1'     'M5N'  /
 'B1'     'M5S'  /
 'G1'     'M5S'  /
/

BRANPROP
--  Downtree  Uptree   #VFP    ALQ
    B1         PLAT-A    9999      1*      /
    C1         PLAT-A    9999      1*      /
/

NODEPROP
--  Node_name Pr    autoChock?      addGasLift?     Group_name
     PLAT-A 21.0   NO     NO    1*  /
     B1    1*  YES      NO    1*  /
     C1    1*  YES     NO     'GROUP' /
/

TSTEP
  10 /

BRANPROP
--  Downtree  Uptree   #VFP    ALQ
    C1         PLAT-A    0 1*      /
/
)");

    const auto expect = std::vector<std::string> {
        "B1", "C1", "PLAT-A"
    };

    auto nodes = sched[0].network.get().node_names();
    std::sort(nodes.begin(), nodes.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(nodes.begin(), nodes.end(), expect.begin(), expect.end());
}
