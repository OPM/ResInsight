/*
  Copyright 2019 Equinor ASA.
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

#include <opm/output/eclipse/Summary.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/Location.hpp>

#include <opm/parser/eclipse/EclipseState/EclipseState.hpp>
#include <opm/parser/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group/Group.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQContext.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/Well.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellProductionProperties.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellInjectionProperties.hpp>
#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Units/Units.hpp>

#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/io/eclipse/OutputStream.hpp>

#include <opm/output/data/Wells.hpp>
#include <opm/output/data/Groups.hpp>
#include <opm/output/eclipse/RegionCache.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cctype>
#include <ctime>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <numeric>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
    struct ParamCTorArgs
    {
        std::string kw;
        Opm::EclIO::SummaryNode::Type type;
    };

    using p_cmode = Opm::Group::ProductionCMode;
    const std::map<p_cmode, int> pCModeToPCntlMode = {
                                                    {p_cmode::NONE,       0},
                                                    {p_cmode::ORAT,       1},
                                                    {p_cmode::WRAT,       2},
                                                    {p_cmode::GRAT,       3},
                                                    {p_cmode::LRAT,       4},
                                                    {p_cmode::CRAT,       9},
                                                    {p_cmode::RESV,       5},
                                                    {p_cmode::PRBL,       6},
                                                    {p_cmode::FLD,        0}, // same as NONE

    };

    using i_cmode = Opm::Group::InjectionCMode;
    const std::map<i_cmode, int> iCModeToICntlMode = {
                                                    {i_cmode::NONE,       0},
                                                    {i_cmode::RATE,       1},
                                                    {i_cmode::RESV,       2},
                                                    {i_cmode::REIN,       3},
                                                    {i_cmode::VREP,       4},
                                                    {i_cmode::FLD,        0},  // same as NONE
                                                    {i_cmode::SALE,       0},  // not used in E100
    };

    std::vector<ParamCTorArgs> requiredRestartVectors()
    {
        using Type = ::Opm::EclIO::SummaryNode::Type;

        return {
            // Production
            ParamCTorArgs{ "OPR" , Type::Rate },
            ParamCTorArgs{ "WPR" , Type::Rate },
            ParamCTorArgs{ "GPR" , Type::Rate },
            ParamCTorArgs{ "VPR" , Type::Rate },
            ParamCTorArgs{ "OPP" , Type::Rate },
            ParamCTorArgs{ "WPP" , Type::Rate },
            ParamCTorArgs{ "GPP" , Type::Rate },
            ParamCTorArgs{ "OPT" , Type::Total },
            ParamCTorArgs{ "WPT" , Type::Total },
            ParamCTorArgs{ "GPT" , Type::Total },
            ParamCTorArgs{ "VPT" , Type::Total },
            ParamCTorArgs{ "OPTH", Type::Total },
            ParamCTorArgs{ "WPTH", Type::Total },
            ParamCTorArgs{ "GPTH", Type::Total },

            // Flow rate ratios (production)
            ParamCTorArgs{ "WCT" , Type::Ratio },
            ParamCTorArgs{ "GOR" , Type::Ratio },

            // injection
            ParamCTorArgs{ "WIR" , Type::Rate },
            ParamCTorArgs{ "GIR" , Type::Rate },
            ParamCTorArgs{ "OPI" , Type::Rate },
            ParamCTorArgs{ "WPI" , Type::Rate },
            ParamCTorArgs{ "GPI" , Type::Rate },
            ParamCTorArgs{ "WIT" , Type::Total },
            ParamCTorArgs{ "GIT" , Type::Total },
            ParamCTorArgs{ "VIT" , Type::Total },
            ParamCTorArgs{ "WITH", Type::Total },
            ParamCTorArgs{ "GITH", Type::Total },
        };
    }

    std::vector<Opm::EclIO::SummaryNode>
    requiredRestartVectors(const ::Opm::Schedule& sched)
    {
        const auto& vectors = requiredRestartVectors();
        const std::vector<ParamCTorArgs> extra_well_vectors {
            { "WBHP",  Opm::EclIO::SummaryNode::Type::Pressure },
            { "WGVIR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "WWVIR", Opm::EclIO::SummaryNode::Type::Rate     },
        };
        const std::vector<ParamCTorArgs> extra_group_vectors {
            { "GMCTG", Opm::EclIO::SummaryNode::Type::Mode },
            { "GMCTP", Opm::EclIO::SummaryNode::Type::Mode },
            { "GMCTW", Opm::EclIO::SummaryNode::Type::Mode },
        };
        const std::vector<ParamCTorArgs> extra_field_vectors {
            { "FMCTG", Opm::EclIO::SummaryNode::Type::Mode },
            { "FMCTP", Opm::EclIO::SummaryNode::Type::Mode },
            { "FMCTW", Opm::EclIO::SummaryNode::Type::Mode },
        };

        std::vector<Opm::EclIO::SummaryNode> entities {} ;

        auto makeEntities = [&vectors, &entities]
            (const char         kwpref,
             const Opm::EclIO::SummaryNode::Category cat,
             const std::string& name) -> void
        {
            for (const auto& vector : vectors) {
                entities.push_back({kwpref + vector.kw, cat, vector.type, name, Opm::EclIO::SummaryNode::default_number });
            }
        };

        auto makeExtraEntities = [&entities]
            (const std::vector<ParamCTorArgs>& extra_vectors,
             const Opm::EclIO::SummaryNode::Category category,
             const std::string& wgname) -> void
        {
            for (const auto &extra_vector : extra_vectors) {
                entities.push_back({ extra_vector.kw, category, extra_vector.type, wgname, Opm::EclIO::SummaryNode::default_number });
            }
        };

        for (const auto& well_name : sched.wellNames()) {
            makeEntities('W', Opm::EclIO::SummaryNode::Category::Well, well_name);
            makeExtraEntities(extra_well_vectors, Opm::EclIO::SummaryNode::Category::Well, well_name);
        }

        for (const auto& grp_name : sched.groupNames()) {
            if (grp_name != "FIELD") {
                makeEntities('G', Opm::EclIO::SummaryNode::Category::Group, grp_name);
                makeExtraEntities(extra_group_vectors, Opm::EclIO::SummaryNode::Category::Group, grp_name);
            }
        }

        makeEntities('F', Opm::EclIO::SummaryNode::Category::Field, "FIELD");
        makeExtraEntities(extra_field_vectors, Opm::EclIO::SummaryNode::Category::Field, "FIELD");

        return entities;
    }

    std::vector<Opm::EclIO::SummaryNode>
    requiredSegmentVectors(const ::Opm::Schedule& sched)
    {
        std::vector<Opm::EclIO::SummaryNode> ret {};

        constexpr Opm::EclIO::SummaryNode::Category category { Opm::EclIO::SummaryNode::Category::Segment };
        const std::vector<std::pair<std::string,Opm::EclIO::SummaryNode::Type>> requiredVectors {
            { "SOFR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "SGFR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "SWFR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "SPR",  Opm::EclIO::SummaryNode::Type::Pressure },
        };

        auto makeVectors =
            [&](const std::string& well,
                const int          segNumber) -> void
        {
            for (const auto &requiredVector : requiredVectors) {
                ret.push_back({requiredVector.first, category, requiredVector.second, well, segNumber});
            }
        };

        for (const auto& wname : sched.wellNames()) {
            const auto& well = sched.getWellatEnd(wname);

            if (! well.isMultiSegment()) {
                // Don't allocate MS summary vectors for non-MS wells.
                continue;
            }

            const auto nSeg = well.getSegments().size();

            for (auto segID = 0*nSeg; segID < nSeg; ++segID) {
                makeVectors(wname, segID + 1); // One-based
            }
        }

        return ret;
    }


Opm::TimeStampUTC make_sim_time(const Opm::Schedule& sched, const Opm::SummaryState& st, double sim_step) {
    auto elapsed = st.get_elapsed() + sim_step;
    return Opm::TimeStampUTC( sched.getStartTime() )  + std::chrono::duration<double>(elapsed);
}



/*
 * This class takes simulator state and parser-provided information and
 * orchestrates ert to write simulation results as requested by the SUMMARY
 * section in eclipse. The implementation is somewhat compact as a lot of the
 * requested output may be similar-but-not-quite. Through various techniques
 * the compiler writes a lot of this code for us.
 */


using rt = Opm::data::Rates::opt;
using measure = Opm::UnitSystem::measure;
constexpr const bool injector = true;
constexpr const bool producer = false;
constexpr const bool polymer = true;

/* Some numerical value with its unit tag embedded to enable caller to apply
 * unit conversion. This removes a lot of boilerplate. ad-hoc solution to poor
 * unit support in general.
 */
measure div_unit( measure denom, measure div ) {
    if( denom == measure::gas_surface_rate &&
        div   == measure::liquid_surface_rate )
        return measure::gas_oil_ratio;

    if( denom == measure::liquid_surface_rate &&
        div   == measure::gas_surface_rate )
        return measure::oil_gas_ratio;

    if( denom == measure::liquid_surface_rate &&
        div   == measure::liquid_surface_rate )
        return measure::water_cut;

    if( denom == measure::liquid_surface_rate &&
        div   == measure::time )
        return measure::liquid_surface_volume;

    if( denom == measure::gas_surface_rate &&
        div   == measure::time )
        return measure::gas_surface_volume;

    if( denom == measure::mass_rate &&
        div   == measure::time )
        return measure::mass;

    return measure::identity;
}

measure mul_unit( measure lhs, measure rhs ) {
    if( lhs == rhs ) return lhs;

    if( ( lhs == measure::liquid_surface_rate && rhs == measure::time ) ||
        ( rhs == measure::liquid_surface_rate && lhs == measure::time ) )
        return measure::liquid_surface_volume;

    if( ( lhs == measure::gas_surface_rate && rhs == measure::time ) ||
        ( rhs == measure::gas_surface_rate && lhs == measure::time ) )
        return measure::gas_surface_volume;

    if( ( lhs == measure::rate && rhs == measure::time ) ||
        ( rhs == measure::rate && lhs == measure::time ) )
        return measure::volume;

    if(  lhs == measure::mass_rate && rhs == measure::time)
        return measure::mass;

    return lhs;
}

struct quantity {
    double value;
    Opm::UnitSystem::measure unit;

    quantity operator+( const quantity& rhs ) const {
        assert( this->unit == rhs.unit );
        return { this->value + rhs.value, this->unit };
    }

    quantity operator*( const quantity& rhs ) const {
        return { this->value * rhs.value, mul_unit( this->unit, rhs.unit ) };
    }

    quantity operator/( const quantity& rhs ) const {
        const auto res_unit = div_unit( this->unit, rhs.unit );

        if( rhs.value == 0 ) return { 0.0, res_unit };
        return { this->value / rhs.value, res_unit };
    }

    quantity operator/( double divisor ) const {
        if( divisor == 0 ) return { 0.0, this->unit };
        return { this->value / divisor , this->unit };
    }

    quantity& operator/=( double divisor ) {
        if( divisor == 0 )
            this->value = 0;
        else
            this->value /= divisor;

        return *this;
    }


    quantity operator-( const quantity& rhs) const {
        return { this->value - rhs.value, this->unit };
    }
};


/*
 * All functions must have the same parameters, so they're gathered in a struct
 * and functions use whatever information they care about.
 *
 * schedule_wells are wells from the deck, provided by opm-parser. active_index
 * is the index of the block in question. wells is simulation data.
 */
struct fn_args {
    const std::vector<Opm::Well>& schedule_wells;
    const std::string group_name;
    double duration;
    const int sim_step;
    int  num;
    const Opm::SummaryState& st;
    const Opm::data::Wells& wells;
    const Opm::data::Group& group;
    const Opm::out::RegionCache& regionCache;
    const Opm::EclipseGrid& grid;
    const std::vector< std::pair< std::string, double > > eff_factors;
};

/* Since there are several enums in opm scattered about more-or-less
 * representing the same thing. Since functions use template parameters to
 * expand into the actual implementations we need a static way to determine
 * what unit to tag the return value with.
 */
template< rt > constexpr
measure rate_unit() { return measure::liquid_surface_rate; }
template< Opm::Phase > constexpr
measure rate_unit() { return measure::liquid_surface_rate; }

template<> constexpr
measure rate_unit< rt::gas >() { return measure::gas_surface_rate; }
template<> constexpr
measure rate_unit< Opm::Phase::GAS >() { return measure::gas_surface_rate; }

template<> constexpr
measure rate_unit< rt::solvent >() { return measure::gas_surface_rate; }

template<> constexpr
measure rate_unit< rt::reservoir_water >() { return measure::rate; }

template<> constexpr
measure rate_unit< rt::reservoir_oil >() { return measure::rate; }

template<> constexpr
measure rate_unit< rt::reservoir_gas >() { return measure::rate; }

template<> constexpr
measure rate_unit < rt::productivity_index_water > () { return measure::liquid_productivity_index; }

template<> constexpr
measure rate_unit < rt::productivity_index_oil > () { return measure::liquid_productivity_index; }

template<> constexpr
measure rate_unit < rt::productivity_index_gas > () { return measure::gas_productivity_index; }

template<> constexpr
measure rate_unit< rt::well_potential_water >() { return measure::liquid_surface_rate; }

template<> constexpr
measure rate_unit< rt::well_potential_oil >() { return measure::liquid_surface_rate; }

template<> constexpr
measure rate_unit< rt::well_potential_gas >() { return measure::gas_surface_rate; }

double efac( const std::vector<std::pair<std::string,double>>& eff_factors, const std::string& name ) {
    auto it = std::find_if( eff_factors.begin(), eff_factors.end(),
                            [&] ( const std::pair< std::string, double > elem )
                            { return elem.first == name; }
                          );

    return (it != eff_factors.end()) ? it->second : 1;
}

template< rt phase, bool injection = true, bool polymer = false >
inline quantity rate( const fn_args& args ) {
    double sum = 0.0;

    for( const auto& sched_well : args.schedule_wells ) {
        const auto& name = sched_well.name();
        if( args.wells.count( name ) == 0 ) continue;

        double eff_fac = efac( args.eff_factors, name );

        double concentration = polymer
                             ? sched_well.getPolymerProperties().m_polymerConcentration
                             : 1;

        const auto v = args.wells.at(name).rates.get(phase, 0.0) * eff_fac * concentration;

        if( ( v > 0 ) == injection )
            sum += v;
    }

    if( !injection ) sum *= -1;

    if( polymer ) return { sum, measure::mass_rate };
    return { sum, rate_unit< phase >() };
}

template< bool injection >
inline quantity flowing( const fn_args& args ) {
    const auto& wells = args.wells;
    auto pred = [&wells]( const Opm::Well& w ) {
        const auto& name = w.name();
        return w.isInjector( ) == injection
            && wells.count( name ) > 0
            && wells.at( name ).flowing();
    };

    return { double( std::count_if( args.schedule_wells.begin(),
                                    args.schedule_wells.end(),
                                    pred ) ),
             measure::identity };
}

template< rt phase, bool injection = true, bool polymer = false >
inline quantity crate( const fn_args& args ) {
    const quantity zero = { 0, rate_unit< phase >() };
    // The args.num value is the literal value which will go to the
    // NUMS array in the eclispe SMSPEC file; the values in this array
    // are offset 1 - whereas we need to use this index here to look
    // up a completion with offset 0.
    const size_t global_index = args.num - 1;
    if( args.schedule_wells.empty() ) return zero;

    const auto& well = args.schedule_wells.front();
    const auto& name = well.name();
    if( args.wells.count( name ) == 0 ) return zero;

    const auto& well_data = args.wells.at( name );
    const auto& completion = std::find_if( well_data.connections.begin(),
                                           well_data.connections.end(),
                                           [=]( const Opm::data::Connection& c ) {
                                                return c.index == global_index;
                                           } );

    if( completion == well_data.connections.end() ) return zero;

    double eff_fac = efac( args.eff_factors, name );
    double concentration = polymer
                           ? well.getPolymerProperties().m_polymerConcentration
                           : 1;

    auto v = completion->rates.get( phase, 0.0 ) * eff_fac * concentration;
    if( ( v > 0 ) != injection ) return zero;
    if( !injection ) v *= -1;

    if( polymer ) return { v, measure::mass_rate };
    return { v, rate_unit< phase >() };
}

template< rt phase, bool polymer = false >
inline quantity srate( const fn_args& args ) {
    const quantity zero = { 0, rate_unit< phase >() };
    // The args.num value is the literal value which will go to the
    // NUMS array in the eclispe SMSPEC file; the values in this array
    // are offset 1 - whereas we need to use this index here to look
    // up a completion with offset 0.
    const size_t segNumber = args.num;
    if( args.schedule_wells.empty() ) return zero;

    const auto& well = args.schedule_wells.front();
    const auto& name = well.name();
    if( args.wells.count( name ) == 0 ) return zero;

    const auto& well_data = args.wells.at( name );

    const auto& segment = well_data.segments.find(segNumber);

    if( segment == well_data.segments.end() ) return zero;

    double eff_fac = efac( args.eff_factors, name );
    double concentration = polymer
                           ? well.getPolymerProperties().m_polymerConcentration
                           : 1;

    auto v = segment->second.rates.get( phase, 0.0 ) * eff_fac * concentration;
    //switch sign of rate - opposite convention in flow vs eclipse
    v *= -1;

    if( polymer ) return { v, measure::mass_rate };
    return { v, rate_unit< phase >() };
}

inline quantity trans_factors ( const fn_args& args ) {
    const quantity zero = { 0, measure::transmissibility };

    if( args.schedule_wells.empty() ) return zero;
    // Like completion rate we need to look
    // up a connection with offset 0.
    const size_t global_index = args.num - 1;

    const auto& well = args.schedule_wells.front();
    const auto& name = well.name();
    if( args.wells.count( name ) == 0 ) return zero;

    const auto& grid = args.grid;
    const auto& connections = well.getConnections();

    const auto& connection = std::find_if(
        connections.begin(),
        connections.end(),
        [=]( const Opm::Connection& c ) {
            return grid.getGlobalIndex(c.getI(), c.getJ(), c.getK()) == global_index;
        } );

    if( connection == connections.end() ) return zero;

    const auto& v = connection->CF();
    return { v, measure::transmissibility };
}

template <Opm::data::SegmentPressures::Value ix>
inline quantity segpress ( const fn_args& args ) {
    const quantity zero = { 0, measure::pressure };

    if( args.schedule_wells.empty() ) return zero;
    // Like completion rate we need to look
    // up a connection with offset 0.
    const size_t segNumber = args.num;
    if( args.schedule_wells.empty() ) return zero;

    const auto& well = args.schedule_wells.front();
    const auto& name = well.name();
    if( args.wells.count( name ) == 0 ) return zero;

    const auto& well_data = args.wells.at( name );

    const auto& segment = well_data.segments.find(segNumber);

    if( segment == well_data.segments.end() ) return zero;


    return { segment->second.pressures[ix], measure::pressure };
}

inline quantity bhp( const fn_args& args ) {
    const quantity zero = { 0, measure::pressure };
    if( args.schedule_wells.empty() ) return zero;

    const auto p = args.wells.find( args.schedule_wells.front().name() );
    if( p == args.wells.end() ) return zero;

    return { p->second.bhp, measure::pressure };
}

inline quantity thp( const fn_args& args ) {
    const quantity zero = { 0, measure::pressure };
    if( args.schedule_wells.empty() ) return zero;

    const auto p = args.wells.find( args.schedule_wells.front().name() );
    if( p == args.wells.end() ) return zero;

    return { p->second.thp, measure::pressure };
}

inline quantity bhp_history( const fn_args& args ) {
    if( args.schedule_wells.empty() ) return { 0.0, measure::pressure };

    const Opm::Well& sched_well = args.schedule_wells.front();

    double bhp_hist;
    if ( sched_well.isProducer(  ) )
        bhp_hist = sched_well.getProductionProperties().BHPH;
    else
        bhp_hist = sched_well.getInjectionProperties().BHPH;

    return { bhp_hist, measure::pressure };
}

inline quantity thp_history( const fn_args& args ) {
    if( args.schedule_wells.empty() ) return { 0.0, measure::pressure };

    const Opm::Well& sched_well = args.schedule_wells.front();

    double thp_hist;
    if ( sched_well.isProducer() )
       thp_hist = sched_well.getProductionProperties().THPH;
    else
       thp_hist = sched_well.getInjectionProperties().THPH;

    return { thp_hist, measure::pressure };
}

template< Opm::Phase phase >
inline quantity production_history( const fn_args& args ) {
    /*
     * For well data, looking up historical rates (both for production and
     * injection) before simulation actually starts is impossible and
     * nonsensical. We therefore default to writing zero (which is what eclipse
     * seems to do as well).
     */

    double sum = 0.0;
    for( const auto& sched_well : args.schedule_wells ){

        double eff_fac = efac( args.eff_factors, sched_well.name() );
        sum += sched_well.production_rate( args.st, phase ) * eff_fac;
    }


    return { sum, rate_unit< phase >() };
}

template< Opm::Phase phase >
inline quantity injection_history( const fn_args& args ) {

    double sum = 0.0;
    for( const auto& sched_well : args.schedule_wells ){
        double eff_fac = efac( args.eff_factors, sched_well.name() );
        sum += sched_well.injection_rate( args.st, phase ) * eff_fac;
    }


    return { sum, rate_unit< phase >() };
}

inline quantity res_vol_production_target( const fn_args& args ) {

    double sum = 0.0;
    for( const Opm::Well& sched_well : args.schedule_wells )
        if (sched_well.getProductionProperties().predictionMode)
            sum += sched_well.getProductionProperties().ResVRate.getSI();

    return { sum, measure::rate };
}

inline quantity duration( const fn_args& args ) {
    return { args.duration, measure::time };
}

template<rt phase , bool injection>
quantity region_rate( const fn_args& args ) {
    double sum = 0;
    const auto& well_connections = args.regionCache.connections( args.num );

    for (const auto& pair : well_connections) {

        double eff_fac = efac( args.eff_factors, pair.first );

        double rate = args.wells.get( pair.first , pair.second , phase ) * eff_fac;

        // We are asking for the production rate in an injector - or
        // opposite. We just clamp to zero.
        if ((rate > 0) != injection)
            rate = 0;

        sum += rate;
    }

    if( injection )
        return { sum, rate_unit< phase >() };
    else
        return { -sum, rate_unit< phase >() };
}

template < rt phase, bool outputProducer = true, bool outputInjector = true>
inline quantity potential_rate( const fn_args& args ) {
    double sum = 0.0;

    for( const auto& sched_well : args.schedule_wells ) {
        const auto& name = sched_well.name();
        if( args.wells.count( name ) == 0 ) continue;

        if (sched_well.isInjector() && outputInjector) {
	    const auto v = args.wells.at(name).rates.get(phase, 0.0);
	    sum += v;
	}
	else if (sched_well.isProducer() && outputProducer) {
	    const auto v = args.wells.at(name).rates.get(phase, 0.0);
	    sum += v;
	}
    }

    return { sum, rate_unit< phase >() };
}

template < bool isGroup, bool Producer, bool waterInjector, bool gasInjector>
inline quantity group_control( const fn_args& args ) {

    std::string g_name = "";
    if (isGroup) {
        const quantity zero = { static_cast<double>(0), Opm::UnitSystem::measure::identity};
        if( args.group_name.empty() ) return zero;

        g_name = args.group_name;
    }
    else {
        g_name = "FIELD";
    }

    int cntl_mode = 0;

    // production control
    if (Producer) {
        const auto it_g = args.group.find(g_name);
        if (it_g != args.group.end()) {
            const auto& value = it_g->second.currentProdConstraint;
            auto it_c = pCModeToPCntlMode.find(value);
            if (it_c == pCModeToPCntlMode.end()) {
                std::stringstream str;
                str << "unknown control CMode: " << static_cast<int>(value);
                throw std::invalid_argument(str.str());
            }
            cntl_mode = it_c->second;
        }
    }
    // water injection control
    else if (waterInjector){
        const auto it_g = args.group.find(g_name);
        if (it_g != args.group.end()) {
            const auto& value = it_g->second.currentWaterInjectionConstraint;
            auto it_c = iCModeToICntlMode.find(value);
            if (it_c == iCModeToICntlMode.end()) {
                std::stringstream str;
                str << "unknown control CMode: " << static_cast<int>(value);
                throw std::invalid_argument(str.str());
            }
            cntl_mode = it_c->second;
        }
    }

    // gas injection control
    else if (gasInjector){
        const auto it_g = args.group.find(g_name);
        if (it_g != args.group.end()) {
            const auto& value = it_g->second.currentGasInjectionConstraint;
            auto it_c = iCModeToICntlMode.find(value);
            if (it_c == iCModeToICntlMode.end()) {
                std::stringstream str;
                str << "unknown control CMode: " << static_cast<int>(value);
                throw std::invalid_argument(str.str());
            }
            cntl_mode = it_c->second;
        }
    }

    return {static_cast<double>(cntl_mode), Opm::UnitSystem::measure::identity};
}

namespace {
    bool well_control_mode_defined(const ::Opm::data::Well& xw)
    {
        using PMode = ::Opm::Well::ProducerCMode;
        using IMode = ::Opm::Well::InjectorCMode;

        const auto& curr = xw.current_control;

        return (curr.isProducer && (curr.prod != PMode::CMODE_UNDEFINED))
            || (!curr.isProducer && (curr.inj != IMode::CMODE_UNDEFINED));
    }
}

inline quantity well_control_mode( const fn_args& args ) {
    const auto unit = Opm::UnitSystem::measure::identity;

    if (args.schedule_wells.empty()) {
        // No wells.  Possibly determining pertinent unit of measure
        // during SMSPEC configuration.
        return { 0.0, unit };
    }

    const auto& well = args.schedule_wells.front();
    auto xwPos = args.wells.find(well.name());
    if (xwPos == args.wells.end()) {
        // No dynamic results for 'well'.  Treat as shut/stopped.
        return { 0.0, unit };
    }

    if (! well_control_mode_defined(xwPos->second)) {
        // No dynamic control mode defined.  Use input control.
        const auto wmctl = ::Opm::eclipseControlMode(well, args.st);

        return { static_cast<double>(wmctl), unit };
    }

    // Well has simulator-provided active control mode.  Pick the
    // appropriate value depending on well type (producer/injector).
    const auto& curr = xwPos->second.current_control;
    const auto wmctl = curr.isProducer
        ? ::Opm::eclipseControlMode(curr.prod, well.getStatus())
        : ::Opm::eclipseControlMode(curr.inj, well.injectorType(),
                                    well.getStatus());

    return { static_cast<double>(wmctl), unit };
}

/*
 * A small DSL, really poor man's function composition, to avoid massive
 * repetition when declaring the handlers for each individual keyword. bin_op
 * and its aliases will apply the pair of functions F and G that all take const
 * fn_args& and return quantity, making them composable.
 */
template< typename F, typename G, typename Op >
struct bin_op {
    bin_op( F fn, G gn = {} ) : f( fn ), g( gn ) {}
    quantity operator()( const fn_args& args ) const {
        return Op()( f( args ), g( args ) );
    }

    private:
        F f;
        G g;
};

template< typename F, typename G >
auto mul( F f, G g ) -> bin_op< F, G, std::multiplies< quantity > >
{ return { f, g }; }

template< typename F, typename G >
auto sum( F f, G g ) -> bin_op< F, G, std::plus< quantity > >
{ return { f, g }; }

template< typename F, typename G >
auto div( F f, G g ) -> bin_op< F, G, std::divides< quantity > >
{ return { f, g }; }

template< typename F, typename G >
auto sub( F f, G g ) -> bin_op< F, G, std::minus< quantity > >
{ return { f, g }; }

using ofun = std::function< quantity( const fn_args& ) >;

static const std::unordered_map< std::string, ofun > funs = {
    { "WWIR", rate< rt::wat, injector > },
    { "WOIR", rate< rt::oil, injector > },
    { "WGIR", rate< rt::gas, injector > },
    { "WNIR", rate< rt::solvent, injector > },
    { "WCIR", rate< rt::wat, injector, polymer > },
    { "WVIR", sum( sum( rate< rt::reservoir_water, injector >, rate< rt::reservoir_oil, injector > ),
                       rate< rt::reservoir_gas, injector > ) },

    { "WWIT", mul( rate< rt::wat, injector >, duration ) },
    { "WOIT", mul( rate< rt::oil, injector >, duration ) },
    { "WGIT", mul( rate< rt::gas, injector >, duration ) },
    { "WNIT", mul( rate< rt::solvent, injector >, duration ) },
    { "WCIT", mul( rate< rt::wat, injector, polymer >, duration ) },
    { "WVIT", mul( sum( sum( rate< rt::reservoir_water, injector >, rate< rt::reservoir_oil, injector > ),
                        rate< rt::reservoir_gas, injector > ), duration ) },

    { "WWPR", rate< rt::wat, producer > },
    { "WOPR", rate< rt::oil, producer > },
    { "WGPR", rate< rt::gas, producer > },
    { "WNPR", rate< rt::solvent, producer > },

    { "WGPRS", rate< rt::dissolved_gas, producer > },
    { "WGPRF", sub( rate< rt::gas, producer >, rate< rt::dissolved_gas, producer > ) },
    { "WOPRS", rate< rt::vaporized_oil, producer > },
    { "WOPRF", sub (rate < rt::oil, producer >, rate< rt::vaporized_oil, producer > )  },
    { "WVPR", sum( sum( rate< rt::reservoir_water, producer >, rate< rt::reservoir_oil, producer > ),
                   rate< rt::reservoir_gas, producer > ) },
    { "WGVPR", rate< rt::reservoir_gas, producer > },

    { "WLPR", sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) },
    { "WWPT", mul( rate< rt::wat, producer >, duration ) },
    { "WOPT", mul( rate< rt::oil, producer >, duration ) },
    { "WGPT", mul( rate< rt::gas, producer >, duration ) },
    { "WNPT", mul( rate< rt::solvent, producer >, duration ) },
    { "WLPT", mul( sum( rate< rt::wat, producer >, rate< rt::oil, producer > ),
                   duration ) },

    { "WGPTS", mul( rate< rt::dissolved_gas, producer >, duration )},
    { "WGPTF", sub( mul( rate< rt::gas, producer >, duration ),
                        mul( rate< rt::dissolved_gas, producer >, duration ))},
    { "WOPTS", mul( rate< rt::vaporized_oil, producer >, duration )},
    { "WOPTF", sub( mul( rate< rt::oil, producer >, duration ),
                        mul( rate< rt::vaporized_oil, producer >, duration ))},
    { "WVPT", mul( sum( sum( rate< rt::reservoir_water, producer >, rate< rt::reservoir_oil, producer > ),
                        rate< rt::reservoir_gas, producer > ), duration ) },

    { "WWCT", div( rate< rt::wat, producer >,
                   sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) ) },
    { "GWCT", div( rate< rt::wat, producer >,
                   sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) ) },
    { "WGOR", div( rate< rt::gas, producer >, rate< rt::oil, producer > ) },
    { "GGOR", div( rate< rt::gas, producer >, rate< rt::oil, producer > ) },
    { "WGLR", div( rate< rt::gas, producer >,
                   sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) ) },

    { "WBHP", bhp },
    { "WTHP", thp },
    { "WVPRT", res_vol_production_target },

    { "WMCTL", well_control_mode },

    { "GWIR", rate< rt::wat, injector > },
    { "WGVIR", rate< rt::reservoir_gas, injector >},
    { "WWVIR", rate< rt::reservoir_water, injector >},
    { "GOIR", rate< rt::oil, injector > },
    { "GGIR", rate< rt::gas, injector > },
    { "GNIR", rate< rt::solvent, injector > },
    { "GCIR", rate< rt::wat, injector, polymer > },
    { "GVIR", sum( sum( rate< rt::reservoir_water, injector >, rate< rt::reservoir_oil, injector > ),
                        rate< rt::reservoir_gas, injector > ) },
    { "GWIT", mul( rate< rt::wat, injector >, duration ) },
    { "GOIT", mul( rate< rt::oil, injector >, duration ) },
    { "GGIT", mul( rate< rt::gas, injector >, duration ) },
    { "GNIT", mul( rate< rt::solvent, injector >, duration ) },
    { "GCIT", mul( rate< rt::wat, injector, polymer >, duration ) },
    { "GVIT", mul( sum( sum( rate< rt::reservoir_water, injector >, rate< rt::reservoir_oil, injector > ),
                        rate< rt::reservoir_gas, injector > ), duration ) },

    { "GWPR", rate< rt::wat, producer > },
    { "GOPR", rate< rt::oil, producer > },
    { "GGPR", rate< rt::gas, producer > },
    { "GNPR", rate< rt::solvent, producer > },
    { "GOPRS", rate< rt::vaporized_oil, producer > },
    { "GOPRF", sub (rate < rt::oil, producer >, rate< rt::vaporized_oil, producer > ) },
    { "GLPR", sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) },
    { "GVPR", sum( sum( rate< rt::reservoir_water, producer >, rate< rt::reservoir_oil, producer > ),
                        rate< rt::reservoir_gas, producer > ) },

    { "GWPT", mul( rate< rt::wat, producer >, duration ) },
    { "GOPT", mul( rate< rt::oil, producer >, duration ) },
    { "GGPT", mul( rate< rt::gas, producer >, duration ) },
    { "GNPT", mul( rate< rt::solvent, producer >, duration ) },
    { "GOPTS", mul( rate< rt::vaporized_oil, producer >, duration ) },
    { "GOPTF", mul( sub (rate < rt::oil, producer >,
                         rate< rt::vaporized_oil, producer > ),
                    duration ) },
    { "GLPT", mul( sum( rate< rt::wat, producer >, rate< rt::oil, producer > ),
                   duration ) },
    { "GVPT", mul( sum( sum( rate< rt::reservoir_water, producer >, rate< rt::reservoir_oil, producer > ),
                        rate< rt::reservoir_gas, producer > ), duration ) },
    // Group potential
    { "GWPP", potential_rate< rt::well_potential_water , true, false>},
    { "GOPP", potential_rate< rt::well_potential_oil , true, false>},
    { "GGPP", potential_rate< rt::well_potential_gas , true, false>},
    { "GWPI", potential_rate< rt::well_potential_water , false, true>},
    { "GOPI", potential_rate< rt::well_potential_oil , false, true>},
    { "GGPI", potential_rate< rt::well_potential_gas , false, true>},

    //Group control mode
    { "GMCTP", group_control< true, true,  false, false >},
    { "GMCTW", group_control< true, false, true,  false >},
    { "GMCTG", group_control< true, false, false, true  >},

    { "WWPRH", production_history< Opm::Phase::WATER > },
    { "WOPRH", production_history< Opm::Phase::OIL > },
    { "WGPRH", production_history< Opm::Phase::GAS > },
    { "WLPRH", sum( production_history< Opm::Phase::WATER >,
                    production_history< Opm::Phase::OIL > ) },

    { "WWPTH", mul( production_history< Opm::Phase::WATER >, duration ) },
    { "WOPTH", mul( production_history< Opm::Phase::OIL >, duration ) },
    { "WGPTH", mul( production_history< Opm::Phase::GAS >, duration ) },
    { "WLPTH", mul( sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ),
                    duration ) },

    { "WWIRH", injection_history< Opm::Phase::WATER > },
    { "WOIRH", injection_history< Opm::Phase::OIL > },
    { "WGIRH", injection_history< Opm::Phase::GAS > },
    { "WWITH", mul( injection_history< Opm::Phase::WATER >, duration ) },
    { "WOITH", mul( injection_history< Opm::Phase::OIL >, duration ) },
    { "WGITH", mul( injection_history< Opm::Phase::GAS >, duration ) },

    /* From our point of view, injectors don't have water cuts and div/sum will return 0.0 */
    { "WWCTH", div( production_history< Opm::Phase::WATER >,
                    sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ) ) },

    /* We do not support mixed injections, and gas/oil is undefined when oil is
     * zero (i.e. pure gas injector), so always output 0 if this is an injector
     */
    { "WGORH", div( production_history< Opm::Phase::GAS >,
                    production_history< Opm::Phase::OIL > ) },
    { "WGLRH", div( production_history< Opm::Phase::GAS >,
                    sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ) ) },

    { "WTHPH", thp_history },
    { "WBHPH", bhp_history },

    { "GWPRH", production_history< Opm::Phase::WATER > },
    { "GOPRH", production_history< Opm::Phase::OIL > },
    { "GGPRH", production_history< Opm::Phase::GAS > },
    { "GLPRH", sum( production_history< Opm::Phase::WATER >,
                    production_history< Opm::Phase::OIL > ) },
    { "GWIRH", injection_history< Opm::Phase::WATER > },
    { "GOIRH", injection_history< Opm::Phase::OIL > },
    { "GGIRH", injection_history< Opm::Phase::GAS > },
    { "GGORH", div( production_history< Opm::Phase::GAS >,
                    production_history< Opm::Phase::OIL > ) },
    { "GWCTH", div( production_history< Opm::Phase::WATER >,
                    sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ) ) },

    { "GWPTH", mul( production_history< Opm::Phase::WATER >, duration ) },
    { "GOPTH", mul( production_history< Opm::Phase::OIL >, duration ) },
    { "GGPTH", mul( production_history< Opm::Phase::GAS >, duration ) },
    { "GGPRF", sub( rate < rt::gas, producer >, rate< rt::dissolved_gas, producer > )},
    { "GGPRS", rate< rt::dissolved_gas, producer> },
    { "GGPTF", mul( sub( rate < rt::gas, producer >, rate< rt::dissolved_gas, producer > ),
                         duration ) },
    { "GGPTS", mul( rate< rt::dissolved_gas, producer>, duration ) },
    { "GGLR",  div( rate< rt::gas, producer >,
                    sum( rate< rt::wat, producer >,
                         rate< rt::oil, producer > ) ) },
    { "GGLRH", div( production_history< Opm::Phase::GAS >,
                    sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ) ) },
    { "GLPTH", mul( sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ),
                    duration ) },
    { "GWITH", mul( injection_history< Opm::Phase::WATER >, duration ) },
    { "GGITH", mul( injection_history< Opm::Phase::GAS >, duration ) },
    { "GMWIN", flowing< injector > },
    { "GMWPR", flowing< producer > },

    { "GVPRT", res_vol_production_target },

    { "CWIR", crate< rt::wat, injector > },
    { "CGIR", crate< rt::gas, injector > },
    { "CCIR", crate< rt::wat, injector, polymer > },
    { "CWIT", mul( crate< rt::wat, injector >, duration ) },
    { "CGIT", mul( crate< rt::gas, injector >, duration ) },
    { "CNIT", mul( crate< rt::solvent, injector >, duration ) },

    { "CWPR", crate< rt::wat, producer > },
    { "COPR", crate< rt::oil, producer > },
    { "CGPR", crate< rt::gas, producer > },
    { "CGFR", sub(crate<rt::gas, producer>, crate<rt::gas, injector>) },
    { "COFR", sub(crate<rt::oil, producer>, crate<rt::oil, injector>) },
    { "CWFR", sub(crate<rt::wat, producer>, crate<rt::wat, injector>) },
    { "CWCT", div( crate< rt::wat, producer >,
                   sum( crate< rt::wat, producer >, crate< rt::oil, producer > ) ) },
    { "CGOR", div( crate< rt::gas, producer >, crate< rt::oil, producer > ) },
    // Minus for injection rates and pluss for production rate
    { "CNFR", sub( crate< rt::solvent, producer >, crate<rt::solvent, injector >) },
    { "CWPT", mul( crate< rt::wat, producer >, duration ) },
    { "COPT", mul( crate< rt::oil, producer >, duration ) },
    { "CGPT", mul( crate< rt::gas, producer >, duration ) },
    { "CNPT", mul( crate< rt::solvent, producer >, duration ) },
    { "CCIT", mul( crate< rt::wat, injector, polymer >, duration ) },
    { "CCPT", mul( crate< rt::wat, producer, polymer >, duration ) },
    { "CTFAC", trans_factors },

    { "FWPR", rate< rt::wat, producer > },
    { "FOPR", rate< rt::oil, producer > },
    { "FGPR", rate< rt::gas, producer > },
    { "FNPR", rate< rt::solvent, producer > },
    { "FVPR", sum( sum( rate< rt::reservoir_water, producer>, rate< rt::reservoir_oil, producer >),
                   rate< rt::reservoir_gas, producer>)},
    { "FGPRS", rate< rt::dissolved_gas, producer > },
    { "FGPRF", sub( rate< rt::gas, producer >, rate< rt::dissolved_gas, producer > ) },
    { "FOPRS", rate< rt::vaporized_oil, producer > },
    { "FOPRF", sub (rate < rt::oil, producer >, rate< rt::vaporized_oil, producer > ) },

    { "FLPR", sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) },
    { "FWPT", mul( rate< rt::wat, producer >, duration ) },
    { "FOPT", mul( rate< rt::oil, producer >, duration ) },
    { "FGPT", mul( rate< rt::gas, producer >, duration ) },
    { "FNPT", mul( rate< rt::solvent, producer >, duration ) },
    { "FLPT", mul( sum( rate< rt::wat, producer >, rate< rt::oil, producer > ),
                   duration ) },
    { "FVPT", mul(sum (sum( rate< rt::reservoir_water, producer>, rate< rt::reservoir_oil, producer >),
                       rate< rt::reservoir_gas, producer>), duration)},
    { "FGPTS", mul( rate< rt::dissolved_gas, producer > , duration )},
    { "FGPTF", mul( sub( rate< rt::gas, producer >, rate< rt::dissolved_gas, producer > ), duration )},
    { "FOPTS", mul( rate< rt::vaporized_oil, producer >, duration ) },
    { "FOPTF", mul( sub (rate < rt::oil, producer >,
                         rate< rt::vaporized_oil, producer > ),
                    duration ) },

    { "FWIR", rate< rt::wat, injector > },
    { "FOIR", rate< rt::oil, injector > },
    { "FGIR", rate< rt::gas, injector > },
    { "FNIR", rate< rt::solvent, injector > },
    { "FCIR", rate< rt::wat, injector, polymer > },
    { "FCPR", rate< rt::wat, producer, polymer > },
    { "FVIR", sum( sum( rate< rt::reservoir_water, injector>, rate< rt::reservoir_oil, injector >),
                   rate< rt::reservoir_gas, injector>)},

    { "FLIR", sum( rate< rt::wat, injector >, rate< rt::oil, injector > ) },
    { "FWIT", mul( rate< rt::wat, injector >, duration ) },
    { "FOIT", mul( rate< rt::oil, injector >, duration ) },
    { "FGIT", mul( rate< rt::gas, injector >, duration ) },
    { "FNIT", mul( rate< rt::solvent, injector >, duration ) },
    { "FCIT", mul( rate< rt::wat, injector, polymer >, duration ) },
    { "FCPT", mul( rate< rt::wat, producer, polymer >, duration ) },
    { "FLIT", mul( sum( rate< rt::wat, injector >, rate< rt::oil, injector > ),
                   duration ) },
    { "FVIT", mul( sum( sum( rate< rt::reservoir_water, injector>, rate< rt::reservoir_oil, injector >),
                   rate< rt::reservoir_gas, injector>), duration)},
    // Field potential
    { "FWPP", potential_rate< rt::well_potential_water , true, false>},
    { "FOPP", potential_rate< rt::well_potential_oil , true, false>},
    { "FGPP", potential_rate< rt::well_potential_gas , true, false>},
    { "FWPI", potential_rate< rt::well_potential_water , false, true>},
    { "FOPI", potential_rate< rt::well_potential_oil , false, true>},
    { "FGPI", potential_rate< rt::well_potential_gas , false, true>},


    { "FWPRH", production_history< Opm::Phase::WATER > },
    { "FOPRH", production_history< Opm::Phase::OIL > },
    { "FGPRH", production_history< Opm::Phase::GAS > },
    { "FLPRH", sum( production_history< Opm::Phase::WATER >,
                    production_history< Opm::Phase::OIL > ) },
    { "FWPTH", mul( production_history< Opm::Phase::WATER >, duration ) },
    { "FOPTH", mul( production_history< Opm::Phase::OIL >, duration ) },
    { "FGPTH", mul( production_history< Opm::Phase::GAS >, duration ) },
    { "FLPTH", mul( sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ),
                    duration ) },

    { "FWIRH", injection_history< Opm::Phase::WATER > },
    { "FOIRH", injection_history< Opm::Phase::OIL > },
    { "FGIRH", injection_history< Opm::Phase::GAS > },
    { "FWITH", mul( injection_history< Opm::Phase::WATER >, duration ) },
    { "FOITH", mul( injection_history< Opm::Phase::OIL >, duration ) },
    { "FGITH", mul( injection_history< Opm::Phase::GAS >, duration ) },

    { "FWCT", div( rate< rt::wat, producer >,
                   sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) ) },
    { "FGOR", div( rate< rt::gas, producer >, rate< rt::oil, producer > ) },
    { "FGLR", div( rate< rt::gas, producer >,
                   sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) ) },
    { "FWCTH", div( production_history< Opm::Phase::WATER >,
                    sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ) ) },
    { "FGORH", div( production_history< Opm::Phase::GAS >,
                    production_history< Opm::Phase::OIL > ) },
    { "FGLRH", div( production_history< Opm::Phase::GAS >,
                    sum( production_history< Opm::Phase::WATER >,
                         production_history< Opm::Phase::OIL > ) ) },
    { "FMWIN", flowing< injector > },
    { "FMWPR", flowing< producer > },
    { "FVPRT", res_vol_production_target },

    //Field control mode
    { "FMCTP", group_control< false, true,  false, false >},
    { "FMCTW", group_control< false, false, true,  false >},
    { "FMCTG", group_control< false, false, false, true  >},

    /* Region properties */
    { "ROIR"  , region_rate< rt::oil, injector > },
    { "RGIR"  , region_rate< rt::gas, injector > },
    { "RWIR"  , region_rate< rt::wat, injector > },
    { "ROPR"  , region_rate< rt::oil, producer > },
    { "RGPR"  , region_rate< rt::gas, producer > },
    { "RWPR"  , region_rate< rt::wat, producer > },
    { "ROIT"  , mul( region_rate< rt::oil, injector >, duration ) },
    { "RGIT"  , mul( region_rate< rt::gas, injector >, duration ) },
    { "RWIT"  , mul( region_rate< rt::wat, injector >, duration ) },
    { "ROPT"  , mul( region_rate< rt::oil, producer >, duration ) },
    { "RGPT"  , mul( region_rate< rt::gas, producer >, duration ) },
    { "RWPT"  , mul( region_rate< rt::wat, producer >, duration ) },
    //Multisegment well segment data
    { "SOFR", srate< rt::oil > },
    { "SWFR", srate< rt::wat > },
    { "SGFR", srate< rt::gas > },
    { "SPR", segpress<Opm::data::SegmentPressures::Value::Pressure> },
    { "SPRD", segpress<Opm::data::SegmentPressures::Value::PDrop> },
    { "SPRDH", segpress<Opm::data::SegmentPressures::Value::PDropHydrostatic> },
    { "SPRDF", segpress<Opm::data::SegmentPressures::Value::PDropFriction> },
    { "SPRDA", segpress<Opm::data::SegmentPressures::Value::PDropAccel> },
    // Well productivity index
    { "WPIW", potential_rate< rt::productivity_index_water >},
    { "WPIO", potential_rate< rt::productivity_index_oil >},
    { "WPIG", potential_rate< rt::productivity_index_gas >},
    { "WPIL", sum( potential_rate< rt::productivity_index_water >, potential_rate< rt::productivity_index_oil>)},
    // Well potential
    { "WWPP", potential_rate< rt::well_potential_water , true, false>},
    { "WOPP", potential_rate< rt::well_potential_oil , true, false>},
    { "WGPP", potential_rate< rt::well_potential_gas , true, false>},
    { "WWPI", potential_rate< rt::well_potential_water , false, true>},
    { "WWIP", potential_rate< rt::well_potential_water , false, true>}, // Alias for 'WWPI'
    { "WOPI", potential_rate< rt::well_potential_oil , false, true>},
    { "WGPI", potential_rate< rt::well_potential_gas , false, true>},
    { "WGIP", potential_rate< rt::well_potential_gas , false, true>}, // Alias for 'WGPI'
};


static const std::unordered_map< std::string, Opm::UnitSystem::measure> single_values_units = {
  {"TCPU"     , Opm::UnitSystem::measure::identity },
  {"ELAPSED"  , Opm::UnitSystem::measure::identity },
  {"NEWTON"   , Opm::UnitSystem::measure::identity },
  {"NLINERS"  , Opm::UnitSystem::measure::identity },
  {"NLINSMIN" , Opm::UnitSystem::measure::identity },
  {"NLINSMAX" , Opm::UnitSystem::measure::identity },
  {"MLINEARS" , Opm::UnitSystem::measure::identity },
  {"MSUMLINS" , Opm::UnitSystem::measure::identity },
  {"MSUMNEWT" , Opm::UnitSystem::measure::identity },
  {"TCPUTS"   , Opm::UnitSystem::measure::identity },
  {"TIMESTEP" , Opm::UnitSystem::measure::time },
  {"TCPUDAY"  , Opm::UnitSystem::measure::time },
  {"STEPTYPE" , Opm::UnitSystem::measure::identity },
  {"TELAPLIN" , Opm::UnitSystem::measure::time },
  {"FWIP"     , Opm::UnitSystem::measure::liquid_surface_volume },
  {"FOIP"     , Opm::UnitSystem::measure::liquid_surface_volume },
  {"FGIP"     , Opm::UnitSystem::measure::gas_surface_volume },
  {"FOIPL"    , Opm::UnitSystem::measure::liquid_surface_volume },
  {"FOIPG"    , Opm::UnitSystem::measure::liquid_surface_volume },
  {"FGIPL"    , Opm::UnitSystem::measure::gas_surface_volume },
  {"FGIPG"    , Opm::UnitSystem::measure::gas_surface_volume },
  {"FPR"      , Opm::UnitSystem::measure::pressure },

};

static const std::unordered_map< std::string, Opm::UnitSystem::measure> region_units = {
  {"RPR"      , Opm::UnitSystem::measure::pressure},
  {"ROIP"     , Opm::UnitSystem::measure::liquid_surface_volume },
  {"ROIPL"    , Opm::UnitSystem::measure::liquid_surface_volume },
  {"ROIPG"    , Opm::UnitSystem::measure::liquid_surface_volume },
  {"RGIP"     , Opm::UnitSystem::measure::gas_surface_volume },
  {"RGIPL"    , Opm::UnitSystem::measure::gas_surface_volume },
  {"RGIPG"    , Opm::UnitSystem::measure::gas_surface_volume },
  {"RWIP"     , Opm::UnitSystem::measure::liquid_surface_volume }
};

static const std::unordered_map< std::string, Opm::UnitSystem::measure> block_units = {
  {"BPR"        , Opm::UnitSystem::measure::pressure},
  {"BPRESSUR"   , Opm::UnitSystem::measure::pressure},
  {"BSWAT"      , Opm::UnitSystem::measure::identity},
  {"BWSAT"      , Opm::UnitSystem::measure::identity},
  {"BSGAS"      , Opm::UnitSystem::measure::identity},
  {"BGSAT"      , Opm::UnitSystem::measure::identity},
  {"BOSAT"      , Opm::UnitSystem::measure::identity},
  {"BWKR"      , Opm::UnitSystem::measure::identity},
  {"BOKR"      , Opm::UnitSystem::measure::identity},
  {"BKRO"      , Opm::UnitSystem::measure::identity},
  {"BGKR"      , Opm::UnitSystem::measure::identity},
  {"BKRG"      , Opm::UnitSystem::measure::identity},
  {"BKRW"      , Opm::UnitSystem::measure::identity},
  {"BWPC"      , Opm::UnitSystem::measure::pressure},
  {"BGPC"      , Opm::UnitSystem::measure::pressure},
  {"BVWAT"      , Opm::UnitSystem::measure::viscosity},
  {"BWVIS"      , Opm::UnitSystem::measure::viscosity},
  {"BVGAS"      , Opm::UnitSystem::measure::viscosity},
  {"BGVIS"      , Opm::UnitSystem::measure::viscosity},
  {"BVOIL"      , Opm::UnitSystem::measure::viscosity},
  {"BOVIS"      , Opm::UnitSystem::measure::viscosity},
};

inline std::vector<Opm::Well> find_wells( const Opm::Schedule& schedule,
                                           const Opm::EclIO::SummaryNode& node,
                                           const int sim_step,
                                           const Opm::out::RegionCache& regionCache ) {
    const auto cat = node.category;

    switch (cat) {
    case Opm::EclIO::SummaryNode::Category::Well: [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Connection: [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Segment: {
        const auto& name = node.wgname;

        if (schedule.hasWell(node.wgname, sim_step)) {
            return { schedule.getWell( name, sim_step ) };
        } else {
            return {};
        }
    }

    case Opm::EclIO::SummaryNode::Category::Group: {
        const auto& name = node.wgname;

        if( !schedule.hasGroup( name ) ) return {};

        return schedule.getChildWells2( name, sim_step);
    }

    case Opm::EclIO::SummaryNode::Category::Field:
        return schedule.getWells(sim_step);

    case Opm::EclIO::SummaryNode::Category::Region: {
        std::vector<Opm::Well> wells;

        const auto region = node.number;

        for ( const auto& connection : regionCache.connections( region ) ){
            const auto& w_name = connection.first;
            if (schedule.hasWell(w_name, sim_step)) {
                const auto& well = schedule.getWell( w_name, sim_step );

                const auto& it = std::find_if( wells.begin(), wells.end(),
                                               [&] ( const Opm::Well& elem )
                                               { return elem.name() == well.name(); });
                if ( it == wells.end() )
                    wells.push_back( well );
            }
        }

        return wells;
    }

    case Opm::EclIO::SummaryNode::Category::Aquifer:       [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Block:         [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Miscellaneous:
        return {};
    }

    throw std::runtime_error("Unhandled summary node category in find_wells");
}

bool need_wells(const Opm::EclIO::SummaryNode& node) {
    static const std::regex region_keyword_regex { "R[OGW][IP][RT]" };

    switch (node.category) {
    case Opm::EclIO::SummaryNode::Category::Connection: [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Field:      [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Group:      [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Segment:    [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Well:
       return true;
    case Opm::EclIO::SummaryNode::Category::Region:
        return std::regex_match(node.keyword, region_keyword_regex);
    case Opm::EclIO::SummaryNode::Category::Aquifer:       [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Miscellaneous: [[fallthrough]];
    case Opm::EclIO::SummaryNode::Category::Block:
        return false;
    }

    throw std::runtime_error("Unhandled summary node category in need_wells");
}

void eval_udq(const Opm::Schedule& schedule, std::size_t sim_step, Opm::SummaryState& st)
{
    using namespace Opm;

    const UDQConfig& udq = schedule.getUDQConfig(sim_step);
    const auto& func_table = udq.function_table();
    UDQContext context(func_table, st);
    {
        const std::vector<std::string> wells = st.wells();

        for (const auto& assign : udq.assignments(UDQVarType::WELL_VAR)) {
            auto ws = assign.eval(wells);
            for (const auto& well : wells) {
                const auto& udq_value = ws[well];
                if (udq_value)
                    st.update_well_var(well, ws.name(), udq_value.value());
            }
        }

        for (const auto& def : udq.definitions(UDQVarType::WELL_VAR)) {
            auto ws = def.eval(context);
            for (const auto& well : wells) {
                const auto& udq_value = ws[well];
                if (udq_value)
                    st.update_well_var(well, def.keyword(), udq_value.value());
            }
        }
    }

    {
        const std::vector<std::string> groups = st.groups();

        for (const auto& assign : udq.assignments(UDQVarType::GROUP_VAR)) {
            auto ws = assign.eval(groups);
            for (const auto& group : groups) {
                const auto& udq_value = ws[group];
                if (udq_value)
                    st.update_group_var(group, ws.name(), udq_value.value());
            }
        }

        for (const auto& def : udq.definitions(UDQVarType::GROUP_VAR)) {
            auto ws = def.eval(context);
            for (const auto& group : groups) {
                const auto& udq_value = ws[group];
                if (udq_value)
                    st.update_group_var(group, def.keyword(), udq_value.value());
            }
        }
    }

    for (const auto& def : udq.definitions(UDQVarType::FIELD_VAR)) {
        auto field_udq = def.eval(context);
        if (field_udq[0])
            st.update(def.keyword(), field_udq[0].value());
    }
}

void updateValue(const Opm::EclIO::SummaryNode& node, const double value, Opm::SummaryState& st)
{
    if (node.category == Opm::EclIO::SummaryNode::Category::Well)
        st.update_well_var(node.wgname, node.keyword, value);

    else if (node.category == Opm::EclIO::SummaryNode::Category::Group)
        st.update_group_var(node.wgname, node.keyword, value);

    else
        st.update(node.unique_key(), value);
}

/*
 * The well efficiency factor will not impact the well rate itself, but is
 * rather applied for accumulated values.The WEFAC can be considered to shut
 * and open the well for short intervals within the same timestep, and the well
 * is therefore solved at full speed.
 *
 * Groups are treated similarly as wells. The group's GEFAC is not applied for
 * rates, only for accumulated volumes. When GEFAC is set for a group, it is
 * considered that all wells are taken down simultaneously, and GEFAC is
 * therefore not applied to the group's rate. However, any efficiency factors
 * applied to the group's wells or sub-groups must be included.
 *
 * Regions and fields will have the well and group efficiency applied for both
 * rates and accumulated values.
 *
 */
struct EfficiencyFactor
{
    using Factor  = std::pair<std::string, double>;
    using FacColl = std::vector<Factor>;

    FacColl factors{};

    void setFactors(const Opm::EclIO::SummaryNode& node,
                    const Opm::Schedule&           schedule,
                    const std::vector<Opm::Well>& schedule_wells,
                    const int                      sim_step);
};

void EfficiencyFactor::setFactors(const Opm::EclIO::SummaryNode& node,
                                  const Opm::Schedule&           schedule,
                                  const std::vector<Opm::Well>&  schedule_wells,
                                  const int                      sim_step)
{
    this->factors.clear();

    const bool is_field  { node.category == Opm::EclIO::SummaryNode::Category::Field  } ;
    const bool is_group  { node.category == Opm::EclIO::SummaryNode::Category::Group  } ;
    const bool is_region { node.category == Opm::EclIO::SummaryNode::Category::Region } ;
    const bool is_rate   { node.type     != Opm::EclIO::SummaryNode::Type::Total      } ;

    if (!is_field && !is_group && !is_region && is_rate)
        return;

    for( const auto& well : schedule_wells ) {
        if (!well.hasBeenDefined(sim_step))
            continue;

        double eff_factor = well.getEfficiencyFactor();
        const auto* group_ptr = std::addressof(schedule.getGroup(well.groupName(), sim_step));

        while (group_ptr) {
            if (is_group && is_rate && group_ptr->name() == node.wgname )
                break;

            eff_factor *= group_ptr->getGroupEfficiencyFactor();

            const auto parent_group = group_ptr->flow_group();

            if (parent_group)
                group_ptr = std::addressof(schedule.getGroup( parent_group.value(), sim_step ));
            else
                group_ptr = nullptr;
        }

        this->factors.emplace_back( well.name(), eff_factor );
    }
}

namespace Evaluator {
    struct InputData
    {
        const Opm::EclipseState& es;
        const Opm::Schedule& sched;
        const Opm::EclipseGrid& grid;
        const Opm::out::RegionCache& reg;
    };

    struct SimulatorResults
    {
        const Opm::data::WellRates& wellSol;
        const Opm::data::Group&  groupSol;
        const std::map<std::string, double>& single;
        const std::map<std::string, std::vector<double>>& region;
        const std::map<std::pair<std::string, int>, double>& block;
    };

    class Base
    {
    public:
        virtual ~Base() {}

        virtual void update(const std::size_t       sim_step,
                            const double            stepSize,
                            const InputData&        input,
                            const SimulatorResults& simRes,
                            Opm::SummaryState&      st) const = 0;
    };

    class FunctionRelation : public Base
    {
    public:
        explicit FunctionRelation(Opm::EclIO::SummaryNode node, ofun fcn)
            : node_(std::move(node))
            , fcn_ (std::move(fcn))
        {}

        void update(const std::size_t       sim_step,
                    const double            stepSize,
                    const InputData&        input,
                    const SimulatorResults& simRes,
                    Opm::SummaryState&      st) const override
        {
            const auto get_wells =
                need_wells(node_);

            const auto wells = get_wells
                ? find_wells(input.sched, this->node_,
                             static_cast<int>(sim_step), input.reg)
                : std::vector<Opm::Well>{};

            if (get_wells && wells.empty())
                // Parameter depends on well information, but no active
                // wells apply at this sim_step.  Nothing to do.
                return;

            std::string group_name = this->node_.category == Opm::EclIO::SummaryNode::Category::Group ? this->node_.wgname : "";

            EfficiencyFactor efac{};
            efac.setFactors(this->node_, input.sched, wells, sim_step);

            const fn_args args {
                wells, group_name, stepSize, static_cast<int>(sim_step),
                std::max(0, this->node_.number),
                st, simRes.wellSol, simRes.groupSol, input.reg, input.grid,
                std::move(efac.factors)
            };

            const auto& usys = input.es.getUnits();
            const auto  prm  = this->fcn_(args);

            updateValue(this->node_, usys.from_si(prm.unit, prm.value), st);
        }

    private:
        Opm::EclIO::SummaryNode node_;
        ofun             fcn_;
    };

    class BlockValue : public Base
    {
    public:
        explicit BlockValue(Opm::EclIO::SummaryNode node,
                            const Opm::UnitSystem::measure m)
            : node_(std::move(node))
            , m_   (m)
        {}

        void update(const std::size_t    /* sim_step */,
                    const double         /* stepSize */,
                    const InputData&        input,
                    const SimulatorResults& simRes,
                    Opm::SummaryState&      st) const override
        {
            auto xPos = simRes.block.find(this->lookupKey());
            if (xPos == simRes.block.end()) {
                return;
            }

            const auto& usys = input.es.getUnits();
            updateValue(this->node_, usys.from_si(this->m_, xPos->second), st);
        }

    private:
        Opm::EclIO::SummaryNode  node_;
        Opm::UnitSystem::measure m_;

        Opm::out::Summary::BlockValues::key_type lookupKey() const
        {
            return { this->node_.keyword, this->node_.number };
        }
    };

    class RegionValue : public Base
    {
    public:
        explicit RegionValue(Opm::EclIO::SummaryNode node,
                             const Opm::UnitSystem::measure m)
            : node_(std::move(node))
            , m_   (m)
        {}

        void update(const std::size_t    /* sim_step */,
                    const double         /* stepSize */,
                    const InputData&        input,
                    const SimulatorResults& simRes,
                    Opm::SummaryState&      st) const override
        {
            if (this->node_.number < 0)
                return;

            auto xPos = simRes.region.find(this->node_.keyword);
            if (xPos == simRes.region.end())
                return;

            const auto ix = this->index();
            if (ix >= xPos->second.size())
                return;

            const auto  val  = xPos->second[ix];
            const auto& usys = input.es.getUnits();

            updateValue(this->node_, usys.from_si(this->m_, val), st);
        }

    private:
        Opm::EclIO::SummaryNode  node_;
        Opm::UnitSystem::measure m_;

        std::vector<double>::size_type index() const
        {
            return this->node_.number - 1;
        }
    };

    class GlobalProcessValue : public Base
    {
    public:
        explicit GlobalProcessValue(Opm::EclIO::SummaryNode node,
                                    const Opm::UnitSystem::measure m)
            : node_(std::move(node))
            , m_   (m)
        {}

        void update(const std::size_t    /* sim_step */,
                    const double         /* stepSize */,
                    const InputData&        input,
                    const SimulatorResults& simRes,
                    Opm::SummaryState&      st) const override
        {
            auto xPos = simRes.single.find(this->node_.keyword);
            if (xPos == simRes.single.end())
                return;

            const auto  val  = xPos->second;
            const auto& usys = input.es.getUnits();

            updateValue(this->node_, usys.from_si(this->m_, val), st);
        }

    private:
        Opm::EclIO::SummaryNode  node_;
        Opm::UnitSystem::measure m_;
    };

    class UserDefinedValue : public Base
    {
    public:
        void update(const std::size_t       /* sim_step */,
                    const double            /* stepSize */,
                    const InputData&        /* input */,
                    const SimulatorResults& /* simRes */,
                    Opm::SummaryState&      /* st */) const override
        {
            // No-op
        }
    };

    class Time : public Base
    {
    public:
        explicit Time(std::string saveKey)
            : saveKey_(std::move(saveKey))
        {}

        void update(const std::size_t       /* sim_step */,
                    const double               stepSize,
                    const InputData&           input,
                    const SimulatorResults& /* simRes */,
                    Opm::SummaryState&         st) const override
        {
            const auto& usys = input.es.getUnits();

            const auto m   = ::Opm::UnitSystem::measure::time;
            const auto val = st.get_elapsed() + stepSize;

            st.update(this->saveKey_, usys.from_si(m, val));
        }

    private:
        std::string saveKey_;
    };

    class Day : public Base
    {
    public:
        explicit Day(std::string saveKey)
            : saveKey_(std::move(saveKey))
        {}

        void update(const std::size_t       /* sim_step */,
                    const double               stepSize,
                    const InputData&           input,
                    const SimulatorResults& /* simRes */,
                    Opm::SummaryState&         st) const override
        {
            auto sim_time = make_sim_time(input.sched, st, stepSize);
            st.update(this->saveKey_, sim_time.day());
        }

    private:
        std::string saveKey_;
    };

    class Month : public Base
    {
    public:
        explicit Month(std::string saveKey)
            : saveKey_(std::move(saveKey))
        {}

        void update(const std::size_t       /* sim_step */,
                    const double               stepSize,
                    const InputData&           input,
                    const SimulatorResults& /* simRes */,
                    Opm::SummaryState&         st) const override
        {
            auto sim_time = make_sim_time(input.sched, st, stepSize);
            st.update(this->saveKey_, sim_time.month());
        }

    private:
        std::string saveKey_;
    };

    class Year : public Base
    {
    public:
        explicit Year(std::string saveKey)
            : saveKey_(std::move(saveKey))
        {}

        void update(const std::size_t       /* sim_step */,
                    const double               stepSize,
                    const InputData&           input,
                    const SimulatorResults& /* simRes */,
                    Opm::SummaryState&         st) const override
        {
            auto sim_time = make_sim_time(input.sched, st, stepSize);
            st.update(this->saveKey_, sim_time.year());
        }

    private:
        std::string saveKey_;
    };

    class Years : public Base
    {
    public:
        explicit Years(std::string saveKey)
            : saveKey_(std::move(saveKey))
        {}

        void update(const std::size_t       /* sim_step */,
                    const double               stepSize,
                    const InputData&        /* input */,
                    const SimulatorResults& /* simRes */,
                    Opm::SummaryState&         st) const override
        {
            using namespace ::Opm::unit;

            const auto val = st.get_elapsed() + stepSize;

            st.update(this->saveKey_, convert::to(val, ecl_year));
        }

    private:
        std::string saveKey_;
    };

    class Factory
    {
    public:
        struct Descriptor
        {
            std::string uniquekey{};
            std::string unit{};
            std::unique_ptr<Base> evaluator{};
        };

        explicit Factory(const Opm::EclipseState& es,
                         const Opm::EclipseGrid&  grid,
                         const Opm::SummaryState& st,
                         const Opm::UDQConfig&    udq)
            : es_(es), grid_(grid), st_(st), udq_(udq)
        {}

        ~Factory() = default;

        Factory(const Factory&) = delete;
        Factory(Factory&&) = delete;
        Factory& operator=(const Factory&) = delete;
        Factory& operator=(Factory&&) = delete;

        Descriptor create(const Opm::EclIO::SummaryNode&);

    private:
        const Opm::EclipseState& es_;
        const Opm::EclipseGrid&  grid_;
        const Opm::SummaryState& st_;
        const Opm::UDQConfig&    udq_;

        const Opm::EclIO::SummaryNode* node_;

        Opm::UnitSystem::measure paramUnit_;
        ofun paramFunction_;

        Descriptor functionRelation();
        Descriptor blockValue();
        Descriptor regionValue();
        Descriptor globalProcessValue();
        Descriptor userDefinedValue();
        Descriptor unknownParameter();

        bool isBlockValue();
        bool isRegionValue();
        bool isGlobalProcessValue();
        bool isFunctionRelation();
        bool isUserDefined();

        std::string functionUnitString() const;
        std::string directUnitString() const;
        std::string userDefinedUnit() const;
    };

    Factory::Descriptor Factory::create(const Opm::EclIO::SummaryNode& node)
    {
        this->node_ = &node;

        if (this->isUserDefined())
            return this->userDefinedValue();

        if (this->isBlockValue())
            return this->blockValue();

        if (this->isRegionValue())
            return this->regionValue();

        if (this->isGlobalProcessValue())
            return this->globalProcessValue();

        if (this->isFunctionRelation())
            return this->functionRelation();

        return this->unknownParameter();
    }

    Factory::Descriptor Factory::functionRelation()
    {
        auto desc = this->unknownParameter();

        desc.unit = this->functionUnitString();
        desc.evaluator.reset(new FunctionRelation {
            *this->node_, std::move(this->paramFunction_)
        });

        return desc;
    }

    Factory::Descriptor Factory::blockValue()
    {
        auto desc = this->unknownParameter();

        desc.unit = this->directUnitString();
        desc.evaluator.reset(new BlockValue {
            *this->node_, this->paramUnit_
        });

        return desc;
    }

    Factory::Descriptor Factory::regionValue()
    {
        auto desc = this->unknownParameter();

        desc.unit = this->directUnitString();
        desc.evaluator.reset(new RegionValue {
            *this->node_, this->paramUnit_
        });

        return desc;
    }

    Factory::Descriptor Factory::globalProcessValue()
    {
        auto desc = this->unknownParameter();

        desc.unit = this->directUnitString();
        desc.evaluator.reset(new GlobalProcessValue {
            *this->node_, this->paramUnit_
        });

        return desc;
    }

    Factory::Descriptor Factory::userDefinedValue()
    {
        auto desc = this->unknownParameter();

        desc.unit = this->userDefinedUnit();
        desc.evaluator.reset(new UserDefinedValue {});

        return desc;
    }

    Factory::Descriptor Factory::unknownParameter()
    {
        auto desc = Descriptor{};

        desc.uniquekey = this->node_->unique_key();

        return desc;
    }

    bool Factory::isBlockValue()
    {
        auto pos = block_units.find(this->node_->keyword);
        if (pos == block_units.end())
            return false;

        if (! this->grid_.cellActive(this->node_->number - 1))
            // 'node_' is a block value, but it is configured in a
            // deactivated cell.  Don't create an evaluation function.
            return false;

        // 'node_' represents a block value in an active cell.
        // Capture unit of measure and return true.
        this->paramUnit_ = pos->second;
        return true;
    }

    bool Factory::isRegionValue()
    {
        auto pos = region_units.find(this->node_->keyword);
        if (pos == region_units.end())
            return false;

        // 'node_' represents a region value.  Capture unit
        // of measure and return true.
        this->paramUnit_ = pos->second;
        return true;
    }

    bool Factory::isGlobalProcessValue()
    {
        auto pos = single_values_units.find(this->node_->keyword);
        if (pos == single_values_units.end())
            return false;

        // 'node_' represents a single value (i.e., global process)
        // value.  Capture unit of measure and return true.
        this->paramUnit_ = pos->second;
        return true;
    }

    bool Factory::isFunctionRelation()
    {
        auto pos = funs.find(this->node_->keyword);
        if (pos == funs.end())
            return false;

        // 'node_' represents a functional relation.
        // Capture evaluation function and return true.
        this->paramFunction_ = pos->second;
        return true;
    }

    bool Factory::isUserDefined()
    {
        return this->node_->is_user_defined();
    }

    std::string Factory::functionUnitString() const
    {
        const auto reg = Opm::out::RegionCache{};

        const fn_args args {
            {}, "", 0.0, 0, std::max(0, this->node_->number),
            this->st_, {}, {}, reg, this->grid_,
            {}
        };

        const auto prm = this->paramFunction_(args);

        return this->es_.getUnits().name(prm.unit);
    }

    std::string Factory::directUnitString() const
    {
        return this->es_.getUnits().name(this->paramUnit_);
    }

    std::string Factory::userDefinedUnit() const
    {
        const auto& kw = this->node_->keyword;

        return this->udq_.has_unit(kw)
            ?  this->udq_.unit(kw) : "?????";
    }
} // namespace Evaluator

void reportUnsupportedKeywords(std::vector<Opm::SummaryConfigNode> keywords)
{
    // Sort by location first, then keyword.
    auto loc_kw_ordering = [](const Opm::SummaryConfigNode& n1, const Opm::SummaryConfigNode& n2) {
        if (n1.location() == n2.location()) {
            return n1.keyword() < n2.keyword();
        }
        if (n1.location().filename == n2.location().filename) {
            return n1.location().lineno < n2.location().lineno;
        }
        return n1.location().filename < n2.location().filename;
    };
    std::sort(keywords.begin(), keywords.end(), loc_kw_ordering);

    // Reorder to remove duplicate { keyword, location } pairs, since
    // that will give duplicate and therefore useless warnings.
    auto same_kw_and_loc = [](const Opm::SummaryConfigNode& n1, const Opm::SummaryConfigNode& n2) {
        return (n1.keyword() == n2.keyword()) && (n1.location() == n2.location());
    };
    auto uend = std::unique(keywords.begin(), keywords.end(), same_kw_and_loc);

    for (auto node = keywords.begin(); node != uend; ++node) {
        const auto& location = node->location();
        ::Opm::OpmLog::warning("Unhandled summary keyword '" + node->keyword() + "' at " + location.filename + ", line " + std::to_string(location.lineno));
    }
}

std::string makeWGName(std::string name)
{
    // Use default WGNAME if 'name' is empty or consists
    // exlusively of white-space (space and tab) characters.
    //
    // Use 'name' itself otherwise.

    const auto use_dflt = name.empty() ||
        (name.find_first_not_of(" \t") == std::string::npos);

    return use_dflt ? std::string(":+:+:+:+") : std::move(name);
}

class SummaryOutputParameters
{
public:
    using EvalPtr = std::unique_ptr<Evaluator::Base>;
    using SMSpecPrm = Opm::EclIO::OutputStream::
        SummarySpecification::Parameters;

    SummaryOutputParameters() = default;
    ~SummaryOutputParameters() = default;

    SummaryOutputParameters(const SummaryOutputParameters& rhs) = delete;
    SummaryOutputParameters(SummaryOutputParameters&& rhs) = default;

    SummaryOutputParameters&
    operator=(const SummaryOutputParameters& rhs) = delete;

    SummaryOutputParameters&
    operator=(SummaryOutputParameters&& rhs) = default;

    void makeParameter(std::string keyword,
                       std::string name,
                       const int   num,
                       std::string unit,
                       EvalPtr     evaluator)
    {
        this->smspec_.add(std::move(keyword), std::move(name),
                          std::max (num, 0), std::move(unit));

        this->evaluators_.push_back(std::move(evaluator));
    }

    const SMSpecPrm& summarySpecification() const
    {
        return this->smspec_;
    }

    const std::vector<EvalPtr>& getEvaluators() const
    {
        return this->evaluators_;
    }

private:
    SMSpecPrm smspec_{};
    std::vector<EvalPtr> evaluators_{};
};

class SMSpecStreamDeferredCreation
{
private:
    using Spec = ::Opm::EclIO::OutputStream::SummarySpecification;

public:
    using ResultSet = ::Opm::EclIO::OutputStream::ResultSet;
    using Formatted = ::Opm::EclIO::OutputStream::Formatted;

    explicit SMSpecStreamDeferredCreation(const Opm::InitConfig&          initcfg,
                                          const Opm::EclipseGrid&         grid,
                                          const std::time_t               start,
                                          const Opm::UnitSystem::UnitType utype);

    std::unique_ptr<Spec>
    createStream(const ResultSet& rset, const Formatted& fmt) const
    {
        return std::make_unique<Spec>(rset, fmt, this->uconv(),
                                      this->cartDims_, this->restart_,
                                      this->start_);
    }

private:
    Opm::UnitSystem::UnitType  utype_;
    std::array<int,3>          cartDims_;
    Spec::StartTime            start_;
    Spec::RestartSpecification restart_{};

    Spec::UnitConvention uconv() const;
};

SMSpecStreamDeferredCreation::
SMSpecStreamDeferredCreation(const Opm::InitConfig&          initcfg,
                             const Opm::EclipseGrid&         grid,
                             const std::time_t               start,
                             const Opm::UnitSystem::UnitType utype)
    : utype_   (utype)
    , cartDims_(grid.getNXYZ())
    , start_   (std::chrono::system_clock::from_time_t(start))
{
    if (initcfg.restartRequested()) {
        this->restart_.root = initcfg.getRestartRootName();
        this->restart_.step = initcfg.getRestartStep();
    }
}

SMSpecStreamDeferredCreation::Spec::UnitConvention
SMSpecStreamDeferredCreation::uconv() const
{
    using UType = ::Opm::UnitSystem::UnitType;

    if (this->utype_ == UType::UNIT_TYPE_METRIC)
        return Spec::UnitConvention::Metric;

    if (this->utype_ == UType::UNIT_TYPE_FIELD)
        return Spec::UnitConvention::Field;

    if (this->utype_ == UType::UNIT_TYPE_LAB)
        return Spec::UnitConvention::Lab;

    if (this->utype_ == UType::UNIT_TYPE_PVT_M)
        return Spec::UnitConvention::Pvt_M;

    throw std::invalid_argument {
        "Unsupported Unit Convention (" +
        std::to_string(static_cast<int>(this->utype_)) + ')'
    };
}

std::unique_ptr<SMSpecStreamDeferredCreation>
makeDeferredSMSpecCreation(const Opm::EclipseState& es,
                           const Opm::EclipseGrid&  grid,
                           const Opm::Schedule&     sched)
{
    return std::make_unique<SMSpecStreamDeferredCreation>
        (es.cfg().init(), grid, sched.posixStartTime(),
         es.getUnits().getType());
}

std::string makeUpperCase(std::string input)
{
    for (auto& c : input) {
        const auto u = std::toupper(static_cast<unsigned char>(c));
        c = static_cast<std::string::value_type>(u);
    }

    return input;
}

Opm::EclIO::OutputStream::ResultSet
makeResultSet(const Opm::IOConfig& iocfg, const std::string& basenm)
{
    const auto& base = basenm.empty()
        ? makeUpperCase(iocfg.getBaseName())
        : basenm;

    return { iocfg.getOutputDir(), base };
}

} // Anonymous namespace

class Opm::out::Summary::SummaryImplementation
{
public:
    explicit SummaryImplementation(const EclipseState&  es,
                                   const SummaryConfig& sumcfg,
                                   const EclipseGrid&   grid,
                                   const Schedule&      sched,
                                   const std::string&   basename);

    SummaryImplementation(const SummaryImplementation& rhs) = delete;
    SummaryImplementation(SummaryImplementation&& rhs) = default;
    SummaryImplementation& operator=(const SummaryImplementation& rhs) = delete;
    SummaryImplementation& operator=(SummaryImplementation&& rhs) = default;

    void eval(const EclipseState&            es,
              const Schedule&                sched,
              const int                      sim_step,
              const double                   duration,
              const data::WellRates&         well_solution,
              const data::Group&             group_solution,
              const GlobalProcessParameters& single_values,
              const RegionParameters&        region_values,
              const BlockValues&             block_values,
              SummaryState&                  st) const;

    void internal_store(const SummaryState& st, const int report_step);
    void write();

private:
    struct MiniStep
    {
        int id{0};
        int seq{-1};
        std::vector<float> params{};
    };

    using EvalPtr = SummaryOutputParameters::EvalPtr;

    std::reference_wrapper<const Opm::EclipseGrid> grid_;
    Opm::out::RegionCache regCache_;

    std::unique_ptr<SMSpecStreamDeferredCreation> deferredSMSpec_;

    Opm::EclIO::OutputStream::ResultSet rset_;
    Opm::EclIO::OutputStream::Formatted fmt_;
    Opm::EclIO::OutputStream::Unified   unif_;

    int miniStepID_{0};
    int prevCreate_{-1};
    int prevReportStepID_{-1};
    std::vector<MiniStep>::size_type numUnwritten_{0};

    SummaryOutputParameters  outputParameters_{};
    std::vector<EvalPtr>     requiredRestartParameters_{};
    std::vector<std::string> valueKeys_{};
    std::vector<MiniStep>    unwritten_{};

    std::unique_ptr<Opm::EclIO::OutputStream::SummarySpecification> smspec_{};
    std::unique_ptr<Opm::EclIO::EclOutput> stream_{};

    void configureTimeVectors(const EclipseState& es, const SummaryConfig& sumcfg);

    void configureSummaryInput(const EclipseState&  es,
                               const SummaryConfig& sumcfg,
                               const EclipseGrid&   grid,
                               const Schedule&      sched);

    void configureRequiredRestartParameters(const SummaryConfig& sumcfg,
                                            const Schedule&      sched);

    MiniStep& getNextMiniStep(const int report_step);
    const MiniStep& lastUnwritten() const;

    void write(const MiniStep& ms);

    void createSMSpecIfNecessary();
    void createSmryStreamIfNecessary(const int report_step);
};

Opm::out::Summary::SummaryImplementation::
SummaryImplementation(const EclipseState&  es,
                      const SummaryConfig& sumcfg,
                      const EclipseGrid&   grid,
                      const Schedule&      sched,
                      const std::string&   basename)
    : grid_          (std::cref(grid))
    , regCache_      (es.globalFieldProps().get_int("FIPNUM"), grid, sched)
    , deferredSMSpec_(makeDeferredSMSpecCreation(es, grid, sched))
    , rset_          (makeResultSet(es.cfg().io(), basename))
    , fmt_           { es.cfg().io().getFMTOUT() }
    , unif_          { es.cfg().io().getUNIFOUT() }
{
    this->configureTimeVectors(es, sumcfg);
    this->configureSummaryInput(es, sumcfg, grid, sched);
    this->configureRequiredRestartParameters(sumcfg, sched);
}

void Opm::out::Summary::SummaryImplementation::
internal_store(const SummaryState& st, const int report_step)
{
    auto& ms = this->getNextMiniStep(report_step);

    const auto nParam = this->valueKeys_.size();

    for (auto i = decltype(nParam){0}; i < nParam; ++i) {
        if (! st.has(this->valueKeys_[i]))
            // Parameter not yet evaluated (e.g., well/group not
            // yet active).  Nothing to do here.
            continue;

        ms.params[i] = st.get(this->valueKeys_[i]);
    }
}

void
Opm::out::Summary::SummaryImplementation::
eval(const EclipseState&            es,
     const Schedule&                sched,
     const int                      sim_step,
     const double                   duration,
     const data::WellRates&         well_solution,
     const data::Group&             group_solution,
     const GlobalProcessParameters& single_values,
     const RegionParameters&        region_values,
     const BlockValues&             block_values,
     Opm::SummaryState&             st) const
{
    const Evaluator::InputData input {
        es, sched, this->grid_, this->regCache_
    };

    const Evaluator::SimulatorResults simRes {
        well_solution, group_solution, single_values, region_values, block_values
    };

    for (auto& evalPtr : this->outputParameters_.getEvaluators()) {
        evalPtr->update(sim_step, duration, input, simRes, st);
    }

    for (auto& evalPtr : this->requiredRestartParameters_) {
        evalPtr->update(sim_step, duration, input, simRes, st);
    }
}

void Opm::out::Summary::SummaryImplementation::write()
{
    const auto zero = std::vector<MiniStep>::size_type{0};
    if (this->numUnwritten_ == zero)
        // No unwritten data.  Nothing to do so return early.
        return;

    this->createSMSpecIfNecessary();

    if (this->prevReportStepID_ < this->lastUnwritten().seq) {
        this->smspec_->write(this->outputParameters_.summarySpecification());
    }

    for (auto i = 0*this->numUnwritten_; i < this->numUnwritten_; ++i)
        this->write(this->unwritten_[i]);

    // Eagerly output last set of parameters to permanent storage.
    this->stream_->flushStream();

    // Reset "unwritten" counter to reflect the fact that we've
    // output all stored ministeps.
    this->numUnwritten_ = zero;
}

void Opm::out::Summary::SummaryImplementation::write(const MiniStep& ms)
{
    this->createSmryStreamIfNecessary(ms.seq);

    if (this->prevReportStepID_ < ms.seq) {
        // XXX: Should probably write SEQHDR = 0 here since
        ///     we do not know the actual encoding needed.
        this->stream_->write("SEQHDR", std::vector<int>{ ms.seq });
        this->prevReportStepID_ = ms.seq;
    }

    this->stream_->write("MINISTEP", std::vector<int>{ ms.id });
    this->stream_->write("PARAMS"  , ms.params);
}

void
Opm::out::Summary::SummaryImplementation::
configureTimeVectors(const EclipseState& es, const SummaryConfig& sumcfg)
{
    const auto dfltwgname = std::string(":+:+:+:+");
    const auto dfltnum    = 0;

    // XXX: Save keys might need/want to include a random component too.
    auto makeKey = [this](const std::string& keyword) -> void
    {
        this->valueKeys_.push_back(
            "SMSPEC.Internal." + keyword + ".Value.SAVE"
        );
    };

    // TIME
    {
        const auto& kw = std::string("TIME");
        makeKey(kw);

        const std::string& unit_string = es.getUnits().name(UnitSystem::measure::time);
        auto eval = std::make_unique<Evaluator::Time>(this->valueKeys_.back());

        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, unit_string, std::move(eval));
    }

    if (sumcfg.hasKeyword("DAY")) {
        const auto& kw = std::string("DAY");
        makeKey(kw);

        auto eval = std::make_unique<Evaluator::Day>(this->valueKeys_.back());
        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, "", std::move(eval));
    }

    if (sumcfg.hasKeyword("MONTH")) {
        const auto& kw = std::string("MONTH");
        makeKey(kw);

        auto eval = std::make_unique<Evaluator::Month>(this->valueKeys_.back());
        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, "", std::move(eval));
    }

    if (sumcfg.hasKeyword("YEAR")) {
        const auto& kw = std::string("YEAR");
        makeKey(kw);

        auto eval = std::make_unique<Evaluator::Year>(this->valueKeys_.back());
        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, "", std::move(eval));
    }

    // YEARS
    {
        const auto& kw = std::string("YEARS");
        makeKey(kw);

        auto eval = std::make_unique<Evaluator::Years>(this->valueKeys_.back());

        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, kw, std::move(eval));
    }
}

void
Opm::out::Summary::SummaryImplementation::
configureSummaryInput(const EclipseState&  es,
                      const SummaryConfig& sumcfg,
                      const EclipseGrid&   grid,
                      const Schedule&      sched)
{
    const auto st = SummaryState {
        std::chrono::system_clock::from_time_t(sched.getStartTime())
    };

    Evaluator::Factory fact {
        es, grid, st, sched.getUDQConfig(sched.size() - 1)
    };

    auto unsuppkw = std::vector<SummaryConfigNode>{};
    for (const auto& node : sumcfg) {
        auto prmDescr = fact.create(node);

        if (! prmDescr.evaluator) {
            // No known evaluation function/type for this keyword
            unsuppkw.push_back(node);
            continue;
        }

        // This keyword has a known evaluation method.

        this->valueKeys_.push_back(std::move(prmDescr.uniquekey));

        this->outputParameters_
            .makeParameter(node.keyword(),
                           makeWGName(node.namedEntity()),
                           node.number(),
                           std::move(prmDescr.unit),
                           std::move(prmDescr.evaluator));
    }

    if (! unsuppkw.empty())
        reportUnsupportedKeywords(std::move(unsuppkw));
}

void
Opm::out::Summary::SummaryImplementation::
configureRequiredRestartParameters(const SummaryConfig& sumcfg,
                                   const Schedule&      sched)
{
    auto makeEvaluator = [&sumcfg, this](const Opm::EclIO::SummaryNode& node) -> void
    {
        if (sumcfg.hasSummaryKey(node.unique_key()))
            // Handler already exists.  Don't add second evaluation.
            return;

        auto fcnPos = funs.find(node.keyword);
        assert ((fcnPos != funs.end()) &&
                "Internal error creating required restart vectors");

        auto eval = std::make_unique<
            Evaluator::FunctionRelation>(node, fcnPos->second);

        this->requiredRestartParameters_.push_back(std::move(eval));
    };

    for (const auto& node : requiredRestartVectors(sched))
        makeEvaluator(node);

    for (const auto& node : requiredSegmentVectors(sched))
        makeEvaluator(node);
}

Opm::out::Summary::SummaryImplementation::MiniStep&
Opm::out::Summary::SummaryImplementation::getNextMiniStep(const int report_step)
{
    if (this->numUnwritten_ == this->unwritten_.size())
        this->unwritten_.emplace_back();

    assert ((this->numUnwritten_ < this->unwritten_.size()) &&
            "Internal inconsistency in 'unwritten' counter");

    auto& ms = this->unwritten_[this->numUnwritten_++];

    ms.id  = this->miniStepID_++;  // MINSTEP IDs start at zero.
    ms.seq = report_step;

    ms.params.resize(this->valueKeys_.size(), 0.0f);

    std::fill(ms.params.begin(), ms.params.end(), 0.0f);

    return ms;
}

const Opm::out::Summary::SummaryImplementation::MiniStep&
Opm::out::Summary::SummaryImplementation::lastUnwritten() const
{
    assert (this->numUnwritten_ <= this->unwritten_.size());
    assert (this->numUnwritten_ >  decltype(this->numUnwritten_){0});

    return this->unwritten_[this->numUnwritten_ - 1];
}

void Opm::out::Summary::SummaryImplementation::createSMSpecIfNecessary()
{
    if (this->deferredSMSpec_) {
        // We need an SMSPEC file and none exists.  Create it and release
        // the resources captured to make the deferred creation call.
        this->smspec_ = this->deferredSMSpec_
            ->createStream(this->rset_, this->fmt_);

        this->deferredSMSpec_.reset();
    }
}

void
Opm::out::Summary::SummaryImplementation::
createSmryStreamIfNecessary(const int report_step)
{
    // Create stream if unset or if non-unified (separate) and new step.

    assert ((this->prevCreate_ <= report_step) &&
            "Inconsistent Report Step Sequence Detected");

    const auto do_create = ! this->stream_
        || (! this->unif_.set && (this->prevCreate_ < report_step));

    if (do_create) {
        this->stream_ = Opm::EclIO::OutputStream::
            createSummaryFile(this->rset_, report_step,
                              this->fmt_, this->unif_);

        this->prevCreate_ = report_step;
    }
}

namespace {

void validateElapsedTime(const double             secs_elapsed,
                         const Opm::EclipseState& es,
                         const Opm::SummaryState& st)
{
    if (! (secs_elapsed < st.get_elapsed()))
        return;

    const auto& usys    = es.getUnits();
    const auto  elapsed = usys.from_si(measure::time, secs_elapsed);
    const auto  prev_el = usys.from_si(measure::time, st.get_elapsed());
    const auto  unt     = '[' + std::string{ usys.name(measure::time) } + ']';

    throw std::invalid_argument {
        "Elapsed time ("
        + std::to_string(elapsed) + ' ' + unt
        + ") must not precede previous elapsed time ("
        + std::to_string(prev_el) + ' ' + unt
        + "). Incorrect restart time?"
    };
}

} // Anonymous namespace

namespace Opm { namespace out {

Summary::Summary(const EclipseState&  es,
                 const SummaryConfig& sumcfg,
                 const EclipseGrid&   grid,
                 const Schedule&      sched,
                 const std::string&   basename)
    : pImpl_(new SummaryImplementation(es, sumcfg, grid, sched, basename))
{}

void Summary::eval(SummaryState&                  st,
                   const int                      report_step,
                   const double                   secs_elapsed,
                   const EclipseState&            es,
                   const Schedule&                schedule,
                   const data::WellRates&         well_solution,
                   const data::Group&             group_solution,
                   const GlobalProcessParameters& single_values,
                   const RegionParameters&        region_values,
                   const BlockValues&             block_values) const
{
    validateElapsedTime(secs_elapsed, es, st);

    const double duration = secs_elapsed - st.get_elapsed();

    /* Report_step is the one-based sequence number of the containing report.
     * Report_step = 0 for the initial condition, before simulation starts.
     * We typically don't get reports_step = 0 here.  When outputting
     * separate summary files 'report_step' is the number that gets
     * incorporated into the filename extension.
     *
     * Sim_step is the timestep which has been effective in the simulator,
     * and as such is the value necessary to use when looking up active
     * wells, groups, connections &c in the Schedule object. */
    const auto sim_step = std::max( 0, report_step - 1 );

    this->pImpl_->eval(es, schedule, sim_step, duration,
                       well_solution, group_solution, single_values,
                       region_values, block_values, st);

    eval_udq(schedule, sim_step, st);

    st.update_elapsed(duration);
}

void Summary::add_timestep(const SummaryState& st, const int report_step)
{
    this->pImpl_->internal_store(st, report_step);
}

void Summary::write() const
{
    this->pImpl_->write();
}

Summary::~Summary() {}

}} // namespace Opm::out
