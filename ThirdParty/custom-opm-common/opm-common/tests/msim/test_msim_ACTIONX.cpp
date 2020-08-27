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

#define BOOST_TEST_MODULE ACTIONX_SIM

#include <boost/test/unit_test.hpp>

#include <opm/msim/msim.hpp>

#include <stdexcept>
#include <iostream>
#include <memory>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionAST.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionContext.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/Actions.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionX.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>

#include <opm/io/eclipse/ESmry.hpp>

#include <opm/output/eclipse/EclipseIO.hpp>

#include <tests/WorkArea.cpp>

using namespace Opm;

namespace {

struct test_data {
    Deck deck;
    EclipseState state;
    std::shared_ptr<Python> python;
    Schedule schedule;
    SummaryConfig summary_config;

    test_data(const Deck& deck_arg) :
        deck(deck_arg),
        state( this->deck ),
        python( std::make_shared<Python>() ),
        schedule( this->deck, this->state, this->python),
        summary_config( this->deck, this->schedule, this->state.getTableManager())
    {
        auto& ioconfig = this->state.getIOConfig();
        ioconfig.setBaseName("MSIM");
    }

    test_data(const std::string& deck_string) :
        test_data( Parser().parseString(deck_string) )
    {}


};


double prod_opr(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    double oil_rate = 1.0;
    return -units.to_si(UnitSystem::measure::rate, oil_rate);
}

double prod_opr_low(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    double oil_rate = 0.5;
    return -units.to_si(UnitSystem::measure::rate, oil_rate);
}

double prod_wpr_P1(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    double water_rate = 0.0;
    return -units.to_si(UnitSystem::measure::rate, water_rate);
}

double prod_wpr_P2(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t report_step, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    double water_rate = 0.0;
    if (report_step > 5)
        water_rate = 2.0;  // => WWCT = WWPR / (WOPR + WWPR) = 2/3

    return -units.to_si(UnitSystem::measure::rate, water_rate);
}

double prod_wpr_P3(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t /* report_step */, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    double water_rate = 0.0;
    return -units.to_si(UnitSystem::measure::rate, water_rate);
}

double prod_wpr_P4(const EclipseState&  es, const Schedule& /* sched */, const SummaryState&, const data::Solution& /* sol */, size_t report_step, double /* seconds_elapsed */) {
    const auto& units = es.getUnits();
    double water_rate = 0.0;
    if (report_step > 10)
        water_rate = 2.0;

    return -units.to_si(UnitSystem::measure::rate, water_rate);
}


double inj_wir_INJ(const EclipseState& , const Schedule& sched, const SummaryState& st, const data::Solution& /* sol */, size_t report_step, double /* seconds_elapsed */) {
    if (st.has("FUINJ")) {
        const auto& well = sched.getWell("INJ", report_step);
        const auto controls = well.injectionControls(st);
        return controls.surface_rate;
    } else
        return -99;
}

bool ecl_sum_has_general_var(const EclIO::ESmry& smry, const std::string& var)
{
    return smry.hasKey(var);
}

float ecl_sum_get_general_var(const EclIO::ESmry& smry, const int timeIdx, const std::string& var)
{
    return smry.get(var)[timeIdx];
}

int ecl_sum_get_data_length(const EclIO::ESmry& smry)
{
    return static_cast<int>(smry.get("TIME").size());
}

int ecl_sum_get_last_report_step(const EclIO::ESmry& smry)
{
    return static_cast<int>(smry.get_at_rstep("TIME").size());
}

int ecl_sum_iget_report_end(const EclIO::ESmry& smry, const int reportStep)
{
    return smry.timestepIdxAtReportstepStart(reportStep + 1) - 1;
}
}


/*
  The deck tested here has a UDQ DEFINE statement which sorts the wells after
  oil production rate, and then subsequently closes the well with lowest OPR
  with a ACTIONX keyword.
*/

BOOST_AUTO_TEST_CASE(UDQ_SORTA_EXAMPLE) {
#include "actionx2.include"

    test_data td( actionx );
    msim sim(td.state);
    {
        WorkArea work_area("test_msim");
        EclipseIO io(td.state, td.state.getInputGrid(), td.schedule, td.summary_config);

        sim.well_rate("P1", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P2", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P3", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P4", data::Rates::opt::oil, prod_opr_low);

        sim.run(td.schedule, io, false);
        {
            const auto& w1 = td.schedule.getWell("P1", 1);
            const auto& w4 = td.schedule.getWell("P4", 1);
            BOOST_CHECK(w1.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w4.getStatus() == Well::Status::OPEN );
        }
        {
            const auto& w1 = td.schedule.getWellatEnd("P1");
            const auto& w4 = td.schedule.getWellatEnd("P4");
            BOOST_CHECK(w1.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w4.getStatus() == Well::Status::SHUT );
        }
    }
}


BOOST_AUTO_TEST_CASE(WELL_CLOSE_EXAMPLE) {
#include "actionx1.include"

    test_data td( actionx1 );
    msim sim(td.state);
    {
        WorkArea work_area("test_msim");
        EclipseIO io(td.state, td.state.getInputGrid(), td.schedule, td.summary_config);

        sim.well_rate("P1", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P2", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P3", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P4", data::Rates::opt::oil, prod_opr);

        sim.well_rate("P1", data::Rates::opt::wat, prod_wpr_P1);
        sim.well_rate("P2", data::Rates::opt::wat, prod_wpr_P2);
        sim.well_rate("P3", data::Rates::opt::wat, prod_wpr_P3);
        sim.well_rate("P4", data::Rates::opt::wat, prod_wpr_P4);

        {
            const auto& w1 = td.schedule.getWell("P1", 15);
            const auto& w2 = td.schedule.getWell("P2", 15);
            const auto& w3 = td.schedule.getWell("P3", 15);
            const auto& w4 = td.schedule.getWell("P4", 15);

            BOOST_CHECK(w1.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w2.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w3.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w4.getStatus() == Well::Status::OPEN );
        }


        sim.run(td.schedule, io, false);
        {
            const auto& w1 = td.schedule.getWell("P1", 15);
            const auto& w3 = td.schedule.getWell("P3", 15);
            BOOST_CHECK(w1.getStatus() ==  Well::Status::OPEN );
            BOOST_CHECK(w3.getStatus() ==  Well::Status::OPEN );
        }
        {
            const auto& w2_5 = td.schedule.getWell("P2", 5);
            const auto& w2_6 = td.schedule.getWell("P2", 6);
            BOOST_CHECK(w2_5.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w2_6.getStatus() == Well::Status::SHUT );
        }
        {
            const auto& w4_10 = td.schedule.getWell("P4", 10);
            const auto& w4_11 = td.schedule.getWell("P4", 11);
            BOOST_CHECK(w4_10.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w4_11.getStatus() == Well::Status::SHUT );
        }
    }
}



BOOST_AUTO_TEST_CASE(UDQ_ASSIGN) {
#include "actionx1.include"

    test_data td( actionx1 );
    msim sim(td.state);
    {
        WorkArea work_area("test_msim");
        EclipseIO io(td.state, td.state.getInputGrid(), td.schedule, td.summary_config);

        sim.well_rate("P1", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P2", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P3", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P4", data::Rates::opt::oil, prod_opr);

        sim.well_rate("P1", data::Rates::opt::wat, prod_wpr_P1);
        sim.well_rate("P2", data::Rates::opt::wat, prod_wpr_P2);
        sim.well_rate("P3", data::Rates::opt::wat, prod_wpr_P3);
        sim.well_rate("P4", data::Rates::opt::wat, prod_wpr_P4);

        sim.run(td.schedule, io, false);

        const auto& base_name = td.state.getIOConfig().getBaseName();

        const EclIO::ESmry ecl_sum(base_name + ".SMSPEC");
        BOOST_CHECK( ecl_sum_has_general_var(ecl_sum, "WUBHP:P1") );
        BOOST_CHECK( ecl_sum_has_general_var(ecl_sum, "WUBHP:P2") );
        BOOST_CHECK( ecl_sum_has_general_var(ecl_sum, "WUOPRL:P3") );
        BOOST_CHECK( ecl_sum_has_general_var(ecl_sum, "WUOPRL:P4") );

#if 0
        BOOST_CHECK_EQUAL( ecl_sum_get_unit(ecl_sum, "WUBHP:P1"), "BARSA");
        BOOST_CHECK_EQUAL( ecl_sum_get_unit(ecl_sum, "WUOPRL:P1"), "SM3/DAY");
#endif

        BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, 1, "WUBHP:P1"), 11);
        BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, 1, "WUBHP:P2"), 12);
        BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, 1, "WUBHP:P3"), 13);
        BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, 1, "WUBHP:P4"), 14);

        BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, 1, "WUOPRL:P1"), 20);
        BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, 1, "WUOPRL:P2"), 20);
        BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, 1, "WUOPRL:P3"), 20);
        BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, 1, "WUOPRL:P4"), 20);
    }
}

BOOST_AUTO_TEST_CASE(UDQ_WUWCT) {
#include "actionx1.include"

    test_data td( actionx1 );
    msim sim(td.state);
    {
        WorkArea work_area("test_msim");
        EclipseIO io(td.state, td.state.getInputGrid(), td.schedule, td.summary_config);

        sim.well_rate("P1", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P2", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P3", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P4", data::Rates::opt::oil, prod_opr_low);

        sim.well_rate("P1", data::Rates::opt::wat, prod_wpr_P1);
        sim.well_rate("P2", data::Rates::opt::wat, prod_wpr_P2);
        sim.well_rate("P3", data::Rates::opt::wat, prod_wpr_P3);
        sim.well_rate("P4", data::Rates::opt::wat, prod_wpr_P4);

        sim.run(td.schedule, io, false);

        const auto& base_name = td.state.getIOConfig().getBaseName();
        const EclIO::ESmry ecl_sum(base_name + ".SMSPEC");

        for (int step = 0; step < ecl_sum_get_data_length(ecl_sum); step++) {
            double wopr_sum = 0;
            for (const auto& well : {"P1", "P2", "P3", "P4"}) {
                std::string wwct_key  = std::string("WWCT:") + well;
                std::string wuwct_key = std::string("WUWCT:") + well;
                std::string wopr_key  = std::string("WOPR:") + well;

                if (ecl_sum_get_general_var(ecl_sum, step, wwct_key.c_str()) != 0)
                    BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, step, wwct_key.c_str()),
                                       ecl_sum_get_general_var(ecl_sum, step, wuwct_key.c_str()));

                wopr_sum += ecl_sum_get_general_var(ecl_sum, step , wopr_key.c_str());
            }
            BOOST_CHECK_EQUAL( ecl_sum_get_general_var(ecl_sum, step, "FOPR"),
                               ecl_sum_get_general_var(ecl_sum, step, "FUOPR"));
            BOOST_CHECK_EQUAL( wopr_sum, ecl_sum_get_general_var(ecl_sum, step, "FOPR"));
        }
    }
}


BOOST_AUTO_TEST_CASE(UDA) {
#include "uda.include"
    test_data td( uda_deck );
    msim sim(td.state);
    auto eps_lim = sim.uda_val().epsilonLimit();

    EclipseIO io(td.state, td.state.getInputGrid(), td.schedule, td.summary_config);

    sim.well_rate("P1", data::Rates::opt::wat, prod_wpr_P1);
    sim.well_rate("P2", data::Rates::opt::wat, prod_wpr_P2);
    sim.well_rate("P3", data::Rates::opt::wat, prod_wpr_P3);
    sim.well_rate("P4", data::Rates::opt::wat, prod_wpr_P4);
    sim.well_rate("INJ", data::Rates::opt::wat, inj_wir_INJ);
    {
        WorkArea work_area("uda_sim");

        sim.run(td.schedule, io, true);

        const auto& base_name = td.state.getIOConfig().getBaseName();
        const EclIO::ESmry ecl_sum(base_name + ".SMSPEC");

        // Should only get at report steps
        const auto last_report = ecl_sum_get_last_report_step(ecl_sum);
        for (int report_step = 2; report_step < last_report; report_step++) {
            double wwpr_sum = 0;
            {
                int prev_tstep = ecl_sum_iget_report_end(ecl_sum, report_step - 1);
                for (const auto& well : {"P1", "P2", "P3", "P4"}) {
                    std::string wwpr_key  = std::string("WWPR:") + well;
                    wwpr_sum += ecl_sum_get_general_var(ecl_sum, prev_tstep, wwpr_key.c_str());
                }
                wwpr_sum = 0.90 * wwpr_sum;
                wwpr_sum = std::max(eps_lim, wwpr_sum);
            }
            BOOST_CHECK_CLOSE( wwpr_sum, ecl_sum_get_general_var(ecl_sum, ecl_sum_iget_report_end(ecl_sum, report_step), "WWIR:INJ"), 1e-3);
        }
    }
}

#ifdef EMBEDDED_PYTHON

BOOST_AUTO_TEST_CASE(PYTHON_WELL_CLOSE_EXAMPLE) {
    const auto& deck = Parser().parseFile("msim/MSIM_PYACTION.DATA");
    test_data td( deck );
    msim sim(td.state);
    {
        WorkArea work_area("test_msim");
        EclipseIO io(td.state, td.state.getInputGrid(), td.schedule, td.summary_config);

        sim.well_rate("P1", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P2", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P3", data::Rates::opt::oil, prod_opr);
        sim.well_rate("P4", data::Rates::opt::oil, prod_opr);

        sim.well_rate("P1", data::Rates::opt::wat, prod_wpr_P1);
        sim.well_rate("P2", data::Rates::opt::wat, prod_wpr_P2);
        sim.well_rate("P3", data::Rates::opt::wat, prod_wpr_P3);
        sim.well_rate("P4", data::Rates::opt::wat, prod_wpr_P4);

        {
            const auto& w1 = td.schedule.getWell("P1", 15);
            const auto& w2 = td.schedule.getWell("P2", 15);
            const auto& w3 = td.schedule.getWell("P3", 15);
            const auto& w4 = td.schedule.getWell("P4", 15);

            BOOST_CHECK(w1.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w2.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w3.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w4.getStatus() == Well::Status::OPEN );
        }


        sim.run(td.schedule, io, false);
        {
            const auto& w1 = td.schedule.getWell("P1", 15);
            const auto& w3 = td.schedule.getWell("P3", 15);
            BOOST_CHECK(w1.getStatus() ==  Well::Status::OPEN );
            BOOST_CHECK(w3.getStatus() ==  Well::Status::OPEN );
        }
        {
            const auto& w2_5 = td.schedule.getWell("P2", 5);
            const auto& w2_6 = td.schedule.getWell("P2", 6);
            BOOST_CHECK(w2_5.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w2_6.getStatus() == Well::Status::SHUT );
        }
        {
            const auto& w4_10 = td.schedule.getWell("P4", 10);
            const auto& w4_11 = td.schedule.getWell("P4", 11);
            BOOST_CHECK(w4_10.getStatus() == Well::Status::OPEN );
            BOOST_CHECK(w4_11.getStatus() == Well::Status::SHUT );
        }
    }
}

#endif

