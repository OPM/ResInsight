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

#include <cstdlib>

#define BOOST_TEST_MODULE EclipseIO
#include <boost/test/unit_test.hpp>

#include <opm/output/eclipse/EclipseIO.hpp>
#include <opm/output/eclipse/RestartIO.hpp>
#include <opm/output/eclipse/RestartValue.hpp>
#include <opm/output/data/Cells.hpp>
#include <opm/output/data/Wells.hpp>
#include <opm/output/data/Groups.hpp>
#include <opm/parser/eclipse/Python/Python.hpp>

#include <opm/parser/eclipse/EclipseState/Tables/Eqldims.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Utility/Functional.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/State.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>

#include <opm/io/eclipse/OutputStream.hpp>
#include <opm/io/eclipse/EclIOdata.hpp>
#include <opm/io/eclipse/ERst.hpp>

#include <tuple>

#include <opm/common/utility/TimeService.hpp>

#include <tests/WorkArea.cpp>

using namespace Opm;

namespace {
    int ecl_file_get_num_named_kw(Opm::EclIO::ERst&  rst,
                                  const std::string& kw)
    {
        int count = 0;
        for (const auto& step : rst.listOfReportStepNumbers()) {
            for (const auto& vec : rst.listOfRstArrays(step)) {
                count += std::get<0>(vec) == kw;
            }
        }

        return count;
    }

    EclIO::EclFile::EclEntry
    ecl_file_iget_named_kw(Opm::EclIO::ERst&  rst,
                           const std::string& kw,
                           const int          seqnum)
    {
        for (const auto& vec : rst.listOfRstArrays(seqnum)) {
            if (std::get<0>(vec) == kw) {
                return vec;
            }
        }

        return EclIO::EclFile::EclEntry{ "NoSuchKeyword", Opm::EclIO::eclArrType::MESS, 0 };
    }

    EclIO::eclArrType ecl_kw_get_type(const EclIO::EclFile::EclEntry& vec)
    {
        return std::get<1>(vec);
    }
}


namespace Opm {
namespace data {

/*
 * Some test specific equivalence definitions and pretty-printing. Not fit as a
 * general purpose implementation, but does its job for testing and
 * pretty-pringing for debugging purposes.
 */

std::ostream& operator<<( std::ostream& stream, const Rates& r ) {
    return stream << "{ "
                  << "wat: " << r.get( Rates::opt::wat, 0.0 ) << ", "
                  << "oil: " << r.get( Rates::opt::oil, 0.0 ) << ", "
                  << "gas: " << r.get( Rates::opt::gas, 0.0 ) << " "
                  << "}";
}

std::ostream& operator<<( std::ostream& stream, const Connection& c ) {
    return stream << "{ index: "
                  << c.index << ", "
                  << c.rates << ", "
                  << c.pressure << " }";
}

std::ostream& operator<<( std::ostream& stream,
                          const std::map< std::string, Well >& m ) {
    stream << "\n";

    for( const auto& p : m ) {
        stream << p.first << ": \n"
               << "\t" << "bhp: " << p.second.bhp << "\n"
               << "\t" << "temp: " << p.second.temperature << "\n"
               << "\t" << "rates: " << p.second.rates << "\n"
               << "\t" << "connections: [\n";

        for( const auto& c : p.second.connections )
            stream << c << " ";

        stream << "]\n";
    }

    return stream;
}

}

data::GroupValues mkGroups() {
    return {};
}

data::Wells mkWells() {
    data::Rates r1, r2, rc1, rc2, rc3;
    r1.set( data::Rates::opt::wat, 5.67 );
    r1.set( data::Rates::opt::oil, 6.78 );
    r1.set( data::Rates::opt::gas, 7.89 );

    r2.set( data::Rates::opt::wat, 8.90 );
    r2.set( data::Rates::opt::oil, 9.01 );
    r2.set( data::Rates::opt::gas, 10.12 );

    rc1.set( data::Rates::opt::wat, 20.41 );
    rc1.set( data::Rates::opt::oil, 21.19 );
    rc1.set( data::Rates::opt::gas, 22.41 );

    rc2.set( data::Rates::opt::wat, 23.19 );
    rc2.set( data::Rates::opt::oil, 24.41 );
    rc2.set( data::Rates::opt::gas, 25.19 );

    rc3.set( data::Rates::opt::wat, 26.41 );
    rc3.set( data::Rates::opt::oil, 27.19 );
    rc3.set( data::Rates::opt::gas, 28.41 );

    data::Well w1, w2;
    w1.rates = r1;
    w1.thp = 1.0;
    w1.bhp = 1.23;
    w1.temperature = 3.45;
    w1.control = 1;

    /*
     *  the completion keys (active indices) and well names correspond to the
     *  input deck. All other entries in the well structures are arbitrary.
     */
    w1.connections.push_back( { 88, rc1, 30.45, 123.4, 543.21, 0.62, 0.15, 1.0e3 } );
    w1.connections.push_back( { 288, rc2, 33.19, 123.4, 432.1, 0.26, 0.45, 2.56 } );

    w2.rates = r2;
    w2.thp = 2.0;
    w2.bhp = 2.34;
    w2.temperature = 4.56;
    w2.control = 2;
    w2.connections.push_back( { 188, rc3, 36.22, 123.4, 256.1, 0.55, 0.0125, 314.15 } );

    {
        data::Wells wellRates;

        wellRates["OP_1"] = w1;
        wellRates["OP_2"] = w2;

        return wellRates;
    }
}

data::Solution mkSolution( int numCells ) {

    using measure = UnitSystem::measure;
    using namespace data;

    data::Solution sol = {
        { "PRESSURE", { measure::pressure, std::vector<double>( numCells ), TargetType::RESTART_SOLUTION } },
        { "TEMP", { measure::temperature,  std::vector<double>( numCells ), TargetType::RESTART_SOLUTION } },
        { "SWAT", { measure::identity,     std::vector<double>( numCells ), TargetType::RESTART_SOLUTION } },
        { "SGAS", { measure::identity,     std::vector<double>( numCells ), TargetType::RESTART_SOLUTION } }
    };


    sol.data("PRESSURE").assign( numCells, 6.0 );
    sol.data("TEMP").assign( numCells, 7.0 );
    sol.data("SWAT").assign( numCells, 8.0 );
    sol.data("SGAS").assign( numCells, 9.0 );

    fun::iota rsi( 300, 300 + numCells );
    fun::iota rvi( 400, 400 + numCells );

    sol.insert( "RS", measure::identity, { rsi.begin(), rsi.end() } , TargetType::RESTART_SOLUTION );
    sol.insert( "RV", measure::identity, { rvi.begin(), rvi.end() } , TargetType::RESTART_SOLUTION );

    return sol;
}

Opm::SummaryState sim_state()
{
    auto state = Opm::SummaryState{std::chrono::system_clock::now()};

    state.update("WOPR:OP_1" ,    1.0);
    state.update("WWPR:OP_1" ,    2.0);
    state.update("WGPR:OP_1" ,    3.0);
    state.update("WVPR:OP_1" ,    4.0);
    state.update("WOPT:OP_1" ,   10.0);
    state.update("WWPT:OP_1" ,   20.0);
    state.update("WGPT:OP_1" ,   30.0);
    state.update("WVPT:OP_1" ,   40.0);
    state.update("WWIR:OP_1" ,    0.0);
    state.update("WGIR:OP_1" ,    0.0);
    state.update("WWIT:OP_1" ,    0.0);
    state.update("WGIT:OP_1" ,    0.0);
    state.update("WVIT:OP_1" ,    0.0);
    state.update("WWCT:OP_1" ,    0.625);
    state.update("WGOR:OP_1" ,  234.5);
    state.update("WBHP:OP_1" ,  314.15);
    state.update("WOPTH:OP_1",  345.6);
    state.update("WWPTH:OP_1",  456.7);
    state.update("WGPTH:OP_1",  567.8);
    state.update("WWITH:OP_1",    0.0);
    state.update("WGITH:OP_1",    0.0);
    state.update("WGVIR:OP_1",    0.0);
    state.update("WWVIR:OP_1",    0.0);

    state.update("WOPR:OP_2" ,    0.0);
    state.update("WWPR:OP_2" ,    0.0);
    state.update("WGPR:OP_2" ,    0.0);
    state.update("WVPR:OP_2" ,    0.0);
    state.update("WOPT:OP_2" ,    0.0);
    state.update("WWPT:OP_2" ,    0.0);
    state.update("WGPT:OP_2" ,    0.0);
    state.update("WVPT:OP_2" ,    0.0);
    state.update("WWIR:OP_2" ,  100.0);
    state.update("WGIR:OP_2" ,  200.0);
    state.update("WWIT:OP_2" , 1000.0);
    state.update("WGIT:OP_2" , 2000.0);
    state.update("WVIT:OP_2" , 1234.5);
    state.update("WWCT:OP_2" ,    0.0);
    state.update("WGOR:OP_2" ,    0.0);
    state.update("WBHP:OP_2" ,  400.6);
    state.update("WOPTH:OP_2",    0.0);
    state.update("WWPTH:OP_2",    0.0);
    state.update("WGPTH:OP_2",    0.0);
    state.update("WWITH:OP_2", 1515.0);
    state.update("WGITH:OP_2", 3030.0);
    state.update("WGVIR:OP_2", 1234.0);
    state.update("WWVIR:OP_2", 4321.0);

    state.update("WOPR:OP_3" ,   11.0);
    state.update("WWPR:OP_3" ,   12.0);
    state.update("WGPR:OP_3" ,   13.0);
    state.update("WVPR:OP_3" ,   14.0);
    state.update("WOPT:OP_3" ,  110.0);
    state.update("WWPT:OP_3" ,  120.0);
    state.update("WGPT:OP_3" ,  130.0);
    state.update("WVPT:OP_3" ,  140.0);
    state.update("WWIR:OP_3" ,    0.0);
    state.update("WGIR:OP_3" ,    0.0);
    state.update("WWIT:OP_3" ,    0.0);
    state.update("WGIT:OP_3" ,    0.0);
    state.update("WVIT:OP_3" ,    0.0);
    state.update("WWCT:OP_3" ,    0.0625);
    state.update("WGOR:OP_3" , 1234.5);
    state.update("WBHP:OP_3" ,  314.15);
    state.update("WOPTH:OP_3", 2345.6);
    state.update("WWPTH:OP_3", 3456.7);
    state.update("WGPTH:OP_3", 4567.8);
    state.update("WWITH:OP_3",    0.0);
    state.update("WGITH:OP_3",    0.0);
    state.update("WGVIR:OP_3",    0.0);
    state.update("WWVIR:OP_3",   43.21);

    state.update("GOPR:OP" ,     110.0);
    state.update("GWPR:OP" ,     120.0);
    state.update("GGPR:OP" ,     130.0);
    state.update("GVPR:OP" ,     140.0);
    state.update("GOPT:OP" ,    1100.0);
    state.update("GWPT:OP" ,    1200.0);
    state.update("GGPT:OP" ,    1300.0);
    state.update("GVPT:OP" ,    1400.0);
    state.update("GWIR:OP" , -   256.0);
    state.update("GGIR:OP" , - 65536.0);
    state.update("GWIT:OP" ,   31415.9);
    state.update("GGIT:OP" ,   27182.8);
    state.update("GVIT:OP" ,   44556.6);
    state.update("GWCT:OP" ,       0.625);
    state.update("GGOR:OP" ,    1234.5);
    state.update("GGVIR:OP",     123.45);
    state.update("GWVIR:OP",    1234.56);
    state.update("GOPTH:OP",    5678.90);
    state.update("GWPTH:OP",    6789.01);
    state.update("GGPTH:OP",    7890.12);
    state.update("GWITH:OP",    8901.23);
    state.update("GGITH:OP",    9012.34);

    state.update("FOPR" ,     1100.0);
    state.update("FWPR" ,     1200.0);
    state.update("FGPR" ,     1300.0);
    state.update("FVPR" ,     1400.0);
    state.update("FOPT" ,    11000.0);
    state.update("FWPT" ,    12000.0);
    state.update("FGPT" ,    13000.0);
    state.update("FVPT" ,    14000.0);
    state.update("FWIR" , -   2560.0);
    state.update("FGIR" , - 655360.0);
    state.update("FWIT" ,   314159.2);
    state.update("FGIT" ,   271828.1);
    state.update("FVIT" ,   445566.77);
    state.update("FWCT" ,        0.625);
    state.update("FGOR" ,     1234.5);
    state.update("FOPTH",    56789.01);
    state.update("FWPTH",    67890.12);
    state.update("FGPTH",    78901.23);
    state.update("FWITH",    89012.34);
    state.update("FGITH",    90123.45);
    state.update("FGVIR",     1234.56);
    state.update("FWVIR",    12345.67);

    return state;
}

struct Setup {
    Deck deck;
    EclipseState es;
    const EclipseGrid& grid;
    std::shared_ptr<Python> python;
    Schedule schedule;
    SummaryConfig summary_config;

    Setup( const char* path) :
        deck( Parser().parseFile( path) ),
        es( deck),
        grid( es.getInputGrid( ) ),
        python( std::make_shared<Python>() ),
        schedule( deck, es, python ),
        summary_config( deck, schedule, es.getTableManager( ))
    {
        auto& io_config = es.getIOConfig();
        io_config.setEclCompatibleRST(false);
    }

};


void init_st(SummaryState& st) {
    st.update_well_var("PROD1", "WOPR", 100);
    st.update_well_var("PROD1", "WLPR", 100);
    st.update_well_var("PROD2", "WOPR", 100);
    st.update_well_var("PROD2", "WLPR", 100);
    st.update_well_var("WINJ1", "WOPR", 100);
    st.update_well_var("WINJ1", "WLPR", 100);
    st.update_well_var("WINJ2", "WOPR", 100);
    st.update_well_var("WINJ2", "WLPR", 100);

    st.update_group_var("GRP1", "GOPR", 100);
    st.update_group_var("WGRP1", "GOPR", 100);
    st.update_group_var("WGRP1", "GOPR", 100);
    st.update("FLPR", 100);
}

RestartValue first_sim(const Setup& setup, Action::State& action_state, SummaryState& st, bool write_double) {
    EclipseIO eclWriter( setup.es, setup.grid, setup.schedule, setup.summary_config);
    auto num_cells = setup.grid.getNumActive( );
    int report_step = 1;
    auto start_time = setup.schedule.getStartTime();
    auto first_step = setup.schedule.simTime(report_step);

    auto sol = mkSolution( num_cells );
    auto wells = mkWells();
    auto groups = mkGroups();
    const auto& udq = setup.schedule.getUDQConfig(report_step);
    RestartValue restart_value(sol, wells, groups);

    init_st(st);
    udq.eval(st);
    eclWriter.writeTimeStep( action_state,
                             st,
                             report_step,
                             false,
                             std::difftime(first_step, start_time),
                             restart_value,
                             write_double);

    return restart_value;
}

RestartValue second_sim(const Setup& setup, Action::State& action_state, SummaryState& summary_state, const std::vector<RestartKey>& solution_keys) {
    EclipseIO writer(setup.es, setup.grid, setup.schedule, setup.summary_config);
    return writer.loadRestart( action_state, summary_state, solution_keys );
}


void compare( const RestartValue& fst,
              const RestartValue& snd,
              const std::vector<RestartKey>& solution_keys) {

    for (const auto& value : solution_keys) {
        double tol = 0.00001;
        const std::string& key = value.key;
        auto first = fst.solution.data( key ).begin();
        auto second = snd.solution.data( key ).begin();

        if (key == "TEMP")
            tol *= 10;

        for( ; first != fst.solution.data( key).end(); ++first, ++second )
            BOOST_CHECK_CLOSE( *first, *second, tol );
    }

    BOOST_CHECK_EQUAL( fst.wells, snd.wells );
}




BOOST_AUTO_TEST_CASE(EclipseReadWriteWellStateData) {
    std::vector<RestartKey> keys {{"PRESSURE" , UnitSystem::measure::pressure},
                                  {"SWAT" , UnitSystem::measure::identity},
                                  {"SGAS" , UnitSystem::measure::identity},
                                  {"TEMP" , UnitSystem::measure::temperature}};
    WorkArea test_area("test_restart");
    test_area.copyIn("BASE_SIM.DATA");
    test_area.copyIn("RESTART_SIM.DATA");

    Setup base_setup("BASE_SIM.DATA");
    SummaryState st(std::chrono::system_clock::now());
    Action::State action_state;
    auto state1 = first_sim( base_setup , action_state, st, false );

    Setup restart_setup("RESTART_SIM.DATA");
    auto state2 = second_sim( restart_setup , action_state, st , keys );
    compare(state1, state2 , keys);

    BOOST_CHECK_THROW( second_sim( restart_setup, action_state, st, {{"SOIL", UnitSystem::measure::pressure}} ) , std::runtime_error );
    BOOST_CHECK_THROW( second_sim( restart_setup, action_state, st, {{"SOIL", UnitSystem::measure::pressure, true}}) , std::runtime_error );
}


BOOST_AUTO_TEST_CASE(ECL_FORMATTED) {
    namespace OS = ::Opm::EclIO::OutputStream;

    WorkArea test_area("test_Restart");
    test_area.copyIn("BASE_SIM.DATA");

    Setup base_setup("BASE_SIM.DATA");
    auto& io_config = base_setup.es.getIOConfig();
    {
        auto num_cells = base_setup.grid.getNumActive( );
        auto cells = mkSolution( num_cells );
        auto wells = mkWells();
        auto groups = mkGroups();
        auto sumState = sim_state();
        Action::State action_state;
        {
            RestartValue restart_value(cells, wells, groups);

            io_config.setEclCompatibleRST( false );
            restart_value.addExtra("EXTRA", UnitSystem::measure::pressure, {10,1,2,3});

            const auto outputDir = test_area.currentWorkingDirectory();

            {
                const auto seqnum = 1;
                auto rstFile = OS::Restart {
                    OS::ResultSet{ outputDir, "OPM_FILE" }, seqnum,
                    OS::Formatted{ false }, OS::Unified{ true }
                };

                RestartIO::save(rstFile, seqnum,
                                100,
                                restart_value,
                                base_setup.es,
                                base_setup.grid,
                                base_setup.schedule,
                                action_state,
                                sumState,
                                true);
            }

            {
                const auto rstFile = ::Opm::EclIO::OutputStream::
                    outputFileName({outputDir, "OPM_FILE"}, "UNRST");

                EclIO::ERst rst{ rstFile };

                BOOST_CHECK_MESSAGE(rst.hasKey("SWAT"), "Restart file must have SWAT vector");
                BOOST_CHECK_MESSAGE(rst.hasKey("EXTRA"), "Restart file must have EXTRA vector");
            }

            io_config.setEclCompatibleRST( true );
            {
                const auto seqnum = 1;
                auto rstFile = OS::Restart {
                    OS::ResultSet{ outputDir, "ECL_FILE" }, seqnum,
                    OS::Formatted{ false }, OS::Unified{ true }
                };

                RestartIO::save(rstFile, seqnum,
                                100,
                                restart_value,
                                base_setup.es,
                                base_setup.grid,
                                base_setup.schedule,
                                action_state,
                                sumState,
                                true);
            }

            {
                const auto rstFile = ::Opm::EclIO::OutputStream::
                    outputFileName({outputDir, "ECL_FILE"}, "UNRST");

                EclIO::ERst rst{ rstFile };

                BOOST_CHECK_MESSAGE(rst.hasKey("SWAT"), "Restart file must have SWAT vector");
                BOOST_CHECK_MESSAGE(!rst.hasKey("EXTRA"), "Restart file must NOT have EXTRA vector");
                BOOST_CHECK_MESSAGE(!rst.hasKey("OPM_IWEL"), "Restart file must NOT have OPM_IWEL vector");
                BOOST_CHECK_MESSAGE(!rst.hasKey("OPM_XWEL"), "Restart file must NOT have OPM_XWEL vector");
            }
        }
    }
}





void compare_equal( const RestartValue& fst,
                    const RestartValue& snd ,
                    const std::vector<RestartKey>& keys) {

    for (const auto& value : keys) {
        const std::string& key = value.key;
        auto first = fst.solution.data( key ).begin();
        auto second = snd.solution.data( key ).begin();

        for( ; first != fst.solution.data( key ).end(); ++first, ++second )
          BOOST_CHECK_EQUAL( *first, *second);
    }

    BOOST_CHECK_EQUAL( fst.wells, snd.wells );
    //BOOST_CHECK_EQUAL( fst.extra, snd.extra );
}

BOOST_AUTO_TEST_CASE(EclipseReadWriteWellStateData_double) {
    /*
      Observe that the purpose of this test is to verify that with
      write_double == true we can load solution fields which are
      bitwise equal to those we stored. Unfortunately the scaling back
      and forth between SI units and output units is enough to break
      this equality for the pressure. For this test we therefor only
      consider the saturations which have identity unit.
    */
    std::vector<RestartKey> solution_keys {RestartKey("SWAT", UnitSystem::measure::identity),
                                           RestartKey("SGAS", UnitSystem::measure::identity)};

    WorkArea test_area("test_Restart");
    test_area.copyIn("RESTART_SIM.DATA");
    test_area.copyIn("BASE_SIM.DATA");
    Setup base_setup("BASE_SIM.DATA");
    SummaryState st(std::chrono::system_clock::now());
    Action::State action_state;

    auto state1 = first_sim( base_setup , action_state, st, true);
    Setup restart_setup("RESTART_SIM.DATA");

    auto state2 = second_sim( restart_setup, action_state, st, solution_keys );
    compare_equal( state1 , state2 , solution_keys);
}


BOOST_AUTO_TEST_CASE(WriteWrongSOlutionSize) {
    namespace OS = ::Opm::EclIO::OutputStream;

    WorkArea test_area("test_Restart");
    test_area.copyIn("BASE_SIM.DATA");
    test_area.copyIn("RESTART_SIM.DATA");
    Setup setup("BASE_SIM.DATA");
    {
        auto num_cells = setup.grid.getNumActive( ) + 1;
        auto cells = mkSolution( num_cells );
        auto wells = mkWells();
        auto groups = mkGroups();
        Opm::SummaryState sumState(std::chrono::system_clock::now());
        Opm::Action::State action_state;

        const auto seqnum = 1;
        auto rstFile = OS::Restart {
            OS::ResultSet { test_area.currentWorkingDirectory(), "FILE" }, seqnum,
            OS::Formatted { false }, OS::Unified { true }
        };

        BOOST_CHECK_THROW( RestartIO::save(rstFile, seqnum,
                                           100,
                                           RestartValue(cells, wells, groups),
                                           setup.es,
                                           setup.grid ,
                                           setup.schedule,
                                           action_state,
                                           sumState),
                           std::runtime_error);
    }
}


BOOST_AUTO_TEST_CASE(ExtraData_KEYS) {
    Setup setup("BASE_SIM.DATA");
    auto num_cells = setup.grid.getNumActive( );
    auto cells = mkSolution( num_cells );
    auto wells = mkWells();
    auto groups = mkGroups();
    RestartValue restart_value(cells, wells, groups);

    BOOST_CHECK_THROW( restart_value.addExtra("TOO-LONG-KEY", {0,1,2}), std::runtime_error);

    // Keys must be unique
    restart_value.addExtra("KEY", {0,1,1});
    BOOST_CHECK_THROW( restart_value.addExtra("KEY", {0,1,1}), std::runtime_error);

    /* The keys must be unique across solution and extra_data */
    BOOST_CHECK_THROW( restart_value.addExtra("PRESSURE", {0,1}), std::runtime_error); 

    /* Must avoid using reserved keys like 'LOGIHEAD' */
    BOOST_CHECK_THROW( restart_value.addExtra("LOGIHEAD", {0,1}), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(ExtraData_content) {
    namespace OS = ::Opm::EclIO::OutputStream;

    WorkArea test_area("test_Restart");
    test_area.copyIn("BASE_SIM.DATA");
    test_area.copyIn("RESTART_SIM.DATA");
    Setup setup("BASE_SIM.DATA");
    {
        Action::State action_state;
        auto num_cells = setup.grid.getNumActive( );
        auto cells = mkSolution( num_cells );
        auto wells = mkWells();
        auto groups = mkGroups();
        const auto& units = setup.es.getUnits();
        {
            RestartValue restart_value(cells, wells, groups);
            SummaryState st(std::chrono::system_clock::now());
            const auto sumState = sim_state();

            restart_value.addExtra("EXTRA", UnitSystem::measure::pressure, {10,1,2,3});

            const auto outputDir = test_area.currentWorkingDirectory();

            {
                const auto seqnum = 1;
                auto rstFile = OS::Restart {
                    OS::ResultSet { outputDir, "FILE" }, seqnum,
                    OS::Formatted { false }, OS::Unified{ true }
                };

                RestartIO::save(rstFile, seqnum,
                                100,
                                restart_value,
                                setup.es,
                                setup.grid,
                                setup.schedule,
                                action_state,
                                sumState);
            }

            const auto rstFile = ::Opm::EclIO::OutputStream::
                outputFileName({outputDir, "FILE"}, "UNRST");

            {
                EclIO::ERst rst{ rstFile };
                BOOST_CHECK_MESSAGE( rst.hasKey("EXTRA"), "Restart file is expexted to have EXTRA vector");

                const auto& ex = rst.getRst<double>("EXTRA", 1, 0);
                BOOST_CHECK_CLOSE( 10 , units.to_si( UnitSystem::measure::pressure, ex[0] ), 0.00001);
                BOOST_CHECK_CLOSE( units.from_si( UnitSystem::measure::pressure, 3), ex[3], 0.00001);
            }

            BOOST_CHECK_THROW( RestartIO::load( rstFile , 1 , action_state, st, {}, setup.es, setup.grid , setup.schedule,
                                                {{"NOT-THIS", UnitSystem::measure::identity, true}}) , std::runtime_error );
            {
                const auto rst_value = RestartIO::load(rstFile , 1 , action_state, st,
                                                       /* solution_keys = */ {
                                                                              RestartKey("SWAT", UnitSystem::measure::identity),
                                                                              RestartKey("NO"  , UnitSystem::measure::identity, false)
                                                       },
                                                       setup.es, setup.grid , setup.schedule,
                                                       /* extra_keys = */ {
                                                                           {"EXTRA" , UnitSystem::measure::pressure, true}  ,
                                                                           {"EXTRA2", UnitSystem::measure::identity, false}
                                                       });

                BOOST_CHECK(!rst_value.hasExtra("EXTRA2"));
                BOOST_CHECK( rst_value.hasExtra("EXTRA"));
                BOOST_CHECK_THROW(rst_value.getExtra("EXTRA2"), std::invalid_argument);
                const auto& extraval = rst_value.getExtra("EXTRA");
                const std::vector<double> expected = {10,1,2,3};

                BOOST_CHECK_EQUAL( rst_value.solution.has("NO") , false );
                for (size_t i=0; i < expected.size(); i++)
                    BOOST_CHECK_CLOSE(extraval[i], expected[i], 1e-5);
            }
        }
    }
}


BOOST_AUTO_TEST_CASE(STORE_THPRES) {
    namespace OS = ::Opm::EclIO::OutputStream;

    WorkArea test_area("test_Restart_THPRES");
    test_area.copyIn("BASE_SIM_THPRES.DATA");
    Setup base_setup("BASE_SIM_THPRES.DATA");
    {
        auto num_cells = base_setup.grid.getNumActive( );
        auto cells = mkSolution( num_cells );
        auto wells = mkWells();
        auto groups = mkGroups();
        const auto outputDir = test_area.currentWorkingDirectory();
        {
            RestartValue restart_value(cells, wells, groups);
            RestartValue restart_value2(cells, wells, groups);

            /* Missing THPRES data in extra container. */
            /* Because it proved to difficult to update the legacy simulators
               to pass THPRES values when writing restart files this BOOST_CHECK_THROW
               had to be disabled. The RestartIO::save() function will just log a warning.

            BOOST_CHECK_THROW( RestartIO::save("FILE.UNRST", 1 ,
                                               100,
                                               restart_value,
                                               setup.es,
                                               setup.grid,
                                               setup.schedule), std::runtime_error);
            */

            restart_value.addExtra("THRESHPR", UnitSystem::measure::pressure, {0,1});
            const auto sumState = sim_state();
            Action::State action_state;

            /* THPRES data has wrong size in extra container. */
            {
                const auto seqnum = 1;
                auto rstFile = OS::Restart {
                    OS::ResultSet { outputDir, "FILE" }, seqnum,
                    OS::Formatted { false }, OS::Unified { true }
                };

                BOOST_CHECK_THROW( RestartIO::save(rstFile, seqnum,
                                                   100,
                                                   restart_value,
                                                   base_setup.es,
                                                   base_setup.grid,
                                                   base_setup.schedule,
                                                   action_state,
                                                   sumState),
                                   std::runtime_error);
            }

            int num_regions = base_setup.es.getTableManager().getEqldims().getNumEquilRegions();
            std::vector<double>  thpres(num_regions * num_regions, 78);
            restart_value2.addExtra("THRESHPR", UnitSystem::measure::pressure, thpres);
            restart_value2.addExtra("EXTRA", UnitSystem::measure::pressure, thpres);

            {
                const auto seqnum = 1;
                auto rstFile = OS::Restart {
                    OS::ResultSet { outputDir, "FILE2" }, seqnum,
                    OS::Formatted { false }, OS::Unified { true }
                };

                RestartIO::save(rstFile, seqnum,
                                100,
                                restart_value2,
                                base_setup.es,
                                base_setup.grid,
                                base_setup.schedule,
                                action_state,
                                sumState);
            }

            {
                const auto rstFile = ::Opm::EclIO::OutputStream::
                    outputFileName({outputDir, "FILE2"}, "UNRST");

                EclIO::ERst rst(rstFile);
                std::map<std::string,int> kw_pos;

                {
                    auto i = 0;
                    for (const auto& vec : rst.listOfRstArrays(1))
                        kw_pos[ std::get<0>(vec) ] = i++;
                }

                BOOST_CHECK( kw_pos["STARTSOL"] < kw_pos["THRESHPR"] );
                BOOST_CHECK( kw_pos["THRESHPR"] < kw_pos["ENDSOL"] );
                BOOST_CHECK( kw_pos["ENDSOL"] < kw_pos["EXTRA"] );

                BOOST_CHECK_EQUAL( ecl_file_get_num_named_kw(rst, "THRESHPR"), 1);
                BOOST_CHECK_EQUAL( ecl_file_get_num_named_kw(rst, "EXTRA"), 1);
                BOOST_CHECK_MESSAGE( ecl_kw_get_type(ecl_file_iget_named_kw(rst, "THRESHPR", 1)) == EclIO::eclArrType::DOUB,
                                     R"("THRESHPR" vector must have type DOUB)");
            }
        }
    }
}



BOOST_AUTO_TEST_CASE(Restore_Cumulatives)
{
    WorkArea wa{"test_Restart"};
    wa.copyIn("BASE_SIM.DATA");
    wa.copyIn("RESTART_SIM.DATA");
    Setup setup("BASE_SIM.DATA");

    // Write fully ECLIPSE compatible output.  This also saves cumulatives.
    setup.es.getIOConfig().setEclCompatibleRST(true);

    const auto restart_value = RestartValue {
        mkSolution(setup.grid.getNumActive()),
        mkWells(),
        mkGroups()
    };
    const auto sumState = sim_state();

    namespace OS = ::Opm::EclIO::OutputStream;

    const auto rset   = OS::ResultSet{ wa.currentWorkingDirectory(), "FILE" };
    const auto seqnum = 1;
    {
        Action::State action_state;
        auto rstFile = OS::Restart {
            rset, seqnum, OS::Formatted{ false }, OS::Unified{ true }
        };

        RestartIO::save(rstFile, seqnum, 100, restart_value,
                        setup.es, setup.grid, setup.schedule, action_state, sumState);
    }

    Action::State action_state;
    SummaryState rstSumState(std::chrono::system_clock::now());
    RestartIO::load(OS::outputFileName(rset, "UNRST"), seqnum, action_state, rstSumState,
                    /* solution_keys = */ {
                                           RestartKey("SWAT", UnitSystem::measure::identity),
                    },
                    setup.es, setup.grid, setup.schedule,
                    /* extra_keys = */ {});


    // Verify that the restored summary state has all of its requisite
    // cumulative summary vectors.

    // Producer => W*IT{,H} saved/restored as zero (0.0)
    BOOST_CHECK(rstSumState.has("WOPT:OP_1"));
    BOOST_CHECK(rstSumState.has("WGPT:OP_1"));
    BOOST_CHECK(rstSumState.has("WWPT:OP_1"));
    BOOST_CHECK(rstSumState.has("WVPT:OP_1"));
    BOOST_CHECK(rstSumState.has("WWIT:OP_1"));
    BOOST_CHECK(rstSumState.has("WGIT:OP_1"));
    BOOST_CHECK(rstSumState.has("WVIT:OP_1"));
    BOOST_CHECK(rstSumState.has("WOPTH:OP_1"));
    BOOST_CHECK(rstSumState.has("WGPTH:OP_1"));
    BOOST_CHECK(rstSumState.has("WWPTH:OP_1"));
    BOOST_CHECK(rstSumState.has("WWITH:OP_1"));
    BOOST_CHECK(rstSumState.has("WGITH:OP_1"));

    BOOST_CHECK_CLOSE(rstSumState.get("WOPT:OP_1"), 10.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WGPT:OP_1"), 30.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WWPT:OP_1"), 20.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WVPT:OP_1"), 40.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WWIT:OP_1"),  0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WGIT:OP_1"),  0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WVIT:OP_1"),  0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WOPTH:OP_1"), 345.6, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WWPTH:OP_1"), 456.7, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WGPTH:OP_1"), 567.8, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WWITH:OP_1"),   0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WGITH:OP_1"),   0.0, 1.0e-10);

    // Gas injector => W*PT{,H} saved/restored as zero (0.0)
    BOOST_CHECK(rstSumState.has("WOPT:OP_2"));
    BOOST_CHECK(rstSumState.has("WGPT:OP_2"));
    BOOST_CHECK(rstSumState.has("WWPT:OP_2"));
    BOOST_CHECK(rstSumState.has("WVPT:OP_2"));
    BOOST_CHECK(rstSumState.has("WWIT:OP_2"));
    BOOST_CHECK(rstSumState.has("WGIT:OP_2"));
    BOOST_CHECK(rstSumState.has("WVIT:OP_2"));
    BOOST_CHECK(rstSumState.has("WOPTH:OP_2"));
    BOOST_CHECK(rstSumState.has("WGPTH:OP_2"));
    BOOST_CHECK(rstSumState.has("WWPTH:OP_2"));
    BOOST_CHECK(rstSumState.has("WWITH:OP_2"));
    BOOST_CHECK(rstSumState.has("WGITH:OP_2"));

    BOOST_CHECK_CLOSE(rstSumState.get("WOPT:OP_2"),    0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WGPT:OP_2"),    0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WWPT:OP_2"),    0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WVPT:OP_2"),    0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WWIT:OP_2"), 1000.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WGIT:OP_2"), 2000.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WVIT:OP_2"), 1234.5, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WOPTH:OP_2"),    0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WGPTH:OP_2"),    0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WWPTH:OP_2"),    0.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WWITH:OP_2"), 1515.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("WGITH:OP_2"), 3030.0, 1.0e-10);

    // Group cumulatives saved/restored for all phases
    BOOST_CHECK(rstSumState.has("GOPT:OP"));
    BOOST_CHECK(rstSumState.has("GGPT:OP"));
    BOOST_CHECK(rstSumState.has("GWPT:OP"));
    BOOST_CHECK(rstSumState.has("GVPT:OP"));
    BOOST_CHECK(rstSumState.has("GWIT:OP"));
    BOOST_CHECK(rstSumState.has("GGIT:OP"));
    BOOST_CHECK(rstSumState.has("GVIT:OP"));
    BOOST_CHECK(rstSumState.has("GOPTH:OP"));
    BOOST_CHECK(rstSumState.has("GGPTH:OP"));
    BOOST_CHECK(rstSumState.has("GWPTH:OP"));
    BOOST_CHECK(rstSumState.has("GWITH:OP"));
    BOOST_CHECK(rstSumState.has("GGITH:OP"));

    BOOST_CHECK_CLOSE(rstSumState.get("GOPT:OP"),  1100.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GWPT:OP"),  1200.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GGPT:OP"),  1300.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GVPT:OP"),  1400.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GWIT:OP"), 31415.9, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GGIT:OP"), 27182.8, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GVIT:OP"), 44556.6, 1.0e-10);

    BOOST_CHECK_CLOSE(rstSumState.get("GOPTH:OP"), 5678.90, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GGPTH:OP"), 7890.12, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GWPTH:OP"), 6789.01, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GWITH:OP"), 8901.23, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("GGITH:OP"), 9012.34, 1.0e-10);

    // Field cumulatives saved/restored for all phases
    BOOST_CHECK(rstSumState.has("FOPT"));
    BOOST_CHECK(rstSumState.has("FGPT"));
    BOOST_CHECK(rstSumState.has("FWPT"));
    BOOST_CHECK(rstSumState.has("FVPT"));
    BOOST_CHECK(rstSumState.has("FWIT"));
    BOOST_CHECK(rstSumState.has("FGIT"));
    BOOST_CHECK(rstSumState.has("FVIT"));
    BOOST_CHECK(rstSumState.has("FOPTH"));
    BOOST_CHECK(rstSumState.has("FGPTH"));
    BOOST_CHECK(rstSumState.has("FWPTH"));
    BOOST_CHECK(rstSumState.has("FWITH"));
    BOOST_CHECK(rstSumState.has("FGITH"));

    BOOST_CHECK_CLOSE(rstSumState.get("FOPT"),  11000.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FWPT"),  12000.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FGPT"),  13000.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FVPT"),  14000.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FWIT"), 314159.2, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FGIT"), 271828.1, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FVIT"), 445566.77, 1.0e-10);

    BOOST_CHECK_CLOSE(rstSumState.get("FOPTH"), 56789.01, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FGPTH"), 78901.23, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FWPTH"), 67890.12, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FWITH"), 89012.34, 1.0e-10);
    BOOST_CHECK_CLOSE(rstSumState.get("FGITH"), 90123.45, 1.0e-10);
}


BOOST_AUTO_TEST_CASE(UDQ_RESTART) {
    std::vector<RestartKey> keys {{"PRESSURE" , UnitSystem::measure::pressure},
        {"SWAT" , UnitSystem::measure::identity},
        {"SGAS" , UnitSystem::measure::identity}};
    WorkArea test_area("test_udq_restart");
    test_area.copyIn("UDQ_BASE.DATA");
    test_area.copyIn("UDQ_RESTART.DATA");

    Setup base_setup("UDQ_BASE.DATA");
    SummaryState st1(std::chrono::system_clock::now());
    SummaryState st2(std::chrono::system_clock::now());
    Action::State action_state;
    auto state1 = first_sim( base_setup , action_state, st1, false );

    Setup restart_setup("UDQ_RESTART.DATA");
    auto state2 = second_sim( restart_setup , action_state, st2 , keys );
    BOOST_CHECK(st1.wells() == st2.wells());
    BOOST_CHECK(st1.groups() == st2.groups());

    const auto& udq = base_setup.schedule.getUDQConfig(1);
    for (const auto& well : st1.wells()) {
        for (const auto& def : udq.definitions(UDQVarType::WELL_VAR)) {
            const auto& kw = def.keyword();
            BOOST_CHECK_EQUAL( st1.has_well_var(well, kw), st2.has_well_var(well, kw));
            if (st1.has_well_var(well, def.keyword()))
                BOOST_CHECK_EQUAL(st1.get_well_var(well, kw), st2.get_well_var(well, kw));
        }
    }

    for (const auto& group : st1.groups()) {
        for (const auto& def : udq.definitions(UDQVarType::GROUP_VAR)) {
            const auto& kw = def.keyword();
            BOOST_CHECK_EQUAL( st1.has_group_var(group, kw), st2.has_group_var(group, kw));
            if (st1.has_group_var(group, def.keyword()))
                BOOST_CHECK_EQUAL(st1.get_group_var(group, kw), st2.get_group_var(group, kw));
        }
    }

    for (const auto& well : st1.wells()) {
        for (const auto& def : udq.assignments(UDQVarType::WELL_VAR)) {
            const auto& kw = def.keyword();
            BOOST_CHECK_EQUAL( st1.has_well_var(well, kw), st2.has_well_var(well, kw));
            if (st1.has_well_var(well, def.keyword()))
                BOOST_CHECK_EQUAL(st1.get_well_var(well, kw), st2.get_well_var(well, kw));
        }
    }

    for (const auto& group : st1.groups()) {
        for (const auto& def : udq.assignments(UDQVarType::GROUP_VAR)) {
            const auto& kw = def.keyword();
            BOOST_CHECK_EQUAL( st1.has_group_var(group, kw), st2.has_group_var(group, kw));
            if (st1.has_group_var(group, def.keyword()))
                BOOST_CHECK_EQUAL(st1.get_group_var(group, kw), st2.get_group_var(group, kw));
        }
    }

    for (const auto& def : udq.assignments(UDQVarType::FIELD_VAR)) {
        const auto& kw = def.keyword();
        BOOST_CHECK_EQUAL( st1.has(kw), st2.has(kw));
            if (st1.has(kw))
                BOOST_CHECK_EQUAL(st1.get(kw), st2.get(kw));
    }

    for (const auto& def : udq.definitions(UDQVarType::FIELD_VAR)) {
        const auto& kw = def.keyword();
        BOOST_CHECK_EQUAL( st1.has(kw), st2.has(kw));
        if (st1.has(kw))
            BOOST_CHECK_EQUAL(st1.get(kw), st2.get(kw));
    }
}
}
