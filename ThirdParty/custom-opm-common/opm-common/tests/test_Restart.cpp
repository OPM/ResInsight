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

#include <opm/output/eclipse/AggregateAquiferData.hpp>
#include <opm/output/eclipse/EclipseIO.hpp>
#include <opm/output/eclipse/RestartIO.hpp>
#include <opm/output/eclipse/RestartValue.hpp>
#include <opm/output/data/Cells.hpp>
#include <opm/output/data/Wells.hpp>
#include <opm/output/data/Groups.hpp>
#include <opm/input/eclipse/Python/Python.hpp>

#include <opm/input/eclipse/EclipseState/Tables/Eqldims.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>
#include <opm/input/eclipse/Utility/Functional.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/Action/State.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/input/eclipse/Schedule/Well/WellTestState.hpp>

#include <opm/io/eclipse/OutputStream.hpp>
#include <opm/io/eclipse/EclIOdata.hpp>
#include <opm/io/eclipse/ERst.hpp>

#include <sstream>
#include <tuple>

#include <opm/common/utility/TimeService.hpp>

#include <tests/WorkArea.hpp>

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

data::GroupAndNetworkValues mkGroups() {
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
    w1.connections.push_back( { 88, rc1, 30.45, 123.4, 543.21, 0.62, 0.15, 1.0e3, 1.234 } );
    w1.connections.push_back( { 288, rc2, 33.19, 123.4, 432.1, 0.26, 0.45, 2.56, 2.345 } );

    w2.rates = r2;
    w2.thp = 2.0;
    w2.bhp = 2.34;
    w2.temperature = 4.56;
    w2.control = 2;
    w2.connections.push_back( { 188, rc3, 36.22, 123.4, 256.1, 0.55, 0.0125, 314.15, 3.456 } );

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

Opm::SummaryState sim_state(const Opm::Schedule& sched)
{
    auto state = Opm::SummaryState{TimeService::now()};
    for (const auto& well : sched.getWellsatEnd()) {
        for (const auto& connection : well.getConnections()) {
            state.update_conn_var(well.name(), "CPR", connection.global_index() + 1, 111);
            if (well.isInjector()) {
                state.update_conn_var(well.name(), "COIR", connection.global_index() + 1, 222);
                state.update_conn_var(well.name(), "CGIR", connection.global_index() + 1, 333);
                state.update_conn_var(well.name(), "CWIR", connection.global_index() + 1, 444);
                state.update_conn_var(well.name(), "CVIR", connection.global_index() + 1, 555);

                state.update_conn_var(well.name(), "COIT", connection.global_index() + 1, 222 * 2.0);
                state.update_conn_var(well.name(), "CGIT", connection.global_index() + 1, 333 * 2.0);
                state.update_conn_var(well.name(), "CWIT", connection.global_index() + 1, 444 * 2.0);
                state.update_conn_var(well.name(), "CWIT", connection.global_index() + 1, 555 * 2.0);
            } else {
                state.update_conn_var(well.name(), "COPR", connection.global_index() + 1, 666);
                state.update_conn_var(well.name(), "CGPR", connection.global_index() + 1, 777);
                state.update_conn_var(well.name(), "CWPR", connection.global_index() + 1, 888);
                state.update_conn_var(well.name(), "CVPR", connection.global_index() + 1, 999);

                state.update_conn_var(well.name(), "CGOR", connection.global_index() + 1, 777.0 / 666.0);

                state.update_conn_var(well.name(), "COPT", connection.global_index() + 1, 555 * 2.0);
                state.update_conn_var(well.name(), "CGPT", connection.global_index() + 1, 666 * 2.0);
                state.update_conn_var(well.name(), "CWPT", connection.global_index() + 1, 777 * 2.0);
                state.update_conn_var(well.name(), "CVPT", connection.global_index() + 1, 999 * 2.0);
            }
        }
    }

    state.update_well_var("OP_1", "WOPR", 1.0);
    state.update_well_var("OP_1", "WWPR", 2.0);
    state.update_well_var("OP_1", "WGPR", 3.0);
    state.update_well_var("OP_1", "WVPR", 4.0);
    state.update_well_var("OP_1", "WOPT", 10.0);
    state.update_well_var("OP_1", "WWPT", 20.0);
    state.update_well_var("OP_1", "WGPT", 30.0);
    state.update_well_var("OP_1", "WVPT", 40.0);
    state.update_well_var("OP_1", "WWIR", 0.0);
    state.update_well_var("OP_1", "WGIR", 0.0);
    state.update_well_var("OP_1", "WWIT", 0.0);
    state.update_well_var("OP_1", "WGIT", 0.0);
    state.update_well_var("OP_1", "WVIT", 0.0);
    state.update_well_var("OP_1", "WWCT", 0.625);
    state.update_well_var("OP_1", "WGOR", 234.5);
    state.update_well_var("OP_1", "WBHP", 314.15);
    state.update_well_var("OP_1", "WTHP", 123.45);
    state.update_well_var("OP_1", "WOPTH", 345.6);
    state.update_well_var("OP_1", "WWPTH", 456.7);
    state.update_well_var("OP_1", "WGPTH", 567.8);
    state.update_well_var("OP_1", "WWITH", 0.0);
    state.update_well_var("OP_1", "WGITH", 0.0);
    state.update_well_var("OP_1", "WGVIR", 0.0);
    state.update_well_var("OP_1", "WWVIR", 0.0);

    state.update_well_var("OP_2", "WOPR", 0.0);
    state.update_well_var("OP_2", "WWPR", 0.0);
    state.update_well_var("OP_2", "WGPR", 0.0);
    state.update_well_var("OP_2", "WVPR", 0.0);
    state.update_well_var("OP_2", "WOPT", 0.0);
    state.update_well_var("OP_2", "WWPT", 0.0);
    state.update_well_var("OP_2", "WGPT", 0.0);
    state.update_well_var("OP_2", "WVPT", 0.0);
    state.update_well_var("OP_2", "WWIR", 100.0);
    state.update_well_var("OP_2", "WGIR", 200.0);
    state.update_well_var("OP_2", "WWIT", 1000.0);
    state.update_well_var("OP_2", "WGIT", 2000.0);
    state.update_well_var("OP_2", "WVIT", 1234.5);
    state.update_well_var("OP_2", "WWCT", 0.0);
    state.update_well_var("OP_2", "WGOR", 0.0);
    state.update_well_var("OP_2", "WBHP", 400.6);
    state.update_well_var("OP_2", "WTHP", 234.5);
    state.update_well_var("OP_2", "WOPTH", 0.0);
    state.update_well_var("OP_2", "WWPTH", 0.0);
    state.update_well_var("OP_2", "WGPTH", 0.0);
    state.update_well_var("OP_2", "WWITH", 1515.0);
    state.update_well_var("OP_2", "WGITH", 3030.0);
    state.update_well_var("OP_2", "WGVIR", 1234.0);
    state.update_well_var("OP_2", "WWVIR", 4321.0);

    state.update_well_var("OP_3", "WOPR", 11.0);
    state.update_well_var("OP_3", "WWPR", 12.0);
    state.update_well_var("OP_3", "WGPR", 13.0);
    state.update_well_var("OP_3", "WVPR", 14.0);
    state.update_well_var("OP_3", "WOPT", 110.0);
    state.update_well_var("OP_3", "WWPT", 120.0);
    state.update_well_var("OP_3", "WGPT", 130.0);
    state.update_well_var("OP_3", "WVPT", 140.0);
    state.update_well_var("OP_3", "WWIR", 0.0);
    state.update_well_var("OP_3", "WGIR", 0.0);
    state.update_well_var("OP_3", "WWIT", 0.0);
    state.update_well_var("OP_3", "WGIT", 0.0);
    state.update_well_var("OP_3", "WVIT", 0.0);
    state.update_well_var("OP_3", "WWCT", 0.0625);
    state.update_well_var("OP_3", "WGOR", 1234.5);
    state.update_well_var("OP_3", "WBHP", 314.15);
    state.update_well_var("OP_3", "WTHP", 246.9);
    state.update_well_var("OP_3", "WOPTH", 2345.6);
    state.update_well_var("OP_3", "WWPTH", 3456.7);
    state.update_well_var("OP_3", "WGPTH", 4567.8);
    state.update_well_var("OP_3", "WWITH", 0.0);
    state.update_well_var("OP_3", "WGITH", 0.0);
    state.update_well_var("OP_3", "WGVIR", 0.0);
    state.update_well_var("OP_3", "WWVIR", 43.21);

    state.update_group_var("OP", "GOPR" ,     110.0);
    state.update_group_var("OP", "GWPR" ,     120.0);
    state.update_group_var("OP", "GGPR" ,     130.0);
    state.update_group_var("OP", "GVPR" ,     140.0);
    state.update_group_var("OP", "GOPT" ,    1100.0);
    state.update_group_var("OP", "GWPT" ,    1200.0);
    state.update_group_var("OP", "GGPT" ,    1300.0);
    state.update_group_var("OP", "GVPT" ,    1400.0);
    state.update_group_var("OP", "GWIR" , -   256.0);
    state.update_group_var("OP", "GGIR" , - 65536.0);
    state.update_group_var("OP", "GWIT" ,   31415.9);
    state.update_group_var("OP", "GGIT" ,   27182.8);
    state.update_group_var("OP", "GVIT" ,   44556.6);
    state.update_group_var("OP", "GWCT" ,       0.625);
    state.update_group_var("OP", "GGOR" ,    1234.5);
    state.update_group_var("OP", "GGVIR",     123.45);
    state.update_group_var("OP", "GWVIR",    1234.56);
    state.update_group_var("OP", "GOPTH",    5678.90);
    state.update_group_var("OP", "GWPTH",    6789.01);
    state.update_group_var("OP", "GGPTH",    7890.12);
    state.update_group_var("OP", "GWITH",    8901.23);
    state.update_group_var("OP", "GGITH",    9012.34);

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
        summary_config( deck, schedule, es.fieldProps(), es.aquifer() )
    {
        auto& io_config = es.getIOConfig();
        io_config.setEclCompatibleRST(false);
    }

};



RestartValue first_sim(const Setup& setup, Action::State& action_state, SummaryState& st, UDQState& udq_state, bool write_double) {
    WellTestState wtest_state;
    EclipseIO eclWriter( setup.es, setup.grid, setup.schedule, setup.summary_config);
    auto num_cells = setup.grid.getNumActive( );
    int report_step = 1;
    auto start_time = setup.schedule.getStartTime();
    auto first_step = setup.schedule.simTime(report_step);

    auto sol = mkSolution( num_cells );
    auto wells = mkWells();
    auto groups = mkGroups();
    const auto& udq = setup.schedule.getUDQConfig(report_step);
    RestartValue restart_value(sol, wells, groups, {});

    udq.eval(report_step, setup.schedule.wellMatcher(report_step), st, udq_state);
    eclWriter.writeTimeStep( action_state,
                             wtest_state,
                             st,
                             udq_state,
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
    auto st = sim_state(base_setup.schedule);
    Action::State action_state;
    UDQState udq_state(19);
    auto state1 = first_sim( base_setup , action_state, st, udq_state, false );

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
        auto sumState = sim_state(base_setup.schedule);
        auto udqState = UDQState{1};
        auto aquiferData = std::optional<Opm::RestartIO::Helpers::AggregateAquiferData>{std::nullopt};
        Action::State action_state;
        WellTestState wtest_state;
        {
            RestartValue restart_value(cells, wells, groups, {});

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
                                wtest_state,
                                sumState,
                                udqState,
                                aquiferData,
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
                                wtest_state,
                                sumState,
                                udqState,
                                aquiferData,
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
    auto st = sim_state(base_setup.schedule);
    Action::State action_state;
    UDQState udq_state(1);

    auto state1 = first_sim( base_setup , action_state, st, udq_state, true);
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
        Opm::SummaryState sumState(TimeService::now());
        Opm::Action::State action_state;
        Opm::UDQState udq_state(19);
        Opm::WellTestState wtest_state;
        auto aquiferData = std::optional<Opm::RestartIO::Helpers::AggregateAquiferData>{std::nullopt};

        const auto seqnum = 1;
        auto rstFile = OS::Restart {
            OS::ResultSet { test_area.currentWorkingDirectory(), "FILE" }, seqnum,
            OS::Formatted { false }, OS::Unified { true }
        };

        BOOST_CHECK_THROW( RestartIO::save(rstFile, seqnum,
                                           100,
                                           RestartValue(cells, wells, groups, {}),
                                           setup.es,
                                           setup.grid ,
                                           setup.schedule,
                                           action_state,
                                           wtest_state,
                                           sumState,
                                           udq_state,
                                           aquiferData),
                           std::runtime_error);
    }
}


BOOST_AUTO_TEST_CASE(ExtraData_KEYS) {
    Setup setup("BASE_SIM.DATA");
    auto num_cells = setup.grid.getNumActive( );
    auto cells = mkSolution( num_cells );
    auto wells = mkWells();
    auto groups = mkGroups();
    RestartValue restart_value(cells, wells, groups, {});

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
        WellTestState wtest_state;
        UDQState udq_state(10);
        auto num_cells = setup.grid.getNumActive( );
        auto cells = mkSolution( num_cells );
        auto wells = mkWells();
        auto groups = mkGroups();
        auto aquiferData = std::optional<Opm::RestartIO::Helpers::AggregateAquiferData>{std::nullopt};
        const auto& units = setup.es.getUnits();
        {
            RestartValue restart_value(cells, wells, groups, {});
            SummaryState st(TimeService::now());
            const auto sumState = sim_state(setup.schedule);

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
                                wtest_state,
                                sumState,
                                udq_state,
                                aquiferData);
            }

            const auto rstFile = ::Opm::EclIO::OutputStream::
                outputFileName({outputDir, "FILE"}, "UNRST");

            {
                EclIO::ERst rst{ rstFile };
                BOOST_CHECK_MESSAGE( rst.hasKey("EXTRA"), "Restart file is expexted to have EXTRA vector");

                const auto& ex = rst.getRestartData<double>("EXTRA", 1, 0);
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
        auto aquiferData = std::optional<Opm::RestartIO::Helpers::AggregateAquiferData>{std::nullopt};
        const auto outputDir = test_area.currentWorkingDirectory();
        {
            RestartValue restart_value(cells, wells, groups, {});
            RestartValue restart_value2(cells, wells, groups, {});

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
            const auto sumState = sim_state(base_setup.schedule);
            Action::State action_state;
            UDQState udq_state(99);
            WellTestState wtest_state;

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
                                                   wtest_state,
                                                   sumState,
                                                   udq_state,
                                                   aquiferData),
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
                                wtest_state,
                                sumState,
                                udq_state,
                                aquiferData);
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
        mkGroups(),
        {}
    };
    const auto sumState = sim_state(setup.schedule);
    UDQState udq_state(98);
    namespace OS = ::Opm::EclIO::OutputStream;

    const auto rset   = OS::ResultSet{ wa.currentWorkingDirectory(), "FILE" };
    const auto seqnum = 1;
    {
        Action::State action_state;
        WellTestState wtest_state;
        auto aquiferData = std::optional<Opm::RestartIO::Helpers::AggregateAquiferData>{std::nullopt};
        auto rstFile = OS::Restart {
            rset, seqnum, OS::Formatted{ false }, OS::Unified{ true }
        };

        RestartIO::save(rstFile, seqnum, 100, restart_value,
                        setup.es, setup.grid, setup.schedule,
                        action_state, wtest_state, sumState, udq_state, aquiferData);
    }

    Action::State action_state;
    SummaryState rstSumState(TimeService::now());
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
    st.update_group_var("WGRP2", "GOPR", 100);
    st.update("FLPR", 100);
}

BOOST_AUTO_TEST_CASE(UDQ_RESTART) {
    std::vector<RestartKey> keys {{"PRESSURE" , UnitSystem::measure::pressure},
        {"SWAT" , UnitSystem::measure::identity},
        {"SGAS" , UnitSystem::measure::identity}};
    WorkArea test_area("test_udq_restart");
    test_area.copyIn("UDQ_BASE.DATA");
    test_area.copyIn("UDQ_RESTART.DATA");

    Setup base_setup("UDQ_BASE.DATA");
    SummaryState st1(TimeService::now());
    SummaryState st2(TimeService::now());
    Action::State action_state;
    UDQState udq_state(1);
    init_st(st1);
    auto state1 = first_sim( base_setup , action_state, st1, udq_state, false );

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

namespace {

struct MessageBuffer
{
    std::stringstream str_;

    template <class T>
    void read( T& value )
    {
        str_.read( (char *) &value, sizeof(value) );
    }

    template <class T>
    void write( const T& value )
    {
        str_.write( (const char *) &value, sizeof(value) );
    }

    void write( const std::string& str)
    {
        int size = str.size();
        write(size);
        for (int k = 0; k < size; ++k) {
            write(str[k]);
        }
    }

    void read( std::string& str)
    {
        int size = 0;
        read(size);
        str.resize(size);
        for (int k = 0; k < size; ++k) {
            read(str[k]);
        }
    }
};

Opm::data::AquiferData getFetkovichAquifer(const int aquiferID = 1)
{
    auto aquifer = Opm::data::AquiferData {
        aquiferID, 123.456, 56.78, 9.0e10, 290.0, 2515.5
    };

    auto* aquFet = aquifer.typeData.create<Opm::data::AquiferType::Fetkovich>();

    aquFet->initVolume = 1.23;
    aquFet->prodIndex = 45.67;
    aquFet->timeConstant = 890.123;

    return aquifer;
}

Opm::data::AquiferData getCarterTracyAquifer(const int aquiferID = 5)
{
    auto aquifer = Opm::data::AquiferData {
        aquiferID, 123.456, 56.78, 9.0e10, 290.0, 2515.5
    };

    auto* aquCT = aquifer.typeData.create<Opm::data::AquiferType::CarterTracy>();

    aquCT->timeConstant = 987.65;
    aquCT->influxConstant = 43.21;
    aquCT->waterDensity = 1014.5;
    aquCT->waterViscosity = 0.00318;
    aquCT->dimensionless_time = 42.0;
    aquCT->dimensionless_pressure = 2.34;

    return aquifer;
}

Opm::data::AquiferData getNumericalAquifer(const int aquiferID = 2)
{
    auto aquifer = Opm::data::AquiferData {
        aquiferID, 123.456, 56.78, 9.0e10, 290.0, 2515.5
    };

    auto* aquNum = aquifer.typeData.create<Opm::data::AquiferType::Numerical>();

    aquNum->initPressure.push_back(1.234);
    aquNum->initPressure.push_back(2.345);
    aquNum->initPressure.push_back(3.4);
    aquNum->initPressure.push_back(9.876);

    return aquifer;
}
} // Anonymous

BOOST_AUTO_TEST_CASE(ReadWrite_CarterTracy_Data)
{
    const auto src = getCarterTracyAquifer(1729);

    BOOST_CHECK_MESSAGE(src.typeData.is<Opm::data::AquiferType::CarterTracy>(),
                        "Carter Tracy aquifer must be represented as AquiferType::CarterTracy");

    MessageBuffer buffer;
    src.write(buffer);

    auto dest = Opm::data::AquiferData{};
    dest.read(buffer);

    BOOST_CHECK_MESSAGE(src == dest, "Serialised/deserialised Carter-Tracy aquifer object must be equal to source object");
}

BOOST_AUTO_TEST_CASE(ReadWrite_Fetkovich_Data)
{
    const auto src = getFetkovichAquifer(42);

    BOOST_CHECK_MESSAGE(src.typeData.is<Opm::data::AquiferType::Fetkovich>(),
                        "Fetkovich aquifer must be represented as AquiferType::Fetkovich");

    MessageBuffer buffer;
    src.write(buffer);

    auto dest = Opm::data::AquiferData{};
    dest.read(buffer);

    BOOST_CHECK_MESSAGE(src == dest, "Serialised/deserialised Fetkovich object must be equal to source object");
}

BOOST_AUTO_TEST_CASE(ReadWrite_NumericalAquifer_Data)
{
    const auto src = getNumericalAquifer(11);

    BOOST_CHECK_MESSAGE(src.typeData.is<Opm::data::AquiferType::Numerical>(),
                        "Numeric aquifer must be represented as AquiferType::Numerical");

    MessageBuffer buffer;
    src.write(buffer);

    auto dest = Opm::data::AquiferData{};
    dest.read(buffer);

    BOOST_CHECK_MESSAGE(src == dest, "Serialised/deserialised Numerical aquifer object must be equal to source object");
}
