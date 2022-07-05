#define BOOST_TEST_MODULE UDQ_Data

#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/AggregateGroupData.hpp>
#include <opm/output/eclipse/AggregateWellData.hpp>
#include <opm/output/eclipse/AggregateConnectionData.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>

#include <opm/output/eclipse/AggregateUDQData.hpp>
#include <opm/output/eclipse/WriteRestartHelpers.hpp>

#include <opm/output/eclipse/InteHEAD.hpp>
#include <opm/output/eclipse/VectorItems/intehead.hpp>
#include <opm/output/eclipse/DoubHEAD.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/input/eclipse/Schedule/UDQ/UDQInput.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQParams.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQSet.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/common/utility/TimeService.hpp>
#include <opm/io/eclipse/ERst.hpp>
#include <opm/io/eclipse/RestartFileView.hpp>
#include <opm/io/eclipse/rst/state.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/output/data/Wells.hpp>

#include <opm/io/eclipse/OutputStream.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "tests/WorkArea.hpp"

namespace {

    Opm::Deck first_sim(std::string fname) {
        return Opm::Parser{}.parseFile(fname);
    }
    /*
     Opm::UDQActive udq_active() {
      int update_count = 0;
      // construct record data for udq_active
      Opm::UDQParams params;
      Opm::UDQConfig conf(params);
      Opm::UDQActive udq_act;
      Opm::UDAValue uda1("WUOPRL");
      update_count += udq_act.update(conf, uda1, "PROD1", Opm::UDAControl::WCONPROD_ORAT);

      Opm::UDAValue uda2("WULPRL");
      update_count += udq_act.update(conf, uda2, "PROD1", Opm::UDAControl::WCONPROD_LRAT);
      Opm::UDAValue uda3("WUOPRU");
      update_count += udq_act.update(conf, uda3, "PROD2", Opm::UDAControl::WCONPROD_ORAT);
      Opm::UDAValue uda4("WULPRU");
      update_count += udq_act.update(conf, uda4, "PROD2", Opm::UDAControl::WCONPROD_LRAT);

      for (std::size_t index=0; index < udq_act.IUAD_size(); index++)
      {
          const auto & record = udq_act[index];
          auto ind = record.input_index;
          auto udq_key = record.udq;
          auto name = record.wgname;
          auto ctrl_type = record.control;
       }
      return udq_act;
    }
    */
}


Opm::UDQSet make_udq_set(const std::string& name, Opm::UDQVarType var_type, const std::vector<std::string>& wgnames, const std::vector<double>& values) {
    Opm::UDQSet s(name, var_type, wgnames);
    for (std::size_t i=0; i < values.size(); i++)
        s.assign(i , values[i]);

    return s;
}

    Opm::UDQState make_udq_state()
    {
        auto state = Opm::UDQState{0};

        state.add_define(0, "WUOPRL", make_udq_set("WUOPRL",
                                                   Opm::UDQVarType::WELL_VAR,
                                                   {"PROD1", "PROD2", "WINJ1", "WINJ2"},
                                                   {210, 211, 212, 213}));

        state.add_define(0, "WUOPRU", make_udq_set("WUOPRU",
                                                   Opm::UDQVarType::WELL_VAR,
                                                   {"PROD1", "PROD2", "WINJ1", "WINJ2"},
                                                   {220, 221, 222, 223}));

        // The WULPRL should really be an ASSIGN
        state.add_define(0, "WULPRL", make_udq_set("WULPRL",
                                                   Opm::UDQVarType::WELL_VAR,
                                                   {"PROD1", "PROD2", "WINJ1", "WINJ2"},
                                                   {400, 400, 400, 400}));

        state.add_define(0, "WULPRU", make_udq_set("WULPRU",
                                                   Opm::UDQVarType::WELL_VAR,
                                                   {"PROD1", "PROD2", "WINJ1", "WINJ2"},
                                                   {160, 161, 162, 163}));

        state.add_define(0, "GUOPRU", make_udq_set("GUOPRU",
                                                   Opm::UDQVarType::GROUP_VAR,
                                                   {"WGRP1", "WGRP2", "GRP1"},
                                                   {360, 361, 362}));

        state.add_define(0, "FULPR", Opm::UDQSet::scalar("FULPR", 460));
        return state;
    }

    Opm::SummaryState sum_state()
    {
        auto state = Opm::SummaryState{Opm::TimeService::now()};
        state.update_well_var("PROD1", "WUOPRL", 210.);
        state.update_well_var("PROD2", "WUOPRL", 211.);
        state.update_well_var("WINJ1", "WUOPRL", 212.);
        state.update_well_var("WINJ2", "WUOPRL", 213.);

        state.update_well_var("PROD1", "WULPRL", 400.);
        state.update_well_var("PROD2", "WULPRL", 400.);
        state.update_well_var("WINJ1", "WULPRL", 400.);
        state.update_well_var("WINJ2", "WULPRL", 400.);

        state.update_well_var("PROD1", "WUOPRU", 220.);
        state.update_well_var("PROD2", "WUOPRU", 221.);
        state.update_well_var("WINJ1", "WUOPRU", 222.);
        state.update_well_var("WINJ2", "WUOPRU", 223.);

        state.update_group_var("WGRP1", "GUOPRU", 360.);
        state.update_group_var("WGRP2", "GUOPRU", 361.);
        state.update_group_var("GRP1",  "GUOPRU", 362.);

        state.update_well_var("PROD1", "WULPRU", 160.);
        state.update_well_var("PROD2", "WULPRU", 161.);
        state.update_well_var("WINJ1", "WULPRU", 162.);
        state.update_well_var("WINJ2", "WULPRU", 163.);
        state.update("FULPR", 460.);

        state.update_well_var("PROD1", "WOPR", 1.0);
        state.update_well_var("PROD2", "WOPR", 1.0);
        state.update_well_var("WINJ1", "WOPR", 0.0);
        state.update_well_var("WINJ2", "WOPR", 0.0);
        state.update_well_var("PROD1", "WLPR", 1.0);
        state.update_well_var("PROD2", "WLPR", 1.0);
        state.update_group_var("GRP1", "GOPR", 1.0);
        state.update("FOPR", 145);
        state.update("FLPR", 45);
        state.update("FWPR", 450);

        return state;
    }


//int main(int argc, char* argv[])
struct SimulationCase
{
    explicit SimulationCase(const Opm::Deck& deck)
        : es   { deck }
        , grid { deck }
        , python { std::make_shared<Opm::Python>()}
        , sched{ deck, es, python }
    {}

    // Order requirement: 'es' must be declared/initialised before 'sched'.
    Opm::EclipseState es;
    Opm::EclipseGrid  grid;
    std::shared_ptr<Opm::Python> python;
    Opm::Schedule     sched;
    Opm::Parser       parser;
};

BOOST_AUTO_TEST_SUITE(Aggregate_UDQ)

bool udq_contains(const std::vector<Opm::UDQActive::RstRecord>& records, Opm::UDAControl control, const std::string& udq, const std::string wgname) {
    auto find_iter = std::find_if(records.begin(),
                                  records.end(),
                                  [&control, &udq, &wgname] (const Opm::UDQActive::RstRecord& record) {
                                      return record.control == control &&
                                             record.wgname == wgname &&
                                             record.value.get<std::string>() == udq;
                                  });
    return find_iter != records.end();
}


// test constructed UDQ restart data
BOOST_AUTO_TEST_CASE (Declared_UDQ_data)
{
    const auto simCase = SimulationCase{first_sim("UDQ_TEST_WCONPROD_IUAD-2.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::SummaryState st = sum_state();
    Opm::UDQState     udq_state = make_udq_state();
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;
    const auto& ioConfig = es.getIOConfig();
    //const auto& restart = es.cfg().restart();

    // Report Step 1: 2008-10-10 --> 2011-01-20
    const auto rptStep = std::size_t{1};


    double secs_elapsed = 3.1536E07;
    const auto ih = Opm::RestartIO::Helpers::
        createInteHead(es, grid, sched, secs_elapsed,
                       rptStep, rptStep, rptStep-1);

    //set dummy value for next_step_size
    const double next_step_size= 0.1;
    const auto dh = Opm::RestartIO::Helpers::createDoubHead(es, sched, rptStep-1, rptStep,
                                                            secs_elapsed, next_step_size);

    const auto& lh = Opm::RestartIO::Helpers::createLogiHead(es);

    const auto udqDims = Opm::RestartIO::Helpers::createUdqDims(sched, rptStep-1, ih);
    auto  udqData = Opm::RestartIO::Helpers::AggregateUDQData(udqDims);
    udqData.captureDeclaredUDQData(sched, rptStep-1, udq_state, ih);

    {
        WorkArea work;
        {
            std::string outputDir = "./";
            std::string baseName = "TEST_UDQRST";
            Opm::EclIO::OutputStream::Restart rstFile {Opm::EclIO::OutputStream::ResultSet {outputDir, baseName},
                                                       rptStep,
                                                       Opm::EclIO::OutputStream::Formatted {ioConfig.getFMTOUT()},
                                                       Opm::EclIO::OutputStream::Unified {ioConfig.getUNIFOUT()}};
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
                auto action_state = Opm::Action::State {};
                auto wtest_state = Opm::WellTestState{};
                auto well_aggregator = Opm::RestartIO::Helpers::AggregateWellData(ih);
                well_aggregator.captureDeclaredWellData(sched, es.tracer(), rptStep-1, action_state, wtest_state, st, ih);
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
            rstFile.write("ZUDN", udqData.getZUDN());
            rstFile.write("ZUDL", udqData.getZUDL());
            rstFile.write("IUDQ", udqData.getIUDQ());
            rstFile.write("DUDF", udqData.getDUDF());
            rstFile.write("DUDG", udqData.getDUDG());
            rstFile.write("DUDW", udqData.getDUDW());
            rstFile.write("IUAD", udqData.getIUAD());
            rstFile.write("IUAP", udqData.getIUAP());
            rstFile.write("IGPH", udqData.getIGPH());
        }

        {
            /*
            Check of InteHEAD and DoubHEAD data for UDQ variables

                    INTEHEAD

                    UDQPARAM (1)  = - InteHead [267 ]

                    ---------------------------------------------------------------------------------------------------------------------

                    DOUBHEAD

                    UDQPARAM (2)  =  Doubhead [212]
                    UDQPARAM (3)  =  Doubhead [213]
                    UDQPARAM (4)  =  Doubhead [214]

            */

            BOOST_CHECK_EQUAL(ih[267], -1);
            BOOST_CHECK_EQUAL(dh[212], 1.0E+20);
            BOOST_CHECK_EQUAL(dh[213], 0.0);
            BOOST_CHECK_EQUAL(dh[214], 1.0E-4);
        }


        {
            /*
            IUDQ
            3- integers pr UDQ (line/quantity)

            Integer no 1 = type of UDQ (       0 - ASSIGN, UPDATE-OFF
                                               1-update+NEXT,
                                               2 - DEFINE,  2- UPDATE-ON
                                               3 - units)

            Integer no 2 = -4    : used for  ASSIGN - numerical value
                           -4   : used for DEFINE
                           -1  : used for DEFINE MIN() function, SUM()  function, AVEA() function
                           -4  : used for DEFINE MAX() - function - also used for SUM() function - must check on (-1 -
            value) 1  : used for UPDATE quantity

            Integer no 3 = sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)
                             (1 - based)

            NOTE: UPDATE - does not define a new quantity, only updates an alredy defined quantity!
            */


            const auto& iUdq = udqData.getIUDQ();

            auto start = 0 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 1 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 1
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                1); // udq NO. 1 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 1 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 0); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], 0); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                2); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 2 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                3); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 3 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                1); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 4 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                4); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 5 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                1); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 6 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                2); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 7 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                3); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 8 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -5); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                4); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 9 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -8); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                5); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 10 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                6); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 11 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -5); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                7); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 12 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -3); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                8); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 13 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                9); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 14 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -2); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                10); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 15 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -3); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                11); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 16 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                12); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 17 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                13); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 18 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -3); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                14); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 19 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -5); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                15); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 20 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -5); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                16); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 21 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], 1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                17); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 22 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], 1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                18); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 23 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                19); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 24 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                20); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 25 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -2); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                21); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 26 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                22); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 27 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -2); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                23); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 28 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -2); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                24); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 29 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -8); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                25); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 30 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -6); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                26); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 31 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                27); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 32 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -5); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                28); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 33 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                29); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 34 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -8); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                30); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 35 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                31); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 36 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -5); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                32); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 37 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -6); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                33); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 38 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                34); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 39 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                35); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 40 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -6); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                36); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 41 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -2); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                37); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 42 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -4); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                38); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)

            start = 43 * udqDims[1];
            BOOST_CHECK_EQUAL(iUdq[start + 0], 2); // udq NO. 2 - ( 0 - ASSIGN, 2 - DEFINE)
            BOOST_CHECK_EQUAL(iUdq[start + 1], -1); // udq NO. 2
            BOOST_CHECK_EQUAL(
                iUdq[start + 2],
                39); // udq NO. 2 - (sequence number of UDQ pr type (CU, FU, GU, RU, , SU, WU, AU or BU etc.)
        }

        {
            /*
            IUAD:
            Sequences of 5 items pr UDQ that is used for various well and group controls,
            i.e. sorted on the various active controls, see list for item (1).This means that
            one udq can occur several times, one for each control it is used for
            Only the active controls are output - and the sequence is according to when
            they are defined

            dimension 5*no_of_udq-constraint-used in well and group controls

            item (1) : =    200000 + 19 for GCONPROD  and  ORAT
                            300000 + 19 for GCONPROD  and  WRAT
                            400000 + 19 for GCONPROD  and  GRAT
                            500000 + 19 for GCONPROD  and   LRAT
                            300000 + 4   for WCONPROD + oil rate target or upper limit
                            400000 + 4   for WCONPROD + water rate target or upper limit
                            500000 + 4   for WCONPROD + gas rate target or upper limit
                            600000 + 4   for WCONPROD + liquid rate target or upper limit
                            ? 300000 + 3   for WCONINJE   + oil rate target or upper limit
                            400000 + 3   for WCONINJE   + surface rate target or upper limit
                            500000 + 3   for WCONINJE   + reservoir volume  rate target or upper limit
                            1000000 + 27 for CECON  + minimum oil rate

            item (2)  - sequence number of UDQ used (from input sequence) for the actual constraint/target

            item (3)  - do not know yet  (value: 1)
            item (4)  - number of times the UDQ variable is used (e.g. for several different wells)
            item (5)  - the sequence number for the first use of the actual UDQ (index  i+1) = 1+sum over <the first i
            udq's in use >(no_use_udq(i))
            */

            const auto& iUad = udqData.getIUAD();

            auto start = 0 * udqDims[3];
            BOOST_CHECK_EQUAL(iUad[start + 0], 300004); // iuad NO. 1
            BOOST_CHECK_EQUAL(iUad[start + 1], 3); // iuad NO. 1
            BOOST_CHECK_EQUAL(iUad[start + 2], 1); // iuad NO. 1
            BOOST_CHECK_EQUAL(iUad[start + 3], 2); // iuad NO. 1
            BOOST_CHECK_EQUAL(iUad[start + 4], 1); // iuad NO. 1

            start = 1 * udqDims[3];
            BOOST_CHECK_EQUAL(iUad[start + 0], 600004); // iuad NO. 2
            BOOST_CHECK_EQUAL(iUad[start + 1], 5); // iuad NO. 2
            BOOST_CHECK_EQUAL(iUad[start + 2], 1); // iuad NO. 2
            BOOST_CHECK_EQUAL(iUad[start + 3], 2); // iuad NO. 2
            BOOST_CHECK_EQUAL(iUad[start + 4], 3); // iuad NO. 2
        }

        {
            /*
            ZUDN:
            contains  UDQ keyword data:
            Pairs of:
                quantity name (item2): e.g. 'WUOPRL  '  and
                units: e.g.: 'SM3/DAY '

            Length is  dependent on number of UDQ quantities =  2*no of UDQ's
            */

            const auto& zUdn = udqData.getZUDN();

            auto start = 0 * udqDims[4];
            BOOST_CHECK_EQUAL(zUdn[start + 0].c_str(), "WUOPRL  "); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdn[start + 1].c_str(), "SM3/DAY "); // udq NO. 1

            start = 1 * udqDims[4];
            BOOST_CHECK_EQUAL(zUdn[start + 0].c_str(), "WULPRL  "); // udq NO. 2
            BOOST_CHECK_EQUAL(zUdn[start + 1].c_str(), "SM3/DAY "); // udq NO. 2

            start = 2 * udqDims[4];
            BOOST_CHECK_EQUAL(zUdn[start + 0].c_str(), "WUOPRU  "); // udq NO. 3
            BOOST_CHECK_EQUAL(zUdn[start + 1].c_str(), "SM3/DAY "); // udq NO. 3

            start = 3 * udqDims[4];
            BOOST_CHECK_EQUAL(zUdn[start + 0].c_str(), "GUOPRU  "); // udq NO. 4
            BOOST_CHECK_EQUAL(zUdn[start + 1].c_str(), "SM3/DAY "); // udq NO. 4

            start = 4 * udqDims[4];
            BOOST_CHECK_EQUAL(zUdn[start + 0].c_str(), "WULPRU  "); // udq NO. 5
            BOOST_CHECK_EQUAL(zUdn[start + 1].c_str(), "SM3/DAY "); // udq NO. 5

            start = 5 * udqDims[4];
            BOOST_CHECK_EQUAL(zUdn[start + 0].c_str(), "FULPR   "); // udq NO. 6
            BOOST_CHECK_EQUAL(zUdn[start + 1].c_str(), "SM3/DAY "); // udq NO. 6
        }


        {
            /*
            ZUDL:
            contains string that define the "Data for operation" for the defined quantity

            e.g.
            '(WOPR OP' 'L01 - 15' '0) * 0.9' '0       ' '        ' '        ' '        '

            The appropriate data are split into strings of 8 characters each.

            Length: No of UDQ's * 16
            */

            const auto& zUdl = udqData.getZUDL();

            auto start = 0 * udqDims[5];
            BOOST_CHECK_EQUAL(zUdl[start + 0].c_str(), "(WOPR 'P"); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 1].c_str(), "ROD1' - "); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 2].c_str(), "170) * 0"); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 3].c_str(), ".6      "); // udq NO. 1

            start = 3 * udqDims[5];
            BOOST_CHECK_EQUAL(zUdl[start + 0].c_str(), "(GOPR 'G"); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 1].c_str(), "RP1' - 4"); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 2].c_str(), "49) * 0."); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 3].c_str(), "77      "); // udq NO. 1

            start = 4 * udqDims[5];
            BOOST_CHECK_EQUAL(zUdl[start + 0].c_str(), "(WLPR 'P"); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 1].c_str(), "ROD2' - "); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 2].c_str(), "300) * 0"); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 3].c_str(), ".8      "); // udq NO. 1

            start = 5 * udqDims[5];
            BOOST_CHECK_EQUAL(zUdl[start + 0].c_str(), "(FLPR - "); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 1].c_str(), "543) * 0"); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 2].c_str(), ".65     "); // udq NO. 1
            BOOST_CHECK_EQUAL(zUdl[start + 3].c_str(), "        "); // udq NO. 1
        }


        {
            /*
            'DUDW    '          24 'DOUB'

            Dimension = max no wells * no of UDQ's
            Value = value of UDQ for the different wells
            */

            const auto& dUdw = udqData.getDUDW();

            auto start = 0 * udqDims[8];
            BOOST_CHECK_EQUAL(dUdw[start + 0], 210); // duDw NO. 1
            BOOST_CHECK_EQUAL(dUdw[start + 1], 211); // duDw NO. 1
            BOOST_CHECK_EQUAL(dUdw[start + 2], 212); // duDw NO. 1
            BOOST_CHECK_EQUAL(dUdw[start + 3], 213); // duDw NO. 1
            BOOST_CHECK_EQUAL(dUdw[start + 4], -0.3E+21); // duDw NO. 1

            start = 1 * udqDims[8];
            BOOST_CHECK_EQUAL(dUdw[start + 0], 400); // duDw NO. 1
            BOOST_CHECK_EQUAL(dUdw[start + 1], 400); // duDw NO. 1
            BOOST_CHECK_EQUAL(dUdw[start + 2], 400); // duDw NO. 1
            BOOST_CHECK_EQUAL(dUdw[start + 3], 400); // duDw NO. 1
            BOOST_CHECK_EQUAL(dUdw[start + 4], -0.3E+21); // duDw NO. 1
        }

        {
            /*
            'DUDG    '          5 'DOUB'

            Dimension = (max no groups+1) * no of group UDQ's
            Value = value of UDQ for the different groups
            */

            const auto& dUdg = udqData.getDUDG();

            auto start = 0 * udqDims[11];
            BOOST_CHECK_EQUAL(dUdg[start + 0], 362); // duDg NO. 1
            BOOST_CHECK_EQUAL(dUdg[start + 1], 360); // duDg NO. 1
            BOOST_CHECK_EQUAL(dUdg[start + 2], 361); // duDg NO. 1
            BOOST_CHECK_EQUAL(dUdg[start + 3], -0.3E+21); // duDg NO. 1
            BOOST_CHECK_EQUAL(dUdg[start + 4], -0.3E+21); // duDg NO. 1
        }


        {
            /*
            'DUDG    '          1 'DOUB'

            Dimension = 1 * no of Field UDQ's
            Value = value of UDQ for the field
            */

            const auto& dUdf = udqData.getDUDF();

            auto start = 0 * udqDims[12];
            BOOST_CHECK_EQUAL(dUdf[start + 0], 460); // duDf NO. 1
        }

        {
            auto rst_file = std::make_shared<Opm::EclIO::ERst>("TEST_UDQRST.UNRST");
            auto rst_view = std::make_shared<Opm::EclIO::RestartFileView>(std::move(rst_file), 1);
            auto rst_state = Opm::RestartIO::RstState::load(std::move(rst_view), es.runspec(), simCase.parser);
            BOOST_CHECK_EQUAL(rst_state.header.nwell_udq, 4);
            BOOST_CHECK_EQUAL(rst_state.header.ngroup_udq, 1);
            BOOST_CHECK_EQUAL(rst_state.header.nfield_udq, 39);
            BOOST_CHECK_EQUAL(rst_state.header.num_udq(), 44);
            BOOST_CHECK_EQUAL(rst_state.udqs.size(), 44);

            std::vector<std::pair<std::string, std::string>> expected = {{"WUOPRL", "SM3/DAY"},
                                                                         {"WULPRL", "SM3/DAY"},
                                                                         {"WUOPRU", "SM3/DAY"},
                                                                         {"GUOPRU", "SM3/DAY"},
                                                                         {"WULPRU", "SM3/DAY"},
                                                                         {"FULPR", "SM3/DAY"}};

            std::size_t iudq = 0;
            for (const auto& [name, unit] : expected) {
                BOOST_CHECK_EQUAL(name, rst_state.udqs[iudq].name);
                BOOST_CHECK_EQUAL(unit, rst_state.udqs[iudq].unit);
                iudq += 1;
            }


            const std::size_t report_step = 1;
            const auto& udq_params = es.runspec().udqParams();
            const auto& input_config = sched[report_step].udq();
            Opm::UDQConfig rst_config(udq_params, rst_state);
            BOOST_CHECK_EQUAL(input_config.size(), rst_config.size());
            BOOST_CHECK_EQUAL(input_config.definitions().size(), rst_config.definitions().size());

            const std::vector<std::string>& wells = {"PROD1", "PROD2", "WINJ1", "WINJ2"};
            Opm::UDQState rst_udq_state(udq_params.undefinedValue());
            Opm::UDQFunctionTable udqft(udq_params);
            Opm::UDQContext input_context(udqft, Opm::WellMatcher(wells), st, udq_state);
            Opm::UDQContext rst_context(udqft, Opm::WellMatcher(wells), st, rst_udq_state);

            rst_udq_state.load_rst(rst_state);
            for (const auto& input_def : input_config.definitions()) {
                const auto& rst_def = rst_config.define(input_def.keyword());
                auto input_eval = input_def.eval(input_context);
                auto rst_eval = rst_def.eval(rst_context);

                BOOST_CHECK(input_eval == rst_eval);
            }

            for (const auto& input_assign : input_config.assignments()) {
                const auto& rst_assign = rst_config.assign(input_assign.keyword());

                auto input_eval = input_assign.eval(wells);
                auto rst_eval = rst_assign.eval(wells);

                BOOST_CHECK(input_eval == rst_eval);
            }


            const auto& uda_records = Opm::UDQActive::load_rst(
                es.getUnits(), input_config, rst_state, sched.wellNames(report_step), sched.groupNames(report_step));

            BOOST_CHECK_EQUAL(uda_records.size(), 4);
            BOOST_CHECK(udq_contains(uda_records, Opm::UDAControl::WCONPROD_ORAT, "WUOPRU", "PROD1"));
            BOOST_CHECK(udq_contains(uda_records, Opm::UDAControl::WCONPROD_LRAT, "WULPRU", "PROD1"));
            BOOST_CHECK(udq_contains(uda_records, Opm::UDAControl::WCONPROD_ORAT, "WUOPRU", "PROD2"));
            BOOST_CHECK(udq_contains(uda_records, Opm::UDAControl::WCONPROD_LRAT, "WULPRU", "PROD2"));
        }
    }
}

// test constructed UDQ restart data
BOOST_AUTO_TEST_CASE (Declared_UDQ_data_2)
{
    const auto simCase = SimulationCase{first_sim("9_4C_WINJ_GINJ_UDQ_MSW-UDARATE_TEST_PACK.DATA")};

    Opm::EclipseState es = simCase.es;
    Opm::SummaryState st = sum_state();
    Opm::UDQState     udq_state = make_udq_state();
    Opm::Schedule     sched = simCase.sched;
    Opm::EclipseGrid  grid = simCase.grid;

    // Report Step 1: 2018-12-05
    auto rptStep = std::size_t{1};
    auto simStep = rptStep - 1;


    double secs_elapsed = 3.1536E07;
    auto ih = Opm::RestartIO::Helpers::
        createInteHead(es, grid, sched, secs_elapsed,
                       rptStep, rptStep, simStep);

    //set dummy value for next_step_size
    double next_step_size= 0.1;
    auto dh = Opm::RestartIO::Helpers::createDoubHead(es, sched, simStep, simStep+1,
                secs_elapsed, next_step_size);

    auto lh = Opm::RestartIO::Helpers::createLogiHead(es);

    auto udqDims = Opm::RestartIO::Helpers::createUdqDims(sched, simStep, ih);
    auto  udqData = Opm::RestartIO::Helpers::AggregateUDQData(udqDims);
    udqData.captureDeclaredUDQData(sched, simStep, udq_state, ih);


    {
        const auto& iGph = udqData.getIGPH();

        auto start = 0*udqDims[1];
        BOOST_CHECK_EQUAL(iGph[start + 0] ,  3); // (3 - gas injection)
    }

    // Report Step 4: 2018-12-20
    rptStep = std::size_t{4};
    simStep = rptStep - 1;

    ih = Opm::RestartIO::Helpers::
        createInteHead(es, grid, sched, secs_elapsed,
                       rptStep, rptStep, simStep);

    //set dummy value for next_step_size
    next_step_size= 0.1;
    dh = Opm::RestartIO::Helpers::createDoubHead(es, sched, simStep, simStep+1,
            secs_elapsed, next_step_size);

    lh = Opm::RestartIO::Helpers::createLogiHead(es);

    udqDims = Opm::RestartIO::Helpers::createUdqDims(sched, simStep, ih);
    udqData = Opm::RestartIO::Helpers::AggregateUDQData(udqDims);
    udqData.captureDeclaredUDQData(sched, simStep, udq_state, ih);


    {
        const auto& iGph = udqData.getIGPH();

        auto start = 0*udqDims[1];
        BOOST_CHECK_EQUAL(iGph[start + 0] ,  2); // (2 - water injection)
    }

}

BOOST_AUTO_TEST_SUITE_END()
