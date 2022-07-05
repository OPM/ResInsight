/*
  Copyright 2021 Equinor ASA.
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
#include <opm/output/eclipse/WStat.hpp>

#include <opm/common/OpmLog/OpmLog.hpp>
#include <opm/common/OpmLog/KeywordLocation.hpp>
#include <opm/common/utility/OpmInputError.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/output/eclipse/Inplace.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/AquiferConfig.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/AquiferCT.hpp>
#include <opm/input/eclipse/EclipseState/Aquifer/Aquifetp.hpp>
#include <opm/input/eclipse/EclipseState/EclipseState.hpp>
#include <opm/input/eclipse/EclipseState/IOConfig/IOConfig.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/ScheduleState.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQContext.hpp>
#include <opm/input/eclipse/Schedule/VFPProdTable.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>
#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>
#include <opm/input/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/input/eclipse/Units/UnitSystem.hpp>
#include <opm/input/eclipse/Units/Units.hpp>

#include <opm/io/eclipse/EclUtil.hpp>
#include <opm/io/eclipse/EclOutput.hpp>
#include <opm/io/eclipse/OutputStream.hpp>
#include <opm/io/eclipse/ExtSmryOutput.hpp>

#include <opm/output/data/Groups.hpp>
#include <opm/output/data/GuideRateValue.hpp>
#include <opm/output/data/Wells.hpp>
#include <opm/output/data/Aquifer.hpp>
#include <opm/output/eclipse/Inplace.hpp>
#include <opm/output/eclipse/RegionCache.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cctype>
#include <ctime>
#include <exception>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fmt/format.h>

template <> struct fmt::formatter<Opm::EclIO::SummaryNode::Category>: fmt::formatter<string_view> {
    // parse is inherited from formatter<string_view>.
    template <typename FormatContext>
    auto format(Opm::EclIO::SummaryNode::Category c, FormatContext& ctx) {
        using Category = Opm::EclIO::SummaryNode::Category;
        string_view name = "unknown";
        switch (c) {
        case Category::Well:          name = "Well"; break;
        case Category::Group:         name = "Group"; break;
        case Category::Field:         name = "Field"; break;
        case Category::Region:        name = "Region"; break;
        case Category::Block:         name = "Block"; break;
        case Category::Connection:    name = "Connection"; break;
        case Category::Segment:       name = "Segment"; break;
        case Category::Aquifer:       name = "Aquifer"; break;
        case Category::Node:          name = "Node"; break;
        case Category::Miscellaneous: name = "Miscellaneous"; break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};

namespace {
    struct ParamCTorArgs
    {
        std::string kw;
        Opm::EclIO::SummaryNode::Type type;
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
            ParamCTorArgs{ "OPTS", Type::Total },
            ParamCTorArgs{ "GPTS", Type::Total },
            ParamCTorArgs{ "OPTH", Type::Total },
            ParamCTorArgs{ "WPTH", Type::Total },
            ParamCTorArgs{ "GPTH", Type::Total },

            // Flow rate ratios (production)
            ParamCTorArgs{ "WCT" , Type::Ratio },
            ParamCTorArgs{ "GOR" , Type::Ratio },

            // injection
            ParamCTorArgs{ "OIR" , Type::Rate },
            ParamCTorArgs{ "WIR" , Type::Rate },
            ParamCTorArgs{ "GIR" , Type::Rate },
            ParamCTorArgs{ "VIR" , Type::Rate },
            ParamCTorArgs{ "OPI" , Type::Rate },
            ParamCTorArgs{ "WPI" , Type::Rate },
            ParamCTorArgs{ "GPI" , Type::Rate },
            ParamCTorArgs{ "OIT" , Type::Total },
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
        auto entities = std::vector<Opm::EclIO::SummaryNode> {};

        const auto vectors = requiredRestartVectors();
        const auto extra_well_vectors = std::vector<ParamCTorArgs> {
            { "WTHP",  Opm::EclIO::SummaryNode::Type::Pressure },
            { "WBHP",  Opm::EclIO::SummaryNode::Type::Pressure },
            { "WGVIR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "WWVIR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "WOPGR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "WGPGR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "WWPGR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "WGIGR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "WWIGR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "WMCTL", Opm::EclIO::SummaryNode::Type::Mode     },
            { "WGLIR", Opm::EclIO::SummaryNode::Type::Rate     },
        };
        const auto extra_group_vectors = std::vector<ParamCTorArgs> {
            { "GOPGR", Opm::EclIO::SummaryNode::Type::Rate },
            { "GGPGR", Opm::EclIO::SummaryNode::Type::Rate },
            { "GWPGR", Opm::EclIO::SummaryNode::Type::Rate },
            { "GGIGR", Opm::EclIO::SummaryNode::Type::Rate },
            { "GWIGR", Opm::EclIO::SummaryNode::Type::Rate },
            { "GMCTG", Opm::EclIO::SummaryNode::Type::Mode },
            { "GMCTP", Opm::EclIO::SummaryNode::Type::Mode },
            { "GMCTW", Opm::EclIO::SummaryNode::Type::Mode },
            { "GMWPR", Opm::EclIO::SummaryNode::Type::Mode },
            { "GMWIN", Opm::EclIO::SummaryNode::Type::Mode },
            { "GPR",   Opm::EclIO::SummaryNode::Type::Pressure },
        };

        const auto extra_field_vectors = std::vector<ParamCTorArgs> {
            { "FMCTG", Opm::EclIO::SummaryNode::Type::Mode },
            { "FMCTP", Opm::EclIO::SummaryNode::Type::Mode },
            { "FMCTW", Opm::EclIO::SummaryNode::Type::Mode },
            { "FMWPR", Opm::EclIO::SummaryNode::Type::Mode },
            { "FMWIN", Opm::EclIO::SummaryNode::Type::Mode },
        };

        const auto extra_connection_vectors = std::vector<ParamCTorArgs> {
            {"COPR", Opm::EclIO::SummaryNode::Type::Rate},
            {"CWPR", Opm::EclIO::SummaryNode::Type::Rate},
            {"CGPR", Opm::EclIO::SummaryNode::Type::Rate},
            {"CVPR", Opm::EclIO::SummaryNode::Type::Rate},
            {"COPT", Opm::EclIO::SummaryNode::Type::Total},
            {"CWPT", Opm::EclIO::SummaryNode::Type::Total},
            {"CGPT", Opm::EclIO::SummaryNode::Type::Total},
            {"CVPT", Opm::EclIO::SummaryNode::Type::Total},
            {"COIR", Opm::EclIO::SummaryNode::Type::Rate},
            {"CWIR", Opm::EclIO::SummaryNode::Type::Rate},
            {"CGIR", Opm::EclIO::SummaryNode::Type::Rate},
            {"CVIR", Opm::EclIO::SummaryNode::Type::Rate},
            {"COIT", Opm::EclIO::SummaryNode::Type::Total},
            {"CWIT", Opm::EclIO::SummaryNode::Type::Total},
            {"CGIT", Opm::EclIO::SummaryNode::Type::Total},
            {"CVIT", Opm::EclIO::SummaryNode::Type::Total},
            {"CPR",  Opm::EclIO::SummaryNode::Type::Pressure},
            {"CGOR", Opm::EclIO::SummaryNode::Type::Ratio},
        };

        using Cat = Opm::EclIO::SummaryNode::Category;

        auto makeEntities = [&vectors, &entities]
            (const char                        kwpref,
             const Cat                         cat,
             const std::vector<ParamCTorArgs>& extra_vectors,
             const std::string&                name) -> void
        {
            const auto dflt_num = Opm::EclIO::SummaryNode::default_number;

            // Recall: Cannot use emplace_back() for PODs.
            for (const auto& vector : vectors) {
                entities.push_back({ kwpref + vector.kw, cat,
                                     vector.type, name, dflt_num, {}, {} });
            }

            for (const auto& extra_vector : extra_vectors) {
                entities.push_back({ extra_vector.kw, cat,
                                     extra_vector.type, name, dflt_num, {}, {} });
            }
        };

        for (const auto& well_name : sched.wellNames()) {
            makeEntities('W', Cat::Well, extra_well_vectors, well_name);

            const auto& well = sched.getWellatEnd(well_name);
            for (const auto& conn : well.getConnections()) {
                for (const auto& conn_vector : extra_connection_vectors)
                    entities.push_back( {conn_vector.kw, Cat::Connection, conn_vector.type, well.name(), static_cast<int>(conn.global_index() + 1), {}, {}} );
            }
        }

        for (const auto& grp_name : sched.groupNames()) {
            if (grp_name == "FIELD") { continue; }

            makeEntities('G', Cat::Group, extra_group_vectors, grp_name);
        }

        makeEntities('F', Cat::Field, extra_field_vectors, "FIELD");

        return entities;
    }

    std::vector<Opm::EclIO::SummaryNode>
    requiredSegmentVectors(const ::Opm::Schedule& sched)
    {
        auto entities = std::vector<Opm::EclIO::SummaryNode> {};

        const auto vectors = std::vector<ParamCTorArgs> {
            { "SOFR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "SGFR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "SWFR", Opm::EclIO::SummaryNode::Type::Rate     },
            { "SPR",  Opm::EclIO::SummaryNode::Type::Pressure },
        };

        using Cat = Opm::EclIO::SummaryNode::Category;

        auto makeVectors = [&](const Opm::Well& well) -> void
        {
            const auto& wname = well.name();
            const auto  nSeg  = static_cast<int>(well.getSegments().size());

            for (auto segID = 0*nSeg + 1; segID <= nSeg; ++segID) {
                for (const auto& vector : vectors) {
                    entities.push_back({ vector.kw, Cat::Segment,
                                         vector.type, wname, segID, {}, {} });
                }
            }
        };

        for (const auto& wname : sched.wellNames()) {
            const auto& well = sched.getWellatEnd(wname);

            if (! well.isMultiSegment()) {
                // Don't allocate MS summary vectors for non-MS wells.
                continue;
            }

            makeVectors(well);
        }

        return entities;
    }

    std::vector<Opm::EclIO::SummaryNode>
    requiredAquiferVectors(const std::vector<int>& aquiferIDs)
    {
        auto entities = std::vector<Opm::EclIO::SummaryNode> {};

        const auto vectors = std::vector<ParamCTorArgs> {
            { "AAQR" , Opm::EclIO::SummaryNode::Type::Rate      },
            { "AAQP" , Opm::EclIO::SummaryNode::Type::Pressure  },
            { "AAQT" , Opm::EclIO::SummaryNode::Type::Total     },
            { "AAQTD", Opm::EclIO::SummaryNode::Type::Undefined },
            { "AAQPD", Opm::EclIO::SummaryNode::Type::Undefined },
        };

        using Cat = Opm::EclIO::SummaryNode::Category;

        for (const auto& aquiferID : aquiferIDs) {
            for (const auto& vector : vectors) {
                entities.push_back({ vector.kw, Cat::Aquifer,
                                     vector.type, "", aquiferID, {}, {} });
            }
        }

        return entities;
    }

    std::vector<Opm::EclIO::SummaryNode>
    requiredNumericAquiferVectors(const std::vector<int>& aquiferIDs)
    {
        auto entities = std::vector<Opm::EclIO::SummaryNode> {};

        const auto vectors = std::vector<ParamCTorArgs> {
            { "ANQR" , Opm::EclIO::SummaryNode::Type::Rate     },
            { "ANQP" , Opm::EclIO::SummaryNode::Type::Pressure },
            { "ANQT" , Opm::EclIO::SummaryNode::Type::Total    },
        };

        using Cat = Opm::EclIO::SummaryNode::Category;

        for (const auto& aquiferID : aquiferIDs) {
            for (const auto& vector : vectors) {
                entities.push_back({ vector.kw, Cat::Aquifer,
                                     vector.type, "", aquiferID, {}, {} });
            }
        }

        return entities;
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

    if( denom == measure::mass_rate &&
        div   == measure::liquid_surface_rate )
        return measure::polymer_density;

    if( denom == measure::energy_rate &&
        div   == measure::time )
        return measure::energy;

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

    if(  lhs == measure::energy_rate && rhs == measure::time)
        return measure::energy;

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
struct fn_args
{
    const std::vector<const Opm::Well*>& schedule_wells;
    const std::string group_name;
    const std::string keyword_name;
    double duration;
    const int sim_step;
    int  num;
    const std::optional<std::variant<std::string, int>> extra_data;
    const Opm::SummaryState& st;
    const Opm::data::Wells& wells;
    const Opm::data::GroupAndNetworkValues& grp_nwrk;
    const Opm::out::RegionCache& regionCache;
    const Opm::EclipseGrid& grid;
    const Opm::Schedule& schedule;
    const std::vector< std::pair< std::string, double > > eff_factors;
    const Opm::Inplace& initial_inplace;
    const Opm::Inplace& inplace;
    const Opm::UnitSystem& unit_system;
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

template <Opm::data::GuideRateValue::Item>
measure rate_unit() { return measure::liquid_surface_rate; }

template<> constexpr
measure rate_unit< rt::gas >() { return measure::gas_surface_rate; }
template<> constexpr
measure rate_unit< Opm::Phase::GAS >() { return measure::gas_surface_rate; }

template<> constexpr
measure rate_unit< rt::dissolved_gas >() { return measure::gas_surface_rate; }

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

template <> constexpr
measure rate_unit<Opm::data::GuideRateValue::Item::Gas>() { return measure::gas_surface_rate; }

template <> constexpr
measure rate_unit<Opm::data::GuideRateValue::Item::ResV>() { return measure::rate; }

double efac( const std::vector<std::pair<std::string,double>>& eff_factors, const std::string& name)
{
    auto it = std::find_if(eff_factors.begin(), eff_factors.end(),
        [&name](const std::pair<std::string, double>& elem)
    {
        return elem.first == name;
    });

    return (it != eff_factors.end()) ? it->second : 1.0;
}

inline bool
has_vfp_table(const Opm::ScheduleState&            sched_state,
              int vfp_table_number)
{
    return sched_state.vfpprod.has(vfp_table_number);
}

inline Opm::VFPProdTable::ALQ_TYPE
alq_type(const Opm::ScheduleState&            sched_state,
         int vfp_table_number)
{
    return sched_state.vfpprod(vfp_table_number).getALQType();
}

inline quantity artificial_lift_quantity( const fn_args& args ) {
    // Note: This function is intentionally supported only at the well level
    // (meaning there's no loop over args.schedule_wells by intention).  Its
    // purpose is to calculate WALQ only.

    // Note: in order to determine the correct dimension to use the Summary code
    // calls the various evaluator functions with a default constructed fn_args
    // instance. In the case of the WALQ function this does not really work,
    // because the correct output dimension depends on exactly what physical
    // quantity is represented by the ALQ - and that again requires quite some
    // context to determine correctly. The current hack is that if WLIFTOPT is
    // configured for at least one well we use dimension
    // measure::gas_surface_rate - otherwise we use measure::identity.

    auto dimension = measure::identity;
    const auto& glo = args.schedule[args.sim_step].glo();
    if (glo.num_wells() != 0)
        dimension = measure::gas_surface_rate;

    auto zero = quantity{0, dimension};
    if (args.schedule_wells.empty()) {
        return zero;
    }

    const auto* well = args.schedule_wells.front();
    if (well->isInjector()) {
        return zero;
    }

    auto xwPos = args.wells.find(well->name());
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        return zero;
    }

    const auto& production = well->productionControls(args.st);
    if (!glo.has_well(well->name()))
        return { production.alq_value, dimension};

    const auto& sched_state = args.schedule[args.sim_step];
    if (alq_type(sched_state, production.vfp_table_number) != Opm::VFPProdTable::ALQ_TYPE::ALQ_GRAT)
        return zero;

    const double eff_fac = efac(args.eff_factors, well->name());
    auto alq_rate = eff_fac * xwPos->second.rates.get(rt::alq, production.alq_value);
    return { alq_rate, dimension };
}


inline quantity glir( const fn_args& args ) {
    if (args.schedule_wells.empty()) {
        return { 0.0, measure::gas_surface_rate };
    }

    const auto& sched_state = args.schedule[args.sim_step];

    double alq_rate = 0.0;
    for (const auto* well : args.schedule_wells) {
        if (well->isInjector()) {
            continue;
        }

        auto xwPos = args.wells.find(well->name());
        if ((xwPos == args.wells.end()) ||
            (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
        {
            continue;
        }

        const auto& production = well->productionControls(args.st);
        if (! has_vfp_table(sched_state, production.vfp_table_number)) {
            continue;
        }

        const auto thisAlqType = alq_type(sched_state, production.vfp_table_number);
        if (thisAlqType == Opm::VFPProdTable::ALQ_TYPE::ALQ_GRAT) {
            const double eff_fac = efac(args.eff_factors, well->name());
            alq_rate += eff_fac * xwPos->second.rates.get(rt::alq, production.alq_value);
        }

        if (thisAlqType == Opm::VFPProdTable::ALQ_TYPE::ALQ_IGLR) {
            const double eff_fac = efac(args.eff_factors, well->name());
            auto glr = production.alq_value;
            auto wpr = xwPos->second.rates.get(rt::wat);
            auto opr = xwPos->second.rates.get(rt::oil);
            alq_rate -= eff_fac * glr * (wpr + opr);
        }
    }

    return { alq_rate, measure::gas_surface_rate };
}

inline quantity wwirt( const fn_args& args ) {
    const quantity zero = { 0, rate_unit< Opm::Phase::WATER >() };

    if (args.schedule_wells.empty()) {
        return zero;
    }

    const auto* well  = args.schedule_wells.front();
    const auto& wtype = well->wellType();
    if (wtype.producer() || (wtype.injector_type() != Opm::InjectorType::WATER)) {
        return zero;
    }

    const auto& injection = well->injectionControls(args.st);
    return { injection.surface_rate, rate_unit<Opm::Phase::WATER>() };
}

template< rt phase, bool injection = true >
inline quantity rate( const fn_args& args ) {
    double sum = 0.0;

    for (const auto* sched_well : args.schedule_wells) {
        const auto& name = sched_well->name();

        auto xwPos = args.wells.find(name);
        if ((xwPos == args.wells.end()) ||
            (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
        {
            continue;
        }

        const double eff_fac = efac(args.eff_factors, name);
        const auto v = xwPos->second.rates.get(phase, 0.0) * eff_fac;

        if ((v > 0.0) == injection) {
            sum += v;
        }
    }

    if (! injection) {
        sum *= -1.0;
    }

    if (phase == rt::polymer || phase == rt::brine) {
        return { sum, measure::mass_rate };
    }

    return { sum, rate_unit< phase >() };
}

template< rt tracer, rt phase, bool injection = true >
inline quantity ratetracer( const fn_args& args ) {
    double sum = 0.0;

    std::string tracer_name = args.keyword_name.substr(4);

    for (const auto* sched_well : args.schedule_wells) {
        const auto& name = sched_well->name();

        auto xwPos = args.wells.find(name);
        if ((xwPos == args.wells.end()) ||
            (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
        {
            continue;
        }

        const double eff_fac = efac(args.eff_factors, name);
        const auto v = xwPos->second.rates.get(tracer, 0.0, tracer_name) * eff_fac;

        if ((v > 0.0) == injection) {
            sum += v;
        }
    }

    if (! injection) {
        sum *= -1.0;
    }

    return { sum, rate_unit< phase >() };
}

template< rt phase, bool injection = true >
inline quantity ratel( const fn_args& args ) {
    const auto unit = ((phase == rt::polymer) || (phase == rt::brine))
        ? measure::mass_rate : rate_unit<phase>();

    const quantity zero = { 0.0, unit };

    if (args.schedule_wells.empty())
        return zero;

    const auto* well = args.schedule_wells.front();
    const auto& name = well->name();

    auto xwPos = args.wells.find(name);
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT) ||
        (xwPos->second.current_control.isProducer == injection))
    {
        return zero;
    }

    const double eff_fac = efac(args.eff_factors, name);
    const auto& well_data = xwPos->second;

    double sum = 0;
    const auto& connections = well->getConnections( args.num );
    for (const auto* conn_ptr : connections) {
        const size_t global_index = conn_ptr->global_index();
        const auto& conn_data =
            std::find_if(well_data.connections.begin(),
                         well_data.connections.end(),
                [global_index](const Opm::data::Connection& cdata)
            {
                return cdata.index == global_index;
            });

        if (conn_data != well_data.connections.end()) {
            sum += conn_data->rates.get(phase, 0.0) * eff_fac;
        }
    }

    if (! injection) {
        sum *= -1;
    }

    return { sum, unit };
}

inline quantity cpr( const fn_args& args ) {
    const quantity zero = { 0, measure::pressure };
    // The args.num value is the literal value which will go to the
    // NUMS array in the eclipse SMSPEC file; the values in this array
    // are offset 1 - whereas we need to use this index here to look
    // up a completion with offset 0.
    const size_t global_index = args.num - 1;
    if (args.schedule_wells.empty())
        return zero;

    const auto& name = args.schedule_wells.front()->name();
    auto xwPos = args.wells.find(name);
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
        return zero;

    const auto& well_data = xwPos->second;
    const auto& connection =
        std::find_if(well_data.connections.begin(),
                     well_data.connections.end(),
            [global_index](const Opm::data::Connection& c)
        {
            return c.index == global_index;
        });

    if (connection == well_data.connections.end())
        return zero;

    return { connection->pressure, measure::pressure };
}

template< rt phase, bool injection = true >
inline quantity cratel( const fn_args& args ) {
    const auto unit = ((phase == rt::polymer) || (phase == rt::brine))
        ? measure::mass_rate : rate_unit<phase>();

    const quantity zero = { 0.0, unit };

    if (args.schedule_wells.empty())
        return zero;

    const auto* well = args.schedule_wells.front();
    const auto& name = well->name();

    auto xwPos = args.wells.find(name);
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT) ||
        (xwPos->second.current_control.isProducer == injection))
    {
        return zero;
    }

    const auto complnum = getCompletionNumberFromGlobalConnectionIndex(well->getConnections(), args.num - 1);
    if (!complnum.has_value())
        // Connection might not yet have come online.
        return zero;

    const auto& well_data = xwPos->second;
    const double eff_fac = efac(args.eff_factors, name);

    double sum = 0;
    const auto& connections = well->getConnections(*complnum);
    for (const auto& conn_ptr : connections) {
        const size_t global_index = conn_ptr->global_index();
        const auto& conn_data =
            std::find_if(well_data.connections.begin(),
                         well_data.connections.end(),
                [global_index] (const Opm::data::Connection& cdata)
            {
                return cdata.index == global_index;
            });

        if (conn_data != well_data.connections.end()) {
            sum += conn_data->rates.get( phase, 0.0 ) * eff_fac;
        }
    }

    if (! injection) {
        sum *= -1;
    }

    return { sum, unit };
}

template< bool injection >
inline quantity flowing( const fn_args& args ) {
    const auto& wells = args.wells;
    auto pred = [&wells]( const Opm::Well* w ) -> bool
    {
        auto xwPos = wells.find(w->name());
        return (xwPos != wells.end())
            && (w->isInjector( ) == injection)
            && (xwPos->second.dynamicStatus == Opm::Well::Status::OPEN)
            && xwPos->second.flowing();
    };

    return { double( std::count_if( args.schedule_wells.begin(),
                                    args.schedule_wells.end(),
                                    pred ) ),
             measure::identity };
}

template< rt phase, bool injection = true>
inline quantity crate( const fn_args& args ) {
    const quantity zero = { 0, rate_unit< phase >() };
    // The args.num value is the literal value which will go to the
    // NUMS array in the eclipse SMSPEC file; the values in this array
    // are offset 1 - whereas we need to use this index here to look
    // up a completion with offset 0.
    const size_t global_index = args.num - 1;
    if (args.schedule_wells.empty())
        return zero;

    const auto& name = args.schedule_wells.front()->name();
    auto xwPos = args.wells.find(name);
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT) ||
        (xwPos->second.current_control.isProducer == injection))
    {
        return zero;
    }

    const auto& well_data = xwPos->second;
    const auto& completion =
        std::find_if(well_data.connections.begin(),
                     well_data.connections.end(),
            [global_index](const Opm::data::Connection& c)
        {
            return c.index == global_index;
        });

    if (completion == well_data.connections.end())
        return zero;

    const double eff_fac = efac( args.eff_factors, name );
    auto v = completion->rates.get( phase, 0.0 ) * eff_fac;
    if (! injection)
        v *= -1;

    if (phase == rt::polymer || phase == rt::brine)
        return { v, measure::mass_rate };

    return { v, rate_unit< phase >() };
}

template <bool injection = true>
quantity crate_resv( const fn_args& args ) {
    const quantity zero = { 0.0, rate_unit<rt::reservoir_oil>() };
    if (args.schedule_wells.empty())
        return zero;

    const auto& name = args.schedule_wells.front()->name();
    auto xwPos = args.wells.find(name);
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT) ||
        (xwPos->second.current_control.isProducer == injection))
    {
        return zero;
    }

    // The args.num value is the literal value which will go to the
    // NUMS array in the eclipse SMSPEC file; the values in this array
    // are offset 1 - whereas we need to use this index here to look
    // up a completion with offset 0.
    const auto global_index = static_cast<std::size_t>(args.num - 1);

    const auto& well_data = xwPos->second;
    const auto completion =
        std::find_if(well_data.connections.begin(),
                     well_data.connections.end(),
            [global_index](const Opm::data::Connection& c)
        {
            return c.index == global_index;
        });

    if (completion == well_data.connections.end())
        return zero;

    const auto eff_fac = efac( args.eff_factors, name );
    auto v = completion->reservoir_rate * eff_fac;
    if (! injection)
        v *= -1;

    return { v, rate_unit<rt::reservoir_oil>() };
}

template< rt phase>
inline quantity srate( const fn_args& args ) {
    const quantity zero = { 0, rate_unit< phase >() };
    // The args.num value is the literal value which will go to the
    // NUMS array in the eclispe SMSPEC file; the values in this array
    // are offset 1 - whereas we need to use this index here to look
    // up a completion with offset 0.
    if (args.schedule_wells.empty()) {
        return zero;
    }

    const auto& name = args.schedule_wells.front()->name();
    auto xwPos = args.wells.find(name);
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        return zero;
    }

    const auto& well_data = xwPos->second;

    const size_t segNumber = args.num;
    const auto& segment = well_data.segments.find(segNumber);

    if (segment == well_data.segments.end())
        return zero;

    const double eff_fac = efac(args.eff_factors, name);
    auto v = segment->second.rates.get( phase, 0.0 ) * eff_fac;

    //switch sign of rate - opposite convention in flow vs eclipse
    v *= -1;

    if (phase == rt::polymer || phase == rt::brine)
        return { v, measure::mass_rate };

    return { v, rate_unit< phase >() };
}

inline quantity trans_factors ( const fn_args& args ) {
    const quantity zero = { 0.0, measure::transmissibility };

    if (args.schedule_wells.empty())
        // No wells.  Before simulation starts?
        return zero;

    auto xwPos = args.wells.find(args.schedule_wells.front()->name());
    if (xwPos == args.wells.end())
        // No dynamic results for this well.  Not open?
        return zero;

    // Like completion rate we need to look up a connection with offset 0.
    const size_t global_index = args.num - 1;
    const auto& connections = xwPos->second.connections;
    auto connPos = std::find_if(connections.begin(), connections.end(),
        [global_index](const Opm::data::Connection& c)
    {
        return c.index == global_index;
    });

    if (connPos == connections.end())
        // No dynamic results for this connection.
        return zero;

    // Dynamic connection result's "trans_factor" includes PI-adjustment.
    return { connPos->trans_factor, measure::transmissibility };
}

template <Opm::data::SegmentPressures::Value ix>
inline quantity segpress ( const fn_args& args )
{
    const quantity zero = { 0, measure::pressure };

    if (args.schedule_wells.empty())
        return zero;

    const auto* well = args.schedule_wells.front();
    auto xwPos = args.wells.find(well->name());
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        return zero;
    }

    // Like completion rate we need to look up a connection with offset 0.
    const size_t segNumber = args.num;

    const auto& well_data = xwPos->second;
    const auto& segment = well_data.segments.find(segNumber);

    if (segment == well_data.segments.end()) {
        return zero;
    }

    return { segment->second.pressures[ix], measure::pressure };
}

inline quantity wstat( const fn_args& args ) {
    const quantity zero = { Opm::WStat::numeric::UNKNOWN, measure::identity};
    if (args.schedule_wells.empty())
        return zero;
    const auto& sched_well = args.schedule_wells.front();
    const auto& arg_well = args.wells.find(sched_well->name());

    if (arg_well == args.wells.end() || arg_well->second.dynamicStatus == Opm::Well::Status::SHUT)
        return {Opm::WStat::numeric::SHUT, measure::identity};

    if (arg_well->second.dynamicStatus == Opm::Well::Status::STOP)
        return {Opm::WStat::numeric::STOP, measure::identity};

    if (sched_well->isInjector())
        return {Opm::WStat::numeric::INJ, measure::identity};

    return {Opm::WStat::numeric::PROD, measure::identity};
}

inline quantity bhp( const fn_args& args ) {
    const quantity zero = { 0, measure::pressure };
    if (args.schedule_wells.empty())
        return zero;

    const auto p = args.wells.find(args.schedule_wells.front()->name());
    if ((p == args.wells.end()) ||
        (p->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        return zero;
    }

    return { p->second.bhp, measure::pressure };
}

/*
  This function is slightly ugly - the evaluation of ROEW uses the already
  calculated COPT results. We do not really have any formalism for such
  dependencies between the summary vectors. For this particualar case there is a
  hack in SummaryConfig which should ensure that this is safe.
*/

quantity roew(const fn_args& args) {
    const quantity zero = { 0, measure::identity };
    const auto& region_name = std::get<std::string>(*args.extra_data);
    if (!args.initial_inplace.has( region_name, Opm::Inplace::Phase::OIL, args.num))
        return zero;

    double oil_prod = 0;
    for (const auto& [well, global_index] : args.regionCache.connections(region_name, args.num)) {
        const auto copt_key = fmt::format("COPT:{}:{}" , well, global_index + 1);
        if (args.st.has(copt_key))
            oil_prod += args.st.get(copt_key);
    }
    oil_prod = args.unit_system.to_si(Opm::UnitSystem::measure::volume, oil_prod);
    return { oil_prod / args.initial_inplace.get( region_name, Opm::Inplace::Phase::OIL, args.num ) , measure::identity };
}

template< bool injection = true>
inline quantity temperature( const fn_args& args ) {
    const quantity zero = { 0, measure::temperature };
    if (args.schedule_wells.empty())
        return zero;

    const auto p = args.wells.find(args.schedule_wells.front()->name());
    if ((p == args.wells.end()) ||
        (p->second.dynamicStatus == Opm::Well::Status::SHUT) ||
        (p->second.current_control.isProducer == injection))
    {
        return zero;
    }

    return { p->second.temperature, measure::temperature };
}

inline quantity thp( const fn_args& args ) {
    const quantity zero = { 0, measure::pressure };
    if (args.schedule_wells.empty())
        return zero;

    const auto p = args.wells.find(args.schedule_wells.front()->name());
    if ((p == args.wells.end()) ||
        (p->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        return zero;
    }

    return { p->second.thp, measure::pressure };
}

inline quantity bhp_history( const fn_args& args ) {
    if( args.schedule_wells.empty() ) return { 0.0, measure::pressure };

    const auto* sched_well = args.schedule_wells.front();

    const auto bhp_hist = sched_well->isProducer()
        ? sched_well->getProductionProperties().BHPH
        : sched_well->getInjectionProperties() .BHPH;

    return { bhp_hist, measure::pressure };
}

inline quantity thp_history( const fn_args& args ) {
    if( args.schedule_wells.empty() ) return { 0.0, measure::pressure };

    const auto* sched_well = args.schedule_wells.front();

    const auto thp_hist = sched_well->isProducer()
        ? sched_well->getProductionProperties().THPH
        : sched_well->getInjectionProperties() .THPH;

    return { thp_hist, measure::pressure };
}

inline quantity node_pressure(const fn_args& args)
{
    auto nodePos = args.grp_nwrk.nodeData.find(args.group_name);
    if (nodePos == args.grp_nwrk.nodeData.end()) {
        return { 0.0, measure::pressure };
    }

    return { nodePos->second.pressure, measure::pressure };
}

template< Opm::Phase phase >
inline quantity production_history( const fn_args& args )
{
    /*
     * For well data, looking up historical rates (both for production and
     * injection) before simulation actually starts is impossible and
     * nonsensical. We therefore default to writing zero (which is what eclipse
     * seems to do as well).
     */

    double sum = 0.0;
    for (const auto* sched_well : args.schedule_wells) {
        const double eff_fac = efac( args.eff_factors, sched_well->name() );
        sum += sched_well->production_rate( args.st, phase ) * eff_fac;
    }

    return { sum, rate_unit< phase >() };
}

template< Opm::Phase phase >
inline quantity injection_history( const fn_args& args )
{
    double sum = 0.0;
    for (const auto* sched_well : args.schedule_wells) {
        const double eff_fac = efac( args.eff_factors, sched_well->name() );
        sum += sched_well->injection_rate( args.st, phase ) * eff_fac;
    }

    return { sum, rate_unit< phase >() };
}

template< bool injection >
inline quantity abandoned_well( const fn_args& args ) {
    std::size_t count = 0;

    for (const auto* sched_well : args.schedule_wells) {
        if (injection && !sched_well->hasInjected())
            continue;

        if (!injection && !sched_well->hasProduced())
            continue;

        const auto& well_name = sched_well->name();
        auto well_iter = args.wells.find( well_name );
        if (well_iter == args.wells.end()) {
            count += 1;
            continue;
        }

        count += !well_iter->second.flowing();
    }

    return { 1.0 * count, measure::identity };
}

inline quantity res_vol_production_target( const fn_args& args )
{
    double sum = 0.0;
    for (const auto* sched_well : args.schedule_wells)
        if (sched_well->getProductionProperties().predictionMode)
            sum += sched_well->getProductionProperties().ResVRate.getSI();

    return { sum, measure::rate };
}

inline quantity duration( const fn_args& args ) {
    return { args.duration, measure::time };
}

template<rt phase , bool injection>
quantity region_rate( const fn_args& args ) {
    double sum = 0;
    const auto& well_connections = args.regionCache.connections( std::get<std::string>(*args.extra_data), args.num );

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

quantity rhpv(const fn_args& args) {
    const auto& inplace = args.inplace;
    const auto& region_name = std::get<std::string>(*args.extra_data);
    if (inplace.has( region_name, Opm::Inplace::Phase::HydroCarbonPV, args.num ))
        return { inplace.get( region_name, Opm::Inplace::Phase::HydroCarbonPV, args.num ), measure::volume };
    else
        return {0, measure::volume};
}

template < rt phase, bool outputProducer = true, bool outputInjector = true>
inline quantity potential_rate( const fn_args& args )
{
    double sum = 0.0;

    for (const auto* sched_well : args.schedule_wells) {
        const auto& name = sched_well->name();

        auto xwPos = args.wells.find(name);
        if ((xwPos == args.wells.end()) ||
            (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
        {
            continue;
        }

        if (sched_well->isInjector() && outputInjector) {
	    const auto v = xwPos->second.rates.get(phase, 0.0);
	    sum += v * efac(args.eff_factors, name);
	}
	else if (sched_well->isProducer() && outputProducer) {
	    const auto v = xwPos->second.rates.get(phase, 0.0);
	    sum += v * efac(args.eff_factors, name);
	}
    }

    return { sum, rate_unit< phase >() };
}

inline quantity preferred_phase_productivty_index(const fn_args& args)
{
    if (args.schedule_wells.empty())
        return {0.0, rate_unit<rt::productivity_index_oil>()};

    const auto* well = args.schedule_wells.front();
    const auto preferred_phase = well->getPreferredPhase();
    if (well->getStatus() == Opm::Well::Status::OPEN) {
        switch (preferred_phase) {
        case Opm::Phase::OIL:
            return potential_rate<rt::productivity_index_oil>(args);

        case Opm::Phase::GAS:
            return potential_rate<rt::productivity_index_gas>(args);

        case Opm::Phase::WATER:
            return potential_rate<rt::productivity_index_water>(args);

        default:
            break;
        }
    }
    else {
        switch (preferred_phase) {
        case Opm::Phase::OIL:
            return {0.0, rate_unit<rt::productivity_index_oil>()};

        case Opm::Phase::GAS:
            return {0.0, rate_unit<rt::productivity_index_gas>()};

        case Opm::Phase::WATER:
            return {0.0, rate_unit<rt::productivity_index_water>()};

        default:
            break;
        }
    }

    throw std::invalid_argument {
        fmt::format("Unsupported \"preferred\" phase: {}",
                    static_cast<int>(args.schedule_wells.front()->getPreferredPhase()))
    };
}

inline quantity connection_productivity_index(const fn_args& args)
{
    const quantity zero = { 0.0, rate_unit<rt::productivity_index_oil>() };

    if (args.schedule_wells.empty())
        return zero;

    auto xwPos = args.wells.find(args.schedule_wells.front()->name());
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        return zero;
    }

    // The args.num value is the literal value which will go to the
    // NUMS array in the eclipse SMSPEC file; the values in this array
    // are offset 1 - whereas we need to use this index here to look
    // up a completion with offset 0.
    const auto global_index = static_cast<std::size_t>(args.num) - 1;

    const auto& xcon = xwPos->second.connections;
    const auto& completion =
        std::find_if(xcon.begin(), xcon.end(),
            [global_index](const Opm::data::Connection& c)
        {
            return c.index == global_index;
        });

    if (completion == xcon.end())
        return zero;

    switch (args.schedule_wells.front()->getPreferredPhase()) {
    case Opm::Phase::OIL:
        return { completion->rates.get(rt::productivity_index_oil, 0.0),
                 rate_unit<rt::productivity_index_oil>() };

    case Opm::Phase::GAS:
        return { completion->rates.get(rt::productivity_index_gas, 0.0),
                 rate_unit<rt::productivity_index_gas>() };

    case Opm::Phase::WATER:
        return { completion->rates.get(rt::productivity_index_water, 0.0),
                 rate_unit<rt::productivity_index_water>() };

    default:
        break;
    }

    throw std::invalid_argument {
        fmt::format("Unsupported \"preferred\" phase: {}",
                    static_cast<int>(args.schedule_wells.front()->getPreferredPhase()))
    };
}

template < bool isGroup, bool Producer, bool waterInjector, bool gasInjector>
inline quantity group_control( const fn_args& args )
{
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
        auto it_g = args.grp_nwrk.groupData.find(g_name);
        if (it_g != args.grp_nwrk.groupData.end())
            cntl_mode = Opm::Group::ProductionCMode2Int(it_g->second.currentControl.currentProdConstraint);
    }
    // water injection control
    else if (waterInjector){
        auto it_g = args.grp_nwrk.groupData.find(g_name);
        if (it_g != args.grp_nwrk.groupData.end())
            cntl_mode = Opm::Group::InjectionCMode2Int(it_g->second.currentControl.currentWaterInjectionConstraint);
    }

    // gas injection control
    else if (gasInjector){
        auto it_g = args.grp_nwrk.groupData.find(g_name);
        if (it_g != args.grp_nwrk.groupData.end())
            cntl_mode = Opm::Group::InjectionCMode2Int(it_g->second.currentControl.currentGasInjectionConstraint);
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

inline quantity well_control_mode( const fn_args& args )
{
    const auto unit = Opm::UnitSystem::measure::identity;

    if (args.schedule_wells.empty()) {
        // No wells.  Possibly determining pertinent unit of measure
        // during SMSPEC configuration.
        return { 0.0, unit };
    }

    const auto* well = args.schedule_wells.front();
    auto xwPos = args.wells.find(well->name());
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT)) {
        // No dynamic results for 'well'.  Treat as shut/stopped.
        return { 0.0, unit };
    }

    if (! well_control_mode_defined(xwPos->second)) {
        // No dynamic control mode defined.  Use input control.
        const auto wmctl = Opm::Well::eclipseControlMode(*well, args.st);

        return { static_cast<double>(wmctl), unit };
    }

    // Well has simulator-provided active control mode.  Pick the
    // appropriate value depending on well type (producer/injector).
    const auto& curr = xwPos->second.current_control;
    const auto wmctl = curr.isProducer
        ? Opm::Well::eclipseControlMode(curr.prod)
        : Opm::Well::eclipseControlMode(curr.inj, well->injectorType());

    return { static_cast<double>(wmctl), unit };
}

template <Opm::data::GuideRateValue::Item i>
quantity guiderate_value(const ::Opm::data::GuideRateValue& grvalue)
{
    return { !grvalue.has(i) ? 0.0 : grvalue.get(i), rate_unit<i>() };
}

template <bool injection, Opm::data::GuideRateValue::Item i>
quantity group_guiderate(const fn_args& args)
{
    auto xgPos = args.grp_nwrk.groupData.find(args.group_name);
    if (xgPos == args.grp_nwrk.groupData.end()) {
        return { 0.0, rate_unit<i>() };
    }

    return injection
        ? guiderate_value<i>(xgPos->second.guideRates.injection)
        : guiderate_value<i>(xgPos->second.guideRates.production);
}

template <bool injection, Opm::data::GuideRateValue::Item i>
quantity well_guiderate(const fn_args& args)
{
    if (args.schedule_wells.empty()) {
        return { 0.0, rate_unit<i>() };
    }

    const auto* well = args.schedule_wells.front();
    if (well->isInjector() != injection) {
        return { 0.0, rate_unit<i>() };
    }

    auto xwPos = args.wells.find(well->name());
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        return { 0.0, rate_unit<i>() };
    }

    return guiderate_value<i>(xwPos->second.guide_rates);
}

quantity well_efficiency_factor(const fn_args& args)
{
    const auto zero = quantity { 0.0, measure::identity };

    if (args.schedule_wells.empty()) {
        return zero;
    }

    const auto* well = args.schedule_wells.front();

    auto xwPos = args.wells.find(well->name());
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        // Non-flowing wells have a zero efficiency factor
        return zero;
    }

    return { well->getEfficiencyFactor(), measure::identity };
}

quantity well_efficiency_factor_grouptree(const fn_args& args)
{
    const auto zero = quantity { 0.0, measure::identity };

    if (args.schedule_wells.empty()) {
        return zero;
    }

    const auto* well = args.schedule_wells.front();

    auto xwPos = args.wells.find(well->name());
    if ((xwPos == args.wells.end()) ||
        (xwPos->second.dynamicStatus == Opm::Well::Status::SHUT))
    {
        // Non-flowing wells have a zero efficiency factor
        return zero;
    }

    auto factor = well->getEfficiencyFactor();
    auto parent = well->groupName();
    while (parent != "FIELD") {
        const auto& grp = args.schedule[args.sim_step].groups(parent);
        factor *= grp.getGroupEfficiencyFactor();

        const auto prnt = grp.control_group();
        parent = prnt.has_value() ? prnt.value() : "FIELD";
    }

    return { factor, measure::identity };
}

quantity group_efficiency_factor(const fn_args& args)
{
    const auto zero = quantity { 0.0, measure::identity };

    if (args.schedule_wells.empty()) {
        return zero;
    }

    const auto gefac =
        args.schedule[args.sim_step].groups(args.group_name)
        .getGroupEfficiencyFactor();

    return { gefac, measure::identity };
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
    { "WWIRT", wwirt },
    { "WOIR", rate< rt::oil, injector > },
    { "WGIR", rate< rt::gas, injector > },
    { "WEIR", rate< rt::energy, injector > },
    { "WTIRHEA", rate< rt::energy, injector > },
    { "WNIR", rate< rt::solvent, injector > },
    { "WCIR", rate< rt::polymer, injector > },
    { "WSIR", rate< rt::brine, injector > },
    // Allow phase specific interpretation of tracer related summary keywords
    { "WTIR#W", ratetracer< rt::tracer, rt::wat, injector > }, // #W: Water tracers
    { "WTIR#O", ratetracer< rt::tracer, rt::oil, injector > }, // #O: Oil tracers
    { "WTIR#G", ratetracer< rt::tracer, rt::gas, injector > }, // #G: Gas tracers
    { "WTIC#W", div( ratetracer< rt::tracer, rt::wat, injector >, rate< rt::wat, injector >) },
    { "WTIC#O", div( ratetracer< rt::tracer, rt::oil, injector >, rate< rt::oil, injector >) },
    { "WTIC#G", div( ratetracer< rt::tracer, rt::gas, injector >, rate< rt::gas, injector >) },
    { "WVIR", sum( sum( rate< rt::reservoir_water, injector >, rate< rt::reservoir_oil, injector > ),
                       rate< rt::reservoir_gas, injector > ) },
    { "WGIGR", well_guiderate<injector, Opm::data::GuideRateValue::Item::Gas> },
    { "WWIGR", well_guiderate<injector, Opm::data::GuideRateValue::Item::Water> },

    { "WWIT", mul( rate< rt::wat, injector >, duration ) },
    { "WOIT", mul( rate< rt::oil, injector >, duration ) },
    { "WGIT", mul( rate< rt::gas, injector >, duration ) },
    { "WEIT", mul( rate< rt::energy, injector >, duration ) },
    { "WTITHEA", mul( rate< rt::energy, injector >, duration ) },
    { "WNIT", mul( rate< rt::solvent, injector >, duration ) },
    { "WCIT", mul( rate< rt::polymer, injector >, duration ) },
    { "WSIT", mul( rate< rt::brine, injector >, duration ) },
    { "WTIT#W", mul( ratetracer< rt::tracer, rt::wat, injector >, duration ) },
    { "WTIT#O", mul( ratetracer< rt::tracer, rt::oil, injector >, duration ) },
    { "WTIT#G", mul( ratetracer< rt::tracer, rt::gas, injector >, duration ) },
    { "WVIT", mul( sum( sum( rate< rt::reservoir_water, injector >, rate< rt::reservoir_oil, injector > ),
                        rate< rt::reservoir_gas, injector > ), duration ) },

    { "WWPR", rate< rt::wat, producer > },
    { "WOPR", rate< rt::oil, producer > },
    { "WWPTL",mul(ratel< rt::wat, producer >, duration) },
    { "WGPTL",mul(ratel< rt::gas, producer >, duration) },
    { "WOPTL",mul(ratel< rt::oil, producer >, duration) },
    { "WWPRL",ratel< rt::wat, producer > },
    { "WGPRL",ratel< rt::gas, producer > },
    { "WOPRL",ratel< rt::oil, producer > },
    { "WOFRL",ratel< rt::oil, producer > },
    { "WWIRL",ratel< rt::wat, injector> },
    { "WWITL",mul(ratel< rt::wat, injector>, duration) },
    { "WGIRL",ratel< rt::gas, injector> },
    { "WGITL",mul(ratel< rt::gas, injector>, duration) },
    { "WLPTL",mul( sum(ratel<rt::wat, producer>, ratel<rt::oil, producer>), duration)},
    { "WWCTL", div( ratel< rt::wat, producer >,
                    sum( ratel< rt::wat, producer >, ratel< rt::oil, producer > ) ) },
    { "WGORL", div( ratel< rt::gas, producer >, ratel< rt::oil, producer > ) },
    { "WGPR", rate< rt::gas, producer > },
    { "WEPR", rate< rt::energy, producer > },
    { "WTPRHEA", rate< rt::energy, producer > },
    { "WGLIR", glir},
    { "WALQ", artificial_lift_quantity },
    { "WNPR", rate< rt::solvent, producer > },
    { "WCPR", rate< rt::polymer, producer > },
    { "WSPR", rate< rt::brine, producer > },
    { "WTPR#W", ratetracer< rt::tracer, rt::wat, producer > },
    { "WTPR#O", ratetracer< rt::tracer, rt::oil, producer > },
    { "WTPR#G", ratetracer< rt::tracer, rt::gas, producer > },
    { "WTPC#W", div( ratetracer< rt::tracer, rt::wat, producer >, rate< rt::wat, producer >) },
    { "WTPC#O", div( ratetracer< rt::tracer, rt::oil, producer >, rate< rt::oil, producer >) },
    { "WTPC#G", div( ratetracer< rt::tracer, rt::gas, producer >, rate< rt::gas, producer >) },
    { "WCPC", div( rate< rt::polymer, producer >, rate< rt::wat, producer >) },
    { "WSPC", div( rate< rt::brine, producer >, rate< rt::wat, producer >) },

    { "WOPGR", well_guiderate<producer, Opm::data::GuideRateValue::Item::Oil> },
    { "WGPGR", well_guiderate<producer, Opm::data::GuideRateValue::Item::Gas> },
    { "WWPGR", well_guiderate<producer, Opm::data::GuideRateValue::Item::Water> },
    { "WVPGR", well_guiderate<producer, Opm::data::GuideRateValue::Item::ResV> },

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
    { "WEPT", mul( rate< rt::energy, producer >, duration ) },
    { "WTPTHEA", mul( rate< rt::energy, producer >, duration ) },
    { "WNPT", mul( rate< rt::solvent, producer >, duration ) },
    { "WCPT", mul( rate< rt::polymer, producer >, duration ) },
    { "WSPT", mul( rate< rt::brine, producer >, duration ) },
    { "WTPT#W", mul( ratetracer< rt::tracer, rt::wat, producer >, duration ) },
    { "WTPT#O", mul( ratetracer< rt::tracer, rt::oil, producer >, duration ) },
    { "WTPT#G", mul( ratetracer< rt::tracer, rt::gas, producer >, duration ) },
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

    { "WSTAT", wstat },
    { "WBHP", bhp },
    { "WTHP", thp },
    { "WTPCHEA", temperature< producer >},
    { "WTICHEA", temperature< injector >},
    { "WVPRT", res_vol_production_target },

    { "WMCTL", well_control_mode },

    { "GWIR", rate< rt::wat, injector > },
    { "WGVIR", rate< rt::reservoir_gas, injector >},
    { "WWVIR", rate< rt::reservoir_water, injector >},
    { "GOIR", rate< rt::oil, injector > },
    { "GGIR", rate< rt::gas, injector > },
    { "GEIR", rate< rt::energy, injector > },
    { "GTIRHEA", rate< rt::energy, injector > },
    { "GNIR", rate< rt::solvent, injector > },
    { "GCIR", rate< rt::polymer, injector > },
    { "GSIR", rate< rt::brine, injector > },
    { "GVIR", sum( sum( rate< rt::reservoir_water, injector >, rate< rt::reservoir_oil, injector > ),
                        rate< rt::reservoir_gas, injector > ) },

    { "GGIGR", group_guiderate<injector, Opm::data::GuideRateValue::Item::Gas> },
    { "GWIGR", group_guiderate<injector, Opm::data::GuideRateValue::Item::Water> },

    { "GWIT", mul( rate< rt::wat, injector >, duration ) },
    { "GOIT", mul( rate< rt::oil, injector >, duration ) },
    { "GGIT", mul( rate< rt::gas, injector >, duration ) },
    { "GEIT", mul( rate< rt::energy, injector >, duration ) },
    { "GTITHEA", mul( rate< rt::energy, injector >, duration ) },
    { "GNIT", mul( rate< rt::solvent, injector >, duration ) },
    { "GCIT", mul( rate< rt::polymer, injector >, duration ) },
    { "GSIT", mul( rate< rt::brine, injector >, duration ) },
    { "GVIT", mul( sum( sum( rate< rt::reservoir_water, injector >, rate< rt::reservoir_oil, injector > ),
                        rate< rt::reservoir_gas, injector > ), duration ) },

    { "GWPR", rate< rt::wat, producer > },
    { "GOPR", rate< rt::oil, producer > },
    { "GGPR", rate< rt::gas, producer > },
    { "GEPR", rate< rt::energy, producer > },
    { "GTPRHEA", rate< rt::energy, producer > },
    { "GGLIR", glir },
    { "GNPR", rate< rt::solvent, producer > },
    { "GCPR", rate< rt::polymer, producer > },
    { "GSPR", rate< rt::brine, producer > },
    { "GCPC", div( rate< rt::polymer, producer >, rate< rt::wat, producer >) },
    { "GSPC", div( rate< rt::brine, producer >, rate< rt::wat, producer >) },
    { "GOPRS", rate< rt::vaporized_oil, producer > },
    { "GOPRF", sub (rate < rt::oil, producer >, rate< rt::vaporized_oil, producer > ) },
    { "GLPR", sum( rate< rt::wat, producer >, rate< rt::oil, producer > ) },
    { "GVPR", sum( sum( rate< rt::reservoir_water, producer >, rate< rt::reservoir_oil, producer > ),
                        rate< rt::reservoir_gas, producer > ) },

    { "GOPGR", group_guiderate<producer, Opm::data::GuideRateValue::Item::Oil> },
    { "GGPGR", group_guiderate<producer, Opm::data::GuideRateValue::Item::Gas> },
    { "GWPGR", group_guiderate<producer, Opm::data::GuideRateValue::Item::Water> },
    { "GVPGR", group_guiderate<producer, Opm::data::GuideRateValue::Item::ResV> },

    { "GPR", node_pressure },

    { "GWPT", mul( rate< rt::wat, producer >, duration ) },
    { "GOPT", mul( rate< rt::oil, producer >, duration ) },
    { "GGPT", mul( rate< rt::gas, producer >, duration ) },
    { "GEPT", mul( rate< rt::energy, producer >, duration ) },
    { "GTPTHEA", mul( rate< rt::energy, producer >, duration ) },
    { "GNPT", mul( rate< rt::solvent, producer >, duration ) },
    { "GCPT", mul( rate< rt::polymer, producer >, duration ) },
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

    { "CPR", cpr  },
    { "CGIRL", cratel< rt::gas, injector> },
    { "CGITL", mul( cratel< rt::gas, injector>, duration) },
    { "CWIRL", cratel< rt::wat, injector> },
    { "CWITL", mul( cratel< rt::wat, injector>, duration) },
    { "CWPRL", cratel< rt::wat, producer > },
    { "CWPTL", mul( cratel< rt::wat, producer >, duration) },
    { "COPRL", cratel< rt::oil, producer > },
    { "COPTL", mul( cratel< rt::oil, producer >, duration) },
    { "CGPRL", cratel< rt::gas, producer > },
    { "CGPTL", mul( cratel< rt::gas, producer >, duration) },
    { "COFRL", cratel< rt::oil, producer > },
    { "CGORL", div( cratel< rt::gas, producer >, cratel< rt::oil, producer > ) },
    { "CWCTL", div( cratel< rt::wat, producer >,
                    sum( cratel< rt::wat, producer >, cratel< rt::oil, producer > ) ) },
    { "CWIR", crate< rt::wat, injector > },
    { "CGIR", crate< rt::gas, injector > },
    { "COIR", crate< rt::oil, injector > },
    { "CVIR", crate_resv<injector> },
    { "CCIR", crate< rt::polymer, injector > },
    { "CSIR", crate< rt::brine, injector > },
    { "COIT", mul( crate< rt::oil, injector >, duration ) },
    { "CWIT", mul( crate< rt::wat, injector >, duration ) },
    { "CGIT", mul( crate< rt::gas, injector >, duration ) },
    { "CVIT", mul( crate_resv<injector>, duration ) },
    { "CNIT", mul( crate< rt::solvent, injector >, duration ) },

    { "CWPR", crate< rt::wat, producer > },
    { "COPR", crate< rt::oil, producer > },
    { "CGPR", crate< rt::gas, producer > },
    { "CVPR", crate_resv<producer> },
    { "CCPR", crate< rt::polymer, producer > },
    { "CSPR", crate< rt::brine, producer > },
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
    { "CVPT", mul( crate_resv<producer>, duration ) },
    { "CNPT", mul( crate< rt::solvent, producer >, duration ) },
    { "CCIT", mul( crate< rt::polymer, injector >, duration ) },
    { "CCPT", mul( crate< rt::polymer, producer >, duration ) },
    { "CSIT", mul( crate< rt::brine, injector >, duration ) },
    { "CSPT", mul( crate< rt::brine, producer >, duration ) },
    { "CTFAC", trans_factors },
    { "CPI", connection_productivity_index },

    { "FWPR", rate< rt::wat, producer > },
    { "FOPR", rate< rt::oil, producer > },
    { "FGPR", rate< rt::gas, producer > },
    { "FEPR", rate< rt::energy, producer > },
    { "FTPRHEA", rate< rt::energy, producer > },
    { "FGLIR", glir },
    { "FNPR", rate< rt::solvent, producer > },
    { "FCPR", rate< rt::polymer, producer > },
    { "FSPR", rate< rt::brine, producer > },
    { "FCPC", div( rate< rt::polymer, producer >, rate< rt::wat, producer >) },
    { "FSPC", div( rate< rt::brine, producer >, rate< rt::wat, producer >) },
    { "FTPR#W", ratetracer< rt::tracer, rt::wat, producer > },
    { "FTPR#O", ratetracer< rt::tracer, rt::oil, producer > },
    { "FTPR#G", ratetracer< rt::tracer, rt::gas, producer > },
    { "FTPC#W", div( ratetracer< rt::tracer, rt::wat, producer >, rate< rt::wat, producer >) },
    { "FTPC#O", div( ratetracer< rt::tracer, rt::oil, producer >, rate< rt::oil, producer >) },
    { "FTPC#G", div( ratetracer< rt::tracer, rt::gas, producer >, rate< rt::gas, producer >) },
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
    { "FEPT", mul( rate< rt::energy, producer >, duration ) },
    { "FTPTHEA", mul( rate< rt::energy, producer >, duration ) },
    { "FNPT", mul( rate< rt::solvent, producer >, duration ) },
    { "FCPT", mul( rate< rt::polymer, producer >, duration ) },
    { "FSPT", mul( rate< rt::brine, producer >, duration ) },
    { "FTPT#W", mul( ratetracer< rt::tracer, rt::wat, producer >, duration ) },
    { "FTPT#O", mul( ratetracer< rt::tracer, rt::oil, producer >, duration ) },
    { "FTPT#G", mul( ratetracer< rt::tracer, rt::gas, producer >, duration ) },
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
    { "FEIR", rate< rt::energy, injector > },
    { "FTIRHEA", rate< rt::energy, injector > },
    { "FNIR", rate< rt::solvent, injector > },
    { "FCIR", rate< rt::polymer, injector > },
    { "FSIR", rate< rt::brine, injector > },
    { "FTIR#W", ratetracer< rt::tracer, rt::wat, injector > },
    { "FTIR#O", ratetracer< rt::tracer, rt::oil, injector > },
    { "FTIR#G", ratetracer< rt::tracer, rt::gas, injector > },
    { "FTIC#W", div( ratetracer< rt::tracer, rt::wat, injector >, rate< rt::wat, injector >) },
    { "FTIC#O", div( ratetracer< rt::tracer, rt::oil, injector >, rate< rt::oil, injector >) },
    { "FTIC#G", div( ratetracer< rt::tracer, rt::gas, injector >, rate< rt::gas, injector >) },
    { "FVIR", sum( sum( rate< rt::reservoir_water, injector>, rate< rt::reservoir_oil, injector >),
                   rate< rt::reservoir_gas, injector>)},

    { "FLIR", sum( rate< rt::wat, injector >, rate< rt::oil, injector > ) },
    { "FWIT", mul( rate< rt::wat, injector >, duration ) },
    { "FOIT", mul( rate< rt::oil, injector >, duration ) },
    { "FGIT", mul( rate< rt::gas, injector >, duration ) },
    { "FEIT", mul( rate< rt::energy, injector >, duration ) },
    { "FTITHEA", mul( rate< rt::energy, injector >, duration ) },
    { "FNIT", mul( rate< rt::solvent, injector >, duration ) },
    { "FCIT", mul( rate< rt::polymer, injector >, duration ) },
    { "FSIT", mul( rate< rt::brine, injector >, duration ) },
    { "FTIT#W", mul( ratetracer< rt::tracer, rt::wat, injector >, duration ) },
    { "FTIT#O", mul( ratetracer< rt::tracer, rt::oil, injector >, duration ) },
    { "FTIT#G", mul( ratetracer< rt::tracer, rt::gas, injector >, duration ) },
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
    { "FMWPA", abandoned_well< producer > },
    { "FMWIA", abandoned_well< injector >},

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
    { "RHPV"  , rhpv },
    //Multisegment well segment data
    { "SOFR", srate< rt::oil > },
    { "SWFR", srate< rt::wat > },
    { "SGFR", srate< rt::gas > },
    { "SWCT", div(srate<rt::wat>, sum(srate<rt::wat>, srate<rt::oil>)) },
    { "SPR", segpress<Opm::data::SegmentPressures::Value::Pressure> },
    { "SPRD", segpress<Opm::data::SegmentPressures::Value::PDrop> },
    { "SPRDH", segpress<Opm::data::SegmentPressures::Value::PDropHydrostatic> },
    { "SPRDF", segpress<Opm::data::SegmentPressures::Value::PDropFriction> },
    { "SPRDA", segpress<Opm::data::SegmentPressures::Value::PDropAccel> },
    // Well productivity index
    { "WPI", preferred_phase_productivty_index },
    { "WPIW", potential_rate< rt::productivity_index_water >},
    { "WPIO", potential_rate< rt::productivity_index_oil >},
    { "WPIG", potential_rate< rt::productivity_index_gas >},
    { "WPIL", sum( potential_rate< rt::productivity_index_water, true, false >,
                   potential_rate< rt::productivity_index_oil, true, false >)},
    // Well potential
    { "WWPP", potential_rate< rt::well_potential_water , true, false>},
    { "WOPP", potential_rate< rt::well_potential_oil , true, false>},
    { "WGPP", potential_rate< rt::well_potential_gas , true, false>},
    { "WWPI", potential_rate< rt::well_potential_water , false, true>},
    { "WWIP", potential_rate< rt::well_potential_water , false, true>}, // Alias for 'WWPI'
    { "WOPI", potential_rate< rt::well_potential_oil , false, true>},
    { "WGPI", potential_rate< rt::well_potential_gas , false, true>},
    { "WGIP", potential_rate< rt::well_potential_gas , false, true>}, // Alias for 'WGPI'
    { "ROEW", roew },

    // Efficiency factors
    {"GEFF" , group_efficiency_factor},
    {"WEFF" , well_efficiency_factor},
    {"WEFFG", well_efficiency_factor_grouptree},
};

static const std::unordered_map< std::string, Opm::UnitSystem::measure> single_values_units = {
  {"TCPU"     , Opm::UnitSystem::measure::runtime },
  {"ELAPSED"  , Opm::UnitSystem::measure::identity },
  {"NEWTON"   , Opm::UnitSystem::measure::identity },
  {"NLINERS"  , Opm::UnitSystem::measure::identity },
  {"NLINSMIN" , Opm::UnitSystem::measure::identity },
  {"NLINSMAX" , Opm::UnitSystem::measure::identity },
  {"MLINEARS" , Opm::UnitSystem::measure::identity },
  {"NLINEARS" , Opm::UnitSystem::measure::identity },
  {"MSUMLINS" , Opm::UnitSystem::measure::identity },
  {"MSUMNEWT" , Opm::UnitSystem::measure::identity },
  {"TCPUTS"   , Opm::UnitSystem::measure::identity },
  {"TIMESTEP" , Opm::UnitSystem::measure::time },
  {"TCPUDAY"  , Opm::UnitSystem::measure::time },
  {"STEPTYPE" , Opm::UnitSystem::measure::identity },
  {"TELAPLIN" , Opm::UnitSystem::measure::time },
  {"FRPV"     , Opm::UnitSystem::measure::volume },
  {"FWIP"     , Opm::UnitSystem::measure::liquid_surface_volume },
  {"FWIPR"    , Opm::UnitSystem::measure::volume },
  {"FOIP"     , Opm::UnitSystem::measure::liquid_surface_volume },
  {"FOIPR"    , Opm::UnitSystem::measure::volume },
  {"FOE"     , Opm::UnitSystem::measure::identity },
  {"FGIP"     , Opm::UnitSystem::measure::gas_surface_volume },
  {"FGIPR"    , Opm::UnitSystem::measure::volume },
  {"FSIP"     , Opm::UnitSystem::measure::mass },
  {"FOIPL"    , Opm::UnitSystem::measure::liquid_surface_volume },
  {"FOIPG"    , Opm::UnitSystem::measure::liquid_surface_volume },
  {"FGIPL"    , Opm::UnitSystem::measure::gas_surface_volume },
  {"FGIPG"    , Opm::UnitSystem::measure::gas_surface_volume },
  {"FPR"      , Opm::UnitSystem::measure::pressure },

};

static const std::unordered_map< std::string, Opm::UnitSystem::measure> region_units = {
  {"RPR"      , Opm::UnitSystem::measure::pressure},
  {"RRPV"     , Opm::UnitSystem::measure::volume },
  {"ROIP"     , Opm::UnitSystem::measure::liquid_surface_volume },
  {"ROIPL"    , Opm::UnitSystem::measure::liquid_surface_volume },
  {"ROIPG"    , Opm::UnitSystem::measure::liquid_surface_volume },
  {"RGIP"     , Opm::UnitSystem::measure::gas_surface_volume },
  {"RGIPL"    , Opm::UnitSystem::measure::gas_surface_volume },
  {"RGIPG"    , Opm::UnitSystem::measure::gas_surface_volume },
  {"RWIP"     , Opm::UnitSystem::measure::liquid_surface_volume },
  {"RRPV"     , Opm::UnitSystem::measure::geometric_volume }
};

static const auto interregion_units =
    std::unordered_map<std::string, Opm::UnitSystem::measure>
{
    // Flow rates (surface volume)
    { "ROFR"  , Opm::UnitSystem::measure::liquid_surface_rate },
    { "ROFR+" , Opm::UnitSystem::measure::liquid_surface_rate },
    { "ROFR-" , Opm::UnitSystem::measure::liquid_surface_rate },
    { "RGFR"  , Opm::UnitSystem::measure::gas_surface_rate    },
    { "RGFR+" , Opm::UnitSystem::measure::gas_surface_rate    },
    { "RGFR-" , Opm::UnitSystem::measure::gas_surface_rate    },
    { "RWFR"  , Opm::UnitSystem::measure::liquid_surface_rate },
    { "RWFR+" , Opm::UnitSystem::measure::liquid_surface_rate },
    { "RWFR-" , Opm::UnitSystem::measure::liquid_surface_rate },

    // Cumulatives (surface volume)
    { "ROFT"  , Opm::UnitSystem::measure::liquid_surface_volume },
    { "ROFT+" , Opm::UnitSystem::measure::liquid_surface_volume },
    { "ROFT-" , Opm::UnitSystem::measure::liquid_surface_volume },
    { "ROFTG" , Opm::UnitSystem::measure::liquid_surface_volume },
    { "ROFTL" , Opm::UnitSystem::measure::liquid_surface_volume },
    { "RGFT"  , Opm::UnitSystem::measure::gas_surface_volume    },
    { "RGFT+" , Opm::UnitSystem::measure::gas_surface_volume    },
    { "RGFT-" , Opm::UnitSystem::measure::gas_surface_volume    },
    { "RGFTG" , Opm::UnitSystem::measure::gas_surface_volume    },
    { "RGFTL" , Opm::UnitSystem::measure::gas_surface_volume    },
    { "RWFT"  , Opm::UnitSystem::measure::liquid_surface_volume },
    { "RWFT+" , Opm::UnitSystem::measure::liquid_surface_volume },
    { "RWFT-" , Opm::UnitSystem::measure::liquid_surface_volume },
};

static const std::unordered_map< std::string, Opm::UnitSystem::measure> block_units = {
  {"BPR"        , Opm::UnitSystem::measure::pressure},
  {"BRPV"     , Opm::UnitSystem::measure::volume },
  {"BOPV"     , Opm::UnitSystem::measure::volume },
  {"BGPV"     , Opm::UnitSystem::measure::volume },
  {"BWPV"     , Opm::UnitSystem::measure::volume },
  {"BPRESSUR"   , Opm::UnitSystem::measure::pressure},
  {"BTCNFHEA"   , Opm::UnitSystem::measure::temperature},
  {"BTEMP"      , Opm::UnitSystem::measure::temperature},
  {"BSWAT"      , Opm::UnitSystem::measure::identity},
  {"BWSAT"      , Opm::UnitSystem::measure::identity},
  {"BSGAS"      , Opm::UnitSystem::measure::identity},
  {"BGSAT"      , Opm::UnitSystem::measure::identity},
  {"BSOIL"      , Opm::UnitSystem::measure::identity},
  {"BOSAT"      , Opm::UnitSystem::measure::identity},
  {"BNSAT"      , Opm::UnitSystem::measure::identity},
  {"BOIP"       , Opm::UnitSystem::measure::liquid_surface_volume},
  {"BOIPG"      , Opm::UnitSystem::measure::liquid_surface_volume},
  {"BOIPL"      , Opm::UnitSystem::measure::liquid_surface_volume},
  {"BGIP"       , Opm::UnitSystem::measure::gas_surface_volume},
  {"BGIPG"      , Opm::UnitSystem::measure::gas_surface_volume},
  {"BGIPL"      , Opm::UnitSystem::measure::gas_surface_volume},
  {"BWIP"       , Opm::UnitSystem::measure::liquid_surface_volume},
  {"BRS"        , Opm::UnitSystem::measure::gas_oil_ratio},
  {"BRV"        , Opm::UnitSystem::measure::oil_gas_ratio},
  {"BWKR"      , Opm::UnitSystem::measure::identity},
  {"BOKR"      , Opm::UnitSystem::measure::identity},
  {"BKRO"      , Opm::UnitSystem::measure::identity},
  {"BKROG"     , Opm::UnitSystem::measure::identity},
  {"BKROW"     , Opm::UnitSystem::measure::identity},
  {"BGKR"      , Opm::UnitSystem::measure::identity},
  {"BKRG"      , Opm::UnitSystem::measure::identity},
  {"BKRW"      , Opm::UnitSystem::measure::identity},
  {"BWPC"      , Opm::UnitSystem::measure::pressure},
  {"BWPR"      , Opm::UnitSystem::measure::pressure},
  {"BGPC"      , Opm::UnitSystem::measure::pressure},
  {"BGPR"      , Opm::UnitSystem::measure::pressure},
  {"BVWAT"      , Opm::UnitSystem::measure::viscosity},
  {"BWVIS"      , Opm::UnitSystem::measure::viscosity},
  {"BVGAS"      , Opm::UnitSystem::measure::viscosity},
  {"BGVIS"      , Opm::UnitSystem::measure::viscosity},
  {"BVOIL"      , Opm::UnitSystem::measure::viscosity},
  {"BOVIS"      , Opm::UnitSystem::measure::viscosity},
};

static const std::unordered_map< std::string, Opm::UnitSystem::measure> aquifer_units = {
    {"AAQT", Opm::UnitSystem::measure::liquid_surface_volume},
    {"AAQR", Opm::UnitSystem::measure::liquid_surface_rate},
    {"AAQP", Opm::UnitSystem::measure::pressure},
    {"ANQP", Opm::UnitSystem::measure::pressure},
    {"ANQT", Opm::UnitSystem::measure::liquid_surface_volume},
    {"ANQR", Opm::UnitSystem::measure::liquid_surface_rate},

    // Dimensionless time and pressure values for CT aquifers
    {"AAQTD", Opm::UnitSystem::measure::identity},
    {"AAQPD", Opm::UnitSystem::measure::identity},
};

void sort_wells_by_insert_index(std::vector<const Opm::Well*>& wells)
{
    std::sort(wells.begin(), wells.end(),
        [](const Opm::Well* w1, const Opm::Well* w2)
    {
        return w1->seqIndex() < w2->seqIndex();
    });
}

std::vector<const Opm::Well*>
find_single_well(const Opm::Schedule& schedule,
                 const std::string&   well_name,
                 const int            sim_step)
{
    auto single_well = std::vector<const Opm::Well*>{};

    if (schedule.hasWell(well_name, sim_step)) {
        single_well.push_back(&schedule.getWell(well_name, sim_step));
    }

    return single_well;
}

std::vector<const Opm::Well*>
find_region_wells(const Opm::Schedule&           schedule,
                  const Opm::EclIO::SummaryNode& node,
                  const int                      sim_step,
                  const Opm::out::RegionCache&   regionCache)
{
    auto result = std::vector<const Opm::Well*>{};
    auto regionwells = std::set<const Opm::Well*>{};

    const auto region = node.number;

    for (const auto& connection : regionCache.connections(*node.fip_region, region)) {
        const auto& w_name = connection.first;
        if (! schedule.hasWell(w_name, sim_step)) {
            continue;
        }

        regionwells.insert(&schedule.getWell(w_name, sim_step));
    }

    result.assign(regionwells.begin(), regionwells.end());
    sort_wells_by_insert_index(result);

    return result;
}

std::vector<const Opm::Well*>
find_group_wells(const Opm::Schedule& schedule,
                 const std::string&   group_name,
                 const int            sim_step)
{
    auto groupwells = std::vector<const Opm::Well*>{};

    const auto& schedState = schedule[sim_step];
    if (! schedState.groups.has(group_name)) {
        return groupwells;      // Empty
    }

    auto downtree = std::vector<std::string>{group_name};
    for (auto i = 0*downtree.size(); i < downtree.size(); ++i) {
        const auto& group = schedState.groups.get(downtree[i]);

        if (group.wellgroup()) {
            for (const auto& wname : group.wells()) {
                groupwells.push_back(& schedState.wells.get(wname));
            }
        }
        else {
            const auto& children = group.groups();
            downtree.insert(downtree.end(), children.begin(), children.end());
        }
    }

    sort_wells_by_insert_index(groupwells);

    return groupwells;
}

std::vector<const Opm::Well*>
find_field_wells(const Opm::Schedule& schedule,
                 const int            sim_step)
{
    auto fieldwells = std::vector<const Opm::Well*>{};

    const auto& wells = schedule[sim_step].wells;
    for (const auto& well : wells.keys()) {
        fieldwells.push_back(&wells.get(well));
    }

    sort_wells_by_insert_index(fieldwells);

    return fieldwells;
}

inline std::vector<const Opm::Well*>
find_wells(const Opm::Schedule&           schedule,
           const Opm::EclIO::SummaryNode& node,
           const int                      sim_step,
           const Opm::out::RegionCache&   regionCache)
{
    switch (node.category) {
    case Opm::EclIO::SummaryNode::Category::Well:
    case Opm::EclIO::SummaryNode::Category::Connection:
    case Opm::EclIO::SummaryNode::Category::Segment:
        return find_single_well(schedule, node.wgname, sim_step);

    case Opm::EclIO::SummaryNode::Category::Group:
        return find_group_wells(schedule, node.wgname, sim_step);

    case Opm::EclIO::SummaryNode::Category::Field:
        return find_field_wells(schedule, sim_step);

    case Opm::EclIO::SummaryNode::Category::Region:
        return find_region_wells(schedule, node, sim_step, regionCache);

    case Opm::EclIO::SummaryNode::Category::Aquifer:
    case Opm::EclIO::SummaryNode::Category::Block:
    case Opm::EclIO::SummaryNode::Category::Node:
    case Opm::EclIO::SummaryNode::Category::Miscellaneous:
        return {};
    }

    throw std::runtime_error {
        fmt::format("Unhandled summary node category \"{}\" in find_wells()",
                    node.category)
    };
}

bool need_wells(const Opm::EclIO::SummaryNode& node)
{
    static const std::regex region_keyword_regex { "R[OGW][IP][RT]" };
    static const std::regex group_guiderate_regex { "G[OGWV][IP]GR" };

    using Cat = Opm::EclIO::SummaryNode::Category;

    switch (node.category) {
    case Cat::Connection: [[fallthrough]];
    case Cat::Field:      [[fallthrough]];
    case Cat::Group:      [[fallthrough]];
    case Cat::Segment:    [[fallthrough]];
    case Cat::Well:
        // Need to capture wells for anything other than guiderates at group
        // level.  Those are directly available in the solution values from
        // the simulator and don't need aggregation from well level.
        return (node.category != Cat::Group)
            || !std::regex_match(node.keyword, group_guiderate_regex);

    case Cat::Region:
        return std::regex_match(node.keyword, region_keyword_regex);

    case Cat::Aquifer:       [[fallthrough]];
    case Cat::Miscellaneous: [[fallthrough]];
    case Cat::Node:          [[fallthrough]];
        // Node values directly available in solution.
    case Cat::Block:
        return false;
    }

    throw std::runtime_error("Unhandled summary node category in need_wells");
}

void updateValue(const Opm::EclIO::SummaryNode& node, const double value, Opm::SummaryState& st)
{
    using Cat = Opm::EclIO::SummaryNode::Category;

    switch (node.category) {
    case Cat::Well:
        st.update_well_var(node.wgname, node.keyword, value);
        break;

    case Cat::Group:
    case Cat::Node:
        st.update_group_var(node.wgname, node.keyword, value);
        break;

    case Cat::Connection:
        st.update_conn_var(node.wgname, node.keyword, node.number, value);
        break;

    default:
        st.update(node.unique_key(), value);
        break;
    }
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

    void setFactors(const Opm::EclIO::SummaryNode&       node,
                    const Opm::Schedule&                 schedule,
                    const std::vector<const Opm::Well*>& schedule_wells,
                    const int                            sim_step);
};

void EfficiencyFactor::setFactors(const Opm::EclIO::SummaryNode&       node,
                                  const Opm::Schedule&                 schedule,
                                  const std::vector<const Opm::Well*>& schedule_wells,
                                  const int                            sim_step)
{
    this->factors.clear();

    const bool is_field  { node.category == Opm::EclIO::SummaryNode::Category::Field  } ;
    const bool is_group  { node.category == Opm::EclIO::SummaryNode::Category::Group  } ;
    const bool is_region { node.category == Opm::EclIO::SummaryNode::Category::Region } ;
    const bool is_rate   { node.type     != Opm::EclIO::SummaryNode::Type::Total      } ;

    if (!is_field && !is_group && !is_region && is_rate)
        return;

    for (const auto* well : schedule_wells) {
        if (!well->hasBeenDefined(sim_step))
            continue;

        double eff_factor = well->getEfficiencyFactor();
        const auto* group_ptr = std::addressof(schedule.getGroup(well->groupName(), sim_step));

        while (group_ptr) {
            if (is_group && is_rate && (group_ptr->name() == node.wgname))
                break;

            eff_factor *= group_ptr->getGroupEfficiencyFactor();

            const auto parent_group = group_ptr->flow_group();

            if (parent_group.has_value())
                group_ptr = std::addressof(schedule.getGroup( parent_group.value(), sim_step ));
            else
                group_ptr = nullptr;
        }

        this->factors.emplace_back(well->name(), eff_factor);
    }
}

namespace Evaluator {
    struct InputData
    {
        const Opm::EclipseState& es;
        const Opm::Schedule& sched;
        const Opm::EclipseGrid& grid;
        const Opm::out::RegionCache& reg;
        const Opm::Inplace initial_inplace;
    };

    struct SimulatorResults
    {
        const Opm::data::Wells& wellSol;
        const Opm::data::GroupAndNetworkValues& grpNwrkSol;
        const std::map<std::string, double>& single;
        const Opm::Inplace inplace;
        const std::map<std::string, std::vector<double>>& region;
        const std::map<std::pair<std::string, int>, double>& block;
        const Opm::data::Aquifers& aquifers;
        const std::unordered_map<std::string, Opm::data::InterRegFlowMap>& ireg;
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
            const auto get_wells = need_wells(this->node_);

            const auto wells = get_wells
                ? find_wells(input.sched, this->node_,
                             static_cast<int>(sim_step), input.reg)
                : std::vector<const Opm::Well*>{};

            if (get_wells && wells.empty())
                // Parameter depends on well information, but no active
                // wells apply at this sim_step.  Nothing to do.
                return;

            EfficiencyFactor efac{};
            efac.setFactors(this->node_, input.sched, wells, sim_step);

            const fn_args args {
                wells, this->group_name(), this->node_.keyword, stepSize, static_cast<int>(sim_step),
                std::max(0, this->node_.number),
                this->node_.fip_region,
                st, simRes.wellSol, simRes.grpNwrkSol,
                input.reg, input.grid, input.sched,
                std::move(efac.factors), input.initial_inplace, simRes.inplace,
                input.sched.getUnits()
            };

            const auto& usys = input.es.getUnits();
            const auto  prm  = this->fcn_(args);

            updateValue(this->node_, usys.from_si(prm.unit, prm.value), st);
        }

    private:
        Opm::EclIO::SummaryNode node_;
        ofun                    fcn_;

        std::string group_name() const
        {
            using Cat = ::Opm::EclIO::SummaryNode::Category;

            const auto need_grp_name =
                (this->node_.category == Cat::Group) ||
                (this->node_.category == Cat::Node);

            return need_grp_name
                ? this->node_.wgname : std::string{""};
        }

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

    class AquiferValue: public Base
    {
    public:
        explicit AquiferValue(Opm::EclIO::SummaryNode node,
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
            auto xPos = simRes.aquifers.find(this->node_.number);
            if (xPos == simRes.aquifers.end()) {
                return;
            }

            const auto& usys = input.es.getUnits();
            updateValue(this->node_, usys.from_si(this->m_, xPos->second.get(this->node_.keyword)), st);
        }
    private:
        Opm::EclIO::SummaryNode  node_;
        Opm::UnitSystem::measure m_;
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

    class InterRegionValue : public Base
    {
    public:
        explicit InterRegionValue(const Opm::EclIO::SummaryNode& node,
                                  const Opm::UnitSystem::measure m)
            : node_   (node)
            , m_      (m)
            , regname_(node_.fip_region.has_value()
                       ? node_.fip_region.value()
                       : std::string{ "FIPNUM" })
        {
            this->analyzeKeyword();
        }

        void update(const std::size_t    /* sim_step */,
                    const double            stepSize,
                    const InputData&        input,
                    const SimulatorResults& simRes,
                    Opm::SummaryState&      st) const override
        {
            if (this->component_ == Component::NumComponents) {
                return;
            }

            auto flows = simRes.ireg.find(this->regname_);
            if (flows == simRes.ireg.end()) {
                return;
            }

            auto flow = flows->second.getInterRegFlows(this->r1_, this->r2_);
            if (! flow.has_value()) {
                return;
            }

            const auto& usys = input.es.getUnits();
            const auto  val  = this->getValue(flow->first, flow->second, stepSize);

            updateValue(this->node_, usys.from_si(this->m_, val), st);
        }

    private:
        using RateWindow = Opm::data::InterRegFlowMap::ReadOnlyWindow;
        using Component  = RateWindow::Component;
        using Direction  = RateWindow::Direction;

        Opm::EclIO::SummaryNode node_;
        Opm::UnitSystem::measure m_;
        std::string regname_{};

        Component component_{ Component::NumComponents };
        Component subtract_ { Component::NumComponents };
        Direction direction_{ Direction::Positive };
        bool useDirection_{ false };
        bool isCumulative_{ false };
        int r1_{ -1 };
        int r2_{ -1 };

        void analyzeKeyword()
        {
            // Valid keywords are
            //
            // - R[OGW]F[TR]
            //     Basic oil/gas/water flow rates and cumulatives.  FIPNUM
            //     region set.
            //
            // - R[OGW]F[TR][-+]
            //     Directional versions of basic oil/gas/water flow rates
            //     and cumulatives.  FIPNUM region set.
            //
            // - R[OG]F[TR][GL]
            //     Flow rates and cumulatives of free oil (ROF[TR]L),
            //     vaporised oil (ROF[TR]G), free gas (RGF[TR]G), and gas
            //     dissolved in liquid (RGF[TR]L).  FIPNUM region set.
            //
            // - R[OGW]F[TR]_[A-Z0-9]{3}
            //     Basic oil/gas/water flow rates and cumulatives.  User
            //     defined region set (FIP* keyword).
            //
            // - R[OGW]F[TR][-+][A-Z0-9]{3}
            //     Directional versions of basic oil/gas/water flow rates
            //     and cumulatives.  User defined region set (FIP* keyword).
            //
            // - R[OG]F[TR][GL][A-Z0-9]{3}
            //     Flow rates and cumulatives of free oil (ROF[TR]L),
            //     vaporised oil (ROF[TR]G), free gas (RGF[TR]G), and gas
            //     dissolved in liquid (RGF[TR]L).  User defined region set
            //     (FIP* keyword).
            //
            // We don't need a full keyword verification here, only to
            // extract the pertinent keyword pieces, because the input
            // keyword validity is enforced at the parser level.  See json
            // descriptions REGION2REGION_PROBE and REGION2REGION_PROBE_E300
            // in input/eclipse/share/keywords.
            //
            // Note that we explicitly disregard the region set name here as
            // this name does not influence the interpretation of the
            // summary vector keyword-only the definition of the individual
            // regions.

            static const auto pattern = std::regex {
                R"~~(R([OGW])F([RT])([-+GL]?)(?:_?[A-Z0-9_]{3})?)~~"
            };

            auto keywordPieces = std::smatch {};
            if (std::regex_match(this->node_.keyword, keywordPieces, pattern)) {
                this->identifyComponent(keywordPieces);
                this->identifyDirection(keywordPieces);
                this->identifyCumulative(keywordPieces);
                this->assignRegionIDs();
            }
        }

        double getValue(const RateWindow& iregFlow,
                        const double      sign,
                        const double      stepSize) const
        {
            const auto prim = this->useDirection_
                ? iregFlow.flow(this->component_, this->direction_)
                : iregFlow.flow(this->component_);

            const auto sub = (this->subtract_ == Component::NumComponents)
                ? 0.0 : iregFlow.flow(this->subtract_);

            const auto val = sign * (prim - sub);

            return this->isCumulative_
                ? stepSize * val
                : val;
        }

        void assignRegionIDs()
        {
            const auto& [r1, r2] =
                Opm::EclIO::splitSummaryNumber(this->node_.number);

            this->r1_ = r1 - 1;
            this->r2_ = r2 - 1;
        }

        void identifyComponent(const std::smatch& keywordPieces)
        {
            const auto main = keywordPieces[1].str();

            if (main == "O") {
                this->component_ = (keywordPieces[3].str() == "G")
                    ? Component::Vapoil
                    : Component::Oil;

                if (keywordPieces[3].str() == "L") {
                    // Free oil = "oil - vapoil"
                    this->subtract_ = Component::Vapoil;
                }
            }
            else if (main == "G") {
                this->component_ = (keywordPieces[3].str() == "L")
                    ? Component::Disgas
                    : Component::Gas;

                if (keywordPieces[3].str() == "G") {
                    // Free gas = "gas - disgas"
                    this->subtract_ = Component::Disgas;
                }
            }
            else if (main == "W") {
                this->component_ = Component::Water;
            }
        }

        void identifyDirection(const std::smatch& keywordPieces)
        {
            if (keywordPieces.length(3) == std::smatch::difference_type{0}) {
                return;
            }

            const auto dir = keywordPieces[3].str();

            this->useDirection_ = (dir == "+") || (dir == "-");

            if (dir == "-") {
                this->direction_ = Direction::Negative;
            }
        }

        void identifyCumulative(const std::smatch& keywordPieces)
        {
            assert (keywordPieces.length(2) != std::smatch::difference_type{0});

            const auto type = keywordPieces[2].str();

            this->isCumulative_ = type == "T";
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
            st.update("TIME", usys.from_si(m, val));
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
                         const Opm::Schedule&     sched,
                         const Opm::SummaryState& st,
                         const Opm::UDQConfig&    udq)
            : es_(es), sched_(sched), grid_(grid), st_(st), udq_(udq)
        {}

        ~Factory() = default;

        Factory(const Factory&) = delete;
        Factory(Factory&&) = delete;
        Factory& operator=(const Factory&) = delete;
        Factory& operator=(Factory&&) = delete;

        Descriptor create(const Opm::EclIO::SummaryNode&);

    private:
        const Opm::EclipseState& es_;
        const Opm::Schedule&     sched_;
        const Opm::EclipseGrid&  grid_;
        const Opm::SummaryState& st_;
        const Opm::UDQConfig&    udq_;

        const Opm::EclIO::SummaryNode* node_;

        Opm::UnitSystem::measure paramUnit_;
        ofun paramFunction_;

        Descriptor functionRelation();
        Descriptor blockValue();
        Descriptor aquiferValue();
        Descriptor regionValue();
        Descriptor interRegionValue();
        Descriptor globalProcessValue();
        Descriptor userDefinedValue();
        Descriptor unknownParameter();

        bool isBlockValue();
        bool isAquiferValue();
        bool isRegionValue();
        bool isInterRegionValue();
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

        if (this->isAquiferValue())
            return this->aquiferValue();

        if (this->isRegionValue())
            return this->regionValue();

        if (this->isInterRegionValue())
            return this->interRegionValue();

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

    Factory::Descriptor Factory::aquiferValue()
    {
        auto desc = this->unknownParameter();

        desc.unit = this->directUnitString();
        desc.evaluator.reset(new AquiferValue {
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

    Factory::Descriptor Factory::interRegionValue()
    {
        auto desc = this->unknownParameter();

        desc.unit = this->directUnitString();
        desc.evaluator.reset(new InterRegionValue {
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

    bool Factory::isAquiferValue()
    {
        auto pos = aquifer_units.find(this->node_->keyword);
        if (pos == aquifer_units.end()) return false;

        // if the aquifer does not exist, should we warn?
        if ( !this->es_.aquifer().hasAquifer(this->node_->number) ) return false;

        this->paramUnit_ = pos->second;
        return true;
    }

    bool Factory::isRegionValue()
    {
        auto keyword = this->node_->keyword;
        auto dash_pos = keyword.find("_");
        if (dash_pos != std::string::npos)
            keyword = keyword.substr(0, dash_pos);

        auto pos = region_units.find(keyword);
        if (pos == region_units.end())
            return false;

        // 'node_' represents a region value.  Capture unit
        // of measure and return true.
        this->paramUnit_ = pos->second;
        return true;
    }

    bool Factory::isInterRegionValue()
    {
        const auto end = std::min({
            // Infinity (std::string::npos) if no underscore
            this->node_->keyword.find('_'),

            // Don't look beyond end of keyword string.
            this->node_->keyword.size(),

            // Always at most 5 characters in the "real" keyword.
            std::string::size_type{5},
        });

        auto pos = interregion_units.find(this->node_->keyword.substr(0, end));
        if (pos == interregion_units.end()) {
            // Node_'s canonical form reduced keyword does not match any of
            // the supported inter-region flow summary vector keywords.
            return false;
        }

        // 'Node_' represents a supported inter-region summary vector.
        // Capture unit of measure and return true.
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
        if (pos != funs.end()) {
            // 'node_' represents a functional relation.
            // Capture evaluation function and return true.
            this->paramFunction_ = pos->second;
            return true;
        }

        auto keyword = this->node_->keyword;
        auto dash_pos = keyword.find("_");
        if (dash_pos != std::string::npos)
            keyword = keyword.substr(0, dash_pos);

        pos = funs.find(keyword);
        if (pos != funs.end()) {
            // 'node_' represents a functional relation.
            // Capture evaluation function and return true.
            this->paramFunction_ = pos->second;
            return true;
        }

        if (keyword.length() > 4 ) {
            std::string tracer_tag = keyword.substr(0, 4);
            std::string tracer_name = keyword.substr(4);
            const auto& tracers = es_.tracer();
            for (const auto& tracer : tracers) {
                if (tracer.name == tracer_name) {
                    if (tracer.phase == Opm::Phase::WATER)
                        tracer_tag += "#W";
                    else if (tracer.phase == Opm::Phase::OIL)
                        tracer_tag += "#O";
                    else if (tracer.phase == Opm::Phase::GAS)
                        tracer_tag += "#G";

                    pos = funs.find(tracer_tag);
                    if (pos != funs.end()) {
                        this->paramFunction_ = pos->second;
                        return true;
                    }

                    break;
                }
            }
        }

        return false;
    }

    bool Factory::isUserDefined()
    {
        return this->node_->is_user_defined();
    }

    std::string Factory::functionUnitString() const
    {
        const auto reg = Opm::out::RegionCache{};

        const fn_args args {
            {}, "", this->node_->keyword, 0.0, 0, std::max(0, this->node_->number),
            this->node_->fip_region,
            this->st_, {}, {}, reg, this->grid_, this->sched_,
            {}, {}, {}, Opm::UnitSystem(Opm::UnitSystem::UnitType::UNIT_TYPE_METRIC)
        };

        const auto prm = this->paramFunction_(args);

        std::string unit_string_tracer = this->es_.tracer().get_unit_string(this->es_.getUnits(), this->node_->keyword);
        if (unit_string_tracer != "") { //Non-default unit for tracer amount.
            return unit_string_tracer;
        }

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
            ?  this->udq_.unit(kw) : "";
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
        Opm::OpmLog::warning(Opm::OpmInputError::format("Unhandled summary keyword {keyword}\n"
                                                        "In {file} line {line}", location));
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
    , start_   (Opm::TimeService::from_time_t(start))
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
        fmt::format("Elapsed time ({} {}) "
                    "must not precede previous elapsed time ({} {}). "
                    "Incorrect restart time?", elapsed, unt, prev_el, unt)
    };
}

} // Anonymous namespace

class Opm::out::Summary::SummaryImplementation
{
public:
    explicit SummaryImplementation(const EclipseState&  es,
                                   const SummaryConfig& sumcfg,
                                   const EclipseGrid&   grid,
                                   const Schedule&      sched,
                                   const std::string&   basename,
                                   const bool           writeEsmry);

    SummaryImplementation(const SummaryImplementation& rhs) = delete;
    SummaryImplementation(SummaryImplementation&& rhs) = default;
    SummaryImplementation& operator=(const SummaryImplementation& rhs) = delete;
    SummaryImplementation& operator=(SummaryImplementation&& rhs) = default;

    void eval(const int                          sim_step,
              const double                       secs_elapsed,
              const data::Wells&                 well_solution,
              const data::GroupAndNetworkValues& grp_nwrk_solution,
              GlobalProcessParameters&           single_values,
              const Inplace&                     initial_inplace,
              const Opm::Inplace&                inplace,
              const RegionParameters&            region_values,
              const BlockValues&                 block_values,
              const data::Aquifers&              aquifer_values,
              const InterRegFlowValues&          interreg_flows,
              SummaryState&                      st) const;

    void internal_store(const SummaryState& st, const int report_step, bool isSubstep);
    void write(const bool is_final_summary);
    PAvgCalculatorCollection wbp_calculators(std::size_t report_step) const;

private:
    struct MiniStep
    {
        int id{0};
        int seq{-1};
        bool isSubstep{false};
        std::vector<float> params{};
    };

    using EvalPtr = SummaryOutputParameters::EvalPtr;

    std::reference_wrapper<const Opm::EclipseGrid> grid_;
    std::reference_wrapper<const Opm::EclipseState> es_;
    std::reference_wrapper<const Opm::Schedule> sched_;
    Opm::out::RegionCache regCache_;
    std::unordered_set<std::string> wbp_wells;

    std::unique_ptr<SMSpecStreamDeferredCreation> deferredSMSpec_;

    Opm::EclIO::OutputStream::ResultSet rset_;
    Opm::EclIO::OutputStream::Formatted fmt_;
    Opm::EclIO::OutputStream::Unified   unif_;

    mutable int miniStepID_{0};
    mutable double prevEvalTime_{std::numeric_limits<double>::lowest()};

    int prevCreate_{-1};
    int prevReportStepID_{-1};
    std::vector<MiniStep>::size_type numUnwritten_{0};

    SummaryOutputParameters                  outputParameters_{};
    std::unordered_map<std::string, EvalPtr> extra_parameters{};
    std::vector<std::string> valueKeys_{};
    std::vector<std::string> valueUnits_{};
    std::vector<MiniStep>    unwritten_{};

    std::unique_ptr<Opm::EclIO::OutputStream::SummarySpecification> smspec_{};
    std::unique_ptr<Opm::EclIO::EclOutput> stream_{};

    std::unique_ptr<Opm::EclIO::ExtSmryOutput> esmry_;

    void configureTimeVector(const EclipseState& es, const std::string& kw);
    void configureTimeVectors(const EclipseState& es, const SummaryConfig& sumcfg);

    void configureSummaryInput(const SummaryConfig& sumcfg,
                               Evaluator::Factory&  evaluatorFactory);

    void configureRequiredRestartParameters(const SummaryConfig& sumcfg,
                                            const AquiferConfig& aqConfig,
                                            const Schedule&      sched,
                                            Evaluator::Factory&  evaluatorFactory);

    void configureUDQ(const EclipseState& es, const SummaryConfig& summary_config, const Schedule& sched);

    MiniStep& getNextMiniStep(const int report_step, bool isSubstep);
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
                      const std::string&   basename,
                      const bool           writeEsmry)
    : grid_          (std::cref(grid))
    , es_            (std::cref(es))
    , sched_         (std::cref(sched))
    , regCache_      (sumcfg.fip_regions(), es.globalFieldProps(), grid, sched)
    , deferredSMSpec_(makeDeferredSMSpecCreation(es, grid, sched))
    , rset_          (makeResultSet(es.cfg().io(), basename))
    , fmt_           { es.cfg().io().getFMTOUT() }
    , unif_          { es.cfg().io().getUNIFOUT() }
{
    const auto st = SummaryState {
        TimeService::from_time_t(sched.getStartTime())
    };

    Evaluator::Factory evaluatorFactory {
        es, grid, sched, st, sched.getUDQConfig(sched.size() - 1)
    };

    this->configureTimeVectors(es, sumcfg);
    this->configureSummaryInput(sumcfg, evaluatorFactory);
    this->configureRequiredRestartParameters(sumcfg, es.aquifer(),
                                             sched, evaluatorFactory);
    this->configureUDQ(es, sumcfg, sched);

    for (const auto& config_node : sumcfg.keywords("WBP*"))
        this->wbp_wells.insert( config_node.namedEntity() );

    std::string esmryFileName = EclIO::OutputStream::outputFileName(this->rset_, "ESMRY");

    if (std::filesystem::exists(esmryFileName))
        std::filesystem::remove(esmryFileName);

    if ((writeEsmry) and (es.cfg().io().getFMTOUT()==false))
        this->esmry_ = std::make_unique<Opm::EclIO::ExtSmryOutput>(this->valueKeys_, this->valueUnits_, es, sched.posixStartTime());

    if ((writeEsmry) and (es.cfg().io().getFMTOUT()))
        OpmLog::warning("ESMRY only supported for unformatted output.  Request ignored.");
}

void Opm::out::Summary::SummaryImplementation::
internal_store(const SummaryState& st, const int report_step, bool isSubstep)
{
    auto& ms = this->getNextMiniStep(report_step, isSubstep);

    const auto nParam = this->valueKeys_.size();

    for (auto i = decltype(nParam){0}; i < nParam; ++i) {
        if (! st.has(this->valueKeys_[i]))
            // Parameter not yet evaluated (e.g., well/group not
            // yet active).  Nothing to do here.
            continue;

        ms.params[i] = st.get(this->valueKeys_[i]);
    }
}

Opm::PAvgCalculatorCollection
Opm::out::Summary::SummaryImplementation::wbp_calculators(std::size_t report_step) const
{
    if (this->wbp_wells.empty())
        return {};

    Opm::PAvgCalculatorCollection calculators;
    const auto& porv = this->es_.get().globalFieldProps().porv(true);
    for (const auto& wname : this->wbp_wells) {
        if (this->sched_.get().hasWell(wname, report_step)) {
            const auto& well = this->sched_.get().getWell(wname, report_step);
            if (well.getStatus() == Opm::Well::Status::OPEN)
                calculators.add(well.pavg_calculator(this->grid_, porv));
        }
    }

    return calculators;
}

void
Opm::out::Summary::SummaryImplementation::
eval(const int                          sim_step,
     const double                       secs_elapsed,
     const data::Wells&                 well_solution,
     const data::GroupAndNetworkValues& grp_nwrk_solution,
     GlobalProcessParameters&           single_values,
     const Inplace&                     initial_inplace,
     const Opm::Inplace&                inplace,
     const RegionParameters&            region_values,
     const BlockValues&                 block_values,
     const data::Aquifers&              aquifer_values,
     const InterRegFlowValues&          interreg_flows,
     Opm::SummaryState&                 st) const
{
    validateElapsedTime(secs_elapsed, this->es_, st);

    const double duration = secs_elapsed - st.get_elapsed();
    single_values["TIMESTEP"] = duration;
    st.update("TIMESTEP", this->es_.get().getUnits().from_si(Opm::UnitSystem::measure::time, duration));

    const Evaluator::InputData input {
        this->es_, this->sched_, this->grid_, this->regCache_, initial_inplace
    };

    const Evaluator::SimulatorResults simRes {
        well_solution, grp_nwrk_solution, single_values, inplace,
        region_values, block_values, aquifer_values, interreg_flows
    };

    for (auto& evalPtr : this->outputParameters_.getEvaluators()) {
        evalPtr->update(sim_step, duration, input, simRes, st);
    }

    for (auto& [_, evalPtr] : this->extra_parameters) {
        (void)_;
        evalPtr->update(sim_step, duration, input, simRes, st);
    }

    st.update_elapsed(duration);

    if (secs_elapsed > this->prevEvalTime_) {
        this->prevEvalTime_ = secs_elapsed;
        ++this->miniStepID_;
    }
}

void Opm::out::Summary::SummaryImplementation::write(const bool is_final_summary)
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

    if (this->esmry_ != nullptr){
        for (auto i = 0*this->numUnwritten_; i < this->numUnwritten_; ++i){
            this->esmry_->write(this->unwritten_[i].params, !this->unwritten_[i].isSubstep, is_final_summary);
        }
    }

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
configureTimeVector(const EclipseState& es, const std::string& kw) {
    const auto dfltwgname = std::string(":+:+:+:+");
    const auto dfltnum    = 0;

    this->valueKeys_.push_back(kw);

    if (kw == "TIME") {
        const std::string& unit_string = es.getUnits().name(UnitSystem::measure::time);
        auto eval = std::make_unique<Evaluator::Time>(kw);

        valueUnits_.push_back(unit_string);


        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, unit_string, std::move(eval));
    }

    else if (kw == "DAY") {
        auto eval = std::make_unique<Evaluator::Day>(kw);
        valueUnits_.push_back("");

        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, "", std::move(eval));
    }

    else if (kw == "MONTH" || kw == "MNTH") {
        auto eval = std::make_unique<Evaluator::Month>(kw);
        valueUnits_.push_back("");

        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, "", std::move(eval));
    }

    else if (kw == "YEAR") {
        auto eval = std::make_unique<Evaluator::Year>(kw);
        valueUnits_.push_back("");
        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, "", std::move(eval));
    }

    else if (kw == "YEARS") {
        auto eval = std::make_unique<Evaluator::Years>(kw);
        valueUnits_.push_back("");

        this->outputParameters_
            .makeParameter(kw, dfltwgname, dfltnum, kw, std::move(eval));
    }
}

void
Opm::out::Summary::SummaryImplementation::
configureTimeVectors(const EclipseState& es, const SummaryConfig& sumcfg)
{
    this->configureTimeVector(es, "TIME");
    this->configureTimeVector(es, "YEARS");

    if (sumcfg.hasKeyword("DAY"))
        this->configureTimeVector(es, "DAY");

    if (sumcfg.hasKeyword("MONTH"))
        this->configureTimeVector(es, "MONTH");

    if (sumcfg.hasKeyword("YEAR"))
        this->configureTimeVector(es, "YEAR");
}

void
Opm::out::Summary::SummaryImplementation::
configureSummaryInput(const SummaryConfig& sumcfg,
                      Evaluator::Factory&  evaluatorFactory)
{
    auto unsuppkw = std::vector<SummaryConfigNode>{};
    for (const auto& node : sumcfg) {
        auto prmDescr = evaluatorFactory.create(node);

        if (! prmDescr.evaluator) {
            // No known evaluation function/type for this keyword
            unsuppkw.push_back(node);
            continue;
        }

        // This keyword has a known evaluation method.

        this->valueKeys_.push_back(std::move(prmDescr.uniquekey));
        this->valueUnits_.push_back(prmDescr.unit);

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

/*
   These nodes are added to the summary evaluation list because they are
   requested by the UDQ system. In the case of well and group variables the code
   will all nodes for all wells / groups - irrespective of what has been
   requested in the UDQ code.
*/

namespace {
    std::vector<Opm::EclIO::SummaryNode>
    make_default_nodes(const std::string& keyword,
                       const Opm::Schedule& sched)
    {
        if (Opm::TimeService::valid_month(keyword))
            return {};

        auto nodes = std::vector<Opm::EclIO::SummaryNode> {};
        auto category = Opm::parseKeywordCategory(keyword);
        auto type = Opm::parseKeywordType(keyword);

        switch (category) {
        case Opm::EclIO::SummaryNode::Category::Field:
        {
            Opm::EclIO::SummaryNode node;
            node.keyword = keyword;
            node.category = category;
            node.type = type;

            nodes.push_back(node);
        }
        break;

        case Opm::EclIO::SummaryNode::Category::Miscellaneous:
        {
            Opm::EclIO::SummaryNode node;
            node.keyword = keyword;
            node.category = category;
            node.type = type;

            nodes.push_back(node);
        }
        break;

        case Opm::EclIO::SummaryNode::Category::Well:
        {
            for (const auto& well : sched.wellNames()) {
                Opm::EclIO::SummaryNode node;
                node.keyword = keyword;
                node.category = category;
                node.type = type;
                node.wgname = well;

                nodes.push_back(node);
            }
        }
        break;

        case Opm::EclIO::SummaryNode::Category::Group:
        {
            for (const auto& group : sched.groupNames()) {
                Opm::EclIO::SummaryNode node;
                node.keyword = keyword;
                node.category = category;
                node.type = type;
                node.wgname = group;

                nodes.push_back(node);
            }
        }
        break;

        default:
            throw std::logic_error {
                fmt::format("Keyword category '{}' (e.g., summary keyword {}) is not supported in ACTIONX",
            category, keyword)
            };
        }

        return nodes;
    }
}

void Opm::out::Summary::SummaryImplementation::configureUDQ(const EclipseState& es,
                                                            const SummaryConfig& summary_config,
                                                            const Schedule& sched)
{
    const std::unordered_set<std::string> time_vectors = {"TIME", "DAY", "MONTH", "YEAR", "YEARS", "MNTH"};
    auto nodes = std::vector<Opm::EclIO::SummaryNode> {};
    std::unordered_set<std::string> summary_keys;
    for (const auto& [_, udq] : sched.unique<UDQConfig>()) {
        (void)_;
        udq.required_summary(summary_keys);
    }

    for (const auto& action : sched.back().actions.get())
        action.required_summary(summary_keys);

    for (const auto& key : summary_keys) {
        const auto& default_nodes = make_default_nodes(key, sched);
        for (const auto& def_node : default_nodes)
            nodes.push_back(def_node);
    }

    for (const auto& node: nodes) {
        // Handler already configured/requested through the normal
        // SummaryConfig path.
        if (summary_config.hasSummaryKey(node.unique_key()))
            continue;

        // Time related vectors are special cased in the valueKeys_ vector
        // and must be checked explicitly.
        if (time_vectors.count(node.keyword) > 0) {
            this->configureTimeVector(es, node.keyword);
            continue;
        }

        // Handler already registered in the summary evaluator, in some
        // other way.
        if (std::find(this->valueKeys_.begin(), this->valueKeys_.end(), node.unique_key()) != this->valueKeys_.end())
            continue;

        auto fun_pos = funs.find(node.keyword);
        if (fun_pos != funs.end()) {
            this->extra_parameters.emplace(node.unique_key(), std::make_unique<Evaluator::FunctionRelation>(node, fun_pos->second));
            continue;
        }

        auto unit = single_values_units.find(node.keyword);
        if (unit != single_values_units.end()) {
            this->extra_parameters.emplace(node.unique_key(), std::make_unique<Evaluator::GlobalProcessValue>(node, unit->second));
            continue;
        }

        if (node.is_user_defined())
            continue;

        throw std::logic_error {
            fmt::format("Evaluation function for: {} not found ", node.keyword)
        };
    }
}

void
Opm::out::Summary::SummaryImplementation::
configureRequiredRestartParameters(const SummaryConfig& sumcfg,
                                   const AquiferConfig& aqConfig,
                                   const Schedule&      sched,
                                   Evaluator::Factory&  evaluatorFactory)
{
    auto makeEvaluator = [&sumcfg, &evaluatorFactory, this]
        (const Opm::EclIO::SummaryNode& node) -> void
    {
        if (sumcfg.hasSummaryKey(node.unique_key()))
            // Handler already exists.  Don't add second evaluation.
            return;

        auto descriptor = evaluatorFactory.create(node);
        if (descriptor.evaluator == nullptr)
            throw std::logic_error {
                fmt::format("Evaluation function for:{} not found", node.keyword)
            };

        this->extra_parameters
            .emplace(node.unique_key(), std::move(descriptor.evaluator));
    };

    for (const auto& node : requiredRestartVectors(sched))
        makeEvaluator(node);

    for (const auto& node : requiredSegmentVectors(sched))
        makeEvaluator(node);

    if (aqConfig.hasAnalyticalAquifer()) {
        const auto aquiferIDs = analyticAquiferIDs(aqConfig);

        for (const auto& node : requiredAquiferVectors(aquiferIDs))
            makeEvaluator(node);
    }

    if (aqConfig.hasNumericalAquifer()) {
        const auto aquiferIDs = numericAquiferIDs(aqConfig);

        for (const auto& node : requiredNumericAquiferVectors(aquiferIDs))
            makeEvaluator(node);
    }
}

Opm::out::Summary::SummaryImplementation::MiniStep&
Opm::out::Summary::SummaryImplementation::getNextMiniStep(const int report_step, bool isSubstep)
{
    if (this->numUnwritten_ == this->unwritten_.size())
        this->unwritten_.emplace_back();

    assert ((this->numUnwritten_ < this->unwritten_.size()) &&
            "Internal inconsistency in 'unwritten' counter");

    auto& ms = this->unwritten_[this->numUnwritten_++];

    ms.id  = this->miniStepID_ - 1;  // MINISTEP IDs start at zero.
    ms.seq = report_step;
    ms.isSubstep = isSubstep;

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

namespace Opm { namespace out {

Summary::Summary(const EclipseState&  es,
                 const SummaryConfig& sumcfg,
                 const EclipseGrid&   grid,
                 const Schedule&      sched,
                 const std::string&   basename,
                 const bool           writeEsmry)
    : pImpl_(new SummaryImplementation(es, sumcfg, grid, sched, basename, writeEsmry))
{}

void Summary::eval(SummaryState&                      st,
                   const int                          report_step,
                   const double                       secs_elapsed,
                   const data::Wells&                 well_solution,
                   const data::GroupAndNetworkValues& grp_nwrk_solution,
                   GlobalProcessParameters            single_values,
                   const Inplace&                     initial_inplace,
                   const Inplace&                     inplace,
                   const PAvgCalculatorCollection&    ,
                   const RegionParameters&            region_values,
                   const BlockValues&                 block_values,
                   const Opm::data::Aquifers&         aquifer_values,
                   const InterRegFlowValues&          interreg_flows) const
{
    // Report_step is the one-based sequence number of the containing report.
    // Report_step = 0 for the initial condition, before simulation starts.
    // We typically don't get reports_step = 0 here.  When outputting
    // separate summary files 'report_step' is the number that gets
    // incorporated into the filename extension.
    //
    // Sim_step is the timestep which has been effective in the simulator,
    // and as such is the value necessary to use when looking up active
    // wells, groups, connections &c in the Schedule object.
    const auto sim_step = std::max(0, report_step - 1);

    this->pImpl_->eval(sim_step, secs_elapsed,
                       well_solution, grp_nwrk_solution, single_values,
                       initial_inplace, inplace,
                       region_values, block_values,
                       aquifer_values, interreg_flows, st);
}

PAvgCalculatorCollection
Summary::wbp_calculators(std::size_t report_step) const
{
    return this->pImpl_->wbp_calculators(report_step);
}

void Summary::add_timestep(const SummaryState& st, const int report_step, bool isSubstep)
{
    this->pImpl_->internal_store(st, report_step, isSubstep);
}

void Summary::write(const bool is_final_summary) const
{
    this->pImpl_->write(is_final_summary);
}

Summary::~Summary() {}

}} // namespace Opm::out
