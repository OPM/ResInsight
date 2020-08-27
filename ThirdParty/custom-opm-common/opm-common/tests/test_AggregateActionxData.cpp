#define BOOST_TEST_MODULE UDQ-ACTIONX_Data

#include <boost/test/unit_test.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/State.hpp>
#include <opm/output/eclipse/AggregateUDQData.hpp>
#include <opm/output/eclipse/AggregateActionxData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/DoubHEAD.hpp>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQInput.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQParams.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionX.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>

#include <opm/io/eclipse/OutputStream.hpp>

#include <stdexcept>
#include <utility>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace {

    Opm::Deck first_sim(std::string fname) {
        return Opm::Parser{}.parseFile(fname);
    }
}

Opm::SummaryState sum_state_TEST1()
    {
        auto state = Opm::SummaryState{std::chrono::system_clock::now()};
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


        state.update_group_var("TEST",  "GMWPR", 4);

        state.update("FWPR", 73.);

        state.update("FMWPR", 4);


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

};

BOOST_AUTO_TEST_SUITE(Aggregate_Actionx)



// test constructed UDQ-Actionx restart data
BOOST_AUTO_TEST_CASE (Declared_Actionx_data)
{
    const auto simCase = SimulationCase{first_sim("UDQ_ACTIONX_TEST1.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::Runspec rspec   = es.runspec();
    Opm::SummaryState st = sum_state_TEST1();
    Opm::Action::State action_state;
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;
    const auto& ioConfig = es.getIOConfig();
    //const auto& restart = es.cfg().restart();


    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t{1};
    std::string outputDir = "./";
    std::string baseName = "UDQ_ACTIONX_TEST1";
    Opm::EclIO::OutputStream::Restart rstFile {
    Opm::EclIO::OutputStream::ResultSet { outputDir, baseName },
    rptStep,
    Opm::EclIO::OutputStream::Formatted { ioConfig.getFMTOUT() },
	  Opm::EclIO::OutputStream::Unified   { ioConfig.getUNIFOUT() }
        };

    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::
        createInteHead(es, grid, sched, secs_elapsed,
                       rptStep, rptStep, rptStep);

    //set dummy value for next_step_size
    const double next_step_size= 0.1;
    const auto dh = Opm::RestartIO::Helpers::createDoubHead(es, sched, rptStep,
                                                            secs_elapsed, next_step_size);

    const auto udqDims = Opm::RestartIO::Helpers::createUdqDims(sched, rptStep, ih);
    auto  udqData = Opm::RestartIO::Helpers::AggregateUDQData(udqDims);
    udqData.captureDeclaredUDQData(sched, rptStep, st, ih);

    const auto actDims = Opm::RestartIO::Helpers::createActionxDims(rspec, sched, rptStep);
    auto  actionxData = Opm::RestartIO::Helpers::AggregateActionxData(actDims);
    actionxData.captureDeclaredActionxData(sched, action_state, st, actDims, rptStep);

    {
        /*
        Check of InteHEAD and DoubHEAD data for UDQ variables

                INTEHEAD

                Intehead[156]  -  The number of ACTIONS
                Intehead[157]  -  The max number of lines of schedule data including ENDACTIO keyword for any ACTION

                ---------------------------------------------------------------------------------------------------------------------]

        */
        const auto rptStep_1 = std::size_t{0};
        const auto ih_1 = Opm::RestartIO::Helpers::createInteHead(es, grid, sched,
                                                secs_elapsed, rptStep, rptStep_1 + 1, rptStep_1);

        BOOST_CHECK_EQUAL(ih_1[156] ,       2);
        BOOST_CHECK_EQUAL(ih_1[157] ,       7);


        const auto rptStep_2 = std::size_t{1};
        const auto ih_2 = Opm::RestartIO::Helpers::createInteHead(es, grid, sched,
                                                secs_elapsed, rptStep, rptStep_2 + 1, rptStep_2);
        BOOST_CHECK_EQUAL(ih_2[156] ,       3);
        BOOST_CHECK_EQUAL(ih_2[157] ,      10);

        const auto rptStep_3 = std::size_t{2};
        const auto ih_3 = Opm::RestartIO::Helpers::createInteHead(es, grid, sched,
                                                secs_elapsed, rptStep, rptStep_3 + 1, rptStep_3);

        BOOST_CHECK_EQUAL(ih_3[156] ,       3);
        BOOST_CHECK_EQUAL(ih_3[157] ,      10);

    }

    {
        /*
        IACT
        --length is equal to 9*the number of ACTIONX keywords
            //item [0]: is unknown, (=0)
            //item [1]: The number of lines of schedule data including ENDACTIO
            //item [2]: is unknown, (=1)
            //item [3]: is unknown, (=7)
            //item [4]: is unknown, (=0)
            //item [5]: The number of times the action is triggered
            //item [6]: is unknown, (=0)
            //item [7]: is unknown, (=0)
            //item [8]: The number of times the action is triggered
        */


        const auto& iAct = actionxData.getIACT();

        auto start = 0*actDims[1];
        BOOST_CHECK_EQUAL(iAct[start + 0] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 1] , 10);
        BOOST_CHECK_EQUAL(iAct[start + 2] ,  1);
        BOOST_CHECK_EQUAL(iAct[start + 3] ,  7);
        BOOST_CHECK_EQUAL(iAct[start + 4] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 5] , 10);
        BOOST_CHECK_EQUAL(iAct[start + 6] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 7] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 8] ,  3);


        start = 1*actDims[1];
        BOOST_CHECK_EQUAL(iAct[start + 0] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 1] ,  7);
        BOOST_CHECK_EQUAL(iAct[start + 2] ,  1);
        BOOST_CHECK_EQUAL(iAct[start + 3] ,  7);
        BOOST_CHECK_EQUAL(iAct[start + 4] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 5] , 11);
        BOOST_CHECK_EQUAL(iAct[start + 6] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 7] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 8] ,  3);

        start = 2*actDims[1];
        BOOST_CHECK_EQUAL(iAct[start + 0] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 1] ,  4);
        BOOST_CHECK_EQUAL(iAct[start + 2] ,  1);
        BOOST_CHECK_EQUAL(iAct[start + 3] ,  7);
        BOOST_CHECK_EQUAL(iAct[start + 4] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 5] , 13);
        BOOST_CHECK_EQUAL(iAct[start + 6] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 7] ,  0);
        BOOST_CHECK_EQUAL(iAct[start + 8] ,  3);

    }

    {
        /*
        ZACT
        --length 4 times 8-chars pr ACTIONX keyword

        Name of action 4 times 8 chars (up to 8 chars for name)

        */

        const auto& zAct = actionxData.getZACT();

        auto start = 0*actDims[3];
        BOOST_CHECK_EQUAL(zAct[start + 0].c_str() ,   "ACT01   ");

        start = 1*actDims[3];
        BOOST_CHECK_EQUAL(zAct[start + 0].c_str() ,   "ACT02   ");

        start = 2*actDims[3];
        BOOST_CHECK_EQUAL(zAct[start + 0].c_str() ,   "ACT03   ");

    }

    {
     /*
     ZLACT
        -- length = ACTDIMS_item3*(max-over-action of number of lines of data pr ACTION)

        */

        const auto& zLact = actionxData.getZLACT();

        //First action
        auto start_a = 0*actDims[4];
        auto start = start_a + 0*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "WELOPEN ");

        start = start_a + 1*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "   '?' '");
        BOOST_CHECK_EQUAL(zLact[start + 1].c_str() ,   "SHUT' 0 ");
        BOOST_CHECK_EQUAL(zLact[start + 2].c_str() ,   "0 0 /   ");

        start = start_a + 2*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "/       ");

        start = start_a +  3*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "WELOPEN ");

        //Second action
        start_a = 1*actDims[4];
        start = start_a + 0*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "WELOPEN ");

        start = start_a + 1*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "   '?' '");
        BOOST_CHECK_EQUAL(zLact[start + 1].c_str() ,   "SHUT' 0 ");
        BOOST_CHECK_EQUAL(zLact[start + 2].c_str() ,   "0 0 /   ");

        start = start_a + 2*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "/       ");

        start = start_a + 3*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "WELOPEN ");

        start = start_a + 4*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "   'OPL0");
        BOOST_CHECK_EQUAL(zLact[start + 1].c_str() ,   "1' 'OPEN");
        BOOST_CHECK_EQUAL(zLact[start + 2].c_str() ,   "' /     ");

        start = start_a + 5*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "/       ");


        start = start_a +  6*actDims[8];
        BOOST_CHECK_EQUAL(zLact[start + 0].c_str() ,   "ENDACTIO");


    }

        {
     /*
     ZACN
        //(Max number of conditions pr ACTIONX) * ((max no characters pr line = 104) / (8 - characters pr string)(104/8 = 13)

        */

        const auto& zAcn = actionxData.getZACN();

        //First action
        auto start_a = 0*actDims[5];
        auto start = start_a + 0*13;
        BOOST_CHECK_EQUAL(zAcn[start + 0].c_str() ,   "FMWPR   ");
        BOOST_CHECK_EQUAL(zAcn[start + 1].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 2].c_str() ,   ">       ");
        BOOST_CHECK_EQUAL(zAcn[start + 3].c_str() ,   "        ");

        start = start_a + 1*13;
        BOOST_CHECK_EQUAL(zAcn[start + 0].c_str() ,   "WUPR3   ");
        BOOST_CHECK_EQUAL(zAcn[start + 1].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 2].c_str() ,   ">       ");
        BOOST_CHECK_EQUAL(zAcn[start + 3].c_str() ,   "OP*     ");
        BOOST_CHECK_EQUAL(zAcn[start + 4].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 5].c_str() ,   "        ");

        start = start_a + 2*13;
        BOOST_CHECK_EQUAL(zAcn[start + 0].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 1].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 2].c_str() ,   ">       ");
        BOOST_CHECK_EQUAL(zAcn[start + 3].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 4].c_str() ,   "        ");



        //Second action
        start_a = 1*actDims[5];
        start = start_a + 0*13;
        BOOST_CHECK_EQUAL(zAcn[start + 0].c_str() ,   "FMWPR   ");
        BOOST_CHECK_EQUAL(zAcn[start + 1].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 2].c_str() ,   ">       ");
        BOOST_CHECK_EQUAL(zAcn[start + 3].c_str() ,   "        ");

        start = start_a + 1*13;
        BOOST_CHECK_EQUAL(zAcn[start + 0].c_str() ,   "WGPR    ");
        BOOST_CHECK_EQUAL(zAcn[start + 1].c_str() ,   "GGPR    ");
        BOOST_CHECK_EQUAL(zAcn[start + 2].c_str() ,   ">       ");
        BOOST_CHECK_EQUAL(zAcn[start + 3].c_str() ,   "OPL02   ");
        BOOST_CHECK_EQUAL(zAcn[start + 4].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 5].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 6].c_str() ,   "LOWER   ");

        start = start_a + 2*13;
        BOOST_CHECK_EQUAL(zAcn[start + 0].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 1].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 2].c_str() ,   ">       ");
        BOOST_CHECK_EQUAL(zAcn[start + 3].c_str() ,   "        ");
        BOOST_CHECK_EQUAL(zAcn[start + 4].c_str() ,   "        ");
    }

        {
        /*
        IACN
        26*Max number of conditions pr ACTIONX

        */


        const auto& iAcn = actionxData.getIACN();

        auto start_a = 0*actDims[6];
        auto start = start_a + 0*26;
        BOOST_CHECK_EQUAL(iAcn[start +  0] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  1] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  2] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  3] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  4] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  5] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  6] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  7] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  8] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  9] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 10] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 11] ,  8);
        BOOST_CHECK_EQUAL(iAcn[start + 12] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 13] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 14] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 15] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 16] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 17] ,  0);

        start = start_a + 1*26;
        BOOST_CHECK_EQUAL(iAcn[start +  0] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  1] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  2] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  3] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  4] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  5] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  6] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  7] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  8] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  9] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 10] ,  2);
        BOOST_CHECK_EQUAL(iAcn[start + 11] ,  8);
        BOOST_CHECK_EQUAL(iAcn[start + 12] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 13] ,  2);
        BOOST_CHECK_EQUAL(iAcn[start + 14] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 15] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 16] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 17] ,  0);

        start_a = 1*actDims[6];
        start = start_a + 0*26;
        BOOST_CHECK_EQUAL(iAcn[start +  0] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  1] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  2] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  3] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  4] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  5] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  6] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  7] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  8] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  9] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 10] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 11] ,  8);
        BOOST_CHECK_EQUAL(iAcn[start + 12] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 13] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 14] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 15] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 16] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 17] ,  0);

        start = start_a + 1*26;
        BOOST_CHECK_EQUAL(iAcn[start +  0] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  1] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  2] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  3] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  4] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  5] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  6] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  7] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  8] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  9] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 10] ,  2);
        BOOST_CHECK_EQUAL(iAcn[start + 11] ,  3);
        BOOST_CHECK_EQUAL(iAcn[start + 12] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 13] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 14] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 15] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 16] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 17] ,  0);

        start = start_a + 2*26;
        BOOST_CHECK_EQUAL(iAcn[start +  0] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  1] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  2] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  3] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  4] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  5] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  6] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  7] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  8] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start +  9] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 10] , 11);
        BOOST_CHECK_EQUAL(iAcn[start + 11] ,  8);
        BOOST_CHECK_EQUAL(iAcn[start + 12] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 13] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 14] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 15] ,  0);
        BOOST_CHECK_EQUAL(iAcn[start + 16] ,  1);
        BOOST_CHECK_EQUAL(iAcn[start + 17] ,  1);

    }



    {
        /*
        SACN
        26*Max number of conditions pr ACTIONX

        */


        const auto& sAcn = actionxData.getSACN();

        auto start_a = 0*actDims[6];
        auto start = start_a + 0*16;
        BOOST_CHECK_EQUAL(sAcn[start +  0] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  1] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  2] , 45);
        BOOST_CHECK_EQUAL(sAcn[start +  3] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  4] ,  4);
        BOOST_CHECK_EQUAL(sAcn[start +  5] , 45);
        BOOST_CHECK_EQUAL(sAcn[start +  6] ,  4);
        BOOST_CHECK_EQUAL(sAcn[start +  7] , 45);
        BOOST_CHECK_EQUAL(sAcn[start +  8] ,  4);
        BOOST_CHECK_EQUAL(sAcn[start +  9] , 45);
        BOOST_CHECK_EQUAL(sAcn[start + 10] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 11] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 12] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 13] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 14] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 15] ,  0);

        start = start_a + 1*16;
        BOOST_CHECK_EQUAL(sAcn[start +  0] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  1] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  2] , 46);
        BOOST_CHECK_EQUAL(sAcn[start +  3] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  4] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  5] , 46);
        BOOST_CHECK_EQUAL(sAcn[start +  6] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  7] , 46);
        BOOST_CHECK_EQUAL(sAcn[start +  8] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  9] , 46);
        BOOST_CHECK_EQUAL(sAcn[start + 10] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 11] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 12] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 13] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 14] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 15] ,  0);

        start = start_a + 2*16;
        BOOST_CHECK_EQUAL(sAcn[start +  0] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  1] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  2] , 10);
        BOOST_CHECK_EQUAL(sAcn[start +  3] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start +  4] ,  1.E+20);
        BOOST_CHECK_EQUAL(sAcn[start +  5] ,  1.E+20);
        BOOST_CHECK_EQUAL(sAcn[start +  6] ,  1.E+20);
        BOOST_CHECK_EQUAL(sAcn[start +  7] ,  1.E+20);
        BOOST_CHECK_EQUAL(sAcn[start +  8] ,  1.E+20);
        BOOST_CHECK_EQUAL(sAcn[start +  9] ,  1.E+20);
        BOOST_CHECK_EQUAL(sAcn[start + 10] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 11] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 12] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 13] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 14] ,  0);
        BOOST_CHECK_EQUAL(sAcn[start + 15] ,  0);

    }

}

BOOST_AUTO_TEST_SUITE_END()
