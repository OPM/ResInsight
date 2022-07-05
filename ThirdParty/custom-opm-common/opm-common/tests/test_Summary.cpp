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

#include <cstddef>
#include <exception>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <cctype>
#include <ctime>
#include <filesystem>

#include <fmt/format.h>

#include <opm/output/data/Groups.hpp>
#include <opm/output/data/GuideRateValue.hpp>
#include <opm/output/data/Wells.hpp>
#include <opm/output/eclipse/WStat.hpp>
#include <opm/output/eclipse/Summary.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/output/eclipse/Inplace.hpp>
#include <opm/input/eclipse/Python/Python.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Deck/Deck.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Parser/Parser.hpp>

#include <opm/input/eclipse/Units/Units.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

#include <opm/io/eclipse/ESmry.hpp>
#include <opm/io/eclipse/ERsm.hpp>

#include <tests/WorkArea.hpp>

using namespace Opm;
using rt = data::Rates::opt;
using p_cmode = Opm::Group::ProductionCMode;
using i_cmode = Opm::Group::InjectionCMode;

namespace {
    double sm3_pr_day()
    {
       return unit::cubic(unit::meter) / unit::day;
    }

    double rm3_pr_day()
    {
       return unit::cubic(unit::meter) / unit::day;
    }

    double cp_rm3_per_db()
    {
        return prefix::centi*unit::Poise * unit::cubic(unit::meter)
            /  (unit::day * unit::barsa);
    }

    std::string toupper(std::string input)
    {
        for (auto& c : input) {
            const auto uc = std::toupper(static_cast<unsigned char>(c));
            c = static_cast<std::string::value_type>(uc);
        }

        return input;
    }
} // Anonymous

namespace SegmentResultHelpers {
    data::Well prod01_results();
    data::Well inje01_results();
} // SegmentResultHelpers

namespace {
/* conversion factor for whenever 'day' is the unit of measure, whereas we
 * expect input in SI units (seconds)
 */
static const int day = 24 * 60 * 60;

double liquid_PI_unit()
{
    return UnitSystem::newMETRIC().to_si(UnitSystem::measure::liquid_productivity_index, 1.0);
}

double gas_PI_unit()
{
    return UnitSystem::newMETRIC().to_si(UnitSystem::measure::gas_productivity_index, 1.0);
}

/*
  This is quite misleading, because the values prepared in the test
  input deck are NOT used.
*/
data::Wells result_wells(const bool w3_injector = true)
{
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
    rates1.set( rt::productivity_index_water, 10.9*liquid_PI_unit());
    rates1.set( rt::productivity_index_oil, 10.11*liquid_PI_unit());
    rates1.set( rt::productivity_index_gas, 10.12*gas_PI_unit());
    rates1.set( rt::well_potential_water, -10.13 / day );
    rates1.set( rt::well_potential_oil, -10.14 / day );
    rates1.set( rt::well_potential_gas, -10.15 / day );
    rates1.set( rt::polymer, -10.16 / day );
    rates1.set( rt::brine, -10.17 / day );
    rates1.set( rt::tracer, -10.18 / day, "SEA" );

    data::Rates rates2;
    rates2.set( rt::wat, -20.0 / day );
    rates2.set( rt::oil, -20.1 / day );
    rates2.set( rt::gas, -20.2 / day );
    rates2.set( rt::solvent, -20.3 / day );
    rates2.set( rt::dissolved_gas, -20.4 / day );
    rates2.set( rt::vaporized_oil, -20.5 / day );
    rates2.set( rt::reservoir_water, -20.6 / day );
    rates2.set( rt::reservoir_oil, -20.7 / day );
    rates2.set( rt::reservoir_gas, -20.8 / day );
    rates2.set( rt::productivity_index_water, 20.9*liquid_PI_unit());
    rates2.set( rt::productivity_index_oil, 20.11*liquid_PI_unit());
    rates2.set( rt::productivity_index_gas, 20.12*gas_PI_unit());
    rates2.set( rt::well_potential_water, -20.13 / day );
    rates2.set( rt::well_potential_oil, -20.14 / day );
    rates2.set( rt::well_potential_gas, -20.15 / day );
    rates2.set( rt::polymer, -20.16 / day );
    rates2.set( rt::brine, -20.17 / day );
    rates2.set( rt::tracer, -20.18 / day, "SEA" );

    data::Rates rates3;
    rates3.set( rt::wat, 30.0 / day );
    rates3.set( rt::oil, 30.1 / day );
    rates3.set( rt::gas, 30.2 / day );
    rates3.set( rt::solvent, 30.3 / day );
    rates3.set( rt::dissolved_gas, 30.4 / day );
    rates3.set( rt::vaporized_oil, 30.5 / day );
    rates3.set( rt::reservoir_water, 30.6 / day );
    rates3.set( rt::reservoir_oil, 30.7 / day );
    rates3.set( rt::reservoir_gas, 30.8 / day );
    rates3.set( rt::productivity_index_water, 30.9*liquid_PI_unit());
    rates3.set( rt::productivity_index_oil, 30.11*liquid_PI_unit());
    rates3.set( rt::productivity_index_gas, 30.12*gas_PI_unit());
    rates3.set( rt::well_potential_water, 30.13 / day );
    rates3.set( rt::well_potential_oil, 30.14 / day );
    rates3.set( rt::well_potential_gas, 30.15 / day );
    rates3.set( rt::polymer, 30.16 / day );
    rates3.set( rt::brine, 30.17 / day );
    rates3.set( rt::tracer, 30.18 / day, "SEA" );

    data::Rates rates6;
    rates6.set( rt::wat, 60.0 / day );
    rates6.set( rt::oil, 60.1 / day );
    rates6.set( rt::gas, 60.2 / day );
    rates6.set( rt::solvent, 60.3 / day );
    rates6.set( rt::dissolved_gas, 60.4 / day );
    rates6.set( rt::vaporized_oil, 60.5 / day );
    rates6.set( rt::reservoir_water, 60.6 / day );
    rates6.set( rt::reservoir_oil, 60.7 / day );
    rates6.set( rt::reservoir_gas, 60.8 / day );
    rates6.set( rt::productivity_index_water, 60.9*liquid_PI_unit());
    rates6.set( rt::productivity_index_oil, 60.11*liquid_PI_unit());
    rates6.set( rt::productivity_index_gas, 60.12*gas_PI_unit());
    rates6.set( rt::well_potential_water, 60.13 / day );
    rates6.set( rt::well_potential_oil, 60.14 / day );
    rates6.set( rt::well_potential_gas, 60.15 / day );
    rates6.set( rt::polymer, 60.16 / day );
    rates6.set( rt::brine, 60.17 / day );
    rates6.set( rt::tracer, 60.18 / day, "SEA" );

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

    data::Rates crates2;
    crates2.set( rt::wat, -200.0 / day );
    crates2.set( rt::oil, -200.1 / day );
    crates2.set( rt::gas, -200.2 / day );
    crates2.set( rt::solvent, -200.3 / day );
    crates2.set( rt::dissolved_gas, -200.4 / day );
    crates2.set( rt::vaporized_oil, -200.5 / day );
    crates2.set( rt::reservoir_water, -200.6 / day );
    crates2.set( rt::reservoir_oil, -200.7 / day );
    crates2.set( rt::reservoir_gas, -200.8 / day );

    data::Rates crates3;
    crates3.set( rt::wat, 300.0 / day );
    crates3.set( rt::oil, 300.1 / day );
    crates3.set( rt::gas, 300.2 / day );
    crates3.set( rt::solvent, 300.3 / day );
    crates3.set( rt::dissolved_gas, 300.4 / day );
    crates3.set( rt::vaporized_oil, 300.5 / day );
    crates3.set( rt::reservoir_water, 300.6 / day );
    crates3.set( rt::reservoir_oil, 300.7 / day );
    crates3.set( rt::reservoir_gas, 300.8 / day );
    crates3.set( rt::polymer, 300.16 / day );
    crates3.set( rt::brine, 300.17 / day );

    data::Rates crates6;
    crates6.set( rt::wat, 600.0 / day );
    crates6.set( rt::oil, 600.1 / day );
    crates6.set( rt::gas, 600.2 / day );
    crates6.set( rt::solvent, 600.3 / day );
    crates6.set( rt::dissolved_gas, 600.4 / day );
    crates6.set( rt::vaporized_oil, 600.5 / day );
    crates6.set( rt::reservoir_water, 600.6 / day );
    crates6.set( rt::reservoir_oil, 600.7 / day );
    crates6.set( rt::reservoir_gas, 600.8 / day );

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
    data::Connection well1_comp1 { 0  , crates1, 1.9 *unit::barsa, -123.4 *rm3_pr_day(), 314.15, 0.35 , 0.25,   2.718e2, 111.222*cp_rm3_per_db() };
    data::Connection well2_comp1 { 1  , crates2, 1.10*unit::barsa, - 23.4 *rm3_pr_day(), 212.1 , 0.78 , 0.0 ,  12.34   , 222.333*cp_rm3_per_db() };
    data::Connection well2_comp2 { 101, crates3, 1.11*unit::barsa, -234.5 *rm3_pr_day(), 150.6 , 0.001, 0.89, 100.0    , 333.444*cp_rm3_per_db() };
    data::Connection well3_comp1 { 2  , crates3, 1.11*unit::barsa,  432.1 *rm3_pr_day(), 456.78, 0.0  , 0.15, 432.1    , 444.555*cp_rm3_per_db() };
    data::Connection well6_comp1 { 77 , crates6, 6.11*unit::barsa,  321.09*rm3_pr_day(), 656.78, 0.0  , 0.65, 632.1    , 555.666*cp_rm3_per_db() };

    /*
      The completions
    */
    data::Well well1 {
        rates1, 0.1 * ps, 0.2 * ps, 0.3 * ps, 1,

        ::Opm::Well::Status::OPEN,

        { {well1_comp1} },
        { { segment.segNumber, segment } },
        data::CurrentControl{},
        data::GuideRateValue{}
    };
    well1.current_control.isProducer = true;
    well1.current_control.prod = ::Opm::Well::ProducerCMode::THP;
    well1.guide_rates.set(data::GuideRateValue::Item::Oil, 123.456*sm3_pr_day())
                     .set(data::GuideRateValue::Item::Gas, 2345.67*sm3_pr_day());

    using SegRes = decltype(well1.segments);
    using Ctrl = data::CurrentControl;
    using GRValue = data::GuideRateValue;

    data::Well well2 {
        rates2, 1.1 * ps, 1.2 * ps, 1.3 * ps, 2,

        ::Opm::Well::Status::OPEN,

        { {well2_comp1 , well2_comp2} }, SegRes{}, Ctrl{}, GRValue{}
    };

    well2.current_control.prod = ::Opm::Well::ProducerCMode::ORAT;
    well2.guide_rates.set(GRValue::Item::Water, 654.321*sm3_pr_day());

    data::Well well3 {
        rates3, 2.1 * ps, 2.2 * ps, 2.3 * ps, 3,

        ::Opm::Well::Status::OPEN,

        { {well3_comp1} }, SegRes{}, Ctrl{}, GRValue{}
    };
    well3.current_control.isProducer = !w3_injector;
    if (! well3.current_control.isProducer) { // W_3 is injector
        well3.current_control.inj = ::Opm::Well::InjectorCMode::BHP;
    }
    else {
        well3.current_control.prod = ::Opm::Well::ProducerCMode::BHP;

        auto& xc = well3.connections[0];
        xc.reservoir_rate = - xc.reservoir_rate;

        for (const auto& p : { rt::wat, rt::oil, rt::gas, rt::solvent,
                    rt::dissolved_gas, rt::vaporized_oil,
                    rt::reservoir_water, rt::reservoir_oil, rt::reservoir_gas,
                    rt::polymer, rt::brine })
        {
            xc.rates.set(p, - xc.rates.get(p));
        }
    }

    well3.guide_rates.set(GRValue::Item::ResV, 355.113*sm3_pr_day());

    data::Well well6 {
        rates6, 2.1 * ps, 2.2 * ps, 2.3 * ps, 3,

        ::Opm::Well::Status::OPEN,

        { {well6_comp1} }, SegRes{}, Ctrl{}, GRValue{}
    };
    well6.current_control.isProducer = false;
    well6.current_control.inj = ::Opm::Well::InjectorCMode::GRUP;
    well6.guide_rates.set(GRValue::Item::Gas, 222.333*sm3_pr_day())
                     .set(GRValue::Item::Water, 333.444*sm3_pr_day());

    data::Wells wellrates;

    wellrates["W_1"] = well1;
    wellrates["W_2"] = well2;
    wellrates["W_3"] = well3;
    wellrates["W_6"] = well6;

    wellrates["INJE01"] = SegmentResultHelpers::inje01_results();
    wellrates["PROD01"] = SegmentResultHelpers::prod01_results();

    return wellrates;
}

data::GroupAndNetworkValues result_group_nwrk()
{
    using GRValue = data::GuideRateValue;

    data::GroupAndNetworkValues grp_nwrk;
    data::GroupConstraints cgc_group;

    cgc_group.set(p_cmode::NONE, i_cmode::VREP, i_cmode::RATE);
    {
        auto& grp = grp_nwrk.groupData["G_1"];
        grp.currentControl = cgc_group;

        grp.guideRates.production
            .set(GRValue::Item::Oil, 1111.2222*sm3_pr_day())
            .set(GRValue::Item::Gas, 2222.3333*sm3_pr_day())
            .set(GRValue::Item::Water, 3333.4444*sm3_pr_day())
            .set(GRValue::Item::ResV, 4444.5555*sm3_pr_day());

        grp.guideRates.injection
            .set(GRValue::Item::Gas, 9999.8888*sm3_pr_day())
            .set(GRValue::Item::Water, 8888.7777*sm3_pr_day());
    }

    cgc_group.set(p_cmode::ORAT, i_cmode::RESV, i_cmode::FLD);
    grp_nwrk.groupData["G_2"].currentControl = cgc_group;

    cgc_group.set(p_cmode::GRAT, i_cmode::REIN, i_cmode::VREP);
    grp_nwrk.groupData["G_3"].currentControl = cgc_group;

    cgc_group.set(p_cmode::NONE, i_cmode::NONE, i_cmode::NONE);
    grp_nwrk.groupData["FIELD"].currentControl = cgc_group;

    grp_nwrk.nodeData["G_1"].pressure = 33.44*Opm::unit::barsa;
    grp_nwrk.nodeData["G_2"].pressure = 23.45*Opm::unit::barsa;
    grp_nwrk.nodeData["PLAT-A"].pressure = 21.0*Opm::unit::barsa;

    return grp_nwrk;
}

std::unique_ptr< EclIO::ESmry > readsum( const std::string& base ) {
    return std::make_unique<EclIO::ESmry>(base);
}

bool ecl_sum_has_key( const EclIO::ESmry* smry,
                      const std::string&  key )
{
    return smry->hasKey(key);
}

bool ecl_sum_has_field_var( const EclIO::ESmry* smry,
                            const std::string&  variable )
{
    return smry->hasKey(variable);
}

double ecl_sum_get_field_var(const EclIO::ESmry* smry,
                             const int           timeIdx,
                             const std::string&  var)
{
    return smry->get(var)[timeIdx];
}

bool ecl_sum_has_general_var( const EclIO::ESmry* smry,
                              const std::string&  variable)
{
    return smry->hasKey(variable);
}

double ecl_sum_get_general_var(const EclIO::ESmry* smry,
                               const int           timeIdx,
                               const std::string&  var)
{
    return smry->get(var)[timeIdx];
}

#if 0
bool ecl_sum_has_well_var( const EclIO::ESmry* smry,
                           const std::string&  wellname,
                           const std::string&  variable )
{
    return smry->hasKey(variable + ':' + wellname);
}
#endif

double ecl_sum_get_well_var( const EclIO::ESmry* smry,
                             const int           timeIdx,
                             const std::string&  wellname,
                             const std::string&  variable )
{
    return smry->get(variable + ':' + wellname)[timeIdx];
}

double ecl_sum_get_group_var( const EclIO::ESmry* smry,
                              const int           timeIdx,
                              const std::string&  groupname,
                              const std::string&  variable )
{
    return smry->get(variable + ':' + groupname)[timeIdx];
}

double ecl_sum_get_well_connection_var( const EclIO::ESmry* smry,
                                        const int           timeIdx,
                                        const std::string&  wellname,
                                        const std::string&  variable,
                                        const int           i,
                                        const int           j,
                                        const int           k)
{
    const auto ijk = std::to_string(i) + ',' + std::to_string(j) + ',' + std::to_string(k);
    return smry->get(variable + ':' + wellname + ':' + ijk)[timeIdx];
}

bool ecl_sum_has_well_connection_var( const EclIO::ESmry* smry,
                                      const std::string&  wellname,
                                      const std::string&  variable,
                                      const int           i,
                                      const int           j,
                                      const int           k)
{
    const auto key = fmt::format("{}:{}:{},{},{}", wellname, variable, i, j, k);
    return ecl_sum_has_key(smry, key);
}

struct setup {
    Deck deck;
    EclipseState es;
    const EclipseGrid& grid;
    std::shared_ptr<Python> python;
    Schedule schedule;
    SummaryConfig config;
    data::Wells wells;
    data::GroupAndNetworkValues grp_nwrk;
    std::string name;
    WorkArea ta;

    /*-----------------------------------------------------------------*/

    setup(std::string fname, const std::string& path = "summary_deck.DATA", const bool w3_injector = true) :
        deck( Parser().parseFile( path) ),
        es( deck ),
        grid( es.getInputGrid() ),
        python( std::make_shared<Python>() ),
        schedule( deck, es, python),
        config( deck, schedule, es.fieldProps(), es.aquifer()),
        wells( result_wells(w3_injector) ),
        grp_nwrk( result_group_nwrk() ),
        name( toupper(std::move(fname)) ),
        ta( "summary_test" )
    {}
};
} // Anonymous namespace

BOOST_AUTO_TEST_SUITE(Summary)
/*
 * Tests works by reading the Deck, write the summary output, then immediately
 * read it again (with ERT), and compare the read values with the input.
 */
BOOST_AUTO_TEST_CASE(well_keywords) {
    setup cfg( "test_summary_well" );

    // Force to run in a directory, to make sure the basename with
    // leading path works.
    cfg.ta.makeSubDir( "PATH" );
    cfg.name = "PATH/CASE";

    SummaryState st(TimeService::now());

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule , cfg.name );
    writer.eval(st, 0, 0*day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);

    writer.eval(st, 1, 1*day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);

    writer.eval(st, 2, 2*day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();




    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    /* Production rates */
    BOOST_CHECK_CLOSE( 10.0, ecl_sum_get_well_var( resp, 1, "W_1", "WWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.18, ecl_sum_get_well_var( resp, 1, "W_1", "WTPRSEA" ), 1e-5 );

    BOOST_CHECK_CLOSE( 20.0, ecl_sum_get_well_var( resp, 1, "W_2", "WWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.18, ecl_sum_get_well_var( resp, 1, "W_2", "WTPRSEA" ), 1e-5 );

    BOOST_CHECK_CLOSE( 10.1, ecl_sum_get_well_var( resp, 1, "W_1", "WOPR" ), 1e-5 );

    BOOST_CHECK_CLOSE( 20.1, ecl_sum_get_well_var( resp, 1, "W_2", "WOPR" ), 1e-5 );

    BOOST_CHECK_CLOSE( 10.2, ecl_sum_get_well_var( resp, 1, "W_1", "WGPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.8, ecl_sum_get_well_var( resp, 1, "W_1", "WGVPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2, ecl_sum_get_well_var( resp, 1, "W_2", "WGPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.8, ecl_sum_get_well_var( resp, 1, "W_2", "WGVPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.0 + 10.1, ecl_sum_get_well_var( resp, 1, "W_1", "WLPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.0 + 20.1, ecl_sum_get_well_var( resp, 1, "W_2", "WLPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.3, ecl_sum_get_well_var( resp, 1, "W_1", "WNPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.3, ecl_sum_get_well_var( resp, 1, "W_2", "WNPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.16, ecl_sum_get_well_var( resp, 1, "W_1", "WCPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.17, ecl_sum_get_well_var( resp, 1, "W_1", "WSPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.4, ecl_sum_get_well_var( resp, 1, "W_1", "WGPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.4, ecl_sum_get_well_var( resp, 1, "W_2", "WGPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 - 10.4, ecl_sum_get_well_var( resp, 1, "W_1", "WGPRF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2 - 20.4, ecl_sum_get_well_var( resp, 1, "W_2", "WGPRF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.6 + 10.7 + 10.8,
                                    ecl_sum_get_well_var( resp, 1, "W_1", "WVPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.6 + 20.7 + 20.8,
                                    ecl_sum_get_well_var( resp, 1, "W_2", "WVPR" ), 1e-5 );

    BOOST_CHECK_CLOSE( 10.5, ecl_sum_get_well_var( resp, 1, "W_1", "WOPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.5, ecl_sum_get_well_var( resp, 1, "W_2", "WOPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.1 - 10.5), ecl_sum_get_well_var( resp, 1, "W_1", "WOPRF" ), 1e-5 );
    BOOST_CHECK_CLOSE( (20.1 - 20.5), ecl_sum_get_well_var( resp, 1, "W_2", "WOPRF" ), 1e-5 );

    BOOST_CHECK_CLOSE( -10.13, ecl_sum_get_well_var( resp, 1, "W_1", "WWPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -10.14, ecl_sum_get_well_var( resp, 1, "W_1", "WOPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -10.15, ecl_sum_get_well_var( resp, 1, "W_1", "WGPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -20.13, ecl_sum_get_well_var( resp, 1, "W_2", "WWPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -20.14, ecl_sum_get_well_var( resp, 1, "W_2", "WOPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -20.15, ecl_sum_get_well_var( resp, 1, "W_2", "WGPP" ), 1e-5 );
    BOOST_CHECK_CLOSE(  30.13, ecl_sum_get_well_var( resp, 1, "W_3", "WWPI" ), 1e-5 );
    BOOST_CHECK_CLOSE(  60.15, ecl_sum_get_well_var( resp, 1, "W_6", "WGPI" ), 1e-5 );

    BOOST_CHECK_CLOSE( 10.9 , ecl_sum_get_well_var( resp, 1, "W_1", "WPIW" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 10.11, ecl_sum_get_well_var( resp, 1, "W_1", "WPIO" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 10.12, ecl_sum_get_well_var( resp, 1, "W_1", "WPIG" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 10.11, ecl_sum_get_well_var( resp, 1, "W_1", "WPI"  ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 21.01, ecl_sum_get_well_var( resp, 1, "W_1", "WPIL" ), 1.0e-5 );

    BOOST_CHECK_CLOSE( 20.9 , ecl_sum_get_well_var( resp, 1, "W_2", "WPIW" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 20.11, ecl_sum_get_well_var( resp, 1, "W_2", "WPIO" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 20.12, ecl_sum_get_well_var( resp, 1, "W_2", "WPIG" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 20.11, ecl_sum_get_well_var( resp, 1, "W_2", "WPI"  ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 41.01, ecl_sum_get_well_var( resp, 1, "W_2", "WPIL" ), 1.0e-5 );

    BOOST_CHECK_CLOSE( 30.9 , ecl_sum_get_well_var( resp, 1, "W_3", "WPIW" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 30.11, ecl_sum_get_well_var( resp, 1, "W_3", "WPIO" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 30.12, ecl_sum_get_well_var( resp, 1, "W_3", "WPIG" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 30.9 , ecl_sum_get_well_var( resp, 1, "W_3", "WPI"  ), 1.0e-5 );

    BOOST_CHECK_CLOSE(  60.9 , ecl_sum_get_well_var( resp, 1, "W_6", "WPIW" ), 1.0e-5 );
    BOOST_CHECK_CLOSE(  60.11, ecl_sum_get_well_var( resp, 1, "W_6", "WPIO" ), 1.0e-5 );
    BOOST_CHECK_CLOSE(  60.12, ecl_sum_get_well_var( resp, 1, "W_6", "WPIG" ), 1.0e-5 );
    BOOST_CHECK_CLOSE(  60.12, ecl_sum_get_well_var( resp, 1, "W_6", "WPI"  ), 1.0e-5 );

    BOOST_CHECK_CLOSE( 123.456, ecl_sum_get_well_var(resp, 1, "W_1", "WOPGR"), 1.0e-5);
    BOOST_CHECK_CLOSE(2345.67 , ecl_sum_get_well_var(resp, 1, "W_1", "WGPGR"), 1.0e-5);
    BOOST_CHECK_CLOSE( 654.321, ecl_sum_get_well_var(resp, 1, "W_2", "WWPGR"), 1.0e-5);
    BOOST_CHECK_CLOSE( 222.333, ecl_sum_get_well_var(resp, 1, "W_6", "WGIGR"), 1.0e-5);
    BOOST_CHECK_CLOSE( 333.444, ecl_sum_get_well_var(resp, 1, "W_6", "WWIGR"), 1.0e-5);

    /* Production totals */
    BOOST_CHECK_CLOSE( 10.0, ecl_sum_get_well_var( resp, 1, "W_1", "WWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.18, ecl_sum_get_well_var( resp, 1, "W_1", "WTPTSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.0, ecl_sum_get_well_var( resp, 1, "W_2", "WWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.18, ecl_sum_get_well_var( resp, 1, "W_2", "WTPTSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1, ecl_sum_get_well_var( resp, 1, "W_1", "WOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.1, ecl_sum_get_well_var( resp, 1, "W_2", "WOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2, ecl_sum_get_well_var( resp, 1, "W_1", "WGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2, ecl_sum_get_well_var( resp, 1, "W_2", "WGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.3, ecl_sum_get_well_var( resp, 1, "W_1", "WNPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.3, ecl_sum_get_well_var( resp, 1, "W_2", "WNPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.0 + 10.1), ecl_sum_get_well_var( resp, 1, "W_1", "WLPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( (20.0 + 20.1), ecl_sum_get_well_var( resp, 1, "W_2", "WLPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.5, ecl_sum_get_well_var( resp, 1, "W_1", "WOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.5, ecl_sum_get_well_var( resp, 1, "W_2", "WOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.1 - 10.5), ecl_sum_get_well_var( resp, 1, "W_1", "WOPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( (20.1 - 20.5), ecl_sum_get_well_var( resp, 1, "W_2", "WOPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.6 + 10.7 + 10.8,
                                        ecl_sum_get_well_var( resp, 1, "W_1", "WVPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.6 + 20.7 + 20.8,
                                        ecl_sum_get_well_var( resp, 1, "W_2", "WVPT" ), 1e-5 );

    BOOST_CHECK_CLOSE( 2 * 10.0, ecl_sum_get_well_var( resp, 2, "W_1", "WWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 10.18, ecl_sum_get_well_var( resp, 2, "W_1", "WTPTSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.0, ecl_sum_get_well_var( resp, 2, "W_2", "WWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 10.1, ecl_sum_get_well_var( resp, 2, "W_1", "WOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.1, ecl_sum_get_well_var( resp, 2, "W_2", "WOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 10.2, ecl_sum_get_well_var( resp, 2, "W_1", "WGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.2, ecl_sum_get_well_var( resp, 2, "W_2", "WGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( ( 20.0 + 20.1 ), ecl_sum_get_well_var( resp, 2, "W_2", "WLPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (20.0 + 20.1), ecl_sum_get_well_var( resp, 2, "W_2", "WLPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 10.3, ecl_sum_get_well_var( resp, 2, "W_1", "WNPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.3, ecl_sum_get_well_var( resp, 2, "W_2", "WNPT" ), 1e-5 );

    BOOST_CHECK_CLOSE( 2 * 10.4, ecl_sum_get_well_var( resp, 2, "W_1", "WGPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.4, ecl_sum_get_well_var( resp, 2, "W_2", "WGPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * ( 10.2 - 10.4 ), ecl_sum_get_well_var( resp, 2, "W_1", "WGPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * ( 20.2 - 20.4 ), ecl_sum_get_well_var( resp, 2, "W_2", "WGPTF" ), 1e-5 );

    BOOST_CHECK_CLOSE( 2 * 10.5, ecl_sum_get_well_var( resp, 2, "W_1", "WOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.5, ecl_sum_get_well_var( resp, 2, "W_2", "WOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * ( 10.1 - 10.5 ), ecl_sum_get_well_var( resp, 2, "W_1", "WOPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * ( 20.1 - 20.5 ), ecl_sum_get_well_var( resp, 2, "W_2", "WOPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.6 + 10.7 + 10.8),
                                        ecl_sum_get_well_var( resp, 2, "W_1", "WVPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (20.6 + 20.7 + 20.8),
                                        ecl_sum_get_well_var( resp, 2, "W_2", "WVPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 10.16, ecl_sum_get_well_var( resp, 2, "W_1", "WCPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 10.17, ecl_sum_get_well_var( resp, 2, "W_1", "WSPT" ), 1e-5 );

    /* Production rates (history) */
    BOOST_CHECK_CLOSE( 10, ecl_sum_get_well_var( resp, 1, "W_1", "WWPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20, ecl_sum_get_well_var( resp, 1, "W_2", "WWPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1, ecl_sum_get_well_var( resp, 1, "W_1", "WOPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.1, ecl_sum_get_well_var( resp, 1, "W_2", "WOPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2, ecl_sum_get_well_var( resp, 1, "W_1", "WGPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2, ecl_sum_get_well_var( resp, 1, "W_2", "WGPRH" ), 1e-5 );

    /* Production totals (history) */
    BOOST_CHECK_CLOSE( 2 * 10.0, ecl_sum_get_well_var( resp, 2, "W_1", "WWPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.0, ecl_sum_get_well_var( resp, 2, "W_2", "WWPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 10.1, ecl_sum_get_well_var( resp, 2, "W_1", "WOPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.1, ecl_sum_get_well_var( resp, 2, "W_2", "WOPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 10.2, ecl_sum_get_well_var( resp, 2, "W_1", "WGPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.2, ecl_sum_get_well_var( resp, 2, "W_2", "WGPTH" ), 1e-5 );

    /* Injection rates */
    BOOST_CHECK_CLOSE( 30.0, ecl_sum_get_well_var( resp, 1, "W_3", "WWIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.18, ecl_sum_get_well_var( resp, 1, "W_3", "WTIRSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.6, ecl_sum_get_well_var( resp, 1, "W_3", "WWVIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.8, ecl_sum_get_well_var( resp, 1, "W_3", "WGVIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.2, ecl_sum_get_well_var( resp, 1, "W_3", "WGIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.3, ecl_sum_get_well_var( resp, 1, "W_3", "WNIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.16, ecl_sum_get_well_var( resp, 1, "W_3", "WCIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.17, ecl_sum_get_well_var( resp, 1, "W_3", "WSIR" ), 1e-5 );

    /* Injection totals */
    BOOST_CHECK_CLOSE( 30.0, ecl_sum_get_well_var( resp, 1, "W_3", "WWIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.18, ecl_sum_get_well_var( resp, 1, "W_3", "WTITSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.2, ecl_sum_get_well_var( resp, 1, "W_3", "WGIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.3, ecl_sum_get_well_var( resp, 1, "W_3", "WNIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.16, ecl_sum_get_well_var( resp, 1, "W_3", "WCIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( (30.6 + 30.7 + 30.8),
                       ecl_sum_get_well_var( resp, 1, "W_3", "WVIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 30.0, ecl_sum_get_well_var( resp, 2, "W_3", "WWIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 30.18, ecl_sum_get_well_var( resp, 2, "W_3", "WTITSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 30.2, ecl_sum_get_well_var( resp, 2, "W_3", "WGIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 30.3, ecl_sum_get_well_var( resp, 2, "W_3", "WNIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 30.16, ecl_sum_get_well_var( resp, 2, "W_3", "WCIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 30.17, ecl_sum_get_well_var( resp, 2, "W_3", "WSIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2* (30.6 + 30.7 + 30.8),
                       ecl_sum_get_well_var( resp, 2, "W_3", "WVIT" ), 1e-5 );

    /* Injection rates (history) */
    BOOST_CHECK_CLOSE( 30.0, ecl_sum_get_well_var( resp, 1, "W_3", "WWIRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0,    ecl_sum_get_well_var( resp, 1, "W_3", "WGIRH" ), 1e-5 );

    /* Injection totals (history) */
    BOOST_CHECK_CLOSE( 30.0, ecl_sum_get_well_var( resp, 1, "W_3", "WWITH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0,    ecl_sum_get_well_var( resp, 1, "W_3", "WGITH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 60.0, ecl_sum_get_well_var( resp, 2, "W_3", "WWITH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0,    ecl_sum_get_well_var( resp, 2, "W_3", "WGITH" ), 1e-5 );

    /* Production targets */
    BOOST_CHECK_CLOSE( 30.1 , ecl_sum_get_well_var( resp, 1, "W_5", "WVPRT" ), 1e-5 );

    /* WWCT - water cut */
    const double wwcut1 = 10.0 / ( 10.0 + 10.1 );
    const double wwcut2 = 20.0 / ( 20.0 + 20.1 );
    const double wwcut3 = 0;

    BOOST_CHECK_CLOSE( wwcut1, ecl_sum_get_well_var( resp, 1, "W_1", "WWCT" ), 1e-5 );
    BOOST_CHECK_CLOSE( wwcut2, ecl_sum_get_well_var( resp, 1, "W_2", "WWCT" ), 1e-5 );
    BOOST_CHECK_CLOSE( wwcut3, ecl_sum_get_well_var( resp, 1, "W_3", "WWCT" ), 1e-5 );

    /* Tracer concentration */
    const double wtpc1 = 10.18 / 10.0;
    const double wtpc2 = 20.18 / 20.0;
    const double wtic3 = 30.18 / 30.0;
    BOOST_CHECK_CLOSE( wtpc1, ecl_sum_get_well_var( resp, 1, "W_1", "WTPCSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( wtpc2, ecl_sum_get_well_var( resp, 1, "W_2", "WTPCSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( wtic3, ecl_sum_get_well_var( resp, 1, "W_3", "WTICSEA" ), 1e-5 );

    /* gas-oil ratio */
    const double wgor1 = 10.2 / 10.1;
    const double wgor2 = 20.2 / 20.1;
    const double wgor3 = 0;

    BOOST_CHECK_CLOSE( wgor1, ecl_sum_get_well_var( resp, 1, "W_1", "WGOR" ), 1e-5 );
    BOOST_CHECK_CLOSE( wgor2, ecl_sum_get_well_var( resp, 1, "W_2", "WGOR" ), 1e-5 );
    BOOST_CHECK_CLOSE( wgor3, ecl_sum_get_well_var( resp, 1, "W_3", "WGOR" ), 1e-5 );

    BOOST_CHECK_CLOSE( wgor1, ecl_sum_get_well_var( resp, 1, "W_1", "WGORH" ), 1e-5 );
    BOOST_CHECK_CLOSE( wgor2, ecl_sum_get_well_var( resp, 1, "W_2", "WGORH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0,     ecl_sum_get_well_var( resp, 1, "W_3", "WGORH" ), 1e-5 );

    /* WGLR - gas-liquid rate */
    const double wglr1 = 10.2 / ( 10.0 + 10.1 );
    const double wglr2 = 20.2 / ( 20.0 + 20.1 );
    const double wglr3 = 0;

    BOOST_CHECK_CLOSE( wglr1, ecl_sum_get_well_var( resp, 1, "W_1", "WGLR" ), 1e-5 );
    BOOST_CHECK_CLOSE( wglr2, ecl_sum_get_well_var( resp, 1, "W_2", "WGLR" ), 1e-5 );
    BOOST_CHECK_CLOSE( wglr3, ecl_sum_get_well_var( resp, 1, "W_3", "WGLR" ), 1e-5 );

    BOOST_CHECK_CLOSE( wglr1, ecl_sum_get_well_var( resp, 1, "W_1", "WGLRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( wglr2, ecl_sum_get_well_var( resp, 1, "W_2", "WGLRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0, ecl_sum_get_well_var( resp, 1, "W_3", "WGLRH" ), 1e-5 );

    /* BHP */
    BOOST_CHECK_CLOSE( 0.1, ecl_sum_get_well_var( resp, 1, "W_1", "WBHP" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.1, ecl_sum_get_well_var( resp, 1, "W_2", "WBHP" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2.1, ecl_sum_get_well_var( resp, 1, "W_3", "WBHP" ), 1e-5 );

    /* THP */
    BOOST_CHECK_CLOSE( 0.2, ecl_sum_get_well_var( resp, 1, "W_1", "WTHP" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.2, ecl_sum_get_well_var( resp, 1, "W_2", "WTHP" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2.2, ecl_sum_get_well_var( resp, 1, "W_3", "WTHP" ), 1e-5 );

    /* BHP (history) */
    BOOST_CHECK_CLOSE( 0.1, ecl_sum_get_well_var( resp, 1, "W_1", "WBHPH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.1, ecl_sum_get_well_var( resp, 1, "W_2", "WBHPH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2.1, ecl_sum_get_well_var( resp, 1, "W_3", "WBHPH" ), 1e-5 );

    /* THP (history) */
    BOOST_CHECK_CLOSE( 0.2, ecl_sum_get_well_var( resp, 1, "W_1", "WTHPH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.2, ecl_sum_get_well_var( resp, 1, "W_2", "WTHPH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2.2, ecl_sum_get_well_var( resp, 1, "W_3", "WTHPH" ), 1e-5 );

    /* State */
    BOOST_CHECK_CLOSE( WStat::numeric::PROD, ecl_sum_get_well_var(resp, 1,"W_1", "WSTAT"), 1e-5 );
    BOOST_CHECK_CLOSE( WStat::numeric::PROD, ecl_sum_get_well_var(resp, 1,"W_2", "WSTAT"), 1e-5 );
    BOOST_CHECK_CLOSE( WStat::numeric::INJ, ecl_sum_get_well_var(resp, 1,"W_3", "WSTAT"), 1e-5 );
}

BOOST_AUTO_TEST_CASE(well_keywords_dynamic_close) {
    setup cfg( "test_summary_well" );

    // Force to run in a directory, to make sure the basename with
    // leading path works.
    cfg.ta.makeSubDir( "PATH" );
    cfg.name = "PATH/CASE";

    SummaryState st(TimeService::now());

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    writer.eval(st, 0, 0*day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);

    cfg.wells.at("W_2").dynamicStatus = ::Opm::Well::Status::SHUT;
    writer.eval(st, 1, 1*day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);

    cfg.wells.at("W_2").dynamicStatus = ::Opm::Well::Status::OPEN;
    writer.eval(st, 2, 2*day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    /* State */
    BOOST_CHECK_CLOSE( WStat::numeric::SHUT, ecl_sum_get_well_var(resp, 1,"W_2", "WSTAT"), 1e-5 );
    BOOST_CHECK_CLOSE( WStat::numeric::PROD, ecl_sum_get_well_var(resp, 2,"W_2", "WSTAT"), 1e-5 );

    /* Production rates */
    BOOST_CHECK_CLOSE(  0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE(  0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WTPRSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE(  0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WOPR" ), 1e-5 );

    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WGPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WGVPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WLPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WNPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WGPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WGPRF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WVPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WOPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WOPRF" ), 1e-5 );

    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WPIW" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WPIO" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WPIG" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WPI"  ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WPIL" ), 1.0e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WWPGR"), 1.0e-5);

    /* Production totals */
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WTPTSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WNPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WLPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WOPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WVPT" ), 1e-5 );

    BOOST_CHECK_CLOSE( 20.0, ecl_sum_get_well_var( resp, 2, "W_2", "WWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.1, ecl_sum_get_well_var( resp, 2, "W_2", "WOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2, ecl_sum_get_well_var( resp, 2, "W_2", "WGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.0 + 20.1, ecl_sum_get_well_var( resp, 2, "W_2", "WLPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.0 + 20.1, ecl_sum_get_well_var( resp, 2, "W_2", "WLPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.3, ecl_sum_get_well_var( resp, 2, "W_2", "WNPT" ), 1e-5 );

    BOOST_CHECK_CLOSE( 20.4, ecl_sum_get_well_var( resp, 2, "W_2", "WGPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2 - 20.4, ecl_sum_get_well_var( resp, 2, "W_2", "WGPTF" ), 1e-5 );

    BOOST_CHECK_CLOSE( 20.5, ecl_sum_get_well_var( resp, 2, "W_2", "WOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.1 - 20.5, ecl_sum_get_well_var( resp, 2, "W_2", "WOPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( (20.6 + 20.7 + 20.8),
                       ecl_sum_get_well_var( resp, 2, "W_2", "WVPT" ), 1e-5 );

    /* Production rates (history) */
    BOOST_CHECK_CLOSE( 20.0, ecl_sum_get_well_var( resp, 1, "W_2", "WWPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.1, ecl_sum_get_well_var( resp, 1, "W_2", "WOPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2, ecl_sum_get_well_var( resp, 1, "W_2", "WGPRH" ), 1e-5 );

    /* Production totals (history) */
    BOOST_CHECK_CLOSE( 2 * 20.0, ecl_sum_get_well_var( resp, 2, "W_2", "WWPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.1, ecl_sum_get_well_var( resp, 2, "W_2", "WOPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 20.2, ecl_sum_get_well_var( resp, 2, "W_2", "WGPTH" ), 1e-5 );

    /* WWCT - water cut */
    const double wwcut = 20.0 / ( 20.0 + 20.1 );

    BOOST_CHECK_CLOSE( wwcut, ecl_sum_get_well_var( resp, 0, "W_2", "WWCT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0  , ecl_sum_get_well_var( resp, 1, "W_2", "WWCT" ), 1e-5 );
    BOOST_CHECK_CLOSE( wwcut, ecl_sum_get_well_var( resp, 2, "W_2", "WWCT" ), 1e-5 );

    /* gas-oil ratio */
    const double wgor = 20.2 / 20.1;

    BOOST_CHECK_CLOSE( wgor, ecl_sum_get_well_var( resp, 0, "W_2", "WGOR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0 , ecl_sum_get_well_var( resp, 1, "W_2", "WGOR" ), 1e-5 );
    BOOST_CHECK_CLOSE( wgor, ecl_sum_get_well_var( resp, 2, "W_2", "WGOR" ), 1e-5 );

    /* WGLR - gas-liquid rate */
    const double wglr = 20.2 / ( 20.0 + 20.1 );

    BOOST_CHECK_CLOSE( wglr, ecl_sum_get_well_var( resp, 0, "W_2", "WGLR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0 , ecl_sum_get_well_var( resp, 1, "W_2", "WGLR" ), 1e-5 );
    BOOST_CHECK_CLOSE( wglr, ecl_sum_get_well_var( resp, 2, "W_2", "WGLR" ), 1e-5 );

    /* BHP */
    BOOST_CHECK_CLOSE( 1.1, ecl_sum_get_well_var( resp, 0, "W_2", "WBHP" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WBHP" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.1, ecl_sum_get_well_var( resp, 2, "W_2", "WBHP" ), 1e-5 );

    /* THP */
    BOOST_CHECK_CLOSE( 1.2, ecl_sum_get_well_var( resp, 0, "W_2", "WTHP" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0.0, ecl_sum_get_well_var( resp, 1, "W_2", "WTHP" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.2, ecl_sum_get_well_var( resp, 2, "W_2", "WTHP" ), 1e-5 );

    /* BHP (history) */
    BOOST_CHECK_CLOSE( 1.1, ecl_sum_get_well_var( resp, 0, "W_2", "WBHPH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.1, ecl_sum_get_well_var( resp, 1, "W_2", "WBHPH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.1, ecl_sum_get_well_var( resp, 2, "W_2", "WBHPH" ), 1e-5 );

    /* THP (history) */
    BOOST_CHECK_CLOSE( 1.2, ecl_sum_get_well_var( resp, 0, "W_2", "WTHPH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.2, ecl_sum_get_well_var( resp, 1, "W_2", "WTHPH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.2, ecl_sum_get_well_var( resp, 2, "W_2", "WTHPH" ), 1e-5 );

    // Dump summary object as RSM file, load the new RSM file and compare.
    {
        std::string rsm_file = "TEST.RSM";
        std::filesystem::path rsm_path{rsm_file};
        resp->write_rsm_file(rsm_path);

        Opm::EclIO::ERsm rsm(rsm_file);
        BOOST_CHECK(cmp(*resp, rsm));
    }
}

BOOST_AUTO_TEST_CASE(udq_keywords) {
    setup cfg( "test_summary_udq" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule , cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);
    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    const auto& udq_params = cfg.es.runspec().udqParams();
    BOOST_CHECK_CLOSE( ecl_sum_get_well_var(resp, 1, "W_1", "WUBHP"), udq_params.undefinedValue(), 1e-5 );
    BOOST_CHECK_CLOSE( ecl_sum_get_well_var(resp, 1, "W_3", "WUBHP"), udq_params.undefinedValue(), 1e-5 );

#if 0
    BOOST_CHECK_EQUAL( std::string(ecl_sum_get_unit(resp, "WUBHP:W_1")), "BARSA");
#endif
}

BOOST_AUTO_TEST_CASE(group_keywords) {
    setup cfg( "test_summary_group" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);

    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);

    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);

    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    /* Production rates */
    BOOST_CHECK_CLOSE( 10.0 + 20.0, ecl_sum_get_group_var( resp, 1, "G_1", "GWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 + 20.1, ecl_sum_get_group_var( resp, 1, "G_1", "GOPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 + 20.2, ecl_sum_get_group_var( resp, 1, "G_1", "GGPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.3 + 20.3, ecl_sum_get_group_var( resp, 1, "G_1", "GNPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.4 + 20.4, ecl_sum_get_group_var( resp, 1, "G_1", "GGPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE((10.2 - 10.4) + (20.2 - 20.4),
                                    ecl_sum_get_group_var( resp, 1, "G_1", "GGPRF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.5 + 20.5, ecl_sum_get_group_var( resp, 1, "G_1", "GOPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE((10.1 - 10.5) + (20.1 - 20.5),
                                    ecl_sum_get_group_var( resp, 1, "G_1", "GOPRF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.6 + 10.7 + 10.8 + 20.6 + 20.7 + 20.8,
                                    ecl_sum_get_group_var( resp, 1, "G_1", "GVPR" ), 1e-5 );

    BOOST_CHECK_CLOSE( -10.13 - 20.13, ecl_sum_get_group_var( resp, 1, "G_1", "GWPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -10.14 - 20.14, ecl_sum_get_group_var( resp, 1, "G_1", "GOPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -10.15 - 20.15, ecl_sum_get_group_var( resp, 1, "G_1", "GGPP" ), 1e-5 );
    BOOST_CHECK_CLOSE(  30.13 + 60.13, ecl_sum_get_group_var( resp, 1, "G_2", "GWPI" ), 1e-5 );
    BOOST_CHECK_CLOSE(  30.15 + 60.15, ecl_sum_get_group_var( resp, 1, "G_2", "GGPI" ), 1e-5 );

    BOOST_CHECK_CLOSE( 10.16 + 20.16, ecl_sum_get_group_var( resp, 1, "G_1", "GCPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.17 + 20.17, ecl_sum_get_group_var( resp, 1, "G_1", "GSPR" ), 1e-5 );

    BOOST_CHECK_CLOSE(1111.2222, ecl_sum_get_group_var(resp, 1, "G_1", "GOPGR"), 1.0e-5);
    BOOST_CHECK_CLOSE(2222.3333, ecl_sum_get_group_var(resp, 1, "G_1", "GGPGR"), 1.0e-5);
    BOOST_CHECK_CLOSE(3333.4444, ecl_sum_get_group_var(resp, 1, "G_1", "GWPGR"), 1.0e-5);
    BOOST_CHECK_CLOSE(4444.5555, ecl_sum_get_group_var(resp, 1, "G_1", "GVPGR"), 1.0e-5);

    BOOST_CHECK_CLOSE(9999.8888, ecl_sum_get_group_var(resp, 1, "G_1", "GGIGR"), 1.0e-5);
    BOOST_CHECK_CLOSE(8888.7777, ecl_sum_get_group_var(resp, 1, "G_1", "GWIGR"), 1.0e-5);

    /* Production totals */
    BOOST_CHECK_CLOSE( 10.0 + 20.0, ecl_sum_get_group_var( resp, 1, "G_1", "GWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 + 20.1, ecl_sum_get_group_var( resp, 1, "G_1", "GOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 + 20.2, ecl_sum_get_group_var( resp, 1, "G_1", "GGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.3 + 20.3, ecl_sum_get_group_var( resp, 1, "G_1", "GNPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.4 + 20.4, ecl_sum_get_group_var( resp, 1, "G_1", "GGPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.5 + 20.5, ecl_sum_get_group_var( resp, 1, "G_1", "GOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.1 - 10.5) + (20.1 - 20.5), ecl_sum_get_group_var( resp, 1, "G_1", "GOPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.2 - 10.4) + (20.2 - 20.4), ecl_sum_get_group_var( resp, 1, "G_1", "GGPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.6 + 10.7 + 10.8 + 20.6 + 20.7 + 20.8,
                                    ecl_sum_get_group_var( resp, 1, "G_1", "GVPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.16 + 20.16, ecl_sum_get_group_var( resp, 1, "G_1", "GCPT" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * (10.0 + 20.0), ecl_sum_get_group_var( resp, 2, "G_1", "GWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * (10.1 + 20.1), ecl_sum_get_group_var( resp, 2, "G_1", "GOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * (10.2 + 20.2), ecl_sum_get_group_var( resp, 2, "G_1", "GGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * (10.3 + 20.3), ecl_sum_get_group_var( resp, 2, "G_1", "GNPT" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * (10.4 + 20.4), ecl_sum_get_group_var( resp, 2, "G_1", "GGPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * (10.5 + 20.5), ecl_sum_get_group_var( resp, 2, "G_1", "GOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * ((10.2 - 10.4) + (20.2 - 20.4)), ecl_sum_get_group_var( resp, 2, "G_1", "GGPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * ((10.1 - 10.5) + (20.1 - 20.5)), ecl_sum_get_group_var( resp, 2, "G_1", "GOPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * (10.6 + 10.7 + 10.8 + 20.6 + 20.7 + 20.8),
                                    ecl_sum_get_group_var( resp, 2, "G_1", "GVPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.16 + 20.16), ecl_sum_get_group_var( resp, 2, "G_1", "GCPT" ), 1e-5 );
    /* Production rates (history) */
    BOOST_CHECK_CLOSE( 10.0 + 20.0, ecl_sum_get_group_var( resp, 1, "G_1", "GWPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 + 20.1, ecl_sum_get_group_var( resp, 1, "G_1", "GOPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 + 20.2, ecl_sum_get_group_var( resp, 1, "G_1", "GGPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.0 + 10.1 + 20.0 + 20.1,
                                    ecl_sum_get_group_var( resp, 1, "G_1", "GLPRH" ), 1e-5 );

    /* Production totals (history) */
    BOOST_CHECK_CLOSE( (10.0 + 20.0), ecl_sum_get_group_var( resp, 1, "G_1", "GWPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0,             ecl_sum_get_group_var( resp, 1, "G_2", "GWPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.1 + 20.1), ecl_sum_get_group_var( resp, 1, "G_1", "GOPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0,             ecl_sum_get_group_var( resp, 1, "G_2", "GOPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.2 + 20.2), ecl_sum_get_group_var( resp, 1, "G_1", "GGPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0,             ecl_sum_get_group_var( resp, 1, "G_2", "GGPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.0 + 20.0 + 10.1 + 20.1),
                                      ecl_sum_get_group_var( resp, 1, "G_1", "GLPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 0,             ecl_sum_get_group_var( resp, 1, "G_2", "GLPTH" ), 1e-5 );

    /* Production targets */
    BOOST_CHECK_CLOSE( 30.1 , ecl_sum_get_group_var( resp, 1, "G_3", "GVPRT" ), 1e-5 );

    /* Injection rates */
    BOOST_CHECK_CLOSE( 30.0 + 60.0, ecl_sum_get_group_var( resp, 1, "G_2", "GWIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.2 + 60.2, ecl_sum_get_group_var( resp, 1, "G_2", "GGIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.3 + 60.3, ecl_sum_get_group_var( resp, 1, "G_2", "GNIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.16 + 60.16, ecl_sum_get_group_var( resp, 1, "G_2", "GCIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.17 + 60.17, ecl_sum_get_group_var( resp, 1, "G_2", "GSIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( (30.6 + 30.7 + 30.8 + 60.6 + 60.7 + 60.8),
                       ecl_sum_get_group_var( resp, 1, "G_2", "GVIR" ), 1e-5 );

    /* Injection totals */
    BOOST_CHECK_CLOSE( 30.0 + 60.0, ecl_sum_get_group_var( resp, 1, "G_2", "GWIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.2 + 60.2, ecl_sum_get_group_var( resp, 1, "G_2", "GGIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.3 + 60.3, ecl_sum_get_group_var( resp, 1, "G_2", "GNIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.16 + 60.16, ecl_sum_get_group_var( resp, 1, "G_2", "GCIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( (30.6 + 30.7 + 30.8 + 60.6 + 60.7 + 60.8),
                       ecl_sum_get_group_var( resp, 1, "G_2", "GVIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.0 + 60.0), ecl_sum_get_group_var( resp, 2, "G_2", "GWIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.2 + 60.2), ecl_sum_get_group_var( resp, 2, "G_2", "GGIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.3 + 60.3), ecl_sum_get_group_var( resp, 2, "G_2", "GNIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.16 + 60.16), ecl_sum_get_group_var( resp, 2, "G_2", "GCIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.17 + 60.17), ecl_sum_get_group_var( resp, 2, "G_2", "GSIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.6 + 30.7 + 30.8 + 60.6 + 60.7 + 60.8),
                       ecl_sum_get_group_var( resp, 2, "G_2", "GVIT" ), 1e-5 );

    /* Injection totals (history) */
    BOOST_CHECK_CLOSE( 30.0, ecl_sum_get_group_var( resp, 1, "G_2", "GWITH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30000.,    ecl_sum_get_group_var( resp, 1, "G_2", "GGITH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 60.0, ecl_sum_get_group_var( resp, 2, "G_2", "GWITH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 60000.,    ecl_sum_get_group_var( resp, 2, "G_2", "GGITH" ), 1e-5 );

    /* gwct - water cut */
    const double gwcut1 = (10.0 + 20.0) / ( 10.0 + 10.1 + 20.0 + 20.1 );
    const double gwcut2 = 0;
    BOOST_CHECK_CLOSE( gwcut1, ecl_sum_get_group_var( resp, 1, "G_1", "GWCT" ), 1e-5 );
    BOOST_CHECK_CLOSE( gwcut2, ecl_sum_get_group_var( resp, 1, "G_2", "GWCT" ), 1e-5 );

    BOOST_CHECK_CLOSE( gwcut1, ecl_sum_get_group_var( resp, 1, "G_1", "GWCTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( gwcut2, ecl_sum_get_group_var( resp, 1, "G_2", "GWCTH" ), 1e-5 );

    /* ggor - gas-oil ratio */
    const double ggor1 = (10.2 + 20.2) / (10.1 + 20.1);
    const double ggor2 = 0;
    BOOST_CHECK_CLOSE( ggor1, ecl_sum_get_group_var( resp, 1, "G_1", "GGOR" ), 1e-5 );
    BOOST_CHECK_CLOSE( ggor2, ecl_sum_get_group_var( resp, 1, "G_2", "GGOR" ), 1e-5 );

    BOOST_CHECK_CLOSE( ggor1, ecl_sum_get_group_var( resp, 1, "G_1", "GGORH" ), 1e-5 );
    BOOST_CHECK_CLOSE( ggor2, ecl_sum_get_group_var( resp, 1, "G_2", "GGORH" ), 1e-5 );

    const double gglr1 = (10.2 + 20.2) / ( 10.0 + 10.1 + 20.0 + 20.1 );
    const double gglr2 = 0;
    BOOST_CHECK_CLOSE( gglr1, ecl_sum_get_group_var( resp, 1, "G_1", "GGLR" ), 1e-5 );
    BOOST_CHECK_CLOSE( gglr2, ecl_sum_get_group_var( resp, 1, "G_2", "GGLR" ), 1e-5 );

    BOOST_CHECK_CLOSE( gglr1, ecl_sum_get_group_var( resp, 1, "G_1", "GGLRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( gglr2, ecl_sum_get_group_var( resp, 1, "G_2", "GGLRH" ), 1e-5 );


    BOOST_CHECK_EQUAL( 0, ecl_sum_get_group_var( resp, 1, "G_1", "GMWIN" ) );
    BOOST_CHECK_EQUAL( 2, ecl_sum_get_group_var( resp, 1, "G_1", "GMWPR" ) );
    BOOST_CHECK_EQUAL( 2, ecl_sum_get_group_var( resp, 1, "G_2", "GMWIN" ) );
    BOOST_CHECK_EQUAL( 0, ecl_sum_get_group_var( resp, 1, "G_2", "GMWPR" ) );
}

BOOST_AUTO_TEST_CASE(group_group) {
    setup cfg( "test_summary_group_group" , "group_group.DATA");

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);
    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    /* Production rates */
    BOOST_CHECK_CLOSE( 10.0 , ecl_sum_get_well_var( resp, 1, "W_1", "WWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.0 , ecl_sum_get_group_var( resp, 1, "G_1", "GWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 , ecl_sum_get_well_var( resp, 1, "W_1", "WOPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 , ecl_sum_get_group_var( resp, 1, "G_1", "GOPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 , ecl_sum_get_well_var( resp, 1, "W_1", "WGPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 , ecl_sum_get_group_var( resp, 1, "G_1", "GGPR" ), 1e-5 );

    BOOST_CHECK_CLOSE( 20.0 , ecl_sum_get_well_var( resp, 1, "W_2", "WWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.0 , ecl_sum_get_group_var( resp, 1, "G_2", "GWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.1 , ecl_sum_get_well_var( resp, 1, "W_2", "WOPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.1 , ecl_sum_get_group_var( resp, 1, "G_2", "GOPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2 , ecl_sum_get_well_var( resp, 1, "W_2", "WGPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 20.2 , ecl_sum_get_group_var( resp, 1, "G_2", "GGPR" ), 1e-5 );


    // Production totals
    for (int step = 1; step <= 2; step++) {
        BOOST_CHECK( ecl_sum_get_group_var( resp , step , "G_1" , "GWPT" ) == ecl_sum_get_well_var( resp , step , "W_1" , "WWPT"));
        BOOST_CHECK( ecl_sum_get_group_var( resp , step , "G_1" , "GOPT" ) == ecl_sum_get_well_var( resp , step , "W_1" , "WOPT"));
        BOOST_CHECK( ecl_sum_get_group_var( resp , step , "G_1" , "GGPT" ) == ecl_sum_get_well_var( resp , step , "W_1" , "WGPT"));

        BOOST_CHECK( ecl_sum_get_group_var( resp , step , "G_2" , "GWPT" ) == ecl_sum_get_well_var( resp , step , "W_2" , "WWPT"));
        BOOST_CHECK( ecl_sum_get_group_var( resp , step , "G_2" , "GOPT" ) == ecl_sum_get_well_var( resp , step , "W_2" , "WOPT"));
        BOOST_CHECK( ecl_sum_get_group_var( resp , step , "G_2" , "GGPT" ) == ecl_sum_get_well_var( resp , step , "W_2" , "WGPT"));
    }

    for (const auto& gvar : {"GGPR", "GOPR", "GWPR"})
        BOOST_CHECK_CLOSE( ecl_sum_get_group_var( resp , 1 , "G" , gvar) ,
                           ecl_sum_get_group_var( resp , 1 , "G_1" , gvar) + ecl_sum_get_group_var( resp , 1 , "G_2" , gvar) , 1e-5);

    for (int step = 1; step <= 2; step++) {
        for (const auto& gvar : {"GGPT", "GOPT", "GWPT"})
            BOOST_CHECK_CLOSE( ecl_sum_get_group_var( resp , step , "G" , gvar) ,
                               ecl_sum_get_group_var( resp , step , "G_1" , gvar) + ecl_sum_get_group_var( resp , step , "G_2" , gvar) , 1e-5);
    }
}

namespace {
    data::Wells glir_alq_data()
    {
        auto wells = data::Wells{};

        using opt = data::Rates::opt;
        auto& b1h = wells["B-1H"];
        auto& b2h = wells["B-2H"];
        auto& b3h = wells["B-3H"];

        b1h.rates.set(opt::alq, 1234.56*unit::cubic(unit::meter)/unit::day);
        b2h.rates.set(opt::alq, 2345.67*unit::cubic(unit::meter)/unit::day);
        b3h.rates.set(opt::alq, 3456.78*unit::cubic(unit::meter)/unit::day);

        return wells;
    }
}

BOOST_AUTO_TEST_CASE(GLIR_and_ALQ)
{
    const auto deck  = Parser{}.parseFile("2_WLIFT_MODEL5_NOINC.DATA");
    const auto es    = EclipseState { deck };
    const auto sched = Schedule { deck, es, std::make_shared<Python>() };
    const auto cfg   = SummaryConfig { deck, sched, es.fieldProps(), es.aquifer() };
    const auto name  = "glir_and_alq";

    WorkArea ta{ "summary_test" };
    ta.makeSubDir(name);

    const auto wellData = glir_alq_data();

    auto st = SummaryState { TimeService::now() };
    auto writer = out::Summary{ es, cfg, es.getInputGrid(), sched, name };
    writer.eval(st, 0, 0*day, wellData, {}, {}, {}, {}, {});
    writer.add_timestep(st, 0, false);

    writer.eval(st, 1, 1*day, wellData, {}, {}, {}, {}, {});
    writer.add_timestep(st, 1, false);

    writer.eval(st, 2, 2*day, wellData, {}, {}, {}, {}, {});
    writer.add_timestep(st, 2, false);
    writer.write();

    auto res = readsum(name);
    const auto* resp = res.get();

    BOOST_CHECK_CLOSE(1234.56, ecl_sum_get_well_var(resp, 1, "B-1H", "WGLIR"), 1.0e-5);
    BOOST_CHECK_CLOSE(2345.67, ecl_sum_get_well_var(resp, 1, "B-2H", "WGLIR"), 1.0e-5);
    BOOST_CHECK_CLOSE(3456.78, ecl_sum_get_well_var(resp, 1, "B-3H", "WGLIR"), 1.0e-5);

    BOOST_CHECK_CLOSE(1234.56 + 2345.67 + 3456.78,
                      ecl_sum_get_group_var(resp, 1, "B1", "GGLIR"), 1.0e-5);

    BOOST_CHECK_EQUAL(ecl_sum_get_well_var(resp, 1, "B-1H", "WGLIR"), ecl_sum_get_well_var(resp, 1, "B-1H", "WALQ"));
    BOOST_CHECK_EQUAL(ecl_sum_get_well_var(resp, 1, "B-2H", "WGLIR"), ecl_sum_get_well_var(resp, 1, "B-2H", "WALQ"));
    BOOST_CHECK_EQUAL(ecl_sum_get_well_var(resp, 1, "B-3H", "WGLIR"), ecl_sum_get_well_var(resp, 1, "B-3H", "WALQ"));
}

BOOST_AUTO_TEST_CASE(connection_kewords) {
    setup cfg( "test_summary_connection" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);
    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    /* Production rates */
    BOOST_CHECK_CLOSE( 100.0,     ecl_sum_get_well_connection_var( resp, 1, "W_1", "CWPR", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 100.1,     ecl_sum_get_well_connection_var( resp, 1, "W_1", "COPR", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 100.2,     ecl_sum_get_well_connection_var( resp, 1, "W_1", "CGPR", 1, 1, 1 ), 1e-5 );

    BOOST_CHECK_CLOSE( 1.9,       ecl_sum_get_well_connection_var( resp, 1, "W_1", "CPR",  1, 1, 1), 1e-5);

    BOOST_CHECK_MESSAGE(! ecl_sum_has_well_connection_var( resp, "W_1", "CVPR", 1, 1, 1 ),
                        "Summary vector CVPR must NOT exist for connection 1,1,1 of well W_1");

    BOOST_CHECK_CLOSE(  23.4,     ecl_sum_get_well_connection_var( resp, 1, "W_2", "CVPR", 2, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 234.5,     ecl_sum_get_well_connection_var( resp, 2, "W_2", "CVPR", 2, 1, 2 ), 1e-5 );
    BOOST_CHECK_CLOSE(   0.0,     ecl_sum_get_well_connection_var( resp, 1, "W_3", "CVPR", 3, 1, 1 ), 1e-5 );

    BOOST_CHECK_CLOSE(ecl_sum_get_well_var(resp, 1, "W_1", "WOPRL__1"), ecl_sum_get_well_connection_var(resp, 1, "W_1", "COPR", 1,1,1), 1e-5);
    BOOST_CHECK_CLOSE(ecl_sum_get_well_var(resp, 1, "W_2", "WOPRL__2"), ecl_sum_get_well_connection_var(resp, 1, "W_2", "COPR", 2,1,1) +
                                                                        ecl_sum_get_well_connection_var(resp, 1, "W_2", "COPR", 2,1,2), 1e-5);
    BOOST_CHECK_CLOSE(ecl_sum_get_well_var(resp, 1, "W_3", "WOPRL__3"), ecl_sum_get_well_connection_var(resp, 1, "W_3", "COPR", 3,1,1), 1e-5);
    BOOST_CHECK_EQUAL(ecl_sum_get_well_var(resp, 1, "W_2", "WOPRL__2"), ecl_sum_get_well_var(resp, 1, "W_2", "WOFRL__2"));

    BOOST_CHECK_CLOSE(ecl_sum_get_well_var(resp, 1, "W_1", "WOPRL__1"), ecl_sum_get_well_connection_var(resp, 1, "W_1", "COPRL", 1,1,1), 1e-5);
    BOOST_CHECK_CLOSE(ecl_sum_get_well_connection_var(resp, 1, "W_2", "COPRL", 2, 1, 1), ecl_sum_get_well_connection_var(resp, 1, "W_2", "COPR", 2,1,1) +
                                                                                         ecl_sum_get_well_connection_var(resp, 1, "W_2", "COPR", 2,1,2), 1e-5);
    BOOST_CHECK_CLOSE(ecl_sum_get_well_connection_var(resp, 1, "W_2", "COPRL", 2, 1, 2), ecl_sum_get_well_connection_var(resp, 1, "W_2", "COPR", 2,1,1) +
                                                                                         ecl_sum_get_well_connection_var(resp, 1, "W_2", "COPR", 2,1,2), 1e-5);

    // Flow ratios
    BOOST_CHECK_CLOSE( 100.2 / 100.1, ecl_sum_get_well_connection_var( resp, 1, "W_1", "CGOR", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE(   0.0,         ecl_sum_get_well_connection_var( resp, 1, "W_6", "CGOR", 8, 8, 1 ), 1e-5 );

    /* Production totals */
    BOOST_CHECK_CLOSE( 100.0,     ecl_sum_get_well_connection_var( resp, 1, "W_1", "CWPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 100.1,     ecl_sum_get_well_connection_var( resp, 1, "W_1", "COPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 100.2,     ecl_sum_get_well_connection_var( resp, 1, "W_1", "CGPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 100.3,     ecl_sum_get_well_connection_var( resp, 1, "W_1", "CNPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 100.0, ecl_sum_get_well_connection_var( resp, 2, "W_1", "CWPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 100.1, ecl_sum_get_well_connection_var( resp, 2, "W_1", "COPT", 1, 1, 1 ), 1e-5 );

    BOOST_CHECK_CLOSE( 2 * 100.2, ecl_sum_get_well_connection_var( resp, 2, "W_1", "CGPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 200.2, ecl_sum_get_well_connection_var( resp, 2, "W_2", "CGPT", 2, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 0        , ecl_sum_get_well_connection_var( resp, 2, "W_3", "CGPT", 3, 1, 1 ), 1e-5 );

    BOOST_CHECK_CLOSE( 1 * 100.2, ecl_sum_get_well_connection_var( resp, 1, "W_1", "CGPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 1 * 200.2, ecl_sum_get_well_connection_var( resp, 1, "W_2", "CGPT", 2, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 0        , ecl_sum_get_well_connection_var( resp, 1, "W_3", "CGPT", 3, 1, 1 ), 1e-5 );

    BOOST_CHECK_CLOSE( 2 * 100.3, ecl_sum_get_well_connection_var( resp, 2, "W_1", "CNPT", 1, 1, 1 ), 1e-5 );

    BOOST_CHECK_CLOSE( 1.0 * 123.4,     ecl_sum_get_well_connection_var( resp, 1, "W_1", "CVPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.0 *  23.4,     ecl_sum_get_well_connection_var( resp, 1, "W_2", "CVPT", 2, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.0 * 234.5,     ecl_sum_get_well_connection_var( resp, 1, "W_2", "CVPT", 2, 1, 2 ), 1e-5 );

    BOOST_CHECK_MESSAGE(! ecl_sum_has_well_connection_var( resp, "W_3", "CVPT", 3, 1, 1 ),
                        "Summary vector CVPT must NOT exist for connection 3,1,1 of well W_3");

    BOOST_CHECK_CLOSE( 2.0 * 123.4,     ecl_sum_get_well_connection_var( resp, 2, "W_1", "CVPT", 1, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2.0 *  23.4,     ecl_sum_get_well_connection_var( resp, 2, "W_2", "CVPT", 2, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2.0 * 234.5,     ecl_sum_get_well_connection_var( resp, 2, "W_2", "CVPT", 2, 1, 2 ), 1e-5 );

    /* Injection rates */
    BOOST_CHECK_CLOSE( 300.0,       ecl_sum_get_well_connection_var( resp, 1, "W_3", "CWIR", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 300.2,       ecl_sum_get_well_connection_var( resp, 1, "W_3", "CGIR", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 300.16, ecl_sum_get_well_connection_var( resp, 1, "W_3", "CCIR", 3, 1, 1 ), 1e-5 );

    BOOST_CHECK_CLOSE(   0.0,     ecl_sum_get_well_connection_var( resp, 1, "W_2", "CVIR", 2, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE(   0.0,     ecl_sum_get_well_connection_var( resp, 2, "W_2", "CVIR", 2, 1, 2 ), 1e-5 );
    BOOST_CHECK_CLOSE( 432.1,     ecl_sum_get_well_connection_var( resp, 1, "W_3", "CVIR", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 321.09,    ecl_sum_get_well_connection_var( resp, 1, "W_6", "CVIR", 8, 8, 1 ), 1e-5 );

    /* Injection totals */
    BOOST_CHECK_CLOSE( 300.0,       ecl_sum_get_well_connection_var( resp, 1, "W_3", "CWIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 300.2,       ecl_sum_get_well_connection_var( resp, 1, "W_3", "CGIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 300.3,       ecl_sum_get_well_connection_var( resp, 1, "W_3", "CNIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 300.16, ecl_sum_get_well_connection_var( resp, 1, "W_3", "CCIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 300.0,   ecl_sum_get_well_connection_var( resp, 2, "W_3", "CWIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 300.2,   ecl_sum_get_well_connection_var( resp, 2, "W_3", "CGIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 300.3,   ecl_sum_get_well_connection_var( resp, 2, "W_3", "CNIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * 300.16,
                                    ecl_sum_get_well_connection_var( resp, 2, "W_3", "CCIT", 3, 1, 1 ), 1e-5 );

    BOOST_CHECK_CLOSE( 1.0 * 432.1,  ecl_sum_get_well_connection_var( resp, 1, "W_3", "CVIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 1.0 * 321.09, ecl_sum_get_well_connection_var( resp, 1, "W_6", "CVIT", 8, 8, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2.0 * 432.1,  ecl_sum_get_well_connection_var( resp, 2, "W_3", "CVIT", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE( 2.0 * 321.09, ecl_sum_get_well_connection_var( resp, 2, "W_6", "CVIT", 8, 8, 1 ), 1e-5 );

    /* Solvent flow rate + or - Note OPM uses negative values for producers, while CNFR outputs positive
    values for producers*/
    BOOST_CHECK_CLOSE( -300.3,     ecl_sum_get_well_connection_var( resp, 1, "W_3", "CNFR", 3, 1, 1 ), 1e-5 );
    BOOST_CHECK_CLOSE(  200.3,     ecl_sum_get_well_connection_var( resp, 1, "W_2", "CNFR", 2, 1, 1 ), 1e-5 );
}

BOOST_AUTO_TEST_CASE(DATE) {
    setup cfg( "test_summary_DATE" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.eval( st, 3, 18 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 3, false);
    writer.eval( st, 4, 22 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 4, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    const auto& days = resp->get_at_rstep("DAY");
    BOOST_CHECK_EQUAL(days[0], 11);
    BOOST_CHECK_EQUAL(days[1], 12);
    BOOST_CHECK_EQUAL(days[2], 28);
    BOOST_CHECK_EQUAL(days[3],  1);

    const auto& month = resp->get_at_rstep("MONTH");
    BOOST_CHECK_EQUAL(month[0], 5);
    BOOST_CHECK_EQUAL(month[1], 5);
    BOOST_CHECK_EQUAL(month[2], 5);
    BOOST_CHECK_EQUAL(month[3], 6);

    const auto& year = resp->get_at_rstep("YEAR");
    BOOST_CHECK_EQUAL(year[0], 2007);
    BOOST_CHECK_EQUAL(year[1], 2007);
    BOOST_CHECK_EQUAL(year[2], 2007);
    BOOST_CHECK_EQUAL(year[3], 2007);
}

BOOST_AUTO_TEST_CASE(field_keywords) {
    setup cfg( "test_summary_field" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);
    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    /* Production rates */
    BOOST_CHECK_CLOSE( 10.0 + 20.0, ecl_sum_get_field_var( resp, 1, "FWPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.18 + 20.18, ecl_sum_get_field_var( resp, 1, "FTPRSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 + 20.1, ecl_sum_get_field_var( resp, 1, "FOPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 + 20.2, ecl_sum_get_field_var( resp, 1, "FGPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.0 + 20.0 + 10.1 + 20.1,
                                    ecl_sum_get_field_var( resp, 1, "FLPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.6 + 10.7 + 10.8 + 20.6 + 20.7 + 20.8,
                                    ecl_sum_get_field_var( resp, 1, "FVPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.4 + 20.4,
                                    ecl_sum_get_field_var( resp, 1, "FGPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 - 10.4 + 20.2 - 20.4,
                                    ecl_sum_get_field_var( resp, 1, "FGPRF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.5 + 20.5,
                                    ecl_sum_get_field_var( resp, 1, "FOPRS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 - 10.5 + 20.1 - 20.5,
                                    ecl_sum_get_field_var( resp, 1, "FOPRF" ), 1e-5 );

    BOOST_CHECK_CLOSE( -10.13 - 20.13, ecl_sum_get_field_var( resp, 1, "FWPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -10.14 - 20.14, ecl_sum_get_field_var( resp, 1, "FOPP" ), 1e-5 );
    BOOST_CHECK_CLOSE( -10.15 - 20.15, ecl_sum_get_field_var( resp, 1, "FGPP" ), 1e-5 );
    BOOST_CHECK_CLOSE(  30.15 + 60.15, ecl_sum_get_field_var( resp, 1, "FGPI" ), 1e-5 );
    BOOST_CHECK_CLOSE(  30.13 + 60.13, ecl_sum_get_field_var( resp, 1, "FWPI" ), 1e-5 );

    BOOST_CHECK_CLOSE(  10.16 + 20.16, ecl_sum_get_field_var( resp, 1, "FCPR" ), 1e-5 );
    BOOST_CHECK_CLOSE(  10.17 + 20.17, ecl_sum_get_field_var( resp, 1, "FSPR" ), 1e-5 );

    /* Production totals */
    BOOST_CHECK_CLOSE( 10.0 + 20.0, ecl_sum_get_field_var( resp, 1, "FWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.18 + 20.18, ecl_sum_get_field_var( resp, 1, "FTPTSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 + 20.1, ecl_sum_get_field_var( resp, 1, "FOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 + 20.2, ecl_sum_get_field_var( resp, 1, "FGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.0 + 20.0 + 10.1 + 20.1,
                                    ecl_sum_get_field_var( resp, 1, "FLPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.6 + 10.7 + 10.8 + 20.6 + 20.7 + 20.8,
                                    ecl_sum_get_field_var( resp, 1, "FVPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.4 + 20.4,
                                    ecl_sum_get_field_var( resp, 1, "FGPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 - 10.4 + 20.2 - 20.4,
                                    ecl_sum_get_field_var( resp, 1, "FGPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.5 + 20.5,
                                    ecl_sum_get_field_var( resp, 1, "FOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 - 10.5 + 20.1 - 20.5,
                                    ecl_sum_get_field_var( resp, 1, "FOPTF" ), 1e-5 );

    BOOST_CHECK_CLOSE( 2 * (10.0 + 20.0), ecl_sum_get_field_var( resp, 2, "FWPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.18 + 20.18), ecl_sum_get_field_var( resp, 2, "FTPTSEA" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.1 + 20.1), ecl_sum_get_field_var( resp, 2, "FOPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.2 + 20.2), ecl_sum_get_field_var( resp, 2, "FGPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.0 + 20.0 + 10.1 + 20.1),
                                    ecl_sum_get_field_var( resp, 2, "FLPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.6 + 10.7 + 10.8 + 20.6 + 20.7 + 20.8),
                                    ecl_sum_get_field_var( resp, 2, "FVPT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.4 + 20.4),
                                    ecl_sum_get_field_var( resp, 2, "FGPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.2 - 10.4 + 20.2 - 20.4),
                                    ecl_sum_get_field_var( resp, 2, "FGPTF" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.5 + 20.5),
                                    ecl_sum_get_field_var( resp, 2, "FOPTS" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.1 - 10.5 + 20.1 - 20.5),
                                    ecl_sum_get_field_var( resp, 2, "FOPTF" ), 1e-5 );

    BOOST_CHECK_CLOSE(  2 * (10.16 + 20.16), ecl_sum_get_field_var( resp, 2, "FCPT" ), 1e-5 );
    BOOST_CHECK_CLOSE(  2 * (10.17 + 20.17), ecl_sum_get_field_var( resp, 2, "FSPT" ), 1e-5 );

    /* Production rates (history) */
    BOOST_CHECK_CLOSE( 10.0 + 20.0, ecl_sum_get_field_var( resp, 1, "FWPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.1 + 20.1, ecl_sum_get_field_var( resp, 1, "FOPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.2 + 20.2, ecl_sum_get_field_var( resp, 1, "FGPRH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 10.0 + 10.1 + 20.0 + 20.1,
                                    ecl_sum_get_field_var( resp, 1, "FLPRH" ), 1e-5 );

    /* Production totals (history) */
    BOOST_CHECK_CLOSE( (10.0 + 20.0), ecl_sum_get_field_var( resp, 1, "FWPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.1 + 20.1), ecl_sum_get_field_var( resp, 1, "FOPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.2 + 20.2), ecl_sum_get_field_var( resp, 1, "FGPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( (10.0 + 20.0 + 10.1 + 20.1),
                                      ecl_sum_get_field_var( resp, 1, "FLPTH" ), 1e-5 );

    BOOST_CHECK_CLOSE( 2 * (10.0 + 20.0), ecl_sum_get_field_var( resp, 2, "FWPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.1 + 20.1), ecl_sum_get_field_var( resp, 2, "FOPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.2 + 20.2), ecl_sum_get_field_var( resp, 2, "FGPTH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (10.0 + 20.0 + 10.1 + 20.1),
                                          ecl_sum_get_field_var( resp, 2, "FLPTH" ), 1e-5 );

    /* Injection rates */
    BOOST_CHECK_CLOSE( 30.0 + 60., ecl_sum_get_field_var( resp, 1, "FWIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.2 + 60.2, ecl_sum_get_field_var( resp, 1, "FGIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.6 + 30.7 + 30.8 + 60.6 + 60.7 + 60.8, ecl_sum_get_field_var( resp, 1, "FVIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.16 + 60.16, ecl_sum_get_field_var( resp, 1, "FCIR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.17 + 60.17, ecl_sum_get_field_var( resp, 1, "FSIR" ), 1e-5 );

    /* Injection totals */
    BOOST_CHECK_CLOSE( 30.0 + 60.,     ecl_sum_get_field_var( resp, 1, "FWIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.2 + 60.2,    ecl_sum_get_field_var( resp, 1, "FGIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.6 + 30.7 + 30.8 + 60.6 + 60.7 + 60.8, ecl_sum_get_field_var( resp, 1, "FVIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 30.16 + 60.16,  ecl_sum_get_field_var( resp, 1, "FCIT" ), 1e-5 );

    BOOST_CHECK_CLOSE( 2 * (30.0 + 60.0), ecl_sum_get_field_var( resp, 2, "FWIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.2 + 60.2), ecl_sum_get_field_var( resp, 2, "FGIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.6 + 30.7 + 30.8 + 60.6 + 60.7 + 60.8), ecl_sum_get_field_var( resp, 2, "FVIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.16 + 60.16),  ecl_sum_get_field_var( resp, 2, "FCIT" ), 1e-5 );
    BOOST_CHECK_CLOSE( 2 * (30.17 + 60.17),  ecl_sum_get_field_var( resp, 2, "FSIT" ), 1e-5 );

    /* Injection totals (history) */
    BOOST_CHECK_CLOSE( 30.0, ecl_sum_get_field_var( resp, 1, "FWITH" ), 1e-5 );
    BOOST_CHECK_CLOSE( 60.0, ecl_sum_get_field_var( resp, 2, "FWITH" ), 1e-5 );

    /* Production targets */
    BOOST_CHECK_CLOSE( 30.1 , ecl_sum_get_field_var( resp, 1, "FVPRT" ), 1e-5 );

    /* fwct - water cut */
    const double wcut = (10.0 + 20.0) / ( 10.0 + 10.1 + 20.0 + 20.1 );
    BOOST_CHECK_CLOSE( wcut, ecl_sum_get_field_var( resp, 1, "FWCT" ), 1e-5 );
    BOOST_CHECK_CLOSE( wcut, ecl_sum_get_field_var( resp, 1, "FWCTH" ), 1e-5 );

    /* ggor - gas-oil ratio */
    const double ggor = (10.2 + 20.2) / (10.1 + 20.1);
    BOOST_CHECK_CLOSE( ggor, ecl_sum_get_field_var( resp, 1, "FGOR" ), 1e-5 );
    BOOST_CHECK_CLOSE( ggor, ecl_sum_get_field_var( resp, 1, "FGORH" ), 1e-5 );

}

#if 0
BOOST_AUTO_TEST_CASE(report_steps_time) {
    setup cfg( "test_summary_report_steps_time" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 1, 2 *  day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 1, 5 *  day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 10 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    BOOST_CHECK( ecl_sum_has_report_step( resp, 1 ) );
    BOOST_CHECK( ecl_sum_has_report_step( resp, 2 ) );
    BOOST_CHECK( !ecl_sum_has_report_step( resp, 3 ) );

    BOOST_CHECK_EQUAL( ecl_sum_iget_sim_days( resp, 0 ), 2 );
    BOOST_CHECK_EQUAL( ecl_sum_iget_sim_days( resp, 1 ), 5 );
    BOOST_CHECK_EQUAL( ecl_sum_iget_sim_days( resp, 2 ), 10 );
    BOOST_CHECK_EQUAL( ecl_sum_get_sim_length( resp ), 10 );
}
#endif

BOOST_AUTO_TEST_CASE(skip_unknown_var) {
    setup cfg( "test_summary_skip_unknown_var" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 1, 2 *  day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 1, 5 *  day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 10 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    /* verify that some non-supported keywords aren't written to the file */
    BOOST_CHECK( !ecl_sum_has_field_var( resp, "FGST" ) );
}



BOOST_AUTO_TEST_CASE(region_vars) {
    setup cfg( "region_vars" );

    std::map<std::string, std::vector<double>> region_values;

    {
        std::vector<double> values(10, 0.0);
        for (size_t r=1; r <= 10; r++) {
            values[r - 1] = r *1.0;
        }
        region_values["RPR"] = values;
    }
    {
        double area = cfg.grid.getNX() * cfg.grid.getNY();
        std::vector<double> values(10, 0.0);
        for (size_t r=1; r <= 10; r++) {
            if (r == 10)
                area -= 1;

            values[r - 1] = area *  2*r * 1.0;
        }
        region_values["ROIP"] = values;
    }
    {
        double area = cfg.grid.getNX() * cfg.grid.getNY();
        std::vector<double> values(10, 0.0);
        for (size_t r=1; r <= 10; r++) {
            if (r == 10)
                area -= 1;

            values[r - 1] = area * 2.2*r * 1.0;
        }
        region_values["RWIP"] = values;
    }
    {
        double area = cfg.grid.getNX() * cfg.grid.getNY();
        std::vector<double> values(10, 0.0);
        for (size_t r=1; r <= 10; r++) {
            if (r == 10)
                area -= 1;

            values[r - 1] = area *   2.1*r * 1.0;
        }
        region_values["RGIP"] = values;
    }
    {
        double area = cfg.grid.getNX() * cfg.grid.getNY();
        std::vector<double> values(10, 0.0);
        for (size_t r=1; r <= 10; r++) {
            if (r == 10)
                area -= 1;

            values[r - 1] = area *  (2*r - 1) * 1.0;
        }
        region_values["ROIPL"] = values;
    }
    {
        double area = cfg.grid.getNX() * cfg.grid.getNY();
        std::vector<double> values(10, 0.0);
        for (size_t r=1; r <= 10; r++) {
            if (r == 10)
                area -= 1;

            values[r - 1] = area *  (2*r + 1 ) * 1.0;
        }
        region_values["ROIPG"] = values;
    }
    {
        double area = cfg.grid.getNX() * cfg.grid.getNY();
        std::vector<double> values(10, 0.0);
        for (size_t r=1; r <= 10; r++) {
            if (r == 10)
                area -= 1;

            values[r - 1] = area *  (2.1*r - 1) * 1.0;
        }
        region_values["RGIPL"] = values;
    }
    {
        double area = cfg.grid.getNX() * cfg.grid.getNY();
        std::vector<double> values(10, 0.0);
        for (size_t r=1; r <= 10; r++) {
            if (r == 10)
                area -= 1;

            values[r - 1] = area *  (2.1*r + 1) * 1.0;
        }
        region_values["RGIPG"] = values;
    }

    {
        out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
        SummaryState st(TimeService::now());
        writer.eval( st, 1, 2 *  day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {}, region_values);
        writer.add_timestep( st, 1, false);
        writer.eval( st, 1, 5 *  day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {}, region_values);
        writer.add_timestep( st, 1, false);
        writer.eval( st, 2, 10 * day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {}, region_values);
        writer.add_timestep( st, 2, false);
        writer.write();
    }

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    BOOST_CHECK( ecl_sum_has_general_var( resp , "RPR:1"));
    BOOST_CHECK( ecl_sum_has_general_var( resp , "RPR:10"));
    BOOST_CHECK( !ecl_sum_has_general_var( resp , "RPR:21"));
    UnitSystem units( UnitSystem::UnitType::UNIT_TYPE_METRIC );

    for (size_t r=1; r <= 10; r++) {
        std::string rpr_key   = "RPR:"   + std::to_string( r );
        std::string roip_key  = "ROIP:"  + std::to_string( r );
        std::string rwip_key  = "RWIP:"  + std::to_string( r );
        std::string rgip_key  = "RGIP:"  + std::to_string( r );
        std::string roipl_key = "ROIPL:" + std::to_string( r );
        std::string roipg_key = "ROIPG:" + std::to_string( r );
        std::string rgipl_key = "RGIPL:" + std::to_string( r );
        std::string rgipg_key = "RGIPG:" + std::to_string( r );
        double area = cfg.grid.getNX() * cfg.grid.getNY();

        //BOOST_CHECK_CLOSE(   r * 1.0        , units.to_si( UnitSystem::measure::pressure , ecl_sum_get_general_var( resp, 1, rpr_key.c_str())) , 1e-5);

        // There is one inactive cell in the bottom layer.
        if (r == 10)
            area -= 1;

        BOOST_CHECK_CLOSE( area *  2*r * 1.0       , units.to_si( UnitSystem::measure::volume   , ecl_sum_get_general_var( resp, 1, roip_key.c_str())) , 1e-5);
        BOOST_CHECK_CLOSE( area * (2*r - 1) * 1.0  , units.to_si( UnitSystem::measure::volume   , ecl_sum_get_general_var( resp, 1, roipl_key.c_str())) , 1e-5);
        BOOST_CHECK_CLOSE( area * (2*r + 1 ) * 1.0 , units.to_si( UnitSystem::measure::volume   , ecl_sum_get_general_var( resp, 1, roipg_key.c_str())) , 1e-5);
        BOOST_CHECK_CLOSE( area *  2.1*r * 1.0     , units.to_si( UnitSystem::measure::volume   , ecl_sum_get_general_var( resp, 1, rgip_key.c_str())) , 1e-5);
        BOOST_CHECK_CLOSE( area * (2.1*r - 1) * 1.0, units.to_si( UnitSystem::measure::volume   , ecl_sum_get_general_var( resp, 1, rgipl_key.c_str())) , 1e-5);
        BOOST_CHECK_CLOSE( area * (2.1*r + 1) * 1.0, units.to_si( UnitSystem::measure::volume   , ecl_sum_get_general_var( resp, 1, rgipg_key.c_str())) , 1e-5);
        BOOST_CHECK_CLOSE( area *  2.2*r * 1.0     , units.to_si( UnitSystem::measure::volume   , ecl_sum_get_general_var( resp, 1, rwip_key.c_str())) , 1e-5);
    }
}


BOOST_AUTO_TEST_CASE(region_production) {
    setup cfg( "region_production" );

    {
        out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
        SummaryState st(TimeService::now());
        writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
        writer.add_timestep( st, 0, false);
        writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
        writer.add_timestep( st, 1, false);
        writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
        writer.add_timestep( st, 2, false);
        writer.write();
    }

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    BOOST_CHECK( ecl_sum_has_general_var( resp , "ROPR:1"));
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var( resp , 1 , "ROPR:1" ) ,
                      ecl_sum_get_general_var( resp , 1 , "COPR:W_1:1,1,1") +
                      ecl_sum_get_general_var( resp , 1 , "COPR:W_2:2,1,1") +
                      ecl_sum_get_general_var( resp , 1 , "COPR:W_3:3,1,1"), 1e-5);



    BOOST_CHECK( ecl_sum_has_general_var( resp , "RGPT:1"));
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var( resp , 2 , "RGPT:1" ) ,
                      ecl_sum_get_general_var( resp , 2 , "CGPT:W_1:1,1,1") +
                      ecl_sum_get_general_var( resp , 2 , "CGPT:W_2:2,1,1") +
                      ecl_sum_get_general_var( resp , 2 , "CGPT:W_3:3,1,1"), 1e-5);
}

BOOST_AUTO_TEST_CASE(region_injection) {
    setup cfg( "region_injection" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);
    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    BOOST_CHECK( ecl_sum_has_general_var( resp , "RWIR:1"));
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var( resp , 1 , "RWIR:1" ) ,
                      ecl_sum_get_general_var( resp , 1 , "CWIR:W_1:1,1,1") +
                      ecl_sum_get_general_var( resp , 1 , "CWIR:W_2:2,1,1") +
                      ecl_sum_get_general_var( resp , 1 , "CWIR:W_3:3,1,1"), 1e-5);



    BOOST_CHECK( ecl_sum_has_general_var( resp , "RGIT:1"));
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var( resp , 2 , "RGIT:1" ) ,
                      ecl_sum_get_general_var( resp , 2 , "CGIT:W_1:1,1,1") +
                      ecl_sum_get_general_var( resp , 2 , "CGIT:W_2:2,1,1") +
                      ecl_sum_get_general_var( resp , 2 , "CGIT:W_3:3,1,1"), 1e-5);
}

namespace {
    Opm::data::InterRegFlowMap::FlowRates ireg_flow_1_11()
    {
        using Component = Opm::data::InterRegFlowMap::Component;
        auto rates = Opm::data::InterRegFlowMap::FlowRates{};

        rates[Component::Oil] = 1.234f;
        rates[Component::Gas] = 23.45f;
        rates[Component::Water] = 0.543f;
        rates[Component::Disgas] = 20.45f;
        rates[Component::Vapoil] = 0.004f;

        return rates;
    }

    Opm::data::InterRegFlowMap::FlowRates ireg_flow_1_2()
    {
        using Component = Opm::data::InterRegFlowMap::Component;
        auto rates = Opm::data::InterRegFlowMap::FlowRates{};

        rates[Component::Oil] = 0.1234f;
        rates[Component::Gas] = -2.345f;
        rates[Component::Water] = 1.729f;
        rates[Component::Disgas] = -0.345f;
        rates[Component::Vapoil] = 0.0004f;

        return rates;
    }

    Opm::data::InterRegFlowMap::FlowRates ireg_flow_9_10()
    {
        using Component = Opm::data::InterRegFlowMap::Component;
        auto rates = Opm::data::InterRegFlowMap::FlowRates{};

        rates[Component::Oil] = -0.271828f;
        rates[Component::Gas] = 3.1415926f;
        rates[Component::Water] = 11.2233f;
        rates[Component::Disgas] = 3.0f;
        rates[Component::Vapoil] = 11.0f;

        return rates;
    }

    Opm::data::InterRegFlowMap::FlowRates ireg_flow_2_12()
    {
        using Component = Opm::data::InterRegFlowMap::Component;
        auto rates = Opm::data::InterRegFlowMap::FlowRates{};

        rates[Component::Oil] = 4.32f;
        rates[Component::Gas] = 10.98f;
        rates[Component::Water] = 54.321f;
        rates[Component::Disgas] = 7.65f;
        rates[Component::Vapoil] = 1.32f;

        return rates;
    }

    Opm::data::InterRegFlowMap::FlowRates ireg_flow_5_6()
    {
        using Component = Opm::data::InterRegFlowMap::Component;
        auto rates = Opm::data::InterRegFlowMap::FlowRates{};

        rates[Component::Oil] = 0.56f;
        rates[Component::Gas] = 6.5f;
        rates[Component::Water] = 0.065f;
        rates[Component::Disgas] = 5.6f;
        rates[Component::Vapoil] = 0.42f;

        return rates;
    }

    Opm::out::Summary::InterRegFlowValues interRegionFlows()
    {
        auto values = Opm::out::Summary::InterRegFlowValues{};

        auto& ireg = values["FIPNUM"];
        ireg.addConnection(0, 10, ireg_flow_1_11());
        ireg.addConnection(0,  1, ireg_flow_1_2 ());
        ireg.addConnection(8,  9, ireg_flow_9_10());
        ireg.addConnection(1, 11, ireg_flow_2_12());
        ireg.addConnection(4,  5, ireg_flow_5_6 ());

        ireg.compress(20);

        return values;
    }
}

BOOST_AUTO_TEST_CASE(inter_region_flows)
{
    const auto cfg = setup{ "inter_region_flows" };

    {
        auto st = SummaryState{ TimeService::now() };
        auto writer = out::Summary {
            cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name
        };

        const auto values = interRegionFlows();

        for (auto i = 0; i < 3; ++i) {
            writer.eval(st, i, i * day, cfg.wells, cfg.grp_nwrk,
                        {}, {}, {}, {}, {}, {}, {}, values);
            writer.add_timestep(st, 0, false);
        }

        writer.write();
    }

    const auto res = readsum(cfg.name);
    const auto* resp = res.get();

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "ROFT:1-11"),
                        "Summary data must have ROFT:1-11");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "ROFT:1-11"), 0 * 86400.0f * 1.234f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "ROFT:1-11"), 1 * 86400.0f * 1.234f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "ROFT:1-11"), 2 * 86400.0f * 1.234f, 1.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "ROFT:1-2"),
                        "Summary data must have ROFT:1-2");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "ROFT:1-2"), 0 * 86400.0f * 0.1234f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "ROFT:1-2"), 1 * 86400.0f * 0.1234f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "ROFT:1-2"), 2 * 86400.0f * 0.1234f, 1.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "ROFT:9-10"),
                        "Summary data must have ROFT:9-10");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "ROFT:9-10"), 0 * 86400.0f * (-0.271828f), 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "ROFT:9-10"), 1 * 86400.0f * (-0.271828f), 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "ROFT:9-10"), 2 * 86400.0f * (-0.271828f), 1.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "RWFR-:2-12"),
                        "Summary data must have RWFR-:2-12");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "RWFR-:2-12"), 0.0f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "RWFR-:2-12"), 0.0f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "RWFR-:2-12"), 0.0f, 1.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "RWFR+:2-12"),
                        "Summary data must have RWFR+:2-12");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "RWFR+:2-12"), 54.321f * 86400.0, 5.0e-6f); // SM3/s -> SM3/d
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "RWFR+:2-12"), 54.321f * 86400.0, 5.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "RWFR+:2-12"), 54.321f * 86400.0, 5.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "RGFTG:5-6"),
                        "Summary data must have RGFTG:5-6");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "RGFTG:5-6"), 0 * 86400.0f * 0.9f, 5.0e-5f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "RGFTG:5-6"), 1 * 86400.0f * 0.9f, 5.0e-5f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "RGFTG:5-6"), 2 * 86400.0f * 0.9f, 5.0e-5f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "RGFTG:1-20"),
                        "Summary data must have RGFTG:1-20");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "RGFTG:1-20"), 0.0f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "RGFTG:1-20"), 0.0f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "RGFTG:1-20"), 0.0f, 1.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "ROFTG:5-6"),
                        "Summary data must have ROFTG:5-6");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "ROFTG:5-6"), 0 * 86400.0f * 0.42f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "ROFTG:5-6"), 1 * 86400.0f * 0.42f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "ROFTG:5-6"), 2 * 86400.0f * 0.42f, 1.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "RGFTL:5-6"),
                        "Summary data must have RGFTL:5-6");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "RGFTL:5-6"), 0 * 86400.0f * 5.6f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "RGFTL:5-6"), 1 * 86400.0f * 5.6f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "RGFTL:5-6"), 2 * 86400.0f * 5.6f, 1.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "ROFTL:5-6"),
                        "Summary data must have ROFTL:5-6");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "ROFTL:5-6"), 0 * 86400.0f * 0.14f, 1.0e-5f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "ROFTL:5-6"), 1 * 86400.0f * 0.14f, 1.0e-5f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "ROFTL:5-6"), 2 * 86400.0f * 0.14f, 1.0e-5f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "RGFR:2-12"),
                        "Summary data must have RGFR:2-12");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "RGFR:2-12"), 86400.0f * 10.98f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "RGFR:2-12"), 86400.0f * 10.98f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "RGFR:2-12"), 86400.0f * 10.98f, 1.0e-6f);

    BOOST_CHECK_MESSAGE(ecl_sum_has_general_var(resp, "RGFR:9-10"),
                        "Summary data must have RGFR:9-10");

    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 0, "RGFR:9-10"), 86400.0f * 3.1415926f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 1, "RGFR:9-10"), 86400.0f * 3.1415926f, 1.0e-6f);
    BOOST_CHECK_CLOSE(ecl_sum_get_general_var(resp, 2, "RGFR:9-10"), 86400.0f * 3.1415926f, 1.0e-6f);
}

BOOST_AUTO_TEST_CASE(BLOCK_VARIABLES) {
    setup cfg( "region_injection" );


    std::map<std::pair<std::string, int>, double> block_values;
    for (size_t r=1; r <= 10; r++) {
        block_values[std::make_pair("BPR", (r-1)*100 + 1)] = r*1.0;
    }
    block_values[std::make_pair("BSWAT", 1)] = 8.0;
    block_values[std::make_pair("BSGAS", 1)] = 9.0;
    block_values[std::make_pair("BOSAT", 1)] = 0.91;
    block_values[std::make_pair("BWKR",  2)] = 0.81;
    block_values[std::make_pair("BOKR",  2)] = 0.71;
    block_values[std::make_pair("BKRO",  2)] = 0.73;
    block_values[std::make_pair("BKROW", 3)] = 0.68;
    block_values[std::make_pair("BKROG", 4)] = 0.82;
    block_values[std::make_pair("BGKR",  2)] = 0.61;
    block_values[std::make_pair("BKRG",  2)] = 0.63;
    block_values[std::make_pair("BKRW",  2)] = 0.51;
    block_values[std::make_pair("BWPC", 11)] = 0.53;
    block_values[std::make_pair("BGPC", 11)] = 5.3;
    block_values[std::make_pair("BVWAT", 1)] = 4.1;
    block_values[std::make_pair("BWVIS", 1)] = 4.3;
    block_values[std::make_pair("BVGAS", 1)] = 0.031;
    block_values[std::make_pair("BGVIS", 1)] = 0.037;
    block_values[std::make_pair("BVOIL", 1)] = 31.0;
    block_values[std::make_pair("BOVIS", 1)] = 33.0;

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {},{}, {}, {}, {}, block_values);
    writer.add_timestep( st, 0, false);
    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {},{}, {}, {}, {}, block_values);
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {},{}, {}, {}, {}, block_values);
    writer.add_timestep( st, 2, false);
    writer.eval( st, 3, 2 * day, cfg.wells , cfg.grp_nwrk, {},{}, {}, {}, {}, block_values);
    writer.add_timestep( st, 3, false);
    writer.eval( st, 4, 2 * day, cfg.wells , cfg.grp_nwrk, {},{}, {}, {}, {}, block_values);
    writer.add_timestep( st, 4, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    UnitSystem units( UnitSystem::UnitType::UNIT_TYPE_METRIC );
    for (size_t r=1; r <= 10; r++) {
        std::string bpr_key   = "BPR:1,1,"   + std::to_string( r );
        BOOST_CHECK( ecl_sum_has_general_var( resp , bpr_key.c_str()));

        BOOST_CHECK_CLOSE( r * 1.0 , units.to_si( UnitSystem::measure::pressure , ecl_sum_get_general_var( resp, 1, bpr_key.c_str())) , 1e-5);

    }

    BOOST_CHECK_CLOSE( 8.0   , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BSWAT:1,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 9.0   , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BSGAS:1,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 0.91  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BOSAT:1,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 0.81  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BWKR:2,1,1"))  , 1e-5);
    BOOST_CHECK_CLOSE( 0.71  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BOKR:2,1,1"))  , 1e-5);
    BOOST_CHECK_CLOSE( 0.73  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BKRO:2,1,1"))  , 1e-5);
    BOOST_CHECK_CLOSE( 0.82  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BKROG:4,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 0.68  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BKROW:3,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 0.61  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BGKR:2,1,1"))  , 1e-5);
    BOOST_CHECK_CLOSE( 0.63  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BKRG:2,1,1"))  , 1e-5);
    BOOST_CHECK_CLOSE( 0.51  , units.to_si( UnitSystem::measure::identity  , ecl_sum_get_general_var( resp, 1, "BKRW:2,1,1"))  , 1e-5);
    BOOST_CHECK_CLOSE( 0.53  , units.to_si( UnitSystem::measure::pressure  , ecl_sum_get_general_var( resp, 1, "BWPC:1,2,1"))  , 1e-5);
    BOOST_CHECK_CLOSE( 5.3   , units.to_si( UnitSystem::measure::pressure  , ecl_sum_get_general_var( resp, 1, "BGPC:1,2,1"))  , 1e-5);
    BOOST_CHECK_CLOSE( 4.1   , units.to_si( UnitSystem::measure::viscosity , ecl_sum_get_general_var( resp, 1, "BVWAT:1,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 4.3   , units.to_si( UnitSystem::measure::viscosity , ecl_sum_get_general_var( resp, 1, "BWVIS:1,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 0.031 , units.to_si( UnitSystem::measure::viscosity , ecl_sum_get_general_var( resp, 1, "BVGAS:1,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 0.037 , units.to_si( UnitSystem::measure::viscosity , ecl_sum_get_general_var( resp, 1, "BGVIS:1,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 31.0  , units.to_si( UnitSystem::measure::viscosity , ecl_sum_get_general_var( resp, 1, "BVOIL:1,1,1")) , 1e-5);
    BOOST_CHECK_CLOSE( 33.0  , units.to_si( UnitSystem::measure::viscosity , ecl_sum_get_general_var( resp, 1, "BOVIS:1,1,1")) , 1e-5);

    BOOST_CHECK_CLOSE( 111.222 , ecl_sum_get_well_connection_var( resp, 1, "W_1", "CTFAC", 1, 1, 1), 1e-5);
    BOOST_CHECK_CLOSE( 222.333 , ecl_sum_get_well_connection_var( resp, 1, "W_2", "CTFAC", 2, 1, 1), 1e-5);
    BOOST_CHECK_CLOSE( 333.444 , ecl_sum_get_well_connection_var( resp, 1, "W_2", "CTFAC", 2, 1, 2), 1e-5);
    BOOST_CHECK_CLOSE( 444.555 , ecl_sum_get_well_connection_var( resp, 1, "W_3", "CTFAC", 3, 1, 1), 1e-5);

    BOOST_CHECK_CLOSE( 111.222 , ecl_sum_get_well_connection_var( resp, 3, "W_1", "CTFAC", 1, 1, 1), 1e-5);
    BOOST_CHECK_CLOSE( 111.222 , ecl_sum_get_well_connection_var( resp, 4, "W_1", "CTFAC", 1, 1, 1), 1e-5);

    // Cell is not active
    BOOST_CHECK( !ecl_sum_has_general_var( resp , "BPR:2,1,10"));
}

BOOST_AUTO_TEST_CASE(NODE_VARIABLES) {
    setup cfg( "test_summary_node" );

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);

    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);

    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);

    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();

    BOOST_CHECK_CLOSE( 21.0 , ecl_sum_get_group_var( resp, 1, "PLAT-A", "GPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 33.44, ecl_sum_get_group_var( resp, 1, "G_1", "GPR" ), 1e-5 );
    BOOST_CHECK_CLOSE( 23.45, ecl_sum_get_group_var( resp, 1, "G_2", "GPR" ), 1e-5 );
}

/*
  The SummaryConfig.require3DField( ) implementation is slightly ugly:

  1. Which 3D fields are required is implicitly given by the
     implementation of the Summary() class here in opm-output.

  2. The implementation of the SummaryConfig.require3DField( ) is
     based on a hardcoded list in SummaryConfig.cpp - i.e. there is a
     inverse dependency between the opm-parser and opm-output modules.

  The test here just to ensure that *something* breaks if the
  opm-parser implementation is changed/removed.
*/



BOOST_AUTO_TEST_CASE( require3D )
{
    setup cfg( "XXXX" );
    const auto summaryConfig = cfg.config;

    BOOST_CHECK( summaryConfig.require3DField( "PRESSURE" ));
    BOOST_CHECK( summaryConfig.require3DField( "SGAS" ));
    BOOST_CHECK( summaryConfig.require3DField( "SWAT" ));
    BOOST_CHECK( summaryConfig.require3DField( "WIP" ));
    BOOST_CHECK( summaryConfig.require3DField( "GIP" ));
    BOOST_CHECK( summaryConfig.require3DField( "OIP" ));
    BOOST_CHECK( summaryConfig.require3DField( "OIPL" ));
    BOOST_CHECK( summaryConfig.require3DField( "OIPG" ));
    BOOST_CHECK( summaryConfig.require3DField( "GIPL" ));
    BOOST_CHECK( summaryConfig.require3DField( "GIPG" ));
}


BOOST_AUTO_TEST_CASE(MISC) {
    setup cfg( "test_misc");

    out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule , cfg.name );
    SummaryState st(TimeService::now());
    writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 0, false);
    writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 1, false);
    writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep( st, 2, false);
    writer.write();

    auto res = readsum( cfg.name );
    const auto* resp = res.get();
    BOOST_CHECK( ecl_sum_has_key( resp , "TCPU" ));
}


BOOST_AUTO_TEST_CASE(EXTRA) {
    setup cfg( "test_extra");

    {
        out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule , cfg.name );
        SummaryState st(TimeService::now());
        writer.eval( st, 0, 0 * day, cfg.wells , cfg.grp_nwrk, { {"TCPU" , 0 }}, {}, {}, {});
        writer.add_timestep( st, 0, false);
        writer.eval( st, 1, 1 * day, cfg.wells , cfg.grp_nwrk, { {"TCPU" , 1 }}, {}, {}, {});
        writer.add_timestep( st, 1, false);
        writer.eval( st, 2, 2 * day, cfg.wells , cfg.grp_nwrk, { {"TCPU" , 2}}, {}, {}, {});
        writer.add_timestep( st, 2, false);

        /* Add a not-recognized key; that is OK */
        BOOST_CHECK_NO_THROW(  writer.eval( st, 3, 3 * day, cfg.wells , cfg.grp_nwrk, { {"MISSING" , 2 }}, {}, {}, {}));
        BOOST_CHECK_NO_THROW(  writer.add_timestep( st, 3, false));

        /* Override a NOT MISC variable - ignored. */
        writer.eval( st, 4, 4 * day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
        writer.add_timestep( st, 4, false);
        writer.write();
    }

    auto res = readsum( cfg.name );
    const auto* resp = res.get();
    BOOST_CHECK( ecl_sum_has_key( resp , "TCPU" ));
    BOOST_CHECK_CLOSE( 1 , ecl_sum_get_general_var( resp , 1 , "TCPU") , 0.001);
    BOOST_CHECK_CLOSE( 2 , ecl_sum_get_general_var( resp , 2 , "TCPU") , 0.001);

    /* Not passed explicitly in timesteps 3 and 4 - the TCPU value will therefor
       stay at the value assigned at step 2 - it is a "state" variable after all ... */
    BOOST_CHECK_CLOSE( 2 , ecl_sum_get_general_var( resp , 4 , "TCPU") , 0.001);

    /* Override a NOT MISC variable - ignored. */
    BOOST_CHECK(  ecl_sum_get_general_var( resp , 4 , "FOPR") > 0.0 );
}

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

BOOST_AUTO_TEST_CASE(READ_WRITE_WELLDATA) {

            Opm::data::Wells wellRates = result_wells();

            MessageBuffer buffer;
            wellRates.write(buffer);

            Opm::data::Wells wellRatesCopy;
            wellRatesCopy.read(buffer);

            BOOST_CHECK_CLOSE( wellRatesCopy.get( "W_1" , rt::tracer, "SEA") , wellRates.get( "W_1" , rt::tracer, "SEA"), 1e-16);

            BOOST_CHECK_CLOSE( wellRatesCopy.get( "W_1" , rt::wat) , wellRates.get( "W_1" , rt::wat), 1e-16);
            BOOST_CHECK_CLOSE( wellRatesCopy.get( "W_2" , 101 , rt::wat) , wellRates.get( "W_2" , 101 , rt::wat), 1e-16);

            const auto& seg = wellRatesCopy.at("W_1").segments.at(1);
            BOOST_CHECK_CLOSE(seg.rates.get(rt::wat),  123.45*sm3_pr_day(), 1.0e-10);
            BOOST_CHECK_CLOSE(seg.rates.get(rt::oil),  543.21*sm3_pr_day(), 1.0e-10);
            BOOST_CHECK_CLOSE(seg.rates.get(rt::gas), 1729.496*sm3_pr_day(), 1.0e-10);
            const auto pres_idx = Opm::data::SegmentPressures::Value::Pressure;
            BOOST_CHECK_CLOSE(seg.pressures[pres_idx], 314.159*unit::barsa, 1.0e-10);
            BOOST_CHECK_EQUAL(seg.segNumber, 1U);

            // No data for segment 10 of well W_2 (or no such segment).
            const auto& W2 = wellRatesCopy.at("W_2");
            BOOST_CHECK_THROW(W2.segments.at(10), std::out_of_range);

            const auto& W6 = wellRatesCopy.at("W_6");
            const auto& curr = W6.current_control;
            BOOST_CHECK_MESSAGE(!curr.isProducer, "W_6 must be an injector");
            BOOST_CHECK_MESSAGE(curr.prod == ::Opm::Well::ProducerCMode::CMODE_UNDEFINED, "W_6 must have an undefined producer control");
            BOOST_CHECK_MESSAGE(curr.inj == ::Opm::Well::InjectorCMode::GRUP, "W_6 must be on GRUP control");

            BOOST_CHECK_MESSAGE(W2.dynamicStatus == ::Opm::Well::Status::OPEN,
                                "W_2 must be dynamically open (dynamicStatus == OPEN)");
}

// Well/group tree structure (SUMMARY_EFF_FAC.DATA):
//
//    W* are wells, G* are groups.
//
//                         +-------+
//                         | FIELD |
//                         +---+---+
//                             |
//                  +----------+-----------------+
//                  |                            |
//             +----+---+                   +----+---+
//             |    G   |                   |   G_4  |
//             +----+---+                   +----+---+
//                  |                            |
//         +--------+----------+            +----+---+
//         |                   |            |   G_3  |
//    +----+---+          +----+---+        +----+---+
//    |   G_1  |          |   G_2  |             |
//    +----+---+          +----+---+        +----+---+
//         |                   |            |   W_3  |
//    +----+---+          +----+---+        +----+---+
//    |   W_1  |          |   W_2  |
//    +----+---+          +----+---+
//

BOOST_AUTO_TEST_CASE(efficiency_factor) {
        // W_3 is a producer in SUMMARY_EFF_FAC.DATA
        setup cfg( "test_efficiency_factor", "SUMMARY_EFF_FAC.DATA", false );

        out::Summary writer( cfg.es, cfg.config, cfg.grid, cfg.schedule, cfg.name );
        SummaryState st(TimeService::now());
        writer.eval( st, 0, 0 * day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
        writer.add_timestep( st, 0, false);
        writer.eval( st, 1, 1 * day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
        writer.add_timestep( st, 1, false);
        writer.eval( st, 2, 2 * day, cfg.wells, cfg.grp_nwrk, {}, {}, {}, {});
        writer.add_timestep( st, 2, false);
        writer.write();
        auto res = readsum( cfg.name );
        const auto* resp = res.get();

        /* No WEFAC assigned to W_1 */
        BOOST_CHECK_CLOSE(     123.4, ecl_sum_get_well_connection_var( resp, 1, "W_1", "CVPT", 1, 1, 1 ), 1e-5 );
        BOOST_CHECK_CLOSE( 2 * 123.4, ecl_sum_get_well_connection_var( resp, 2, "W_1", "CVPT", 1, 1, 1 ), 1e-5 );

        BOOST_CHECK_CLOSE( 100.2 / 100.1, ecl_sum_get_well_connection_var( resp, 1, "W_1", "CGOR", 1, 1, 1 ), 1e-5 );
        BOOST_CHECK_CLOSE( 100.2 / 100.1, ecl_sum_get_well_connection_var( resp, 2, "W_1", "CGOR", 1, 1,
 1 ), 1e-5 );

        BOOST_CHECK_CLOSE( 10.1, ecl_sum_get_well_var( resp, 1, "W_1", "WOPT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 2 * 10.1, ecl_sum_get_well_var( resp, 2, "W_1", "WOPT" ), 1e-5 );

        BOOST_CHECK_CLOSE( 1.0, ecl_sum_get_well_var( resp, 1, "W_1", "WEFF" ), 1.0e-5 );
        BOOST_CHECK_CLOSE( 1.0, ecl_sum_get_well_var( resp, 2, "W_1", "WEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( 1.0, ecl_sum_get_well_var( resp, 1, "W_1", "WEFFG" ), 1.0e-5 );
        BOOST_CHECK_CLOSE( 1.0, ecl_sum_get_well_var( resp, 2, "W_1", "WEFFG" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( 10.1, ecl_sum_get_well_var( resp, 1, "W_1", "WOPR" ), 1e-5 );
        BOOST_CHECK_CLOSE( 10.1, ecl_sum_get_well_var( resp, 1, "W_1", "WOPT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 2 * 10.1, ecl_sum_get_well_var( resp, 2, "W_1", "WOPT" ), 1e-5 );

        BOOST_CHECK_CLOSE( -10.13, ecl_sum_get_group_var( resp, 1, "G_1", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -10.14, ecl_sum_get_group_var( resp, 1, "G_1", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -10.15, ecl_sum_get_group_var( resp, 1, "G_1", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0 , ecl_sum_get_group_var( resp, 1, "G_1", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0 , ecl_sum_get_group_var( resp, 1, "G_1", "GGPI" ), 1e-5 );

        BOOST_CHECK_CLOSE( -10.13, ecl_sum_get_group_var( resp, 2, "G_1", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -10.14, ecl_sum_get_group_var( resp, 2, "G_1", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -10.15, ecl_sum_get_group_var( resp, 2, "G_1", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0 , ecl_sum_get_group_var( resp, 2, "G_1", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0 , ecl_sum_get_group_var( resp, 2, "G_1", "GGPI" ), 1e-5 );

        BOOST_CHECK_CLOSE( 1.0, ecl_sum_get_group_var( resp, 1, "G_1", "GEFF" ), 1.0e-5 );
        BOOST_CHECK_CLOSE( 1.0, ecl_sum_get_group_var( resp, 2, "G_1", "GEFF" ), 1.0e-5 );

        /* WEFAC 0.2 assigned to W_2.
         * W_2 assigned to group G2. GEFAC G2 = 0.01 */
        BOOST_CHECK_CLOSE( 20.1, ecl_sum_get_well_var( resp, 1, "W_2", "WOPR" ), 1e-5 );
        BOOST_CHECK_CLOSE( 20.1 * 0.2 * 0.01, ecl_sum_get_well_var( resp, 1, "W_2", "WOPT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 2 * 20.1 * 0.2 * 0.01, ecl_sum_get_well_var( resp, 2, "W_2", "WOPT" ), 1e-5 );
        BOOST_CHECK_CLOSE(  23.4,     ecl_sum_get_well_connection_var( resp, 1, "W_2", "CVPR", 2, 1, 1 ), 1e-5 );
        BOOST_CHECK_CLOSE( 234.5,     ecl_sum_get_well_connection_var( resp, 2, "W_2", "CVPR", 2, 1, 2 ), 1e-5 );

        BOOST_CHECK_CLOSE( 0.2, ecl_sum_get_well_var( resp, 1, "W_2", "WEFF" ), 1.0e-5 );
        BOOST_CHECK_CLOSE( 0.2, ecl_sum_get_well_var( resp, 2, "W_2", "WEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( 0.2 * 0.01, ecl_sum_get_well_var( resp, 1, "W_2", "WEFFG" ), 1.0e-5 );
        BOOST_CHECK_CLOSE( 0.2 * 0.01, ecl_sum_get_well_var( resp, 2, "W_2", "WEFFG" ), 1.0e-5 );

        BOOST_CHECK_CLOSE(      23.4 * 0.2 * 0.01, ecl_sum_get_well_connection_var( resp, 1, "W_2", "CVPT", 2, 1, 1 ), 1e-5 );
        BOOST_CHECK_CLOSE( 2 * 234.5 * 0.2 * 0.01, ecl_sum_get_well_connection_var( resp, 2, "W_2", "CVPT", 2, 1, 2 ), 1e-5 );

        BOOST_CHECK_CLOSE( -20.13 * 0.2, ecl_sum_get_group_var( resp, 1, "G_2", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -20.14 * 0.2, ecl_sum_get_group_var( resp, 1, "G_2", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -20.15 * 0.2, ecl_sum_get_group_var( resp, 1, "G_2", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0       , ecl_sum_get_group_var( resp, 1, "G_2", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0       , ecl_sum_get_group_var( resp, 1, "G_2", "GGPI" ), 1e-5 );

        BOOST_CHECK_CLOSE(   0.0       , ecl_sum_get_group_var( resp, 1, "G_2", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0       , ecl_sum_get_group_var( resp, 1, "G_2", "GGPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.01      , ecl_sum_get_group_var( resp, 1, "G_2", "GEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( -10.13 - (20.13 * 0.2 * 0.01), ecl_sum_get_group_var( resp, 1, "G", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -10.14 - (20.14 * 0.2 * 0.01), ecl_sum_get_group_var( resp, 1, "G", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -10.15 - (20.15 * 0.2 * 0.01), ecl_sum_get_group_var( resp, 1, "G", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0                        , ecl_sum_get_group_var( resp, 1, "G", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0                        , ecl_sum_get_group_var( resp, 1, "G", "GGPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   1.0                        , ecl_sum_get_group_var( resp, 1, "G", "GEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( -20.13 * 0.2, ecl_sum_get_group_var( resp, 2, "G_2", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -20.14 * 0.2, ecl_sum_get_group_var( resp, 2, "G_2", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -20.15 * 0.2, ecl_sum_get_group_var( resp, 2, "G_2", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0       , ecl_sum_get_group_var( resp, 2, "G_2", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0       , ecl_sum_get_group_var( resp, 2, "G_2", "GGPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.01      , ecl_sum_get_group_var( resp, 2, "G_2", "GEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( -10.13 - (20.13 * 0.2 * 0.01), ecl_sum_get_group_var( resp, 2, "G", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -10.14 - (20.14 * 0.2 * 0.01), ecl_sum_get_group_var( resp, 2, "G", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( -10.15 - (20.15 * 0.2 * 0.01), ecl_sum_get_group_var( resp, 2, "G", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0                        , ecl_sum_get_group_var( resp, 2, "G", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   0.0                        , ecl_sum_get_group_var( resp, 2, "G", "GGPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(   1.0                        , ecl_sum_get_group_var( resp, 2, "G", "GEFF" ), 1.0e-5 );

        /* WEFAC 0.3 assigned to W_3.
         * W_3 assigned to group G3. GEFAC G_3 = 0.02
         * G_3 assigned to group G4. GEFAC G_4 = 0.03*/
        BOOST_CHECK_CLOSE( 300.2 / 300.1, ecl_sum_get_well_connection_var( resp, 1, "W_3", "CGOR", 3, 1, 1 ), 1e-5 );

        BOOST_CHECK_CLOSE( 300.2 / 300.1, ecl_sum_get_well_connection_var( resp, 2, "W_3", "CGOR", 3, 1, 1 ), 1e-5 );

        BOOST_CHECK_CLOSE( 0.3, ecl_sum_get_well_var( resp, 1, "W_3", "WEFF" ), 1.0e-5 );
        BOOST_CHECK_CLOSE( 0.3, ecl_sum_get_well_var( resp, 2, "W_3", "WEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( 0.3 * 0.02 * 0.03, ecl_sum_get_well_var( resp, 1, "W_3", "WEFFG" ), 1.0e-5 );
        BOOST_CHECK_CLOSE( 0.3 * 0.02 * 0.04, ecl_sum_get_well_var( resp, 2, "W_3", "WEFFG" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( 432.1 * 0.3 * 0.02 * 0.03,
                           ecl_sum_get_well_connection_var( resp, 1, "W_3", "CVPT", 3, 1, 1 ), 1e-5 );
        BOOST_CHECK_CLOSE( 432.1 * 0.3 * 0.02 * 0.03 +
                           432.1 * 0.3 * 0.02 * 0.04,
                           ecl_sum_get_well_connection_var( resp, 2, "W_3", "CVPT", 3, 1, 1 ), 1e-5 );

        BOOST_CHECK_CLOSE( 30.1, ecl_sum_get_well_var( resp, 1, "W_3", "WOIR" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03, ecl_sum_get_well_var( resp, 1, "W_3", "WOIT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03 + 30.1 * 0.3 * 0.02 * 0.04, ecl_sum_get_well_var( resp, 2, "W_3", "WOIT" ), 1e-5 );

        BOOST_CHECK_CLOSE( 30.13 * 0.3, ecl_sum_get_group_var( resp, 1, "G_3", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.14 * 0.3, ecl_sum_get_group_var( resp, 1, "G_3", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.15 * 0.3, ecl_sum_get_group_var( resp, 1, "G_3", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.0       , ecl_sum_get_group_var( resp, 1, "G_3", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.0       , ecl_sum_get_group_var( resp, 1, "G_3", "GGPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.02      , ecl_sum_get_group_var( resp, 1, "G_3", "GEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( 30.13 * 0.3 * 0.02, ecl_sum_get_group_var( resp, 1, "G_4", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.14 * 0.3 * 0.02, ecl_sum_get_group_var( resp, 1, "G_4", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.15 * 0.3 * 0.02, ecl_sum_get_group_var( resp, 1, "G_4", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.0              , ecl_sum_get_group_var( resp, 1, "G_4", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.0              , ecl_sum_get_group_var( resp, 1, "G_4", "GGPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.03             , ecl_sum_get_group_var( resp, 1, "G_4", "GEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( 30.13 * 0.3, ecl_sum_get_group_var( resp, 2, "G_3", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.14 * 0.3, ecl_sum_get_group_var( resp, 2, "G_3", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.15 * 0.3, ecl_sum_get_group_var( resp, 2, "G_3", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.0       , ecl_sum_get_group_var( resp, 2, "G_3", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.0       , ecl_sum_get_group_var( resp, 2, "G_3", "GGPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.02      , ecl_sum_get_group_var( resp, 2, "G_3", "GEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( 30.13 * 0.3 * 0.02, ecl_sum_get_group_var( resp, 2, "G_4", "GWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.14 * 0.3 * 0.02, ecl_sum_get_group_var( resp, 2, "G_4", "GOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.15 * 0.3 * 0.02, ecl_sum_get_group_var( resp, 2, "G_4", "GGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.0              , ecl_sum_get_group_var( resp, 2, "G_4", "GWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.0              , ecl_sum_get_group_var( resp, 2, "G_4", "GGPI" ), 1e-5 );
        BOOST_CHECK_CLOSE(  0.04             , ecl_sum_get_group_var( resp, 2, "G_4", "GEFF" ), 1.0e-5 );

        BOOST_CHECK_CLOSE( - 10.13
                           - (20.13 * 0.2 * 0.01)
                           + (30.13 * 0.3 * 0.02)*0.03, ecl_sum_get_field_var( resp, 1, "FWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( - 10.14
                           - (20.14 * 0.2 * 0.01)
                           + (30.14 * 0.3 * 0.02)*0.03, ecl_sum_get_field_var( resp, 1, "FOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( - 10.15
                           - (20.15 * 0.2 * 0.01)
                           + (30.15 * 0.3 * 0.02)*0.03, ecl_sum_get_field_var( resp, 1, "FGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 0.0                        , ecl_sum_get_field_var( resp, 1, "FWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE( 0.0                        , ecl_sum_get_field_var( resp, 1, "FGPI" ), 1e-5 );

        BOOST_CHECK_CLOSE( - 10.13
                           - (20.13 * 0.2 * 0.01)
                           + (30.13 * 0.3 * 0.02)*0.04, ecl_sum_get_field_var( resp, 2, "FWPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( - 10.14
                           - (20.14 * 0.2 * 0.01)
                           + (30.14 * 0.3 * 0.02)*0.04, ecl_sum_get_field_var( resp, 2, "FOPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( - 10.15
                           - (20.15 * 0.2 * 0.01)
                           + (30.15 * 0.3 * 0.02)*0.04, ecl_sum_get_field_var( resp, 2, "FGPP" ), 1e-5 );
        BOOST_CHECK_CLOSE( 0.0                        , ecl_sum_get_field_var( resp, 2, "FWPI" ), 1e-5 );
        BOOST_CHECK_CLOSE( 0.0                        , ecl_sum_get_field_var( resp, 2, "FGPI" ), 1e-5 );

        /* WEFAC 0.2 assigned to W_2.
         * W_2 assigned to group G2. GEFAC G2 = 0.01 */
        BOOST_CHECK_CLOSE( 20.1 * 0.2, ecl_sum_get_group_var( resp, 1, "G_2", "GOPR" ), 1e-5 );
        BOOST_CHECK_CLOSE( 20.1 * 0.2 * 0.01, ecl_sum_get_group_var( resp, 1, "G_2", "GOPT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 2 * 20.1 * 0.2 * 0.01, ecl_sum_get_group_var( resp, 2, "G_2", "GOPT" ), 1e-5 );

        /* WEFAC 0.3 assigned to W_3.
         * W_3 assigned to group G3. GEFAC G_3 = 0.02
         * G_3 assigned to group G4. GEFAC G_4 = 0.03*/
        BOOST_CHECK_CLOSE( 30.1 * 0.3, ecl_sum_get_group_var( resp, 1, "G_3", "GOIR" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03, ecl_sum_get_group_var( resp, 1, "G_3", "GOIT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03 + 30.1 * 0.3 * 0.02 * 0.04, ecl_sum_get_group_var( resp, 2, "G_3", "GOIT" ), 1e-5 );

        /* WEFAC 0.3 assigned to W_3.
         * W_3 assigned to group G3. GEFAC G_3 = 0.02
         * G_3 assigned to group G4. GEFAC G_4 = 0.03
         * The rate for a group is calculated including WEFAC and GEFAC for subgroups */
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02, ecl_sum_get_group_var( resp, 1, "G_4", "GOIR" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03, ecl_sum_get_group_var( resp, 1, "G_4", "GOIT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03 + 30.1 * 0.3 * 0.02 * 0.04, ecl_sum_get_group_var( resp, 2, "G_4", "GOIT" ), 1e-5 );

        BOOST_CHECK_CLOSE( 10.1 + 20.1 * 0.2 * 0.01, ecl_sum_get_field_var( resp, 1, "FOPR" ), 1e-5 );
        BOOST_CHECK_CLOSE( 10.1 + 20.1 * 0.2 * 0.01, ecl_sum_get_field_var( resp, 1, "FOPT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 2 * (10.1 + 20.1 * 0.2 * 0.01), ecl_sum_get_field_var( resp, 2, "FOPT" ), 1e-5 );

        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03, ecl_sum_get_field_var( resp, 1, "FOIR" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03, ecl_sum_get_field_var( resp, 1, "FOIT" ), 1e-5 );
        BOOST_CHECK_CLOSE( 30.1 * 0.3 * 0.02 * 0.03 + 30.1 * 0.3 * 0.02 * 0.04, ecl_sum_get_field_var( resp, 2, "FOIT" ), 1e-5 );

        BOOST_CHECK_CLOSE( 200.1 * 0.2 * 0.01
                           + 300.1 * 0.3 * 0.02 * 0.03,
                           ecl_sum_get_general_var( resp , 1 , "ROPR:1" ) , 1e-5);

        BOOST_CHECK_CLOSE( 100.1, ecl_sum_get_general_var( resp , 1 , "ROPR:2" ) , 1e-5);

        BOOST_CHECK_CLOSE( 300 * 0.2 * 0.01, ecl_sum_get_general_var( resp , 1 , "RWIR:1" ) , 1e-5);

        BOOST_CHECK_CLOSE( 200.1, ecl_sum_get_well_connection_var( resp, 1, "W_2", "COPR", 2, 1, 1 ), 1e-5 );
        BOOST_CHECK_CLOSE( 200.1 * 0.2 * 0.01, ecl_sum_get_well_connection_var( resp, 1, "W_2", "COPT", 2, 1, 1 ), 1e-5 );
}




BOOST_AUTO_TEST_CASE(Test_SummaryState) {
    Opm::SummaryState st(TimeService::now());
    st.update("WWCT:OP_2", 100);
    BOOST_CHECK_CLOSE(st.get("WWCT:OP_2"), 100, 1e-5);
    BOOST_CHECK_THROW(st.get("NO_SUCH_KEY"), std::out_of_range);
    BOOST_CHECK(st.has("WWCT:OP_2"));
    BOOST_CHECK(!st.has("NO_SUCH_KEY"));
    BOOST_CHECK_EQUAL(st.get("WWCT:OP_99", -1), -1);

    st.update_well_var("OP1", "WWCT", 0.75);
    st.update_well_var("OP2", "WWCT", 0.75);
    st.update_well_var("OP3", "WOPT", 0.75);
    st.update_well_var("OP3", "WGPR", 0.75);
    BOOST_CHECK( st.has_well_var("OP1", "WWCT"));
    BOOST_CHECK_EQUAL( st.get_well_var("OP1", "WWCT"), 0.75);
    BOOST_CHECK_EQUAL( st.get_well_var("OP1", "WWCT"), st.get("WWCT:OP1"));
    const auto& wopr_wells = st.wells("WOPR");
    BOOST_CHECK_EQUAL( wopr_wells.size() , 0U);

    BOOST_CHECK_EQUAL( st.get_well_var("OP99", "WWCT", 0.50), 0.50);
    BOOST_CHECK( st.has_well_var("WWCT") );
    BOOST_CHECK( !st.has_well_var("NO_SUCH_VARIABLE") );

    const auto& wwct_wells = st.wells("WWCT");
    BOOST_CHECK_EQUAL( wwct_wells.size(), 2U);

    st.update_group_var("G1", "GWCT", 0.25);
    st.update_group_var("G2", "GWCT", 0.25);
    st.update_group_var("G3", "GOPT", 0.25);
    BOOST_CHECK( st.has_group_var("G1", "GWCT"));
    BOOST_CHECK_EQUAL( st.get_group_var("G1", "GWCT"), 0.25);
    BOOST_CHECK_EQUAL( st.get_group_var("G1", "GWCT"), st.get("GWCT:G1"));
    BOOST_CHECK_EQUAL( st.get_group_var("G99", "GWCT", 1.00), 1.00);
    BOOST_CHECK( !st.has_group_var("NO_SUCH_VARIABLE"));
    BOOST_CHECK( st.has_group_var("GWCT"));
    const auto& gopr_groups = st.groups("GOPR");
    BOOST_CHECK_EQUAL( gopr_groups.size() , 0U);

    const auto& gwct_groups = st.groups("GWCT");
    BOOST_CHECK_EQUAL( gwct_groups.size(), 2U);
    BOOST_CHECK_EQUAL(std::count(gwct_groups.begin(), gwct_groups.end(), "G1"), 1U);
    BOOST_CHECK_EQUAL(std::count(gwct_groups.begin(), gwct_groups.end(), "G2"), 1U);
    const auto& all_groups = st.groups();
    BOOST_CHECK_EQUAL( all_groups.size(), 3U);
    BOOST_CHECK_EQUAL(std::count(all_groups.begin(), all_groups.end(), "G1"), 1U);
    BOOST_CHECK_EQUAL(std::count(all_groups.begin(), all_groups.end(), "G2"), 1U);
    BOOST_CHECK_EQUAL(std::count(all_groups.begin(), all_groups.end(), "G3"), 1U);

    const auto& all_wells = st.wells();
    BOOST_CHECK_EQUAL( all_wells.size(), 3U);
    BOOST_CHECK_EQUAL(std::count(all_wells.begin(), all_wells.end(), "OP1"), 1U);
    BOOST_CHECK_EQUAL(std::count(all_wells.begin(), all_wells.end(), "OP2"), 1U);
    BOOST_CHECK_EQUAL(std::count(all_wells.begin(), all_wells.end(), "OP3"), 1U);

    // The well 'OP_2' which was indirectly added with the
    // st.update("WWCT:OP_2", 100) call is *not* counted as a well!
    BOOST_CHECK_EQUAL(st.num_wells(), 3U);


    BOOST_CHECK( st.erase("WWCT:OP2") );
    BOOST_CHECK( !st.has("WWCT:OP2") );
    BOOST_CHECK( !st.erase("WWCT:OP2") );

    BOOST_CHECK( st.erase_well_var("OP1", "WWCT") );
    BOOST_CHECK( !st.has_well_var("OP1", "WWCT"));
    BOOST_CHECK( !st.has("WWCT:OP1") );

    BOOST_CHECK( st.erase_group_var("G1", "GWCT") );
    BOOST_CHECK( !st.has_group_var("G1", "GWCT"));
    BOOST_CHECK( !st.has("GWCT:G1") );

    BOOST_CHECK(!st.has_conn_var("OP2", "COPR", 100));
    st.update_conn_var("OP2", "COPR", 100, 123);
    BOOST_CHECK(st.has_conn_var("OP2", "COPR", 100));
    BOOST_CHECK_EQUAL(st.get_conn_var("OP2", "COPR", 100), 123);
    BOOST_CHECK_EQUAL(st.get_conn_var("OP2", "COPR", 101, 99), 99);


    auto buffer = st.serialize();
    Opm::SummaryState st2(TimeService::now());
    st2.deserialize(buffer);

    BOOST_CHECK( st == st2 );
}

BOOST_AUTO_TEST_SUITE_END()

// ####################################################################

namespace {
    Opm::SummaryState calculateRestartVectors(const setup& config)
    {
        ::Opm::out::Summary smry {
            config.es, config.config, config.grid,
            config.schedule, "Ignore.This"
        };

      SummaryState st(TimeService::now());
      smry.eval(st, 0, 0*day, config.wells, config.grp_nwrk, {}, {}, {}, {});
      smry.add_timestep(st, 0, false);
      smry.eval(st, 1, 1*day, config.wells, config.grp_nwrk, {}, {}, {}, {});
      smry.add_timestep(st, 1, false);
      smry.eval(st, 2, 2*day, config.wells, config.grp_nwrk, {}, {}, {}, {});
      smry.add_timestep(st, 2, false);

      return st;
    }

    auto calculateRestartVectors()
        -> decltype(calculateRestartVectors({"test.Restart"}))
    {
        return calculateRestartVectors({"test.Restart"});
    }

    auto calculateRestartVectorsEffFac()
        -> decltype(calculateRestartVectors({"test.Restart.EffFac",
                                             "SUMMARY_EFF_FAC.DATA", false}))
    {
        // W_3 is a producer in SUMMARY_EFF_FAC.DATA
        const auto w3_injector = false;

        return calculateRestartVectors({
            "test.Restart.EffFac", "SUMMARY_EFF_FAC.DATA", w3_injector
        });
    }

    auto calculateRestartVectorsSegment()
        -> decltype(calculateRestartVectors({"test.Restart.Segment",
                                             "SOFR_TEST.DATA"}))
    {
        return calculateRestartVectors({
            "test.Restart.Segment", "SOFR_TEST.DATA"
        });
    }

    std::vector<std::string> restartVectors()
    {
        return {
            "WPR", "OPR", "GPR", "VPR",
            "WPT", "OPT", "GPT", "VPT",
            "WIR", "GIR", "WIT", "GIT",
            "GOR", "WCT",
        };
    }

    std::vector<std::string> activeWells()
    {
        return { "W_1", "W_2", "W_3" };
    }

    std::vector<std::string> activeGroups()
    {
        return { "G_1", "G_2" };
    }

    std::vector<std::string> activeGroupsEffFac()
    {
        return { "G_1", "G", "G_2", "G_3", "G_4" };
    }
}

// ====================================================================

BOOST_AUTO_TEST_SUITE(Restart)

BOOST_AUTO_TEST_CASE(Well_Vectors_Present)
{
    const auto rstrt = calculateRestartVectors();

    for (const auto& vector : restartVectors()) {
        for (const auto& w : activeWells()) {
            BOOST_CHECK( rstrt.has("W" + vector + ':' + w));
            BOOST_CHECK(!rstrt.has("W" + vector));
        }
    }

    for (const auto& w : activeWells()) {
        BOOST_CHECK( rstrt.has("WBHP:" + w));
        BOOST_CHECK(!rstrt.has("WBHP"));
    }
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Well_Vectors_Correct)
{
    const auto rstrt = calculateRestartVectors();

    // W_1 (Producer)
    {
        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("WWPR:W_1"), 10.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPR:W_1"), 10.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPR:W_1"), 10.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPR:W_1"), 10.6 + 10.7 + 10.8, 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("WWPT:W_1"), 2 * 1.0 * 10.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPT:W_1"), 2 * 1.0 * 10.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPT:W_1"), 2 * 1.0 * 10.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPT:W_1"), 2 * 1.0 * (10.6 + 10.7 + 10.8), 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("WWIR:W_1"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIR:W_1"), 0.0, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("WWIT:W_1"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIT:W_1"), 0.0, 1.0e-10);

        // BHP
        BOOST_CHECK_CLOSE(rstrt.get("WBHP:W_1"), 0.1, 1.0e-10);  // Bars

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("WWCT:W_1"), 10.0 / (10.0 + 10.1), 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("WGOR:W_1"), 10.2 / 10.1, 1.0e-10);
    }

    // W_2 (Producer)
    {
        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("WWPR:W_2"), 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPR:W_2"), 20.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPR:W_2"), 20.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPR:W_2"), 20.6 + 20.7 + 20.8, 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("WWPT:W_2"), 2 * 1.0 * 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPT:W_2"), 2 * 1.0 * 20.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPT:W_2"), 2 * 1.0 * 20.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPT:W_2"), 2 * 1.0 * (20.6 + 20.7 + 20.8), 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("WWIR:W_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIR:W_2"), 0.0, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("WWIT:W_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIT:W_2"), 0.0, 1.0e-10);

        // BHP
        BOOST_CHECK_CLOSE(rstrt.get("WBHP:W_2"), 1.1, 1.0e-10);  // Bars

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("WWCT:W_2"), 20.0 / (20.0 + 20.1), 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("WGOR:W_2"), 20.2 / 20.1, 1.0e-10);
    }

    // W_3 (Injector)
    {
        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("WWPR:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPR:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPR:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPR:W_3"), 0.0, 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("WWPT:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPT:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPT:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPT:W_3"), 0.0, 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("WWIR:W_3"), 30.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIR:W_3"), 30.2, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("WWIT:W_3"), 2 * 1.0 * 30.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIT:W_3"), 2 * 1.0 * 30.2, 1.0e-10);

        // BHP
        BOOST_CHECK_CLOSE(rstrt.get("WBHP:W_3"), 2.1, 1.0e-10);  // Bars

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("WWCT:W_3"), 0.0, 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("WGOR:W_3"), 0.0, 1.0e-10);
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(Group_Vectors_Present)
{
    const auto& rstrt = calculateRestartVectors();

    for (const auto& vector : restartVectors()) {
        for (const auto& g : activeGroups()) {
            BOOST_CHECK( rstrt.has("G" + vector + ':' + g));
            BOOST_CHECK(!rstrt.has("G" + vector));
        }
    }
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Group_Vectors_Correct)
{
    const auto rstrt = calculateRestartVectors();

    // G_1 (Producer, W_1 + W_2)
    {
        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("GWPR:G_1"), 10.0 + 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPR:G_1"), 10.1 + 20.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPR:G_1"), 10.2 + 20.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPR:G_1"),
                          (10.6 + 10.7 + 10.8) +
                          (20.6 + 20.7 + 20.8), 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("GWPT:G_1"), 2 * 1.0 * (10.0 + 20.0), 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPT:G_1"), 2 * 1.0 * (10.1 + 20.1), 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPT:G_1"), 2 * 1.0 * (10.2 + 20.2), 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPT:G_1"),
                          2 * 1.0 *
                          ((10.6 + 10.7 + 10.8) +
                           (20.6 + 20.7 + 20.8)), 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("GWIR:G_1"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIR:G_1"), 0.0, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("GWIT:G_1"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIT:G_1"), 0.0, 1.0e-10);

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("GWCT:G_1"),
                          (10.0 + 20.0) / ((10.0 + 10.1) + (20.0 + 20.1)), 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("GGOR:G_1"),
                          (10.2 + 20.2) / (10.1 + 20.1), 1.0e-10);
    }

    // G_2 (Injector, W_3)
    {
        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("GWPR:G_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPR:G_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPR:G_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPR:G_2"), 0.0, 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("GWPT:G_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPT:G_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPT:G_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPT:G_2"), 0.0, 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("GWIR:G_2"), 30.0 + 60.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIR:G_2"), 30.2 + 60.2, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("GWIT:G_2"), 2 * 1.0 * (30.0 + 60.0), 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIT:G_2"), 2 * 1.0 * (30.2 + 60.2), 1.0e-10);

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("GWCT:G_2"), 0.0, 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("GGOR:G_2"), 0.0, 1.0e-10);
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(Field_Vectors_Present)
{
    const auto& rstrt = calculateRestartVectors();

    for (const auto& vector : restartVectors()) {
        BOOST_CHECK( rstrt.has("F" + vector));
        BOOST_CHECK(!rstrt.has("F" + vector + ":FIELD"));
    }
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Field_Vectors_Correct)
{
    const auto rstrt = calculateRestartVectors();

    // Production rates (F = G_1 = W_1 + W_2)
    BOOST_CHECK_CLOSE(rstrt.get("FWPR"), 10.0 + 20.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FOPR"), 10.1 + 20.1, 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FGPR"), 10.2 + 20.2, 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FVPR"),
                      (10.6 + 10.7 + 10.8) +
                      (20.6 + 20.7 + 20.8), 1.0e-10);

    // Production cumulative totals (F = G_1 = W_1 + W_2)
    BOOST_CHECK_CLOSE(rstrt.get("FWPT"), 2 * 1.0 * (10.0 + 20.0), 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FOPT"), 2 * 1.0 * (10.1 + 20.1), 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FGPT"), 2 * 1.0 * (10.2 + 20.2), 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FVPT"),
                      2 * 1.0 *
                      ((10.6 + 10.7 + 10.8) +
                       (20.6 + 20.7 + 20.8)), 1.0e-10);

    // Injection rates (F = G_2 = W_3)
    BOOST_CHECK_CLOSE(rstrt.get("FWIR"), (30.0 + 60.0), 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FGIR"), (30.2 + 60.2), 1.0e-10);

    // Injection totals (F = G_2 = W_3)
    BOOST_CHECK_CLOSE(rstrt.get("FWIT"), 2 * 1.0 * (30.0 + 60.0), 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FGIT"), 2 * 1.0 * (30.2 + 60.2), 1.0e-10);

    // Water cut (F = G_1 = W_1 + W_2)
    BOOST_CHECK_CLOSE(rstrt.get("FWCT"),
                      (10.0 + 20.0) / ((10.0 + 10.1) + (20.0 + 20.1)), 1.0e-10);

    // Producing gas/oil ratio (F = G_1 = W_1 + W_2)
    BOOST_CHECK_CLOSE(rstrt.get("FGOR"),
                      (10.2 + 20.2) / (10.1 + 20.1), 1.0e-10);
}

BOOST_AUTO_TEST_SUITE_END()

// ####################################################################

BOOST_AUTO_TEST_SUITE(Restart_EffFac)

BOOST_AUTO_TEST_CASE(Well_Vectors_Correct)
{
    const auto rstrt = calculateRestartVectorsEffFac();

    // W_1 (Producer, efficiency factor = 1--no difference)
    {
        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("WWPR:W_1"), 10.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPR:W_1"), 10.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPR:W_1"), 10.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPR:W_1"), 10.6 + 10.7 + 10.8, 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("WWPT:W_1"), 2 * 1.0 * 10.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPT:W_1"), 2 * 1.0 * 10.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPT:W_1"), 2 * 1.0 * 10.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPT:W_1"), 2 * 1.0 * (10.6 + 10.7 + 10.8), 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("WWIR:W_1"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIR:W_1"), 0.0, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("WWIT:W_1"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIT:W_1"), 0.0, 1.0e-10);

        // BHP
        BOOST_CHECK_CLOSE(rstrt.get("WBHP:W_1"), 0.1, 1.0e-10);  // Bars

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("WWCT:W_1"), 10.0 / (10.0 + 10.1), 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("WGOR:W_1"), 10.2 / 10.1, 1.0e-10);
    }

    // W_2 (Producer, efficiency factor = 0.2)
    {
        const auto wefac = 0.2;
        const auto gefac = 0.01;

        // Production rates (unaffected by WEFAC)
        BOOST_CHECK_CLOSE(rstrt.get("WWPR:W_2"), 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPR:W_2"), 20.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPR:W_2"), 20.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPR:W_2"), (20.6 + 20.7 + 20.8), 1.0e-10);

        // Production cumulative totals (affected by WEFAC and containing GEFAC)
        BOOST_CHECK_CLOSE(rstrt.get("WWPT:W_2"), 2 * 1.0 * wefac * gefac * 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPT:W_2"), 2 * 1.0 * wefac * gefac * 20.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPT:W_2"), 2 * 1.0 * wefac * gefac * 20.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPT:W_2"), 2 * 1.0 * wefac * gefac * (20.6 + 20.7 + 20.8), 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("WWIR:W_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIR:W_2"), 0.0, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("WWIT:W_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIT:W_2"), 0.0, 1.0e-10);

        // BHP
        BOOST_CHECK_CLOSE(rstrt.get("WBHP:W_2"), 1.1, 1.0e-10);  // Bars

        // Water cut (unaffected by WEFAC)
        BOOST_CHECK_CLOSE(rstrt.get("WWCT:W_2"), 20.0 / (20.0 + 20.1), 1.0e-10);

        // Producing gas/oil ratio (unaffected by WEFAC)
        BOOST_CHECK_CLOSE(rstrt.get("WGOR:W_2"), 20.2 / 20.1, 1.0e-10);
    }

    // W_3 (Injector, efficiency factor = 0.3)
    {
        const auto wefac = 0.3;
        const auto gefac = 0.02; // G_3

        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("WWPR:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPR:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPR:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPR:W_3"), 0.0, 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("WWPT:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WOPT:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGPT:W_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WVPT:W_3"), 0.0, 1.0e-10);

        // Injection rates (unaffected by WEFAC)
        BOOST_CHECK_CLOSE(rstrt.get("WWIR:W_3"), 30.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("WGIR:W_3"), 30.2, 1.0e-10);

        // Injection totals (affected by WEFAC and containing GEFAC)
        //    GEFAC(G_4) = 0.03 at sim_step = 1
        //    GEFAC(G_4) = 0.04 at sim_step = 2
        BOOST_CHECK_CLOSE(rstrt.get("WWIT:W_3"),
                          30.0 * wefac * gefac *
                          ((1.0 * 0.03) + (1.0 * 0.04)), 1.0e-10);

        BOOST_CHECK_CLOSE(rstrt.get("WGIT:W_3"),
                          30.2 * wefac * gefac *
                          ((1.0 * 0.03) + (1.0 * 0.04)), 1.0e-10);

        // BHP
        BOOST_CHECK_CLOSE(rstrt.get("WBHP:W_3"), 2.1, 1.0e-10);  // Bars

        // Water cut (zero for injectors)
        BOOST_CHECK_CLOSE(rstrt.get("WWCT:W_3"), 0.0, 1.0e-10);

        // Producing gas/oil ratio (zero for injectors)
        BOOST_CHECK_CLOSE(rstrt.get("WGOR:W_3"), 0.0, 1.0e-10);
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(Group_Vectors_Present)
{
    const auto& rstrt = calculateRestartVectorsEffFac();

    for (const auto& vector : restartVectors()) {
        for (const auto& g : activeGroupsEffFac()) {
            BOOST_CHECK( rstrt.has("G" + vector + ':' + g));
            BOOST_CHECK(!rstrt.has("G" + vector));
        }
    }
}

// --------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(Group_Vectors_Correct)
{
    const auto rstrt = calculateRestartVectorsEffFac();

    // G_1 (Producer, W_1, GEFAC = 1--no change)
    {
        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("GWPR:G_1"), 10.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPR:G_1"), 10.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPR:G_1"), 10.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPR:G_1"), (10.6 + 10.7 + 10.8), 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("GWPT:G_1"), 2 * 1.0 * 10.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPT:G_1"), 2 * 1.0 * 10.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPT:G_1"), 2 * 1.0 * 10.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPT:G_1"),
                          2 * 1.0 * (10.6 + 10.7 + 10.8), 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("GWIR:G_1"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIR:G_1"), 0.0, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("GWIT:G_1"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIT:G_1"), 0.0, 1.0e-10);

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("GWCT:G_1"),
                          10.0 / (10.0 + 10.1), 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("GGOR:G_1"),
                          10.2 / 10.1, 1.0e-10);
    }

    // G_2 (Producer, W_2, GEFAC = 0.01)
    {
        const auto wefac = 0.2;
        const auto gefac = 0.01;

        // Production rates (affected by WEFAC)
        BOOST_CHECK_CLOSE(rstrt.get("GWPR:G_2"), wefac * 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPR:G_2"), wefac * 20.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPR:G_2"), wefac * 20.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPR:G_2"), wefac * (20.6 + 20.7 + 20.8), 1.0e-10);

        // Production cumulative totals (affected by both WEFAC and GEFAC)
        BOOST_CHECK_CLOSE(rstrt.get("GWPT:G_2"), 2 * 1.0 * gefac * wefac * 20.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPT:G_2"), 2 * 1.0 * gefac * wefac * 20.1, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPT:G_2"), 2 * 1.0 * gefac * wefac * 20.2, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPT:G_2"), 2 * 1.0 * gefac * wefac * (20.6 + 20.7 + 20.8), 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("GWIR:G_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIR:G_2"), 0.0, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("GWIT:G_2"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIT:G_2"), 0.0, 1.0e-10);

        // Water cut (unaffected by WEFAC or GEFAC since G_2 = W_2)
        BOOST_CHECK_CLOSE(rstrt.get("GWCT:G_2"), 20.0 / (20.0 + 20.1), 1.0e-10);

        // Producing gas/oil ratio (unaffected by WEFAC or GEFAC since G_2 = W_2)
        BOOST_CHECK_CLOSE(rstrt.get("GGOR:G_2"), 20.2 / 20.1, 1.0e-10);
    }

    // G (Producer, G_1 + G_2)
    {
        const auto gwefac = 0.01 * 0.2;

        BOOST_CHECK_CLOSE(rstrt.get("GWPR:G"), 10.0 + (gwefac * 20.0), 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPR:G"), 10.1 + (gwefac * 20.1), 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPR:G"), 10.2 + (gwefac * 20.2), 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPR:G"),
                                    (10.6 + 10.7 + 10.8) +
                          (gwefac * (20.6 + 20.7 + 20.8)), 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("GWPT:G"),
                          2 * 1.0 * (10.0 + (gwefac * 20.0)), 1.0e-10);

        BOOST_CHECK_CLOSE(rstrt.get("GOPT:G"),
                          2 * 1.0 * (10.1 + (gwefac * 20.1)), 1.0e-10);

        BOOST_CHECK_CLOSE(rstrt.get("GGPT:G"),
                          2 * 1.0 * (10.2 + (gwefac * 20.2)), 1.0e-10);

        BOOST_CHECK_CLOSE(rstrt.get("GVPT:G"),
                          2 * 1.0 *
                         (          (10.6 + 10.7 + 10.8) +
                          (gwefac * (20.6 + 20.7 + 20.8))), 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("GWIR:G"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIR:G"), 0.0, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("GWIT:G"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIT:G"), 0.0, 1.0e-10);

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("GWCT:G"),
                          (10.0 +        (gwefac *  20.0)) /
                          (10.0 + 10.1 + (gwefac * (20.0 + 20.1))), 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("GGOR:G"),
                          (10.2 + (gwefac * 20.2)) /
                          (10.1 + (gwefac * 20.1)), 1.0e-10);
    }

    // G_3 (Injector, W_3)
    {
        const auto wefac   = 0.3;
        const auto gefac_3 = 0.02;

        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("GWPR:G_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPR:G_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPR:G_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPR:G_3"), 0.0, 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("GWPT:G_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPT:G_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPT:G_3"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPT:G_3"), 0.0, 1.0e-10);

        // Injection rates
        BOOST_CHECK_CLOSE(rstrt.get("GWIR:G_3"), wefac * 30.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIR:G_3"), wefac * 30.2, 1.0e-10);

        // Injection totals
        BOOST_CHECK_CLOSE(rstrt.get("GWIT:G_3"),
                          30.0 * gefac_3 * wefac *
                          ((1.0 * 0.03) + (1.0 * 0.04)), 1.0e-10);

        BOOST_CHECK_CLOSE(rstrt.get("GGIT:G_3"),
                          30.2 * gefac_3 * wefac *
                          ((1.0 * 0.03) + (1.0 * 0.04)), 1.0e-10);

        // Water cut (zero for injectors)
        BOOST_CHECK_CLOSE(rstrt.get("GWCT:G_3"), 0.0, 1.0e-10);

        // Producing gas/oil ratio (zero for injectors)
        BOOST_CHECK_CLOSE(rstrt.get("GGOR:G_3"), 0.0, 1.0e-10);
    }

    // G_4 (Injector, G_3, GEFAC = 0.03 and 0.04)
    {
        // Production rates
        BOOST_CHECK_CLOSE(rstrt.get("GWPR:G_4"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPR:G_4"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPR:G_4"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPR:G_4"), 0.0, 1.0e-10);

        // Production cumulative totals
        BOOST_CHECK_CLOSE(rstrt.get("GWPT:G_4"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GOPT:G_4"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGPT:G_4"), 0.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GVPT:G_4"), 0.0, 1.0e-10);

        // Injection rates (at sim_step = 2)
        BOOST_CHECK_CLOSE(rstrt.get("GWIR:G_4"), 0.02 * 0.3 * 30.0, 1.0e-10);
        BOOST_CHECK_CLOSE(rstrt.get("GGIR:G_4"), 0.02 * 0.3 * 30.2, 1.0e-10);

        // Injection totals (GEFAC(G_4) = 0.03 at sim_step = 1,
        //                   GEFAC(G_4) = 0.04 at sim_step = 2)
        BOOST_CHECK_CLOSE(rstrt.get("GWIT:G_4"),
                          30.0 * 0.3 * 0.02 *
                          ((0.03 * 1.0) + (0.04 * 1.0)), 1.0e-10);

        BOOST_CHECK_CLOSE(rstrt.get("GGIT:G_4"),
                          30.2 * 0.3 * 0.02 *
                          ((0.03 * 1.0) + (0.04 * 1.0)), 1.0e-10);

        // Water cut
        BOOST_CHECK_CLOSE(rstrt.get("GWCT:G_4"), 0.0, 1.0e-10);

        // Producing gas/oil ratio
        BOOST_CHECK_CLOSE(rstrt.get("GGOR:G_4"), 0.0, 1.0e-10);
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(Field_Vectors_Correct)
{
    const auto rstrt = calculateRestartVectorsEffFac();

    // Field = G + G_4
    const auto efac_G = 0.01 * 0.2;

    BOOST_CHECK_CLOSE(rstrt.get("FWPR"), 10.0 + (efac_G * 20.0), 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FOPR"), 10.1 + (efac_G * 20.1), 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FGPR"), 10.2 + (efac_G * 20.2), 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FVPR"),
                                (10.6 + 10.7 + 10.8) +
                      (efac_G * (20.6 + 20.7 + 20.8)), 1.0e-10);

    // Production cumulative totals
    BOOST_CHECK_CLOSE(rstrt.get("FWPT"),
                      2 * 1.0 * (10.0 + (efac_G * 20.0)), 1.0e-10);

    BOOST_CHECK_CLOSE(rstrt.get("FOPT"),
                      2 * 1.0 * (10.1 + (efac_G * 20.1)), 1.0e-10);

    BOOST_CHECK_CLOSE(rstrt.get("FGPT"),
                      2 * 1.0 * (10.2 + (efac_G * 20.2)), 1.0e-10);

    BOOST_CHECK_CLOSE(rstrt.get("FVPT"),
                      2 * 1.0 *
                      (          (10.6 + 10.7 + 10.8) +
                       (efac_G * (20.6 + 20.7 + 20.8))), 1.0e-10);

    // Injection rates (at sim_step = 2, GEFAC(G_4) = 0.04)
    BOOST_CHECK_CLOSE(rstrt.get("FWIR"), 0.02 * 0.04 * 0.3 * 30.0, 1.0e-10);
    BOOST_CHECK_CLOSE(rstrt.get("FGIR"), 0.02 * 0.04 * 0.3 * 30.2, 1.0e-10);

    // Injection totals (GEFAC(G_4) = 0.03 at sim_step = 1,
    //                   GEFAC(G_4) = 0.04 at sim_step = 2)
    BOOST_CHECK_CLOSE(rstrt.get("FWIT"),
                      30.0 * 0.3 * 0.02 *
                      ((0.03 * 1.0) + (0.04 * 1.0)), 1.0e-10);

    BOOST_CHECK_CLOSE(rstrt.get("FGIT"),
                      30.2 * 0.3 * 0.02 *
                      ((0.03 * 1.0) + (0.04 * 1.0)), 1.0e-10);

    // Water cut
    BOOST_CHECK_CLOSE(rstrt.get("FWCT"),
                      (10.0 +        (efac_G *  20.0)) /
                      (10.0 + 10.1 + (efac_G * (20.0 + 20.1))), 1.0e-10);

    // Producing gas/oil ratio
    BOOST_CHECK_CLOSE(rstrt.get("FGOR"),
                      (10.2 + (efac_G * 20.2)) /
                      (10.1 + (efac_G * 20.1)), 1.0e-10);
}

BOOST_AUTO_TEST_SUITE_END()

// ####################################################################

namespace {
    void fill_surface_rates(const std::size_t id,
                            const double      sign,
                            data::Rates&      rates)
    {
        const auto topRate = id * 1000*sm3_pr_day();

        rates.set(data::Rates::opt::wat, sign * (topRate + 100*sm3_pr_day()));
        rates.set(data::Rates::opt::oil, sign * (topRate + 200*sm3_pr_day()));
        rates.set(data::Rates::opt::gas, sign * (topRate + 400*sm3_pr_day()));
    }

    std::size_t numSegProd01()
    {
        return 26;
    }

    data::Connection conn_results(const std::size_t connID,
                                  const std::size_t cellID,
                                  const double      sign)
    {
        auto res = data::Connection{};

        res.index = cellID;

        fill_surface_rates(connID, sign, res.rates);

        // Not meant to be realistic, other than possibly order of magnitude.
        res.pressure       = (200.0 + connID)*unit::barsa;
        res.reservoir_rate = (125.0 + connID)*sm3_pr_day();
        res.cell_pressure  = (250.0 + cellID)*unit::barsa;

        return res;
    }

    data::Segment seg_results(const std::size_t segID, const double sign)
    {
        auto res = data::Segment{};

        fill_surface_rates(segID, sign, res.rates);

        const auto pres_idx = Opm::data::SegmentPressures::Value::Pressure;
        res.pressures[pres_idx] = (100.0 + segID)*unit::barsa;

        res.segNumber = segID;

        return res;
    }

    std::unordered_map<std::size_t, data::Segment> prod01_seg_results()
    {
        auto res = std::unordered_map<std::size_t, data::Segment>{};

        // Flow's producer rates are negative (positive fluxes well -> reservoir).
        const auto sign = -1.0;

        for (auto nSeg = numSegProd01(), segID = 0*nSeg;
             segID < nSeg; ++segID)
        {
            res[segID + 1] = seg_results(segID + 1, sign);
        }

        return res;
    }

    std::vector<data::Connection> prod01_conn_results()
    {
        auto res = std::vector<data::Connection>{};
        res.reserve(26);

        const auto cellID = std::vector<std::size_t> {
             99, // IJK = (10, 10,  1)
            199, // IJK = (10, 10,  2)
            299, // IJK = (10, 10,  3)
            399, // IJK = (10, 10,  4)
            499, // IJK = (10, 10,  5)
            599, // IJK = (10, 10,  6)

            198, // IJK = ( 9, 10,  2)
            197, // IJK = ( 8, 10,  2)
            196, // IJK = ( 7, 10,  2)
            195, // IJK = ( 6, 10,  2)
            194, // IJK = ( 5, 10,  2)

            289, // IJK = (10,  9,  3)
            279, // IJK = (10,  8,  3)
            269, // IJK = (10,  7,  3)
            259, // IJK = (10,  6,  3)
            249, // IJK = (10,  5,  3)

            498, // IJK = ( 9, 10,  5)
            497, // IJK = ( 8, 10,  5)
            496, // IJK = ( 7, 10,  5)
            495, // IJK = ( 6, 10,  5)
            494, // IJK = ( 5, 10,  5)

            589, // IJK = (10,  9,  6)
            579, // IJK = (10,  8,  6)
            569, // IJK = (10,  7,  6)
            559, // IJK = (10,  6,  6)
            549, // IJK = (10,  5,  6)
        };

        // Flow's producer rates are negative (positive fluxes well -> reservoir).
        const auto sign = -1.0;

        for (auto nConn = cellID.size(), connID = 0*nConn;
             connID < nConn; ++connID)
        {
            res.push_back(conn_results(connID, cellID[connID], sign));
        }

        return res;
    }

    std::vector<data::Connection> inje01_conn_results()
    {
        auto res = std::vector<data::Connection>{};
        res.reserve(3);

        const auto cellID = std::vector<std::size_t> {
            600, // IJK = ( 1,  1,  7)
            700, // IJK = ( 1,  1,  8)
            800, // IJK = ( 1,  1,  9)
        };

        // Flow's injection rates are positive (positive fluxes well -> reservoir).
        const auto sign = +1.0;

        for (auto nConn = cellID.size(), connID = 0*nConn;
             connID < nConn; ++connID)
        {
            res.push_back(conn_results(connID, cellID[connID], sign));
        }

        return res;
    }

    std::string genKeyPROD01(const std::string& vector,
                             const std::size_t  segID)
    {
        return vector + ":PROD01:" + std::to_string(segID);
    }
} // Anonymous

data::Well SegmentResultHelpers::prod01_results()
{
    auto res = data::Well{};

    fill_surface_rates(0, -1.0, res.rates);

    res.bhp         = 123.45*unit::barsa;
    res.thp         = 60.221409*unit::barsa;
    res.temperature = 298.15;
    res.control     = 0;

    res.dynamicStatus = ::Opm::Well::Status::OPEN;

    res.connections = prod01_conn_results();
    res.segments    = prod01_seg_results();

    return res;
}

data::Well SegmentResultHelpers::inje01_results()
{
    auto res = data::Well{};

    fill_surface_rates(0, 1.0, res.rates);

    res.bhp         = 543.21*unit::barsa;
    res.thp         = 256.821*unit::barsa;
    res.temperature = 298.15;
    res.control     = 0;

    res.dynamicStatus = ::Opm::Well::Status::OPEN;

    res.connections = inje01_conn_results();

    return res;
}

// ====================================================================

BOOST_AUTO_TEST_SUITE(Restart_Segment)

BOOST_AUTO_TEST_CASE(Vectors_Present)
{
    const auto rstrt = calculateRestartVectorsSegment();

    for (const auto* vector : { "SGFR", "SOFR", "SPR", "SWFR"}) {
        for (auto nSeg = numSegProd01(), segID = 0*nSeg;
             segID < nSeg; ++segID)
        {
            BOOST_CHECK(rstrt.has(genKeyPROD01(vector, segID + 1)));
        }

        BOOST_CHECK(!rstrt.has(genKeyPROD01(vector, 27)));
        BOOST_CHECK(!rstrt.has(vector + std::string{":INJE01:1"}));
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(Pressure_Correct)
{
    const auto rstrt = calculateRestartVectorsSegment();
    for (auto nSeg = numSegProd01(), segID = 0*nSeg;
         segID < nSeg; ++segID)
    {
        const auto& key = genKeyPROD01("SPR", segID + 1);

        // Pressure value converted to METRIC output units (bars).
        BOOST_CHECK_CLOSE(rstrt.get(key), 100.0 + (segID + 1), 1.0e-10);
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(OilRate_Correct)
{
    const auto rstrt = calculateRestartVectorsSegment();
    for (auto nSeg = numSegProd01(), segID = 0*nSeg;
         segID < nSeg; ++segID)
    {
        const auto& key = genKeyPROD01("SOFR", segID + 1);

        // Producer rates positive in 'rstrt', converted to METRIC
        // output units (SM3/day).
        BOOST_CHECK_CLOSE(rstrt.get(key), 1000.0*(segID + 1) + 200, 1.0e-10);
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(GasRate_Correct)
{
    const auto rstrt = calculateRestartVectorsSegment();
    for (auto nSeg = numSegProd01(), segID = 0*nSeg;
         segID < nSeg; ++segID)
    {
        const auto& key = genKeyPROD01("SGFR", segID + 1);

        // Producer rates positive in 'rstrt', converted to METRIC
        // output units (SM3/day).
        BOOST_CHECK_CLOSE(rstrt.get(key), 1000.0*(segID + 1) + 400, 1.0e-10);
    }
}

// ====================================================================

BOOST_AUTO_TEST_CASE(WaterRate_Correct)
{
    const auto rstrt = calculateRestartVectorsSegment();
    for (auto nSeg = numSegProd01(), segID = 0*nSeg;
         segID < nSeg; ++segID)
    {
        const auto& key = genKeyPROD01("SWFR", segID + 1);

        // Producer rates positive in 'rstrt', converted to METRIC
        // output units (SM3/day).
        BOOST_CHECK_CLOSE(rstrt.get(key), 1000.0*(segID + 1) + 100, 1.0e-10);
    }
}

// ====================================================================

namespace {
    bool hasSegmentVariable_Prod01(const Opm::EclIO::ESmry* ecl_sum,
                                   const char*              vector,
                                   const int                segID)
    {
        const auto lookup_kw = genKeyPROD01(vector, segID);

        return ecl_sum_has_general_var(ecl_sum, lookup_kw);
    }

    double getSegmentVariable_Prod01(const Opm::EclIO::ESmry* ecl_sum,
                                     const int                timeIdx,
                                     const char*              vector,
                                     const int                segID)
    {
        const auto lookup_kw = genKeyPROD01(vector, segID);

        return ecl_sum_get_general_var(ecl_sum, timeIdx, lookup_kw);
    }
} // Anonymous

BOOST_AUTO_TEST_CASE(Write_Read)
{
    const setup config{"test.Restart.Segment.RW", "SOFR_TEST.DATA"};

    ::Opm::out::Summary writer {
        config.es, config.config, config.grid, config.schedule
    };

    SummaryState st(TimeService::now());
    writer.eval(st, 0, 0*day, config.wells, config.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep(st, 0, false);
    writer.eval(st, 1, 1*day, config.wells, config.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep(st, 1, false);
    writer.eval(st, 2, 2*day, config.wells, config.grp_nwrk, {}, {}, {}, {});
    writer.add_timestep(st, 2, false);
    writer.write();

    auto res = readsum("SOFR_TEST");
    const auto* resp = res.get();

    const int timeIdx = 2;

    // Rate Setup
    //
    // const auto topRate = id * 1000*sm3_pr_day();
    // rates.set(data::Rates::opt::wat, sign * (topRate + 100*sm3_pr_day()));
    // rates.set(data::Rates::opt::oil, sign * (topRate + 200*sm3_pr_day()));
    // rates.set(data::Rates::opt::gas, sign * (topRate + 400*sm3_pr_day()));
    //
    // Pressure Setup
    // res.pressure = (100.0 + segID)*unit::barsa;

    // Note: Producer rates reported as positive.

    // SOFR_TEST Summary Section:
    //
    //   SUMMARY
    //
    //   -- ALL
    //
    //   SOFR
    //     'PROD01'  1 /
    //     'PROD01'  10 /
    //     'PROD01'  21 /
    //   /
    //
    //   SGFR
    //     'PROD01' /
    //   /
    //
    //   SPR
    //     1*  10 /
    //   /
    //
    //   SWFR
    //   /

    // Segment 1: SOFR, SGFR, SWFR
    {
        const auto segID = 1;

        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SOFR", segID),
                          segID*1000.0 + 200.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 2: SGFR, SWFR
    {
        const auto segID = 2;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 3: SGFR, SWFR
    {
        const auto segID = 3;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 4: SGFR, SWFR
    {
        const auto segID = 4;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 5: SGFR, SWFR
    {
        const auto segID = 5;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 6: SGFR, SWFR
    {
        const auto segID = 6;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 7: SGFR, SWFR
    {
        const auto segID = 7;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 8: SGFR, SWFR
    {
        const auto segID = 8;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 9: SGFR, SWFR
    {
        const auto segID = 9;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 10: SOFR, SGFR, SWFR, SPR
    {
        const auto segID = 10;

        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SPR", segID),
                          100.0 + segID, 1.0e-10);
    }

    // Segment 11: SGFR, SWFR
    {
        const auto segID = 11;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 12: SGFR, SWFR
    {
        const auto segID = 12;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 13: SGFR, SWFR
    {
        const auto segID = 13;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 14: SGFR, SWFR
    {
        const auto segID = 14;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 15: SGFR, SWFR
    {
        const auto segID = 15;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 16: SGFR, SWFR
    {
        const auto segID = 16;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 17: SGFR, SWFR
    {
        const auto segID = 17;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 18: SGFR, SWFR
    {
        const auto segID = 18;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 19: SGFR, SWFR
    {
        const auto segID = 19;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 20: SGFR, SWFR
    {
        const auto segID = 20;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 21: SOFR, SGFR, SWFR
    {
        const auto segID = 21;

        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SOFR", segID),
                          segID*1000.0 + 200.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 22: SGFR, SWFR
    {
        const auto segID = 22;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 23: SGFR, SWFR
    {
        const auto segID = 23;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 24: SGFR, SWFR
    {
        const auto segID = 24;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 25: SGFR, SWFR
    {
        const auto segID = 25;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 26: SGFR, SWFR
    {
        const auto segID = 26;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK( hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SGFR", segID),
                          segID*1000.0 + 400.0, 1.0e-10);

        BOOST_CHECK_CLOSE(getSegmentVariable_Prod01(resp, timeIdx, "SWFR", segID),
                          segID*1000.0 + 100.0, 1.0e-10);
    }

    // Segment 256: No such segment
    {
        const auto segID = 256;

        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SOFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SGFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SWFR", segID));
        BOOST_CHECK(!hasSegmentVariable_Prod01(resp, "SPR" , segID));
    }
}

BOOST_AUTO_TEST_SUITE_END()

// =====================================================================

BOOST_AUTO_TEST_SUITE(Summary_State)

BOOST_AUTO_TEST_CASE(SummaryState_TOTAL) {
    SummaryState st(TimeService::now());
    st.update("FOPR", 100);
    BOOST_CHECK_EQUAL(st.get("FOPR"), 100);
    st.update("FOPR", 100);
    BOOST_CHECK_EQUAL(st.get("FOPR"), 100);
    st.update("WOPR:OP1", 100);
    BOOST_CHECK_EQUAL(st.get("WOPR:OP1"), 100);
    st.update("WOPR:OP1", 100);
    BOOST_CHECK_EQUAL(st.get("WOPR:OP1"), 100);

    st.update("FOPT", 100);
    BOOST_CHECK_EQUAL(st.get("FOPT"), 100);
    st.update("FOPT", 100);
    BOOST_CHECK_EQUAL(st.get("FOPT"), 200);
    st.update("WOPT:OP1", 100);
    BOOST_CHECK_EQUAL(st.get("WOPT:OP1"), 100);
    st.update("WOPT:OP1", 100);
    BOOST_CHECK_EQUAL(st.get("WOPT:OP1"), 200);

    st.update_well_var("OP1", "WOPR", 100);
    BOOST_CHECK_EQUAL(st.get_well_var("OP1", "WOPR"), 100);
    st.update_well_var("OP1", "WOPR", 100);
    BOOST_CHECK_EQUAL(st.get_well_var("OP1", "WOPR"), 100);

    st.update_well_var("OP1", "WWCT", 0.50);
    BOOST_CHECK_EQUAL(st.get_well_var("OP1", "WWCT"), 0.50);
    st.update_well_var("OP1", "WWCT", 0.50);
    BOOST_CHECK_EQUAL(st.get_well_var("OP1", "WWCT"), 0.50);

    st.update_well_var("OP1", "WOPT", 100);
    BOOST_CHECK_EQUAL(st.get_well_var("OP1", "WOPT"), 100);
    st.update_well_var("OP1", "WOPT", 100);
    BOOST_CHECK_EQUAL(st.get_well_var("OP1", "WOPT"), 200);

    st.update_well_var("OP1", "WOPTH", 100);
    BOOST_CHECK_EQUAL(st.get_well_var("OP1", "WOPTH"), 100);
    st.update_well_var("OP1", "WOPTH", 100);
    BOOST_CHECK_EQUAL(st.get_well_var("OP1", "WOPTH"), 200);

    st.update_group_var("G1", "GOPTH", 100);
    BOOST_CHECK_EQUAL(st.get_group_var("G1", "GOPTH"), 100);
    st.update_group_var("G1", "GOPTH", 100);
    BOOST_CHECK_EQUAL(st.get_group_var("G1", "GOPTH"), 200);

    st.update("FOPTH", 100);
    BOOST_CHECK_EQUAL(st.get("FOPTH"), 100);
    st.update("FOPTH", 100);
    BOOST_CHECK_EQUAL(st.get("FOPTH"), 200);

    st.update("WGPTS", 100);
    BOOST_CHECK_EQUAL(st.get("WGPTS"), 100);
    st.update("WGPTS", 100);
    BOOST_CHECK_EQUAL(st.get("WGPTS"), 200);

    st.update_elapsed(100);
    BOOST_CHECK_EQUAL(st.get_elapsed(), 100);
    st.update_elapsed(100);
    BOOST_CHECK_EQUAL(st.get_elapsed(), 200);
}

namespace {
void test_serialize(const SummaryState& st) {
    SummaryState st2(TimeService::now());
    auto serial = st.serialize();
    st2.deserialize(serial);
    BOOST_CHECK( st == st2 );

    st2.update_elapsed(1234567.09);
    st2.update("FOPT", 200);
    st2.deserialize(serial);
    BOOST_CHECK(st == st2);
}
}

BOOST_AUTO_TEST_CASE(serialize_sumary_state) {
    SummaryState st(TimeService::now());
    test_serialize(st);

    st.update_elapsed(1000);
    test_serialize(st);

    st.update("FOPT", 100);
    test_serialize(st);

    st.update("FGPT", 100);
    test_serialize(st);

    st.update_well_var("OP_1", "WOPR", 1000);
    test_serialize(st);

    st.update_well_var("OP_2", "WGOR", 0.67);
    test_serialize(st);

    st.update_group_var("G1", "GOPR", 1000);
    test_serialize(st);

    st.update_group_var("G2", "GGOR", 0.67);
    test_serialize(st);

}


BOOST_AUTO_TEST_SUITE_END()
