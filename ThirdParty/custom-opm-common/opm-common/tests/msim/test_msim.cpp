/*
  Copyright 2018 Equinor ASA.

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

#define BOOST_TEST_MODULE MSIM_BASIC
#include <boost/test/unit_test.hpp>

#include <opm/msim/msim.hpp>

#include <stdexcept>
#include <iostream>

#include <opm/common/utility/FileSystem.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ERsm.hpp>

#include <opm/output/data/Wells.hpp>
#include <opm/output/eclipse/EclipseIO.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>

#include <tests/WorkArea.cpp>

using namespace Opm;

namespace {

double prod_opr(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double seconds_elapsed) {
    const auto& units = es.getUnits();
    return -units.to_si(UnitSystem::measure::rate, seconds_elapsed);
}

void pressure(const EclipseState& es, const Schedule& /* sched */, data::Solution& sol, size_t /* report_step */, double seconds_elapsed) {
    const auto& grid = es.getInputGrid();
    const auto& units = es.getUnits();
    if (!sol.has("PRESSURE"))
        sol.insert("PRESSURE", UnitSystem::measure::pressure, std::vector<double>(grid.getNumActive()), data::TargetType::RESTART_SOLUTION);

    auto& data = sol.data("PRESSURE");
    std::fill(data.begin(), data.end(), units.to_si(UnitSystem::measure::pressure, seconds_elapsed));
}

bool is_file(const Opm::filesystem::path& name)
{
    return Opm::filesystem::exists(name)
        && Opm::filesystem::is_regular_file(name);
}

}

BOOST_AUTO_TEST_CASE(RUN) {
    Parser parser;
    auto python = std::make_shared<Python>();
    Deck deck = parser.parseFile("SPE1CASE1.DATA");
    EclipseState state(deck);
    Schedule schedule(deck, state, python);
    SummaryConfig summary_config(deck, schedule, state.getTableManager());
    msim msim(state);

    msim.well_rate("PROD", data::Rates::opt::oil, prod_opr);
    msim.well_rate("RFT", data::Rates::opt::oil, prod_opr);
    msim.solution("PRESSURE", pressure);
    {
        const WorkArea work_area("test_msim");
        EclipseIO io(state, state.getInputGrid(), schedule, summary_config);

        msim.run(schedule, io, false);

        for (const auto& fname : {"SPE1CASE1.INIT", "SPE1CASE1.UNRST", "SPE1CASE1.EGRID", "SPE1CASE1.SMSPEC", "SPE1CASE1.UNSMRY", "SPE1CASE1.RSM"})
            BOOST_CHECK( is_file( fname ));

        {
            const auto  smry  = EclIO::ESmry("SPE1CASE1");
            const auto& time  = smry.get("TIME");
            const auto& press = smry.get("WOPR:PROD");

            for (auto nstep = time.size(), time_index=0*nstep; time_index < nstep; time_index++) {
                double seconds_elapsed = time[time_index] * 86400;
                BOOST_CHECK_CLOSE(seconds_elapsed, press[time_index], 1e-3);
            }

            const auto& dates = smry.dates();
            const auto& day   = smry.get("DAY");
            const auto& month = smry.get("MONTH");
            const auto& year  = smry.get("YEAR");

            for (auto nstep = dates.size(), time_index=0*nstep; time_index < nstep; time_index++) {
                auto ts = TimeStampUTC( std::chrono::system_clock::to_time_t( dates[time_index]) );
                BOOST_CHECK_EQUAL( ts.day(), day[time_index]);
                BOOST_CHECK_EQUAL( ts.month(), month[time_index]);
                BOOST_CHECK_EQUAL( ts.year(), year[time_index]);
            }

            const auto rsm = EclIO::ERsm("SPE1CASE1.RSM");
            BOOST_CHECK( EclIO::cmp( smry, rsm ));
        }

        {
            auto rst = EclIO::ERst("SPE1CASE1.UNRST");

            for (const auto& step : rst.listOfReportStepNumbers()) {
                const auto& dh    = rst.getRst<double>("DOUBHEAD", step, 0);
                const auto& press = rst.getRst<float>("PRESSURE", step, 0);

                // DOUBHEAD[0] is elapsed time in days since start of simulation.
                BOOST_CHECK_CLOSE( press[0], dh[0] * 86400, 1e-3 );
            }

            const int report_step = 50;
            const auto& rst_state = Opm::RestartIO::RstState::load(rst, report_step);
            Schedule sched_rst(deck, state, python, &rst_state);
            const auto& rft_well = sched_rst.getWell("RFT", report_step);
            BOOST_CHECK(rft_well.getStatus() == Well::Status::SHUT);
        }
    }
}
