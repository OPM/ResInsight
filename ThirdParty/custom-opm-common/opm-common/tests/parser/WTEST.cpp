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

#define BOOST_TEST_MODULE WTEST
#include <boost/test/unit_test.hpp>

#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/ScheduleTypes.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestConfig.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>

#include "tests/MessageBuffer.cpp"

using namespace Opm;


BOOST_AUTO_TEST_CASE(CreateWellTestConfig) {
    WellTestConfig wc;

    BOOST_CHECK(wc.empty());


    wc.add_well("NAME", "P", 10, 10, 10, 1);
    BOOST_CHECK(!wc.empty());
    BOOST_CHECK_THROW(wc.add_well("NAME2", "", 10.0,10,10.0, 1), std::exception);
    BOOST_CHECK_THROW(wc.add_well("NAME3", "X", 1,2,3, 1), std::exception);

    wc.add_well("NAME", "PEGDC", 10, 10, 10, 1);
    wc.add_well("NAMEX", "PGDC", 10, 10, 10, 1);
    wc.drop_well("NAME");
    BOOST_CHECK(wc.has("NAMEX"));
    BOOST_CHECK(wc.has("NAMEX", WellTestConfig::Reason::PHYSICAL));
    BOOST_CHECK(!wc.has("NAMEX", WellTestConfig::Reason::ECONOMIC));
    BOOST_CHECK(!wc.has("NAME"));
}


BOOST_AUTO_TEST_CASE(WTEST_STATE2) {
    WellTestConfig wc;
    WellTestState st;
    wc.add_well("WELL_NAME", "P", 0, 0, 0, 0);
    st.close_well("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 100);
    BOOST_CHECK_EQUAL(st.num_closed_wells(), 1U);

    const UnitSystem us{};
    auto shut_wells = st.test_wells(wc, 5000);
    BOOST_CHECK_EQUAL( shut_wells.size(), 1U);
}

BOOST_AUTO_TEST_CASE(WTEST_STATE) {
    const double day = 86400.;
    WellTestState st;
    st.close_well("WELL_NAME", WellTestConfig::Reason::ECONOMIC, 100. * day);
    BOOST_CHECK_EQUAL(st.num_closed_wells(), 1U);

    st.open_well("WELL_NAME");
    BOOST_CHECK_EQUAL(st.num_closed_wells(), 0);

    st.close_well("WELL_NAME", WellTestConfig::Reason::ECONOMIC, 100. * day);
    BOOST_CHECK_EQUAL(st.num_closed_wells(), 1U);

    st.close_well("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 100. * day);
    BOOST_CHECK_EQUAL(st.num_closed_wells(), 1U);

    st.close_well("WELLX", WellTestConfig::Reason::PHYSICAL, 100. * day);
    BOOST_CHECK_EQUAL(st.num_closed_wells(), 2U);

    {
        auto st2 = st;
        st2.filter_wells(std::vector<std::string>{"WELLX"});

        BOOST_CHECK(!st2.well_is_closed("WELL_NAME"));  // This well has been opened/removed by the filter_wells() call
        BOOST_CHECK( st2.well_is_closed("WELLX"));
    }

    const UnitSystem us{};

    WellTestConfig wc;
    {
        auto shut_wells = st.test_wells(wc, 110. * day);
        BOOST_CHECK_EQUAL(shut_wells.size(), 0U);
    }
    {
        auto shut_wells = st.test_wells(wc, 110. * day);
        BOOST_CHECK_EQUAL(shut_wells.size(), 0U);
    }

    wc.add_well("WELL_NAME", "P", 1000. * day, 2, 0, 1);
    // Not sufficient time has passed.
    BOOST_CHECK_EQUAL( st.test_wells(wc, 200. * day).size(), 0U);

    // We should test it:
    BOOST_CHECK_EQUAL( st.test_wells(wc, 1200. * day).size(), 1U);

    // Not sufficient time has passed.
    BOOST_CHECK_EQUAL( st.test_wells(wc, 1700. * day).size(), 0U);

    st.open_well("WELL_NAME");

    // We should not test it - well is open:
    BOOST_CHECK_EQUAL( st.test_wells(wc, 2400. * day).size(), 0U);

    st.close_well("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 2500. * day);
    // We should not test it - insufficient time
    BOOST_CHECK_EQUAL( st.test_wells(wc, 2600. * day).size(), 0U);

    // We should test it now:
    BOOST_CHECK_EQUAL( st.test_wells(wc, 4000. * day).size(), 1U);

    // Too many attempts:
    BOOST_CHECK_EQUAL( st.test_wells(wc, 5000. * day).size(), 0U);

    wc.add_well("WELL_NAME", "P", 1000. * day, 3, 0, 5);


    BOOST_CHECK_EQUAL( st.test_wells(wc, 5100. * day).size(), 1U);
    BOOST_CHECK_EQUAL( st.test_wells(wc, 6200. * day).size(), 1U);

    wc.drop_well("WELL_NAME");
    BOOST_CHECK_EQUAL( st.test_wells(wc, 7300. * day).size(), 0U);
}


BOOST_AUTO_TEST_CASE(WTEST_STATE_COMPLETIONS) {
    WellTestConfig wc;
    WellTestState st;
    st.close_completion("WELL_NAME", 2, 100);
    BOOST_CHECK_EQUAL(st.num_closed_completions(), 1U);

    st.close_completion("WELL_NAME", 2, 100);
    BOOST_CHECK_EQUAL(st.num_closed_completions(), 1U);

    st.close_completion("WELL_NAME", 3, 100);
    BOOST_CHECK_EQUAL(st.num_closed_completions(), 2U);

    st.close_completion("WELLX", 3, 100);
    BOOST_CHECK_EQUAL(st.num_closed_completions(), 3U);

    const UnitSystem us{};
    auto num_closed_completions = st.test_wells(wc, 5000);
    BOOST_CHECK_EQUAL( num_closed_completions.size(), 0U);

    BOOST_CHECK_NO_THROW( st.open_completion("WELL_NAME", 20000));

    st.open_completion("WELL_NAME", 2);
    st.open_completion("WELLX", 3);
    BOOST_CHECK_EQUAL(st.num_closed_completions(), 1U);

    BOOST_CHECK_NO_THROW( st.open_completion("NO_SUCH_WELL", 3) );

    BOOST_CHECK_NO_THROW(st.open_completion("NO_SUCH_WELL", 1000));
    BOOST_CHECK_NO_THROW(st.open_completion("NO_SUCH_WELL", 1000));
}




BOOST_AUTO_TEST_CASE(WTEST_PACK_UNPACK) {
    WellTestState st, st2;
    st.close_completion("WELL_NAME", 2, 100);
    st.close_completion("WELL_NAME", 2, 100);
    st.close_completion("WELL_NAME", 3, 100);
    st.close_completion("WELLX", 3, 100);

    st.close_well("WELL_NAME", WellTestConfig::Reason::ECONOMIC, 100);
    st.close_well("WELL_NAME", WellTestConfig::Reason::PHYSICAL, 100);
    st.close_well("WELLX", WellTestConfig::Reason::PHYSICAL, 100);

    BOOST_CHECK(!(st == st2));

    MessageBuffer buffer;
    st.pack(buffer);

    st2.unpack(buffer);
    BOOST_CHECK(st == st2);
}


BOOST_AUTO_TEST_CASE(WTEST_RESTART) {
    UnitSystem us;
    WellTestConfig wc;
    WellTestState ws;
    wc.add_well("W1", "PGD", 1, 10, 100, 1000);
    wc.add_well("W2", "PGD", 1, 10, 100, 1000);

    auto rst_well0 = ws.restart_well(wc, "W0");
    BOOST_CHECK(!rst_well0.has_value());

    auto rst_well1 = ws.restart_well(wc, "W1");
    BOOST_CHECK(rst_well1.has_value());
    {
        const auto& well = rst_well1.value();
        BOOST_CHECK_EQUAL(well.name, "W1");
        BOOST_CHECK_EQUAL(well.test_interval, 1);
        BOOST_CHECK_EQUAL(well.startup_time, 100);
    }
}
