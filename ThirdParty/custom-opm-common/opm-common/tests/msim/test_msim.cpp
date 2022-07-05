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

#include <algorithm>
#include <filesystem>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ERsm.hpp>
#include <opm/io/eclipse/RestartFileView.hpp>
#include <opm/io/eclipse/rst/state.hpp>

#include <opm/output/data/Wells.hpp>
#include <opm/output/eclipse/EclipseIO.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>

#include <tests/WorkArea.hpp>

using namespace Opm;

namespace {

double prod_opr(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double seconds_elapsed) {
    const auto& units = es.getUnits();
    return -units.to_si(UnitSystem::measure::rate, seconds_elapsed);
}

double prod_rft(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    return -units.to_si(UnitSystem::measure::rate, 0.0);
}

double inj_rfti(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    return units.to_si(UnitSystem::measure::rate, 0.0);
}

double inj_inj(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    return units.to_si(UnitSystem::measure::rate, 100);
}

void pressure(const EclipseState& es, const Schedule& /* sched */, data::Solution& sol, size_t /* report_step */, double seconds_elapsed) {
    const auto& grid = es.getInputGrid();
    const auto& units = es.getUnits();
    if (!sol.has("PRESSURE"))
        sol.insert("PRESSURE", UnitSystem::measure::pressure, std::vector<double>(grid.getNumActive()), data::TargetType::RESTART_SOLUTION);

    auto& data = sol.data("PRESSURE");
    std::fill(data.begin(), data.end(), units.to_si(UnitSystem::measure::pressure, seconds_elapsed));
}

bool is_file(const std::filesystem::path& name)
{
    return std::filesystem::exists(name)
        && std::filesystem::is_regular_file(name);
}

}

BOOST_AUTO_TEST_CASE(RUN) {
    Parser parser;
    auto python = std::make_shared<Python>();
    Deck deck = parser.parseFile("SPE1CASE1.DATA");
    EclipseState state(deck);
    Schedule schedule(deck, state, python);
    SummaryConfig summary_config(deck, schedule, state.fieldProps(), state.aquifer());
    msim msim(state, schedule);

    msim.well_rate("PROD", data::Rates::opt::oil, prod_opr);
    msim.well_rate("RFTP", data::Rates::opt::oil, prod_rft);
    msim.well_rate("RFTI", data::Rates::opt::wat, inj_rfti);
    msim.well_rate("INJ",  data::Rates::opt::gas, inj_inj);
    msim.solution("PRESSURE", pressure);
    {
        const WorkArea work_area("test_msim");
        EclipseIO io(state, state.getInputGrid(), schedule, summary_config);

        msim.run(io, false);

        for (const auto& fname : {"SPE1CASE1.INIT", "SPE1CASE1.UNRST", "SPE1CASE1.EGRID", "SPE1CASE1.SMSPEC", "SPE1CASE1.UNSMRY", "SPE1CASE1.RSM"})
            BOOST_CHECK( is_file( fname ));

        {
            const auto  smry  = EclIO::ESmry("SPE1CASE1");
            const auto& time  = smry.get("TIME");
            const auto& press = smry.get("WOPR:PROD");
            BOOST_CHECK( smry.hasKey("RPR__NUM:1"));

            for (auto nstep = time.size(), time_index=0*nstep; time_index < nstep; time_index++) {
                double seconds_elapsed = time[time_index] * 86400;
                BOOST_CHECK_CLOSE(seconds_elapsed, press[time_index], 1e-3);
            }

            const auto& fmwpa = smry.get("FMWPA");
            const auto& fmwia = smry.get("FMWIA");
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

            BOOST_CHECK_EQUAL( fmwpa[0], 0.0 );
            BOOST_CHECK_EQUAL( fmwia[0], 0.0 );

            // The RFTP /RFTI wells will appear as an abondoned well.
            BOOST_CHECK_EQUAL( fmwpa[dates.size() - 1], 1.0 );
            BOOST_CHECK_EQUAL( fmwia[dates.size() - 1], 1.0 );

            const auto rsm = EclIO::ERsm("SPE1CASE1.RSM");
            BOOST_CHECK( EclIO::cmp( smry, rsm ));
        }

        {
            auto rst = std::make_shared<EclIO::ERst>("SPE1CASE1.UNRST");

            for (const auto& step : rst->listOfReportStepNumbers()) {
                const auto& dh    = rst->getRestartData<double>("DOUBHEAD", step, 0);
                const auto& press = rst->getRestartData<float>("PRESSURE", step, 0);

                // DOUBHEAD[0] is elapsed time in days since start of simulation.
                BOOST_CHECK_CLOSE( press[0], dh[0] * 86400, 1e-3 );
            }

            const int report_step = 50;
            auto rst_view = std::make_shared<EclIO::RestartFileView>(std::move(rst), report_step);
            const auto rst_state = Opm::RestartIO::RstState::load(std::move(rst_view), state.runspec(), parser);
            Schedule sched_rst(deck, state, python, {}, &rst_state);
            const auto& rfti_well = sched_rst.getWell("RFTI", report_step);
            const auto& rftp_well = sched_rst.getWell("RFTP", report_step);
            BOOST_CHECK(rftp_well.getStatus() == Well::Status::SHUT);
            BOOST_CHECK(rfti_well.getStatus() == Well::Status::SHUT);
        }
    }
}

BOOST_AUTO_TEST_CASE(RUN_SUMTHIN) {
    Parser parser;
    auto python = std::make_shared<Python>();
    Deck deck = parser.parseFile("SPE1CASE1_SUMTHIN.DATA");
    EclipseState state(deck);
    Schedule schedule(deck, state, python);
    SummaryConfig summary_config(deck, schedule, state.fieldProps(), state.aquifer());
    msim msim(state, schedule);

    msim.well_rate("PROD", data::Rates::opt::oil, prod_opr);
    msim.well_rate("RFTP", data::Rates::opt::oil, prod_rft);
    msim.well_rate("RFTI", data::Rates::opt::wat, inj_rfti);
    msim.well_rate("INJ",  data::Rates::opt::gas, inj_inj);
    msim.solution("PRESSURE", pressure);
    {
        const WorkArea work_area("test_msim");
        EclipseIO io(state, state.getInputGrid(), schedule, summary_config);

        // TSTEP = N*7
        msim.run(io, false);

        // clang-format off
        const auto expect_smry_time = std::vector<double> {
            // SUMTHIN = 10
              7.0,  21.0,  35.0,  49.0,  63.0,  77.0,  91.0, 105.0,
            119.0, 133.0, 147.0, 161.0, 175.0, 189.0, 203.0, 217.0,
            231.0, 245.0, 259.0, 273.0, 287.0, 301.0, 315.0, 329.0,
            343.0, 357.0,
            365.0, // Report step.  365 - 357 = 8 (< 10)

            // SUMTHIN = 20
            379.0, // Note: Interval since time = 357, 379 - 365 = 14 (< 20)
            400.0, 421.0, 442.0, 463.0, 484.0, 505.0, 526.0, 547.0,
            568.0, 589.0, 610.0, 631.0, 652.0, 673.0, 694.0, 715.0,
            731.0, // Report step.  731 - 715 = 16 (< 20)
        };
        // clang-format on

        {
            const auto  smry  = EclIO::ESmry("SPE1CASE1_SUMTHIN");
            const auto& time  = smry.get("TIME");
            const auto& dates = smry.dates();
            const auto report_date = TimeStampUTC(2016, 1, 1);

            /*
              Verify that:

               1. Summary output happens at expected times.
               2. The exact report date halfway through the run is present.
            */

            const auto nstep = expect_smry_time.size();
            BOOST_REQUIRE_EQUAL(time.size(), nstep);
            for (auto step = 0*nstep; step < nstep; ++step) {
                BOOST_CHECK_CLOSE(time[step], expect_smry_time[step], 1.0e-10);
            }

            const auto report_found =
                std::any_of(dates.begin(), dates.begin() + nstep - 1,
                    [&report_date](const auto date)
                {
                    return report_date == TimeStampUTC(std::chrono::system_clock::to_time_t(date));
                });

            BOOST_CHECK_MESSAGE(report_found, "Expected report date missing");
        }
    }
}

BOOST_AUTO_TEST_CASE(RUN_RPTONLY) {
    const Deck deck = Parser{}.parseFile("SPE1CASE1_RPTONLY.DATA");
    const EclipseState state(deck);
    Schedule schedule(deck, state, std::make_shared<Python>());
    const SummaryConfig summary_config(deck, schedule, state.fieldProps(), state.aquifer());

    msim msim(state, schedule);

    msim.well_rate("PROD", data::Rates::opt::oil, prod_opr);
    msim.well_rate("RFTP", data::Rates::opt::oil, prod_rft);
    msim.well_rate("RFTI", data::Rates::opt::wat, inj_rfti);
    msim.well_rate("INJ",  data::Rates::opt::gas, inj_inj);
    msim.solution("PRESSURE", pressure);
    {
        const WorkArea work_area("test_msim");
        EclipseIO io(state, state.getInputGrid(), schedule, summary_config);

        // TSTEP = N*7
        msim.run(io, false);

        // clang-format off
        const auto expect_smry_time = std::vector<double> {
            // RPTONLY
             31.0, // 2015-02-01
             59.0, // 2015-03-01
             90.0, // 2015-04-01
            120.0, // 2015-05-01
            151.0, // 2015-06-01
            181.0, // 2015-07-01
            212.0, // 2015-08-01
            243.0, // 2015-09-01
            273.0, // 2015-10-01
            304.0, // 2015-11-01
            334.0, // 2015-12-01
            365.0, // 2016-01-01

            // RPTONLYO (turn off 'RPTONLY')
            // => summary output every timestep (DT = 7 days)
            372.0, 379.0, 386.0, 393.0, 400.0, 407.0, 414.0,
            421.0, 428.0, 435.0, 442.0, 449.0, 456.0,
        };
        // clang-format on

        {
            const auto  smry  = EclIO::ESmry("SPE1CASE1_RPTONLY");
            const auto& time  = smry.get("TIME");
            const auto& dates = smry.dates();
            const auto report_date = TimeStampUTC(2016, 1, 1);

            /*
              Verify that:

               1. Summary output happens at expected times.
               2. The exact report date 2016-01-01 is present.
            */

            const auto nstep = expect_smry_time.size();
            BOOST_REQUIRE_EQUAL(time.size(), nstep);
            for (auto step = 0*nstep; step < nstep; ++step) {
                BOOST_CHECK_CLOSE(time[step], expect_smry_time[step], 1.0e-10);
            }

            const auto report_found =
                std::any_of(dates.begin(), dates.begin() + nstep - 1,
                    [&report_date](const auto date)
                {
                    return report_date == TimeStampUTC(std::chrono::system_clock::to_time_t(date));
                });

            BOOST_CHECK_MESSAGE(report_found, "Expected report date missing");
        }
    }
}
