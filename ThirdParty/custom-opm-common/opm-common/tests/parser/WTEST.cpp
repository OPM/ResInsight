/*
  Copyright 2018 Statoil ASA.

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

#include <stdexcept>
#include <iostream>
#include <boost/filesystem.hpp>

#define BOOST_TEST_MODULE WTEST
#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleTypes.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellTestState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellTestConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellConnections.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>

using namespace Opm;


BOOST_AUTO_TEST_CASE(CreateWellTestConfig) {
    WellTestConfig wc;

    BOOST_CHECK_EQUAL(wc.size() , 0);


    wc.add_well("NAME", WellTestConfig::Reason::PHYSICAL, 10, 10, 10, 1);
    BOOST_CHECK_EQUAL(wc.size(), 1);
    BOOST_CHECK_THROW(wc.add_well("NAME2", "", 10.0,10,10.0, 1), std::invalid_argument);
    BOOST_CHECK_THROW(wc.add_well("NAME3", "X", 1,2,3, 1), std::invalid_argument);

    wc.add_well("NAME", "PEGDC", 10, 10, 10, 1);
    BOOST_CHECK_EQUAL(wc.size(), 5);
    wc.add_well("NAMEX", "PGDC", 10, 10, 10, 1);
    BOOST_CHECK_EQUAL(wc.size(), 9);
    wc.drop_well("NAME");
    BOOST_CHECK_EQUAL(wc.size(), 4);
    BOOST_CHECK(wc.has("NAMEX"));
    BOOST_CHECK(wc.has("NAMEX", WellTestConfig::Reason::PHYSICAL));
    BOOST_CHECK(!wc.has("NAMEX", WellTestConfig::Reason::ECONOMIC));
    BOOST_CHECK(!wc.has("NAME"));

    BOOST_CHECK_THROW(wc.get("NAMEX", WellTestConfig::Reason::ECONOMIC), std::invalid_argument);
    BOOST_CHECK_THROW(wc.get("NO_NAME", WellTestConfig::Reason::ECONOMIC), std::invalid_argument);
    const auto& wt = wc.get("NAMEX", WellTestConfig::Reason::PHYSICAL);
    BOOST_CHECK_EQUAL(wt.name, "NAMEX");
}


BOOST_AUTO_TEST_CASE(WTEST_STATE2) {
    WellTestConfig wc;
    WellTestState st;
    wc.add_well("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 0, 0, 0, 0);
    st.closeWell("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 100);
    BOOST_CHECK_EQUAL(st.sizeWells(), 1);

    const UnitSystem us{};
    std::vector<Well> wells;
    wells.emplace_back("WELL_NAME", "A", 0, 0, 1, 1, 200., WellType(Phase::OIL), Well::ProducerCMode::NONE, Connection::Order::TRACK, us, 0., 1.0, true, true, 0, Well::GasInflowEquation::STD);
    {
        wells[0].updateStatus(Well::Status::SHUT, false);
        auto shut_wells = st.updateWells(wc, wells, 5000);
        BOOST_CHECK_EQUAL(shut_wells.size(), 0);
    }

    {
        wells[0].updateStatus(Well::Status::OPEN, false);
        auto shut_wells = st.updateWells(wc, wells, 5000);
        BOOST_CHECK_EQUAL( shut_wells.size(), 1);
    }
}

BOOST_AUTO_TEST_CASE(WTEST_STATE) {
    const double day = 86400.;
    WellTestState st;
    st.closeWell("WELL_NAME", WellTestConfig::Reason::ECONOMIC, 100. * day);
    BOOST_CHECK_EQUAL(st.sizeWells(), 1);

    st.openWell("WELL_NAME", WellTestConfig::Reason::ECONOMIC);
    BOOST_CHECK_EQUAL(st.sizeWells(), 1);

    st.closeWell("WELL_NAME", WellTestConfig::Reason::ECONOMIC, 100. * day);
    BOOST_CHECK_EQUAL(st.sizeWells(), 1);

    st.closeWell("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 100. * day);
    BOOST_CHECK_EQUAL(st.sizeWells(), 2);

    st.closeWell("WELLX", WellTestConfig::Reason::PHYSICAL, 100. * day);
    BOOST_CHECK_EQUAL(st.sizeWells(), 3);

    const UnitSystem us{};
    std::vector<Well> wells;
    wells.emplace_back("WELL_NAME", "A", 0, 0, 1, 1, 200., WellType(Phase::OIL), Well::ProducerCMode::NONE, Connection::Order::TRACK, us, 0., 1.0, true, true, 0, Well::GasInflowEquation::STD);
    wells.emplace_back("WELLX", "A", 0, 0, 2, 2, 200.,     WellType(Phase::OIL), Well::ProducerCMode::NONE, Connection::Order::TRACK, us, 0., 1.0, true, true, 0, Well::GasInflowEquation::STD);

    WellTestConfig wc;
    {
        wells[0].updateStatus(Well::Status::SHUT, false);
        auto shut_wells = st.updateWells(wc, wells, 110. * day);
        BOOST_CHECK_EQUAL(shut_wells.size(), 0);
    }
    {
        wells[0].updateStatus(Well::Status::OPEN, false);
        auto shut_wells = st.updateWells(wc, wells, 110. * day);
        BOOST_CHECK_EQUAL(shut_wells.size(), 0);
    }

    wc.add_well("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 1000. * day, 2, 0, 1);
    // Not sufficient time has passed.
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 200. * day).size(), 0);

    // We should test it:
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 1200. * day).size(), 1);

    // Not sufficient time has passed.
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 1700. * day).size(), 0);

    st.openWell("WELL_NAME", WellTestConfig::Reason::PHYSICAL);

    st.closeWell("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 1900. * day);

    // We should not test it:
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 2400. * day).size(), 0);

    // We should test it now:
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 3000. * day).size(), 1);

    // Too many attempts:
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 4000. * day).size(), 0);

    wc.add_well("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 1000. * day, 3, 0, 5);


    wells[0].updateStatus(Well::Status::SHUT, false);
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 4100. * day).size(), 0);

    wells[0].updateStatus(Well::Status::OPEN, false);
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 4100. * day).size(), 1);

    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 5200. * day).size(), 1);

    wc.drop_well("WELL_NAME");
    BOOST_CHECK_EQUAL( st.updateWells(wc, wells, 6300. * day).size(), 0);
}


BOOST_AUTO_TEST_CASE(WTEST_STATE_COMPLETIONS) {
    WellTestConfig wc;
    WellTestState st;
    st.addClosedCompletion("WELL_NAME", 2, 100);
    BOOST_CHECK_EQUAL(st.sizeCompletions(), 1);

    st.addClosedCompletion("WELL_NAME", 2, 100);
    BOOST_CHECK_EQUAL(st.sizeCompletions(), 1);

    st.addClosedCompletion("WELL_NAME", 3, 100);
    BOOST_CHECK_EQUAL(st.sizeCompletions(), 2);

    st.addClosedCompletion("WELLX", 3, 100);
    BOOST_CHECK_EQUAL(st.sizeCompletions(), 3);

    const UnitSystem us{};
    std::vector<Well> wells;
    wells.emplace_back("WELL_NAME", "A", 0, 0, 1, 1, 200., WellType(Phase::OIL), Well::ProducerCMode::NONE, Connection::Order::TRACK, us, 0., 1.0, true, true, 0, Well::GasInflowEquation::STD);
    wells[0].updateStatus(Well::Status::OPEN, false);
    wells.emplace_back("WELLX", "A", 0, 0, 2, 2, 200., WellType(Phase::OIL), Well::ProducerCMode::NONE, Connection::Order::TRACK, us, 0., 1.0, true, true, 0, Well::GasInflowEquation::STD);
    wells[1].updateStatus(Well::Status::OPEN, false);

    auto closed_completions = st.updateWells(wc, wells, 5000);
    BOOST_CHECK_EQUAL( closed_completions.size(), 0);

    wc.add_well("WELL_NAME", WellTestConfig::Reason::COMPLETION, 1000, 2, 0, 0);
    // Not sufficient time has passed.
    BOOST_CHECK_EQUAL( st.updateCompletion(wc, 200).size(), 0);

    // We should test it:
    BOOST_CHECK_EQUAL( st.updateCompletion(wc, 1200).size(), 2);

    // Not sufficient time has passed.
    BOOST_CHECK_EQUAL( st.updateCompletion(wc, 1700).size(), 0);

    // We should test it:
    BOOST_CHECK_EQUAL( st.updateCompletion(wc, 2400).size(), 2);

    // Too many attempts:
    BOOST_CHECK_EQUAL( st.updateCompletion(wc, 24000).size(), 0);

    st.dropCompletion("WELL_NAME", 2);
    st.dropCompletion("WELLX", 3);
    BOOST_CHECK_EQUAL(st.sizeCompletions(), 1);
}



