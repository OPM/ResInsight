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

#include <opm/common/utility/TimeService.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/OilVaporizationProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Parser/ParserKeywords/D.hpp>

#include <opm/input/eclipse/Deck/DeckValue.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Deck/FileDeck.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/io/eclipse/rst/state.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/RestartFileView.hpp>

#include <cstddef>
#include <filesystem>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

using namespace Opm;

void compare_connections(const RestartIO::RstConnection& rst_conn, const Connection& sched_conn) {
    BOOST_CHECK_EQUAL(rst_conn.ijk[0], sched_conn.getI());
    BOOST_CHECK_EQUAL(rst_conn.ijk[1], sched_conn.getJ());
    BOOST_CHECK_EQUAL(rst_conn.ijk[2], sched_conn.getK());

    BOOST_CHECK_EQUAL(rst_conn.segment, sched_conn.segment());
    BOOST_CHECK_EQUAL(rst_conn.rst_index, sched_conn.sort_value());
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
    auto rst_file = std::make_shared<EclIO::ERst>("SPE1CASE2.X0060");
    auto rst_view = std::make_shared<EclIO::RestartFileView>(std::move(rst_file), 60);
    auto rst_state = RestartIO::RstState::load(std::move(rst_view), Runspec{}, parser);
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


std::tuple<Schedule, Schedule, RestartIO::RstState> load_schedule_pair(const std::string& base_deck,
                                                                       const std::string& rst_deck,
                                                                       const std::string& rst_fname,
                                                                       std::size_t restart_step) {
    Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseFile(base_deck);
    EclipseState ecl_state(deck);
    Schedule sched(deck, ecl_state, python);

    auto restart_deck = parser.parseFile(rst_deck);
    auto rst_file = std::make_shared<EclIO::ERst>(rst_fname);
    auto rst_view = std::make_shared<EclIO::RestartFileView>(std::move(rst_file), restart_step);
    auto rst_state = RestartIO::RstState::load(std::move(rst_view), ecl_state.runspec(), parser);
    EclipseState ecl_state_restart(restart_deck);
    Schedule restart_sched(restart_deck, ecl_state_restart, python, {}, &rst_state);

    return {sched, restart_sched, std::move(rst_state)};
}


void compare_sched(const std::string& base_deck,
                   const std::string& rst_deck,
                   const std::string& rst_fname,
                   std::size_t restart_step)
{
    const auto& [sched, restart_sched, _] = load_schedule_pair(base_deck, rst_deck, rst_fname, restart_step);
    (void) _;
    BOOST_CHECK_EQUAL(restart_sched.size(), sched.size());
    for (std::size_t report_step=restart_step; report_step < sched.size(); report_step++) {
        const auto& base = sched[report_step];
        auto rst = restart_sched[report_step];

        BOOST_CHECK(base.start_time() == rst.start_time());
        if (report_step < sched.size() - 1)
            BOOST_CHECK(base.end_time() == rst.end_time());

        // Should ideally do a base == rst check here, but for now the members
        // wells, rft_config, m_first_in_year and m_first_in_month fail.
        // BOOST_CHECK(base == rst);
    }
}



BOOST_AUTO_TEST_CASE(LoadRestartSim) {
    compare_sched("SPE1CASE2.DATA", "SPE1CASE2_RESTART_SKIPREST.DATA", "SPE1CASE2.X0060", 60);
    compare_sched("SPE1CASE2.DATA", "SPE1CASE2_RESTART.DATA", "SPE1CASE2.X0060", 60);
}


BOOST_AUTO_TEST_CASE(LoadUDQRestartSim) {
    const auto& [sched, restart_sched, rst_state] = load_schedule_pair("UDQ_WCONPROD.DATA", "UDQ_WCONPROD_RESTART.DATA", "UDQ_WCONPROD.X0006", 6);
    std::size_t report_step = 10;
    SummaryState st(TimeService::now());
    st.update_well_var("OPL02", "WUOPRL", 1);
    st.update_well_var("OPL02", "WULPRL", 11);
    st.update_well_var("OPU02", "WUOPRU", 111);
    st.update_well_var("OPU02", "WULPRU", 1111);

    for (const auto& wname : sched.wellNames(report_step)) {
        const auto& well = sched.getWell(wname, report_step);
        const auto& rst_well = restart_sched.getWell(wname, report_step);

        if (well.isProducer()) {
            const auto& controls = well.productionControls(st);
            auto rst_controls = rst_well.productionControls(st);
            /*
              The cmode in the base case is the cmode set by the input deck,
              whereas the cmode in restart case is what cmode was active when
              the restart file was written - these can deviate.
            */
            rst_controls.cmode = controls.cmode;
            BOOST_CHECK( controls == rst_controls );
        }
    }

    UDQState udq_state(0);
    udq_state.load_rst(rst_state);
}

BOOST_AUTO_TEST_CASE(LoadActionRestartSim) {
    const auto& [sched, restart_sched, rst_state] = load_schedule_pair("UDQ_ACTIONX.DATA", "UDQ_ACTIONX_RESTART.DATA", "UDQ_ACTIONX.X0007", 7);
    const auto& input_actions = sched[7].actions();
    const auto& rst_actions = restart_sched[7].actions();

    BOOST_CHECK_EQUAL(input_actions.ecl_size(), rst_actions.ecl_size());
    for (std::size_t iact = 0; iact < input_actions.ecl_size(); iact++) {
        const auto& input_action = input_actions[iact];
        const auto& rst_action = rst_actions[iact];

        auto input_iter = input_action.begin();
        auto rst_iter = rst_action.begin();

        BOOST_REQUIRE_EQUAL( std::distance(input_action.begin(), input_action.end()),
                             std::distance(rst_action.begin(), rst_action.end()) );

        while (input_iter != input_action.end()) {
            BOOST_CHECK( *input_iter == *rst_iter );
            input_iter++;
            rst_iter++;
        }
    }

    Action::State action_state;
    action_state.load_rst(rst_actions, rst_state);
}

BOOST_AUTO_TEST_CASE(LoadUDQRestartSim0) {
    const auto& [sched, restart_sched, _] = load_schedule_pair("ACTIONX_M1.DATA", "ACTIONX_M1_RESTART.DATA", "ACTIONX_M1.X0010", 10);
    (void)_;
    std::size_t report_step = 12;

    const auto& group = sched.getGroup("TEST", report_step);
    const auto& rst_group = restart_sched.getGroup("TEST", report_step);

    // The GCONINJE which connects a UDA with the water injection control
    // in group TEST is evaluated in an ACTIONX statement, i.e. it has not
    // been initialized in the normal deck.
    BOOST_CHECK_NO_THROW( rst_group.injectionProperties(Phase::WATER) );
    BOOST_CHECK_THROW( group.injectionProperties(Phase::WATER), std::exception );
}

BOOST_AUTO_TEST_CASE(LoadWLISTRestartSim) {
    const auto& [sched, restart_sched, _] = load_schedule_pair("ACTIONX_M1.DATA", "ACTIONX_M1_RESTART.DATA", "ACTIONX_M1.X0010", 10);
    (void)_;
    std::size_t report_step = 12;

    const auto& wlm = sched[report_step].wlist_manager();
    const auto& rst_wlm = restart_sched[report_step].wlist_manager();

    BOOST_CHECK(wlm == rst_wlm);
}


BOOST_AUTO_TEST_CASE(TestFileDeck)
{
    Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseFile("UDQ_WCONPROD.DATA");
    FileDeck fd(deck);

    {
        auto index = fd.start();
        for (const auto& kw : deck) {
            BOOST_CHECK(kw == fd[index]);
            index++;
        }
    }

    {
        auto index = fd.stop();
        for (std::size_t deck_index = deck.size(); deck_index > 0; deck_index--)
            BOOST_CHECK(deck[deck_index - 1] == fd[--index]);
    }


    const auto& index1 = fd.find("RADIAL");
    BOOST_CHECK( !index1.has_value() );
    BOOST_CHECK_EQUAL( fd.count("RADIAL"), 0);

    BOOST_CHECK_EQUAL( fd.count("DIMENS"), 1);
    BOOST_CHECK_EQUAL( fd.count("DATES"), 5);

    const auto& index2 = fd.find("DIMENS");
    BOOST_CHECK( index2.has_value());
    BOOST_CHECK_EQUAL( index2.value().file_index , 0);
    BOOST_CHECK_EQUAL( index2.value().keyword_index, 3);
    auto offset = index2.value();
    const auto& dimens2 = fd.find("DIMENS", ++offset);
    BOOST_CHECK(!dimens2.has_value());

    auto index2p1 = index2.value() + 1;
    BOOST_CHECK(index2.value() < index2p1);
    const auto& kw = fd[index2p1];
    BOOST_CHECK_EQUAL(kw.name(), "START");


    const auto& index3 = fd.find("COORD");
    BOOST_CHECK_EQUAL( index3.value().file_index , 1);
    BOOST_CHECK_EQUAL( index3.value().keyword_index, 3);

    const auto& index4 = fd.find("SGOF");
    BOOST_CHECK_EQUAL( index4.value().file_index , 2);
    BOOST_CHECK_EQUAL( index4.value().keyword_index, 2);

    fd.erase(index4.value());
    const auto& index5 = fd.find("SGOF");
    BOOST_CHECK( !index5.has_value() );


    const auto& index6 = fd.find("SWOF");
    const auto& index7 = fd.find("SOLUTION");

    fd.erase(index6.value(), index7.value());
}

BOOST_AUTO_TEST_CASE(RestartTest2)
{
    Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseFile("UDQ_WCONPROD.DATA");
    FileDeck fd(deck);

    fd.rst_solution("RESTART", 6);
    fd.insert_skiprest();

    auto solution_index = fd.find("SOLUTION").value();
    auto restart_index = fd.find("RESTART").value();
    auto summary_index = fd.find("SUMMARY").value();
    auto schedule_index = fd.find("SCHEDULE").value();
    auto skiprest_index = fd.find("SKIPREST").value();

    BOOST_CHECK(solution_index < restart_index);
    BOOST_CHECK(restart_index < summary_index);
    BOOST_CHECK(summary_index < schedule_index);
    BOOST_CHECK(schedule_index < skiprest_index);
}

BOOST_AUTO_TEST_CASE(RestartTest23)
{
    Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseFile("UDQ_WCONPROD.DATA");
    FileDeck fd(deck);

    fd.skip(7);
    BOOST_CHECK_EQUAL(fd.count("DATES"), 1);
    BOOST_CHECK(fd.find("SCHEDULE").has_value());

    auto dates_index = fd.find("DATES");
    BOOST_CHECK(dates_index.has_value());
    auto kw = fd[dates_index.value()];
    auto rec0 = kw[0];
    BOOST_CHECK_EQUAL( rec0.getItem<ParserKeywords::DATES::MONTH>().get<std::string>(0), "APR");
    BOOST_CHECK_EQUAL( kw.size() , 5);
}

BOOST_AUTO_TEST_CASE(RestartTest24)
{
    Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseFile("UDQ_WCONPROD.DATA");
    FileDeck fd(deck);

    fd.skip(4);
    BOOST_CHECK_EQUAL(fd.count("DATES"), 3);
    BOOST_CHECK(fd.find("SCHEDULE").has_value());
}

BOOST_AUTO_TEST_CASE(RestartTest)
{
    Parser parser;
    auto python = std::make_shared<Python>();
    auto deck = parser.parseFile("UDQ_WCONPROD.DATA");
    FileDeck fd(deck);

    const auto solution = fd.find("SOLUTION");
    const auto schedule = fd.find("SCHEDULE");

    auto index = solution.value();
    while (true) {
        const auto& keyword = fd[++index];
        if (keyword.name() == "EQUIL")
            fd.erase(index);

        if (index == schedule)
            break;
    }

    UnitSystem units;
    std::vector<DeckValue> values{ DeckValue{"RESTART_BASE"}, DeckValue{100} };
    DeckKeyword restart(parser.getKeyword("RESTART"), std::vector<std::vector<DeckValue>>{ values }, units, units);

    index = solution.value();
    fd.insert(++index, restart);

    BOOST_CHECK( fd[index] == restart );


    // time_point tp = asTimePoint( TimeStampUTC(2018, 1, 11) );

    // auto find_date = [tp](const DeckKeyword& keyword)
    // {
    //     if (keyword.name() == "DATES") {
    //         const auto& record = keyword[0];


    //         const auto &dayItem = record.getItem(0);
    //         const auto &monthItem = record.getItem(1);
    //         const auto &yearItem = record.getItem(2);

    //         // Accept lower- and mixed-case month names.
    //         std::string monthname = uppercase(monthItem.get<std::string>(0));
    //         auto date = asTimePoint( TimeStampUTC( yearItem.get<int>(0), TimeService::eclipseMonthIndices().at(monthname), dayItem.get<int>(0)));

    //         return date == tp;
    //     }
    // };



}
