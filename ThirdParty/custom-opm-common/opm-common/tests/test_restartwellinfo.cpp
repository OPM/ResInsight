
/*
  Copyright 2014 Statoil IT
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

#include "config.h"

#define BOOST_TEST_MODULE EclipseWriter
#include <opm/common/utility/platform_dependent/disable_warnings.h>
#include <boost/test/unit_test.hpp>
#include <opm/common/utility/platform_dependent/reenable_warnings.h>

#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/output/eclipse/EclipseIO.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Schedule/Well/WellConnections.hpp>
#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>

#include <opm/io/eclipse/EclFile.hpp>

#include <tuple>
#include <stdio.h>

#include "tests/WorkArea.hpp"

void verifyWellState(const std::string& rst_filename, const Opm::Schedule& schedule) {

  /*
      reference variables ref_wellList, ref_wellHead and ref_wellConn are 
      based on WELSPECS and COMPDAT in data deck -> testblackoilstate3.DATA.  

  */
  
    int step = std::stoi( rst_filename.substr(rst_filename.size()-1)) -1;
    Opm::EclIO::EclFile rstFile(rst_filename);

    std::vector<std::vector<std::string>> ref_wellList = {
        {},
        {"OP_1", "OP_2", "OP_3"},
        {"OP_1", "OP_2", "OP_3", "OP_4", "OP_5", "OP_6"},
        {"OP_1", "OP_2", "OP_3", "OP_4", "OP_5", "OP_6", "WI_1", "WI_2", "GI_1"}
    };

    using T2 = std::tuple<int,int>;
    using T3 = std::tuple<int,int,int>;

    std::vector<std::vector<std::tuple<int, int>>> ref_wellHead = {
        {},
        {T2{9,9}, T2{8,8}, T2{7,7}},
        {T2{9,9}, T2{8,8}, T2{7,7}, T2{2,2}, T2{5,4}, T2{8,2}},
        {T2{9,9}, T2{8,8}, T2{7,7}, T2{2,2}, T2{5,4}, T2{8,2}, T2{3,3}, T2{3,9}, T2{3,6}}
    };

    std::vector<std::vector<std::vector<std::tuple<int, int, int>>>> ref_wellConn = {
        {{{}}},  // <- timestep 0
        {   {T3{9,9,1},T3{9,9,2},T3{9,9,3},T3{9,9,4},T3{9,9,5},T3{9,9,6},T3{9,9,7},T3{9,9,8},T3{9,9,9},T3{9,9,10}}, // OP_1
            {T3{8,8,1},T3{8,8,2},T3{8,8,3},T3{8,7,3},T3{8,7,4},T3{8,7,5},T3{8,7,6}}, // OP_2
            {T3{7,7,1},T3{7,7,2},T3{7,7,3},T3{7,7,4},T3{7,7,5}} // OP_3
        }, // <- timestep 1
        {   {T3{9,9,1},T3{9,9,2},T3{9,9,3},T3{9,9,4},T3{9,9,5},T3{9,9,6},T3{9,9,7},T3{9,9,8},T3{9,9,9},T3{9,9,10}},
            {T3{8,8,1},T3{8,8,2},T3{8,8,3},T3{8,7,3},T3{8,7,4},T3{8,7,5},T3{8,7,6}}, // OP_2
            {T3{7,7,1},T3{7,7,2},T3{7,7,3},T3{7,7,4},T3{7,7,5}},  // OP_3
            {T3{2,2,1},T3{2,2,2},T3{2,2,3},T3{2,2,4},T3{2,2,5},T3{2,2,6},T3{2,2,7},T3{2,2,8},T3{2,2,9},T3{2,2,10}}, // OP_4
            {T3{5,4,1},T3{5,4,2},T3{5,4,3}},  // OP_5
            {T3{8,2,1},T3{8,2,2},T3{8,2,3},T3{8,3,3},T3{8,4,3},T3{8,5,3},T3{8,5,4},T3{8,5,5},T3{8,5,6}} // OP_6
        }, // <- timestep 2
        {   {T3{9,9,1},T3{9,9,2},T3{9,9,3},T3{9,9,4},T3{9,9,5},T3{9,9,6},T3{9,9,7},T3{9,9,8},T3{9,9,9},T3{9,9,10}},
            {T3{8,8,1},T3{8,8,2},T3{8,8,3},T3{8,7,3},T3{8,7,4},T3{8,7,5},T3{8,7,6}}, // OP_2
            {T3{7,7,1},T3{7,7,2},T3{7,7,3},T3{7,7,4},T3{7,7,5}},  // OP_3
            {T3{2,2,1},T3{2,2,2},T3{2,2,3},T3{2,2,4},T3{2,2,5},T3{2,2,6},T3{2,2,7},T3{2,2,8},T3{2,2,9},T3{2,2,10}}, // OP_4
            {T3{5,4,1},T3{5,4,2},T3{5,4,3}},  // OP_5
            {T3{8,2,1},T3{8,2,2},T3{8,2,3},T3{8,3,3},T3{8,4,3},T3{8,5,3},T3{8,5,4},T3{8,5,5},T3{8,5,6}}, // OP_6
            {T3{3,3,1},T3{3,3,2},T3{3,3,3},T3{3,3,4},T3{3,3,5},T3{3,3,6},T3{3,3,7},T3{3,3,8},T3{3,3,9},T3{3,3,10}},  // WI_1
            {T3{3,9,1},T3{3,9,2},T3{3,9,3},T3{3,9,4},T3{3,9,5},T3{3,9,6},T3{3,9,7}},  // WI_2
            {T3{3,6,1},T3{3,6,2},T3{3,6,3}}   // WI_3
        } // <- timestep 3

    };

    std::vector<int> intehead = rstFile.get<int>("INTEHEAD");
    std::vector<std::string> zwel;
    std::vector<int> iwel;
    std::vector<int> icon;

    int ncwmax = intehead[17];
    int niwelz = intehead[24];
    int niconz = intehead[32];

    if (rstFile.hasKey("ZWEL")) {
        zwel = rstFile.get<std::string>("ZWEL");
    }

    if (rstFile.hasKey("IWEL")) {
        iwel = rstFile.get<int>("IWEL");
    }

    if (rstFile.hasKey("ICON")) {
        icon = rstFile.get<int>("ICON");
    }

    const auto& wellList = schedule.getWells(step);

    //Verify number of active wells
    BOOST_CHECK_EQUAL( wellList.size(), static_cast<std::size_t>(intehead[16]));

    for (size_t i=0; i< wellList.size(); i++) {

        // Verify wellname
        BOOST_CHECK_EQUAL(zwel[i*3], ref_wellList[step][i]);
        BOOST_CHECK_EQUAL(zwel[i*3], wellList[i].name());

        // Verify well I, J head

        BOOST_CHECK_EQUAL(iwel[i*niwelz], std::get<0>(ref_wellHead[step][i]));
        BOOST_CHECK_EQUAL(iwel[i*niwelz + 1], std::get<1>(ref_wellHead[step][i]));

        Opm::Well sched_well2 = schedule.getWell(wellList[i].name(), step);

        BOOST_CHECK_EQUAL(iwel[i*niwelz], sched_well2.getHeadI() +1 );
        BOOST_CHECK_EQUAL(iwel[i*niwelz + 1], sched_well2.getHeadJ() +1 );

        int sched_wtype = -99;

        if (sched_well2.isProducer()) {
            sched_wtype = 1;
        } else {
            switch( sched_well2.getInjectionProperties(  ).injectorType ) {
            case Opm::InjectorType::WATER:
                sched_wtype = 3;
                break;
            case Opm::InjectorType::GAS:
                sched_wtype = 4;
                break;
            case Opm::InjectorType::OIL:
                sched_wtype = 2;
                break;
            default:
                break;
            }
        }

        // Verify well type
        //    1 = producer, 2 = oil injection, 3 = water injector, 4 = gas injector

        BOOST_CHECK_EQUAL(iwel[i*niwelz + 6], sched_wtype );

        const auto& connections_set = sched_well2.getConnections();

        // Verify number of connections

        BOOST_CHECK_EQUAL(static_cast<std::size_t>(iwel[i*niwelz + 4]), connections_set.size() );
        BOOST_CHECK_EQUAL(ref_wellConn[step][i].size(), connections_set.size() );


        for (size_t n=0; n< connections_set.size(); n++) {
            const auto& completion = connections_set.get(n);

            // Verify I, J and K indices for each connection

            BOOST_CHECK_EQUAL(completion.getI()+1 , std::get<0>(ref_wellConn[step][i][n]));
            BOOST_CHECK_EQUAL(completion.getJ()+1 , std::get<1>(ref_wellConn[step][i][n]));
            BOOST_CHECK_EQUAL(completion.getK()+1 , std::get<2>(ref_wellConn[step][i][n]));

            size_t ind = i*ncwmax*niconz+n*niconz;

            BOOST_CHECK_EQUAL(completion.getI()+1 , icon[ind+1]);
            BOOST_CHECK_EQUAL(completion.getJ()+1 , icon[ind+2]);
            BOOST_CHECK_EQUAL(completion.getK()+1 , icon[ind+3]);
        }
    }
}


BOOST_AUTO_TEST_CASE(EclipseWriteRestartWellInfo) {

    WorkArea work;
    std::string eclipse_data_filename    = "testblackoilstate3.DATA";
    work.copyIn(eclipse_data_filename);

    auto python = std::make_shared<Opm::Python>();
    Opm::Parser parser;
    Opm::Deck deck( parser.parseFile( eclipse_data_filename ));
    Opm::EclipseState es(deck);
    const Opm::EclipseGrid& grid = es.getInputGrid();
    Opm::Schedule schedule( deck, es, python);
    Opm::SummaryConfig summary_config( deck, schedule, es.fieldProps(), es.aquifer());
    const auto num_cells = grid.getCartesianSize();
    Opm::EclipseIO eclipseWriter( es,  grid , schedule, summary_config);
    int countTimeStep = schedule.size() - 1;
    Opm::SummaryState st(Opm::TimeService::from_time_t(schedule.getStartTime()));
    Opm::Action::State action_state;
    Opm::UDQState udq_state(123);

    Opm::data::Solution solution;
    solution.insert( "PRESSURE", Opm::UnitSystem::measure::pressure , std::vector< double >( num_cells, 1 ) , Opm::data::TargetType::RESTART_SOLUTION);
    solution.insert( "SWAT"    , Opm::UnitSystem::measure::identity , std::vector< double >( num_cells, 1 ) , Opm::data::TargetType::RESTART_SOLUTION);
    solution.insert( "SGAS"    , Opm::UnitSystem::measure::identity , std::vector< double >( num_cells, 1 ) , Opm::data::TargetType::RESTART_SOLUTION);
    Opm::data::Wells wells;
    Opm::data::GroupAndNetworkValues group_nwrk;
    Opm::WellTestState wtest_state;

    for(int timestep = 0; timestep <= countTimeStep; ++timestep) {

        eclipseWriter.writeTimeStep( action_state,
                                     wtest_state,
                                     st,
                                     udq_state,
                                     timestep,
                                     false,
                                     schedule.seconds(timestep),
                                     Opm::RestartValue(solution, wells, group_nwrk, {}));
    }

    for (int i=1; i <=4; i++) {
        verifyWellState("TESTBLACKOILSTATE3.X000" + std::to_string(i), schedule);
    }

    // cleaning up after test

    for (int i=1; i <=4; i++) {
        std::string fileName = "TESTBLACKOILSTATE3.X000" + std::to_string(i);
        remove(fileName.c_str());

        fileName = "testblackoilstate3.s000" + std::to_string(i);
        remove(fileName.c_str());
    }

    remove("testblackoilstate3.smspec");
}
