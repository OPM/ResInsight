/*
  Copyright 2016 Statoil ASA.

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

#define BOOST_TEST_MODULE Wells
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <cctype>
#include <ctime>

#include <opm/output/data/Wells.hpp>
#include <opm/output/data/Groups.hpp>
#include <opm/output/eclipse/Summary.hpp>

#include <opm/parser/eclipse/Python/Python.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <opm/parser/eclipse/Units/Units.hpp>

#include <opm/io/eclipse/ESmry.hpp>

#include <tests/WorkArea.cpp>

using namespace Opm;
using rt = data::Rates::opt;

namespace {
    double sm3_pr_day()
    {
       return unit::cubic(unit::meter) / unit::day;
    }

    std::string toupper(std::string input)
    {
        for (auto& c : input) {
            const auto uc = std::toupper(static_cast<unsigned char>(c));
            c = static_cast<std::string::value_type>(uc);
        }

        return input;
    }

    bool ecl_sum_has_group_var( const EclIO::ESmry* smry,
                           const std::string&  groupname,
                           const std::string&  variable )
    {
        return smry->hasKey(variable + ':' + groupname);
    }

    double ecl_sum_get_group_var( const EclIO::ESmry* smry,
                              const int           timeIdx,
                              const std::string&  groupname,
                              const std::string&  variable )
    {
        return smry->get(variable + ':' + groupname)[timeIdx];
    }

} // Anonymous



namespace {
/* conversion factor for whenever 'day' is the unit of measure, whereas we
 * expect input in SI units (seconds)
 */

std::unique_ptr< EclIO::ESmry > readsum( const std::string& base ) {
    return std::make_unique<EclIO::ESmry>(base);
}

using p_cmode = Opm::Group::ProductionCMode;
using i_cmode = Opm::Group::InjectionCMode;

static const int day = 24 * 60 * 60;

static data::Wells result_wells() {
        /* populate with the following pattern:
     *
     * Wells are named W_1, W_2 etc, i.e. wells are 1 indexed.
     *
     * rates on a well are populated with 10 * wellidx . type (where type is
     * 0-1-2 from owg)
     *
     * bhp is wellidx.1
     * bhp is wellidx.2
     *
     * completions are 100*wellidx . type
     */

    // conversion factor Pascal (simulator output) <-> barsa
    const double ps = 100000;

    data::Rates rates1;
    rates1.set( rt::wat, -10.0 / day );
    rates1.set( rt::oil, -10.1 / day );
    rates1.set( rt::gas, -10.2 / day );
    rates1.set( rt::solvent, -10.3 / day );
    rates1.set( rt::dissolved_gas, -10.4 / day );
    rates1.set( rt::vaporized_oil, -10.5 / day );
    rates1.set( rt::reservoir_water, -10.6 / day );
    rates1.set( rt::reservoir_oil, -10.7 / day );
    rates1.set( rt::reservoir_gas, -10.8 / day );
    rates1.set( rt::productivity_index_water, -10.9 / day );
    rates1.set( rt::productivity_index_oil, -10.11 / day );
    rates1.set( rt::productivity_index_gas, -10.12 / day );
    rates1.set( rt::well_potential_water, -10.13 / day );
    rates1.set( rt::well_potential_oil, -10.14 / day );
    rates1.set( rt::well_potential_gas, -10.15 / day );

    /* completion rates */
    data::Rates crates1;
    crates1.set( rt::wat, -100.0 / day );
    crates1.set( rt::oil, -100.1 / day );
    crates1.set( rt::gas, -100.2 / day );
    crates1.set( rt::solvent, -100.3 / day );
    crates1.set( rt::dissolved_gas, -100.4 / day );
    crates1.set( rt::vaporized_oil, -100.5 / day );
    crates1.set( rt::reservoir_water, -100.6 / day );
    crates1.set( rt::reservoir_oil, -100.7 / day );
    crates1.set( rt::reservoir_gas, -100.8 / day );

    // Segment vectors
    auto segment = ::Opm::data::Segment{};
    segment.rates.set(rt::wat,  123.45*sm3_pr_day());
    segment.rates.set(rt::oil,  543.21*sm3_pr_day());
    segment.rates.set(rt::gas, 1729.496*sm3_pr_day());
    {
        const auto pres_idx = Opm::data::SegmentPressures::Value::Pressure;
        segment.pressures[pres_idx] = 314.159*unit::barsa;
    }
    segment.segNumber = 1;

    /*
      The global index assigned to the completion must be manually
      syncronized with the global index in the COMPDAT keyword in the
      input deck.
    */
    data::Connection well1_comp1 { 0  , crates1, 1.9 , 123.4, 314.15, 0.35, 0.25, 2.718e2};

    /*
      The completions
    */
    data::Well well1 {
        rates1, 0.1 * ps, 0.2 * ps, 0.3 * ps, 1,
        { {well1_comp1} },
        { { segment.segNumber, segment } },
        data::CurrentControl{}
    };
    well1.current_control.isProducer = true;
    well1.current_control.prod = ::Opm::Well::ProducerCMode::BHP;

    data::Wells wellrates;

    wellrates["OPU01"] = well1;

    return wellrates;

}

static data::Group result_groups() {
    data::Group groups;
    data::currentGroupConstraints cgc_group;

    cgc_group.set(p_cmode::NONE, i_cmode::VREP, i_cmode::RATE);
    groups.emplace("TEST", cgc_group);

    cgc_group.set(p_cmode::ORAT, i_cmode::RESV, i_cmode::REIN);
    groups.emplace("LOWER", cgc_group);

    cgc_group.set(p_cmode::GRAT, i_cmode::REIN, i_cmode::VREP);
    groups.emplace("UPPER", cgc_group);

    cgc_group.set(p_cmode::NONE, i_cmode::NONE, i_cmode::NONE);
    groups.emplace("FIELD", cgc_group);

    return groups;
}


struct setup {
    Deck deck;
    EclipseState es;
    const EclipseGrid& grid;
    std::shared_ptr<Python> python;
    Schedule schedule;
    SummaryConfig config;
    data::Wells wells;
    data::Group groups;
    std::string name;
    WorkArea ta;

    /*-----------------------------------------------------------------*/

    setup(std::string fname, const std::string& path = "UDQ_ACTIONX_TEST1_U.DATA") :
        deck( Parser().parseFile( path) ),
        es( deck ),
        grid( es.getInputGrid() ),
        python( std::make_shared<Python>() ),
        schedule( deck, es, python),
        config( deck, schedule, es.getTableManager()),
        wells( result_wells() ),
        groups( result_groups() ),
        name( toupper(std::move(fname)) ),
        ta( "test_summary_group_constraints" )
    {}
    };
} // Anonymous namespace

BOOST_AUTO_TEST_SUITE(Summary)
/*
 * Tests works by reading the Deck, write the summary output, then immediately
 * read it again (with ERT), and compare the read values with the input.
 */
BOOST_AUTO_TEST_CASE(group_keywords) {
    setup cfg( "test_summary_group_constraints");

    // Force to run in a directory, to make sure the basename with
    // leading path works.
    cfg.ta.makeSubDir( "PATH" );
    cfg.name = "PATH/CASE";

    SummaryState st(std::chrono::system_clock::now());

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule , cfg.name );
    writer.eval(st, 0, 0*day, cfg.es, cfg.schedule, cfg.wells, cfg.groups, {});
    writer.add_timestep( st, 0);

    writer.eval(st, 1, 1*day, cfg.es, cfg.schedule, cfg.wells, cfg.groups, {});
    writer.add_timestep( st, 1);

    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    //BOOST_CHECK( ecl_sum_has_report_step( resp, 1 ) );
    BOOST_CHECK( ecl_sum_has_group_var( resp, "TEST", "GMCTP" ) );


    // Integer flag indicating current active group control
    BOOST_CHECK_EQUAL( static_cast<int>(ecl_sum_get_group_var( resp, 1, "TEST", "GMCTP" )), 0 );
    BOOST_CHECK_EQUAL( static_cast<int>(ecl_sum_get_group_var( resp, 1, "LOWER", "GMCTW" )), 3 );
    BOOST_CHECK_EQUAL( static_cast<int>(ecl_sum_get_group_var( resp, 1, "LOWER", "GMCTP" )), 1 );

    BOOST_CHECK_EQUAL( static_cast<int>(ecl_sum_get_group_var( resp, 1, "UPPER", "GMCTP" )), 3 );
    BOOST_CHECK_EQUAL( static_cast<int>(ecl_sum_get_group_var( resp, 1, "UPPER", "GMCTW" )), 4 );
    BOOST_CHECK_EQUAL( static_cast<int>(ecl_sum_get_group_var( resp, 1, "UPPER", "GMCTG" )), 3 );


}

BOOST_AUTO_TEST_SUITE_END()
