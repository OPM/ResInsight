#define BOOST_TEST_MODULE UDQ-ACTIONX_Data

#include <boost/test/unit_test.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>

#include <opm/output/eclipse/AggregateGroupData.hpp>
#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/output/eclipse/AggregateConnectionData.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>

#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/output/eclipse/AggregateUDQData.hpp>
#include <opm/output/eclipse/AggregateActionxData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/DoubHEAD.hpp>

#include <opm/input/eclipse/Schedule/UDQ/UDQInput.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQParams.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionX.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionContext.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/common/utility/TimeService.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/RestartFileView.hpp>
#include <opm/io/eclipse/rst/state.hpp>
#include <opm/output/data/Wells.hpp>

#include <opm/io/eclipse/OutputStream.hpp>

#include <stdexcept>
#include <utility>
#include <exception>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <fmt/format.h>

#include "tests/WorkArea.hpp"

namespace {

    Opm::Deck first_sim(std::string fname) {
        return Opm::Parser{}.parseFile(fname);
    }
}

bool compare_tokens(const std::vector<std::string>& rst_tokens, const std::vector<std::string>& deck_tokens) {
    if (deck_tokens == rst_tokens)
        return true;

    fmt::print("Deck tokens : ");
    for (const auto& dt : deck_tokens)
        fmt::print("{} ", dt);
    fmt::print("\n");

    fmt::print("Rst tokens  : ");
    for (const auto& rt : rst_tokens)
        fmt::print("{} ", rt);
    fmt::print("\n");

    return false;
}


Opm::SummaryState sum_state_TEST1()
    {
        auto state = Opm::SummaryState{Opm::TimeService::now()};
        state.update_well_var("OPU01", "WWPR", 21.);
        state.update_well_var("OPU02", "WWPR", 22.);
        state.update_well_var("OPL01", "WWPR", 23.);
        state.update_well_var("OPL02", "WWPR", 24.);

        state.update_well_var("OPU01", "WGPR", 230.);
        state.update_well_var("OPU02", "WGPR", 231.);
        state.update_well_var("OPL01", "WGPR", 232.);
        state.update_well_var("OPL02", "WGPR", 233.);

        state.update_group_var("UPPER", "GWPR", 36.);
        state.update_group_var("LOWER", "GWPR", 37.);
        state.update_group_var("TEST",  "GWPR", 73.);

        state.update_group_var("UPPER", "GGPR", 460.);
        state.update_group_var("LOWER", "GGPR", 461.);
        state.update_group_var("TEST",  "GGPR", 821.);

        state.update_group_var("UPPER",  "GMCTW", 0);
        state.update_group_var("LOWER",  "GMCTW", 0);

        state.update_group_var("TEST",  "GMWPR", 4);

        state.update("FWPR", 73.);

        state.update("FMWPR", 4);
        state.update("MNTH", 1);
        state.update("YEAR", 2020);

        return state;
}

//int main(int argc, char* argv[])
struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   { deck }
        , grid { deck }
        , python{ std::make_shared<Opm::Python>() }
        , sched{ deck, es, python }
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid  grid;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule     sched;
    Opm::Parser       parser;
};

BOOST_AUTO_TEST_SUITE(Aggregate_Actionx)



// test constructed UDQ-Actionx restart data
BOOST_AUTO_TEST_CASE(Declared_Actionx_data)
{
    const auto simCase = SimulationCase {first_sim("UDQ_ACTIONX_TEST1.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::Runspec rspec = es.runspec();
    Opm::SummaryState st = sum_state_TEST1();
    Opm::UDQState udq_state(1);
    Opm::Action::State action_state;
    Opm::Schedule sched = simCase.sched;
    Opm::EclipseGrid grid = simCase.grid;
    const auto& start_time = sched.getStartTime();
    const auto& ioConfig = es.getIOConfig();
    // const auto& restart = es.cfg().restart();


    // Report Step 3: 2008-08-22 --> 2018-10-01
    const auto rptStep = std::size_t {3};
    std::string outputDir = "./";
    std::string baseName = "UDQ_ACTIONX_TEST1";
    Opm::Action::ActionX actx_14 = Opm::Action::ActionX("ACT14", 10, 0.543, start_time);
    Opm::Action::Result result = Opm::Action::Result(true);
    action_state.add_run(actx_14, start_time + 1.E09, result);

    double secs_elapsed = start_time + 2.E09;
    // set dummy value for next_step_size

    {
        WorkArea work;
        {
            Opm::EclIO::OutputStream::Restart rstFile {Opm::EclIO::OutputStream::ResultSet {outputDir, baseName},
                                                       rptStep,
                                                       Opm::EclIO::OutputStream::Formatted {ioConfig.getFMTOUT()},
                                                       Opm::EclIO::OutputStream::Unified {ioConfig.getUNIFOUT()}};

            std::vector<int> ih
                = Opm::RestartIO::Helpers::createInteHead(es, grid, sched, secs_elapsed, rptStep, rptStep, rptStep-1);

            // set dummy value for next_step_size
            const double next_step_size = 0.1;
            const auto dh = Opm::RestartIO::Helpers::createDoubHead(es, sched, rptStep-1, rptStep, secs_elapsed, next_step_size);

            const auto& lh = Opm::RestartIO::Helpers::createLogiHead(es);

            // Not really interested in the UDQ data
            ih[Opm::RestartIO::Helpers::VectorItems::intehead::NO_WELL_UDQS] = 0;
            ih[Opm::RestartIO::Helpers::VectorItems::intehead::NO_GROUP_UDQS] = 0;
            ih[Opm::RestartIO::Helpers::VectorItems::intehead::NO_FIELD_UDQS] = 0;

            rstFile.write("INTEHEAD", ih);
            rstFile.write("DOUBHEAD", dh);
            rstFile.write("LOGIHEAD", lh);
            {
                auto group_aggregator = Opm::RestartIO::Helpers::AggregateGroupData(ih);
                group_aggregator.captureDeclaredGroupData(sched, es.getUnits(), rptStep-1, st, ih);
                rstFile.write("IGRP", group_aggregator.getIGroup());
                rstFile.write("SGRP", group_aggregator.getSGroup());
                rstFile.write("XGRP", group_aggregator.getXGroup());
                rstFile.write("ZGRP", group_aggregator.getZGroup());
            }
            {
                auto well_aggregator = Opm::RestartIO::Helpers::AggregateWellData(ih);
                well_aggregator.captureDeclaredWellData(sched, es.tracer(), rptStep-1, {}, {}, st, ih);
                rstFile.write("IWEL", well_aggregator.getIWell());
                rstFile.write("SWEL", well_aggregator.getSWell());
                rstFile.write("XWEL", well_aggregator.getXWell());
                rstFile.write("ZWEL", well_aggregator.getZWell());
            }
            {
                auto conn_aggregator = Opm::RestartIO::Helpers::AggregateConnectionData(ih);
                auto xw = Opm::data::Wells {};
                conn_aggregator.captureDeclaredConnData(sched, grid, es.getUnits(), xw, st, rptStep-1);
                rstFile.write("ICON", conn_aggregator.getIConn());
                rstFile.write("SCON", conn_aggregator.getSConn());
                rstFile.write("XCON", conn_aggregator.getXConn());
            }

            const auto actDims = Opm::RestartIO::Helpers::createActionRSTDims(sched, rptStep-1);
            Opm::RestartIO::Helpers::AggregateActionxData actionxData {sched, action_state, st, rptStep-1};

            rstFile.write("IACT", actionxData.getIACT());
            rstFile.write("SACT", actionxData.getSACT());
            rstFile.write("ZACT", actionxData.getZACT());
            rstFile.write("ZLACT", actionxData.getZLACT());
            rstFile.write("ZACN", actionxData.getZACN());
            rstFile.write("IACN", actionxData.getIACN());
            rstFile.write("SACN", actionxData.getSACN());
            {
                /*
                Check of InteHEAD and DoubHEAD data for UDQ variables

                        INTEHEAD

                        Intehead[156]  -  The number of ACTIONS
                        Intehead[157]  -  The max number of lines of schedule data including ENDACTIO keyword for any
                ACTION

                        ---------------------------------------------------------------------------------------------------------------------]

                */
                const auto rptStep_1 = std::size_t {1};
                const auto ih_1 = Opm::RestartIO::Helpers::createInteHead(
                    es, grid, sched, secs_elapsed, rptStep, rptStep_1, rptStep_1-1);

                BOOST_CHECK_EQUAL(ih_1[156], 2);
                BOOST_CHECK_EQUAL(ih_1[157], 7);



                const auto rptStep_2 = std::size_t {2};
                const auto ih_2 = Opm::RestartIO::Helpers::createInteHead(
                    es, grid, sched, secs_elapsed, rptStep, rptStep_2, rptStep_2-1);
                BOOST_CHECK_EQUAL(ih_2[156], 14);
                BOOST_CHECK_EQUAL(ih_2[157], 10);

                const auto rptStep_3 = std::size_t {3};
                const auto ih_3 = Opm::RestartIO::Helpers::createInteHead(
                    es, grid, sched, secs_elapsed, rptStep, rptStep_3, rptStep_3-1);

                BOOST_CHECK_EQUAL(ih_3[156], 14);
                BOOST_CHECK_EQUAL(ih_3[157], 10);
            }

            {
                /*
                  SACT
                  --length is equal to 9*the number of ACTIONX keywords
                  //item [0]: is unknown, (=0)
                  //item [1]: is unknown, (=0)
                  //item [2]: is unknown, (=0)
                  //item [3]:  Minimum time interval between action triggers.
                  //item [4]:  last time that the action was triggered
                  */


                const auto& sAct = actionxData.getSACT();

                auto start = 0 * actDims[2];
                BOOST_CHECK_CLOSE(sAct[start + 3], 0.543, 1.0e-5f);
                start = 1 * actDims[2];
                BOOST_CHECK_CLOSE(sAct[start + 3], 0.567, 1.0e-5f);
                // actx_14
                start = 13 * actDims[2];
                BOOST_CHECK_CLOSE(sAct[start + 4], 1.E09 / 86400., 1.0e-5f);
            }

            {
                /*
                IACT
                --length is equal to 9*the number of ACTIONX keywords
                    //item [0]: is unknown, (=0)
                    //item [1]: The number of lines of schedule data including ENDACTIO
                    //item [2]: is the number of times an action has been triggered
                    //item [3]: is unknown, (=7)
                    //item [4]: is unknown, (=0)
                    //item [5]: The number of times the action is triggered
                    //item [6]: is unknown, (=0)
                    //item [7]: is unknown, (=0)
                    //item [8]: The number of times the action is triggered
                */


                const auto& iAct = actionxData.getIACT();

                auto start = 0 * actDims[1];
                BOOST_CHECK_EQUAL(iAct[start + 0], 0);
                BOOST_CHECK_EQUAL(iAct[start + 1], 10);
                BOOST_CHECK_EQUAL(iAct[start + 2], 1);
                BOOST_CHECK_EQUAL(iAct[start + 3], 7);
                BOOST_CHECK_EQUAL(iAct[start + 4], 0);
                BOOST_CHECK_EQUAL(iAct[start + 5], 10);
                BOOST_CHECK_EQUAL(iAct[start + 6], 0);
                BOOST_CHECK_EQUAL(iAct[start + 7], 0);
                BOOST_CHECK_EQUAL(iAct[start + 8], 3);


                start = 1 * actDims[1];
                BOOST_CHECK_EQUAL(iAct[start + 0], 0);
                BOOST_CHECK_EQUAL(iAct[start + 1], 7);
                BOOST_CHECK_EQUAL(iAct[start + 2], 1);
                BOOST_CHECK_EQUAL(iAct[start + 3], 7);
                BOOST_CHECK_EQUAL(iAct[start + 4], 0);
                BOOST_CHECK_EQUAL(iAct[start + 5], 11);
                BOOST_CHECK_EQUAL(iAct[start + 6], 0);
                BOOST_CHECK_EQUAL(iAct[start + 7], 0);
                BOOST_CHECK_EQUAL(iAct[start + 8], 3);

                start = 2 * actDims[1];
                BOOST_CHECK_EQUAL(iAct[start + 0], 0);
                BOOST_CHECK_EQUAL(iAct[start + 1], 4);
                BOOST_CHECK_EQUAL(iAct[start + 2], 1);
                BOOST_CHECK_EQUAL(iAct[start + 3], 7);
                BOOST_CHECK_EQUAL(iAct[start + 4], 0);
                BOOST_CHECK_EQUAL(iAct[start + 5], 13);
                BOOST_CHECK_EQUAL(iAct[start + 6], 0);
                BOOST_CHECK_EQUAL(iAct[start + 7], 0);
                BOOST_CHECK_EQUAL(iAct[start + 8], 3);

                start = 13 * actDims[1];
                BOOST_CHECK_EQUAL(iAct[start + 2], 2);
            }

            {
                /*
                SACT
                --length is equal to 9*the number of ACTIONX keywords
                    //item [0]: is unknown, (=0)
                    //item [1]: is unknown, (=0)
                    //item [2]: is unknown, (=0)
                    //item [3]:  Minimum time interval between action triggers.
                    //item [4]: is unknown, (=0)
                */


                const auto& sAct = actionxData.getSACT();

                auto start = 0 * actDims[2];
                BOOST_CHECK_CLOSE(sAct[start + 3], 0.543, 1.0e-5f);
                start = 1 * actDims[2];
                BOOST_CHECK_CLOSE(sAct[start + 3], 0.567, 1.0e-5f);
            }

            {
                /*
                ZACT
                --length 4 times 8-chars pr ACTIONX keyword

                Name of action 4 times 8 chars (up to 8 chars for name)

                */

                const auto& zAct = actionxData.getZACT();

                auto start = 0 * actDims[3];
                BOOST_CHECK_EQUAL(zAct[start + 0].c_str(), "ACT01   ");

                start = 1 * actDims[3];
                BOOST_CHECK_EQUAL(zAct[start + 0].c_str(), "ACT02   ");

                start = 2 * actDims[3];
                BOOST_CHECK_EQUAL(zAct[start + 0].c_str(), "ACT03   ");
            }

            {
                /*
                ZLACT
                   -- length = ACTDIMS_item3*(max-over-action of number of lines of data pr ACTION)

                   */

                const auto& zLact = actionxData.getZLACT();

                // First action
                auto start_a = 0 * actDims[4];
                auto start = start_a + 0 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "WELOPEN ");

                start = start_a + 1 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "'?' 'SHU");
                BOOST_CHECK_EQUAL(zLact[start + 1].c_str(), "T' 0 0 0");
                BOOST_CHECK_EQUAL(zLact[start + 2].c_str(), " /      ");

                start = start_a + 2 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "/       ");

                start = start_a + 3 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "WELOPEN ");

                // Second action
                start_a = 1 * actDims[4];
                start = start_a + 0 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "WELOPEN ");

                start = start_a + 1 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "'?' 'SHU");
                BOOST_CHECK_EQUAL(zLact[start + 1].c_str(), "T' 0 0 0");
                BOOST_CHECK_EQUAL(zLact[start + 2].c_str(), " /      ");

                start = start_a + 2 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "/       ");

                start = start_a + 3 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "WELOPEN ");

                start = start_a + 4 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "'OPL01' ");
                BOOST_CHECK_EQUAL(zLact[start + 1].c_str(), "'OPEN' /");
                BOOST_CHECK_EQUAL(zLact[start + 2].c_str(), "        ");

                start = start_a + 5 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "/       ");


                start = start_a + 6 * actDims[8];
                BOOST_CHECK_EQUAL(zLact[start + 0].c_str(), "ENDACTIO");
            }

            {
                /*
                ZACN
                   //(Max number of conditions pr ACTIONX) * ((max no characters pr line = 104) / (8 - characters pr
                string)(104/8 = 13)

                   */

                const auto& zAcn = actionxData.getZACN();

                // First action
                auto start_a = 0 * actDims[5];
                auto start = start_a + 0 * 13;
                BOOST_CHECK_EQUAL(zAcn[start + 0].c_str(), "FMWPR   ");
                BOOST_CHECK_EQUAL(zAcn[start + 1].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 2].c_str(), ">       ");
                BOOST_CHECK_EQUAL(zAcn[start + 3].c_str(), "        ");

                start = start_a + 1 * 13;
                BOOST_CHECK_EQUAL(zAcn[start + 0].c_str(), "WUPR3   ");
                BOOST_CHECK_EQUAL(zAcn[start + 1].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 2].c_str(), ">       ");
                BOOST_CHECK_EQUAL(zAcn[start + 3].c_str(), "OP*     ");
                BOOST_CHECK_EQUAL(zAcn[start + 4].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 5].c_str(), "        ");

                start = start_a + 2 * 13;
                BOOST_CHECK_EQUAL(zAcn[start + 0].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 1].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 2].c_str(), "<       ");
                BOOST_CHECK_EQUAL(zAcn[start + 3].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 4].c_str(), "        ");



                // Second action
                start_a = 1 * actDims[5];
                start = start_a + 0 * 13;
                BOOST_CHECK_EQUAL(zAcn[start + 0].c_str(), "FMWPR   ");
                BOOST_CHECK_EQUAL(zAcn[start + 1].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 2].c_str(), ">       ");
                BOOST_CHECK_EQUAL(zAcn[start + 3].c_str(), "        ");

                start = start_a + 1 * 13;
                BOOST_CHECK_EQUAL(zAcn[start + 0].c_str(), "WGPR    ");
                BOOST_CHECK_EQUAL(zAcn[start + 1].c_str(), "GGPR    ");
                BOOST_CHECK_EQUAL(zAcn[start + 2].c_str(), ">       ");
                BOOST_CHECK_EQUAL(zAcn[start + 3].c_str(), "OPL02   ");
                BOOST_CHECK_EQUAL(zAcn[start + 4].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 5].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 6].c_str(), "LOWER   ");

                start = start_a + 2 * 13;
                BOOST_CHECK_EQUAL(zAcn[start + 0].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 1].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 2].c_str(), ">       ");
                BOOST_CHECK_EQUAL(zAcn[start + 3].c_str(), "        ");
                BOOST_CHECK_EQUAL(zAcn[start + 4].c_str(), "        ");
            }

            {
                /*
                IACN
                26*Max number of conditions pr ACTIONX

                */


                const auto& iAcn = actionxData.getIACN();

                auto start_a = 0 * actDims[6];
                auto start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 0], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 1], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 2], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 3], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 4], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 5], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 6], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 7], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 8], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 9], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 10], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 11], 8);
                BOOST_CHECK_EQUAL(iAcn[start + 12], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 14], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 16], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 0], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 1], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 2], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 3], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 4], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 5], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 6], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 7], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 8], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 9], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 10], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 11], 8);
                BOOST_CHECK_EQUAL(iAcn[start + 12], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 14], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 16], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start_a = 1 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 0], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 1], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 2], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 3], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 4], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 5], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 6], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 7], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 8], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 9], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 10], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 11], 8);
                BOOST_CHECK_EQUAL(iAcn[start + 12], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 14], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 16], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 0], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 1], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 2], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 3], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 4], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 5], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 6], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 7], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 8], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 9], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 10], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 11], 3);
                BOOST_CHECK_EQUAL(iAcn[start + 12], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 14], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 16], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 0], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 1], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 2], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 3], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 4], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 5], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 6], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 7], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 8], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 9], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 10], 11);
                BOOST_CHECK_EQUAL(iAcn[start + 11], 8);
                BOOST_CHECK_EQUAL(iAcn[start + 12], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 14], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 16], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start_a = 3 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start_a = 4 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start_a = 5 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start_a = 6 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start_a = 7 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 5 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 6 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start_a = 8 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 5 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 6 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);


                start_a = 9 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 5 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 6 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);


                start_a = 10 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 5 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 6 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);


                start_a = 11 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 5 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 6 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);


                start_a = 12 * actDims[6];
                start = start_a + 0 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 1 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 2 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 3 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 4 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 2);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 1);

                start = start_a + 5 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 1);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);

                start = start_a + 6 * 26;
                BOOST_CHECK_EQUAL(iAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 15], 0);
                BOOST_CHECK_EQUAL(iAcn[start + 17], 0);
            }



            {
                /*
                SACN
                26*Max number of conditions pr ACTIONX

                */


                const auto& sAcn = actionxData.getSACN();

                auto start_a = 0 * actDims[6];
                auto start = start_a + 0 * 16;
                BOOST_CHECK_EQUAL(sAcn[start + 0], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 1], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 2], 45);
                BOOST_CHECK_EQUAL(sAcn[start + 3], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 4], 4);
                BOOST_CHECK_EQUAL(sAcn[start + 5], 45);
                BOOST_CHECK_EQUAL(sAcn[start + 6], 4);
                BOOST_CHECK_EQUAL(sAcn[start + 7], 45);
                BOOST_CHECK_EQUAL(sAcn[start + 8], 4);
                BOOST_CHECK_EQUAL(sAcn[start + 9], 45);
                BOOST_CHECK_EQUAL(sAcn[start + 10], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 11], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 12], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 14], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 15], 0);

                start = start_a + 1 * 16;
                BOOST_CHECK_EQUAL(sAcn[start + 0], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 1], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 2], 46);
                BOOST_CHECK_EQUAL(sAcn[start + 3], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 4], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 5], 46);
                BOOST_CHECK_EQUAL(sAcn[start + 6], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 7], 46);
                BOOST_CHECK_EQUAL(sAcn[start + 8], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 9], 46);
                BOOST_CHECK_EQUAL(sAcn[start + 10], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 11], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 12], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 14], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 15], 0);

                start = start_a + 2 * 16;
                BOOST_CHECK_EQUAL(sAcn[start + 0], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 1], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 2], 5);
                BOOST_CHECK_EQUAL(sAcn[start + 3], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 4], 1.E+20);
                BOOST_CHECK_EQUAL(sAcn[start + 5], 1.E+20);
                BOOST_CHECK_EQUAL(sAcn[start + 6], 1.E+20);
                BOOST_CHECK_EQUAL(sAcn[start + 7], 1.E+20);
                BOOST_CHECK_EQUAL(sAcn[start + 8], 1.E+20);
                BOOST_CHECK_EQUAL(sAcn[start + 9], 1.E+20);
                BOOST_CHECK_EQUAL(sAcn[start + 10], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 11], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 12], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 13], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 14], 0);
                BOOST_CHECK_EQUAL(sAcn[start + 15], 0);
            }
        }

        {
            auto rst_file = std::make_shared<Opm::EclIO::ERst>("UDQ_ACTIONX_TEST1.UNRST");
            auto rst_view = std::make_shared<Opm::EclIO::RestartFileView>(std::move(rst_file), 3);
            auto rst_state = Opm::RestartIO::RstState::load(std::move(rst_view), es.runspec(), simCase.parser);
            const auto& input_actions = sched[rptStep-1].actions();

            BOOST_CHECK(rst_state.actions.size() == input_actions.ecl_size());

            BOOST_CHECK_EQUAL(rst_state.actions[0].conditions.size(), 3);
            BOOST_CHECK_EQUAL(rst_state.actions[1].conditions.size(), 3);
            BOOST_CHECK_EQUAL(rst_state.actions[2].conditions.size(), 3);
            BOOST_CHECK_EQUAL(rst_state.actions[3].conditions.size(), 5);
            BOOST_CHECK_EQUAL(rst_state.actions[4].conditions.size(), 5);
            BOOST_CHECK_EQUAL(rst_state.actions[5].conditions.size(), 5);



            {
                const auto& action = rst_state.actions[0];
                BOOST_CHECK(compare_tokens(action.conditions[0].tokens(), {"FMWPR", ">", "45", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[1].tokens(), {"WUPR3", "OP*", ">", "46", "OR"}));
                BOOST_CHECK(compare_tokens(action.conditions[2].tokens(), {"MNTH", "<", "MAY"}));
            }
            {
                const auto& action = rst_state.actions[1];
                BOOST_CHECK(compare_tokens(action.conditions[0].tokens(), {"FMWPR", ">", "25", "AND"}));
                BOOST_CHECK(
                    compare_tokens(action.conditions[1].tokens(), {"WGPR", "OPL02", ">", "GGPR", "LOWER", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[2].tokens(), {"MNTH", ">", "NOV"}));
            }
            {
                const auto& action = rst_state.actions[2];
                BOOST_CHECK(
                    compare_tokens(action.conditions[0].tokens(), {"WWPR", "OPU01", ">", "WWPR", "OPU02", "OR"}));
                BOOST_CHECK(compare_tokens(action.conditions[1].tokens(), {"GMWPR", "TEST", ">", "39", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[2].tokens(), {"YEAR", ">", "2020"}));
            }
            {
                const auto& action = rst_state.actions[3];
                BOOST_CHECK(compare_tokens(action.conditions[0].tokens(), {"FMWPR", ">", "3", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[1].tokens(), {"(", "WUPR3", "OP*", ">", "46", "OR"}));
                BOOST_CHECK(compare_tokens(action.conditions[2].tokens(), {"WOPR", "OP*", ">", "32", ")", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[3].tokens(), {"WWPR", "OP*", ">", "57", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[4].tokens(), {"MNTH", ">", "OCT"}));
            }
            {
                const auto& action = rst_state.actions[7];
                BOOST_CHECK(compare_tokens(action.conditions[0].tokens(), {"(", "FMWPR", ">", "3", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[1].tokens(), {"WUPR3", "OP*", ">", "46", ")", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[2].tokens(), {"WOPR", "OP*", ">", "32", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[3].tokens(), {"(", "WLPR", "OP*", ">", "43", "AND"}));
                BOOST_CHECK(
                    compare_tokens(action.conditions[4].tokens(), {"WWCT", "OP*", ">", "0.310000", ")", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[5].tokens(), {"WWPR", "OP*", ">", "23", "AND"}));
                BOOST_CHECK(compare_tokens(action.conditions[6].tokens(), {"MNTH", ">", "OCT"}));
            }


            std::vector<Opm::Action::ActionX> actions;
            for (const auto& rst_action : rst_state.actions)
                actions.emplace_back(rst_action);

            std::unordered_set<std::string> input_keys;
            std::unordered_set<std::string> rst_keys;
            for (std::size_t iact = 0; iact < input_actions.ecl_size(); iact++) {
                const auto& input_action = input_actions[iact];
                const auto& rst_action = actions[iact];

                BOOST_CHECK_EQUAL(input_action.name(), rst_action.name());
                BOOST_CHECK_EQUAL(input_action.max_run(), rst_action.max_run());
                BOOST_CHECK_CLOSE(input_action.min_wait(), rst_action.min_wait(), 1e-5);

                input_action.required_summary(input_keys);
                rst_action.required_summary(rst_keys);
            }
            BOOST_CHECK(input_keys == rst_keys);

            {
                std::mt19937 rng;
                std::uniform_real_distribution<> random_uniform(-100, 100);
                Opm::WListManager wlm;
                auto year = 2020;
                auto day = 1;
                for (std::size_t month = 1; month <= 12; month++) {
                    auto state = Opm::SummaryState {Opm::asTimePoint(Opm::TimeStampUTC(year, month, day))};
                    Opm::Action::Context context(state, wlm);
                    for (const auto& key : rst_keys) {
                        const auto& first_char = key[0];
                        if (first_char == 'W') {
                            for (const auto& well : {"OPU01", "OPU02", "OPL01", "OPL02"})
                                state.update_well_var(well, key, random_uniform(rng));
                        } else if (first_char == 'G') {
                            for (const auto& group : {"UPPER", "LOWER", "TEST"})
                                state.update_group_var(group, key, random_uniform(rng));
                        } else
                            state.update(key, random_uniform(rng));
                    }

                    for (std::size_t iact = 0; iact < input_actions.ecl_size(); iact++) {
                        const auto& input_action = input_actions[iact];
                        const auto& rst_action = actions[iact];

                        const auto& input_res = input_action.eval(context);
                        const auto& rst_res = rst_action.eval(context);

                        BOOST_CHECK(input_res == rst_res);
                    }
                }
            }
        }
    }
}


BOOST_AUTO_TEST_SUITE_END()
