/*
  Copyright 2013 Statoil ASA.

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

#define BOOST_TEST_MODULE ScheduleTests

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/RFTConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/OilVaporizationProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>

#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/io/eclipse/rst/state.hpp>
#include <opm/io/eclipse/ERst.hpp>

using namespace Opm;

void compare_connections(const RestartIO::RstConnection& rst_conn, const Connection& sched_conn) {
    BOOST_CHECK_EQUAL(rst_conn.ijk[0], sched_conn.getI());
    BOOST_CHECK_EQUAL(rst_conn.ijk[1], sched_conn.getJ());
    BOOST_CHECK_EQUAL(rst_conn.ijk[2], sched_conn.getK());

    BOOST_CHECK_EQUAL(rst_conn.segment, sched_conn.segment());
    BOOST_CHECK_EQUAL(rst_conn.rst_index, static_cast<int>(sched_conn.sort_value()));
    BOOST_CHECK(rst_conn.state == sched_conn.state());
    BOOST_CHECK(rst_conn.dir == sched_conn.dir());
    BOOST_CHECK_CLOSE( rst_conn.cf, sched_conn.CF() , 1e-6);
}



void compare_wells(const RestartIO::RstWell& rst_well, const Well& sched_well) {
    BOOST_CHECK_EQUAL(rst_well.name, sched_well.name());
    BOOST_CHECK_EQUAL(rst_well.group, sched_well.groupName());

    const auto& sched_connections = sched_well.getConnections();
    BOOST_CHECK_EQUAL(sched_connections.size(), rst_well.connections.size());

    for (std::size_t ic=0; ic < rst_well.connections.size(); ic++) {
        const auto& rst_conn = rst_well.connections[ic];
        const auto& sched_conn = sched_connections[ic];
        compare_connections(rst_conn, sched_conn);
    }
}


BOOST_AUTO_TEST_CASE(LoadRST) {
    Parser parser;
    auto deck = parser.parseFile("SPE1CASE2.DATA");
    EclIO::ERst rst_file("SPE1CASE2.X0060");
    auto rst_state = RestartIO::RstState::load(rst_file, 60);
    BOOST_REQUIRE_THROW( rst_state.get_well("NO_SUCH_WELL"), std::out_of_range);
    auto python = std::make_shared<Python>();
    EclipseState ecl_state(deck);
    Schedule sched(deck, ecl_state, python);
    const auto& well_names = sched.wellNames(60);
    BOOST_CHECK_EQUAL(well_names.size(), rst_state.wells.size());

    for (const auto& wname : well_names) {
        const auto& rst_well = rst_state.get_well(wname);
        const auto& sched_well = sched.getWell(wname, 60);
        compare_wells(rst_well, sched_well);
    }
}

BOOST_AUTO_TEST_CASE(LoadRestartSim) {
    auto python = std::make_shared<Python>();
    Parser parser;
    auto deck = parser.parseFile("SPE1CASE2.DATA");
    EclipseState ecl_state(deck);
    Schedule sched(deck, ecl_state, python);

    auto restart_deck = parser.parseFile("SPE1CASE2_RESTART.DATA");
    EclIO::ERst rst_file("SPE1CASE2.X0060");
    auto rst_state = RestartIO::RstState::load(rst_file, 60);
    EclipseState ecl_state_restart(restart_deck);
    Schedule restart_sched(restart_deck, ecl_state_restart, python, &rst_state);

    // Verify that sched and restart_sched are identical from report_step 60 and onwords.
}
