/*
  Copyright 2019 Equinor ASA.

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

#include <opm/input/eclipse/Schedule/Well/Well.hpp>

#include <opm/io/eclipse/rst/well.hpp>
#include <opm/output/eclipse/VectorItems/well.hpp>

#include <opm/input/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/EclipseState/TracerConfig.hpp>

#include <opm/input/eclipse/Schedule/ScheduleGrid.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/Well/WellInjectionProperties.hpp>
#include <opm/input/eclipse/Schedule/Well/WellProductionProperties.hpp>

#include <opm/common/utility/shmatch.hpp>

#include <opm/input/eclipse/Parser/ParserKeywords/W.hpp>

#include <opm/input/eclipse/Deck/DeckRecord.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>

#include "../MSW/Compsegs.hpp"

#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>

namespace Opm {

namespace {

    bool defaulted(const DeckRecord& rec, const std::string& s) {
        const auto& item = rec.getItem( s );
        if (item.defaultApplied(0))
            return true;

        if (item.get<int>(0) == 0)
            return true;

        return false;
    }


    int limit(const DeckRecord& rec, const std::string&s , int shift) {
        const auto& item = rec.getItem( s );
        return shift + item.get<int>(0);
    }

    bool match_le(int value, const DeckRecord& rec, const std::string& s, int shift = 0) {
        if (defaulted(rec,s))
            return true;

        return (value <= limit(rec,s,shift));
    }

    bool match_ge(int value, const DeckRecord& rec, const std::string& s, int shift = 0) {
        if (defaulted(rec,s))
            return true;

        return (value >= limit(rec,s,shift));
    }


    bool match_eq(int value, const DeckRecord& rec, const std::string& s, int shift = 0) {
        if (defaulted(rec,s))
            return true;

        return (limit(rec,s,shift) == value);
    }

}

namespace {

Connection::Order order_from_int(int int_value) {
    switch(int_value) {
    case 0:
        return Connection::Order::TRACK;
    case 1:
        return Connection::Order::DEPTH;
    case 2:
        return Connection::Order::INPUT;
    default:
        throw std::invalid_argument(fmt::format("Invalid integer value: {} encountered when determining connection ordering", int_value));
    }
}

Well::Status status_from_int(int int_value) {
    using Value = RestartIO::Helpers::VectorItems::IWell::Value::Status;

    switch (int_value) {
        case Value::Shut:
            return Well::Status::SHUT;
        case Value::Stop:
            return Well::Status::STOP;
        case Value::Open:
            return Well::Status::OPEN;
        case Value::Auto:
            return Well::Status::AUTO;
    default:
        throw std::logic_error(fmt::format("integer value: {} could not be converted to a valid state", int_value));
    }
}

Well::ProducerCMode producer_cmode_from_int(const int pmode)
{
    using CModeVal = ::Opm::RestartIO::Helpers::VectorItems::
        IWell::Value::WellCtrlMode;

    switch (pmode) {
    case CModeVal::Group:    return Well::ProducerCMode::GRUP;
    case CModeVal::OilRate:  return Well::ProducerCMode::ORAT;
    case CModeVal::WatRate:  return Well::ProducerCMode::WRAT;
    case CModeVal::GasRate:  return Well::ProducerCMode::GRAT;
    case CModeVal::LiqRate:  return Well::ProducerCMode::LRAT;
    case CModeVal::ResVRate: return Well::ProducerCMode::RESV;
    case CModeVal::THP:      return Well::ProducerCMode::THP;
    case CModeVal::BHP:      return Well::ProducerCMode::BHP;
    }

    throw std::invalid_argument {
        fmt::format("Cannot convert integer value {} to producer control mode", pmode)
    };
}

Well::InjectorCMode injector_cmode_from_int(const int imode)
{
    using CModeVal = ::Opm::RestartIO::Helpers::VectorItems::
        IWell::Value::WellCtrlMode;

    switch (imode) {
    case CModeVal::Group:    return Well::InjectorCMode::GRUP;
    case CModeVal::OilRate:  [[fallthrough]];
    case CModeVal::WatRate:  [[fallthrough]];
    case CModeVal::GasRate:  [[fallthrough]];
    case CModeVal::LiqRate:  return Well::InjectorCMode::RATE;
    case CModeVal::ResVRate: return Well::InjectorCMode::RESV;
    case CModeVal::THP:      return Well::InjectorCMode::THP;
    case CModeVal::BHP:      return Well::InjectorCMode::BHP;
    }

    throw std::invalid_argument {
        fmt::format("Cannot convert integer value {} to injector control mode", imode)
    };
}

constexpr Well::ProducerCMode def_whistctl_cmode = Well::ProducerCMode::CMODE_UNDEFINED;
const static Well::WellGuideRate def_guide_rate = {true, -1, Well::GuideRateTarget::UNDEFINED, ParserKeywords::WGRUPCON::SCALING_FACTOR::defaultValue};
const static bool def_automatic_shutin = true;
constexpr double def_solvent_fraction = 0;

}

Well::Well(const RestartIO::RstWell& rst_well,
           int report_step,
           const TracerConfig& tracer_config,
           const UnitSystem& unit_system_arg,
           double udq_undefined_arg) :
    wname(rst_well.name),
    group_name(rst_well.group),
    init_step(report_step),
    headI(rst_well.ij[0]),
    headJ(rst_well.ij[1]),
    ref_depth((std::abs(rst_well.datum_depth) < 1.0e+20) ? std::optional<double>{ rst_well.datum_depth } : std::nullopt),
    drainage_radius(rst_well.drainage_radius),
    allow_cross_flow(rst_well.allow_xflow == 1),
    automatic_shutin(def_automatic_shutin),
    pvt_table(rst_well.pvt_table),
    unit_system(unit_system_arg),
    udq_undefined(udq_undefined_arg),
    wtype(rst_well.wtype),
    guide_rate(def_guide_rate),
    efficiency_factor(rst_well.efficiency_factor),
    solvent_fraction(def_solvent_fraction),
    prediction_mode(rst_well.hist_requested_control == 0),
    econ_limits(std::make_shared<WellEconProductionLimits>()),
    foam_properties(std::make_shared<WellFoamProperties>()),
    polymer_properties(std::make_shared<WellPolymerProperties>()),
    micp_properties(std::make_shared<WellMICPProperties>()),
    brine_properties(std::make_shared<WellBrineProperties>()),
    tracer_properties(std::make_shared<WellTracerProperties>()),
    connections(std::make_shared<WellConnections>(order_from_int(rst_well.completion_ordering), headI, headJ)),
    production(std::make_shared<WellProductionProperties>(unit_system_arg, wname)),
    injection(std::make_shared<WellInjectionProperties>(unit_system_arg, wname)),
    wvfpexp(std::make_shared<WVFPEXP>()),
    status(status_from_int(rst_well.well_status))
{
    if (this->wtype.producer()) {
        auto p = std::make_shared<WellProductionProperties>(this->unit_system, wname);
        // Reverse of function ctrlMode() in AggregateWellData.cpp
        p->whistctl_cmode = def_whistctl_cmode;
        p->BHPTarget.update(rst_well.bhp_target_float);
        p->OilRate.update(rst_well.orat_target);
        p->WaterRate.update(rst_well.wrat_target);
        p->GasRate.update(rst_well.grat_target);
        p->LiquidRate.update(rst_well.lrat_target) ;
        p->ResVRate.update(rst_well.resv_target);
        p->VFPTableNumber = rst_well.vfp_table;
        p->ALQValue.update(rst_well.alq_value); // Uncertain of whether the dimension comes through correct here.
        p->predictionMode = this->prediction_mode;

        if (rst_well.orat_target != 0)
            p->addProductionControl( Well::ProducerCMode::ORAT );

        if (rst_well.wrat_target != 0)
            p->addProductionControl( Well::ProducerCMode::WRAT );

        if (rst_well.grat_target != 0)
            p->addProductionControl( Well::ProducerCMode::GRAT );

        if (rst_well.lrat_target != 0)
            p->addProductionControl( Well::ProducerCMode::LRAT );

        if (rst_well.resv_target != 0)
            p->addProductionControl( Well::ProducerCMode::RESV );

        if (rst_well.thp_target != 0) {
            p->THPTarget.update(rst_well.thp_target);
            p->addProductionControl( Well::ProducerCMode::THP );
        }

        if (! p->predictionMode)
            p->clearControls();

        p->controlMode = producer_cmode_from_int(rst_well.active_control);
        p->addProductionControl(p->controlMode);

        p->addProductionControl(Well::ProducerCMode::BHP);
        if (! p->predictionMode) {
            p->BHPTarget.update(0.0);
            p->setBHPLimit(rst_well.bhp_target_double);
            p->controlMode = p->whistctl_cmode =
                producer_cmode_from_int(rst_well.hist_requested_control);
        }
        else if (this->isAvailableForGroupControl())
            p->addProductionControl(Well::ProducerCMode::GRUP);

        this->updateProduction(std::move(p));
    }
    else {
        auto i = std::make_shared<WellInjectionProperties>(this->unit_system, wname);
        i->VFPTableNumber = rst_well.vfp_table;
        i->predictionMode = this->prediction_mode;

        if ((std::abs(rst_well.wrat_target) > 0.0f) ||
            (std::abs(rst_well.grat_target) > 0.0f))
            i->addInjectionControl(Well::InjectorCMode::RATE);

        if (std::abs(rst_well.resv_target) > 0.0f) {
            i->reservoirInjectionRate.update(rst_well.resv_target);
            i->addInjectionControl(Well::InjectorCMode::RESV);
        }

        i->injectorType = rst_well.wtype.injector_type();
        switch (i->injectorType) {
        case InjectorType::WATER:
            i->surfaceInjectionRate.update(rst_well.wrat_target);
            break;
        case InjectorType::GAS:
            i->surfaceInjectionRate.update(rst_well.grat_target);
            break;
        default:
            throw std::invalid_argument("What ...");
        }

        if (rst_well.thp_target != 0.0f) {
            i->THPTarget.update(rst_well.thp_target);
            i->addInjectionControl(Well::InjectorCMode::THP);
        }

        const auto active_control = i->predictionMode
            ? injector_cmode_from_int(rst_well.active_control)
            : injector_cmode_from_int(rst_well.hist_requested_control);

        if (! i->predictionMode) {
            i->clearControls();
            if ((active_control != Well::InjectorCMode::RATE) &&
                (active_control != Well::InjectorCMode::BHP))
            {
                throw std::invalid_argument {
                    fmt::format("Unsupported control mode '{}' for "
                                "history controlled injection well '{}'",
                                Well::InjectorCMode2String(active_control), this->name())
                };
            }
        }

        i->controlMode = active_control;
        i->addInjectionControl(active_control);

        i->addInjectionControl(Well::InjectorCMode::BHP);
        i->BHPTarget.update(rst_well.bhp_target_float);
        if (! i->predictionMode) {
            if (i->controlMode == Well::InjectorCMode::BHP)
                i->bhp_hist_limit = rst_well.hist_bhp_target;
            else
                i->resetDefaultHistoricalBHPLimit();
        }
        else if (this->isAvailableForGroupControl())
            i->addInjectionControl(Well::InjectorCMode::GRUP);

        this->updateInjection(std::move(i));

        if (!rst_well.tracer_concentration_injection.empty()) {
            auto tracer = std::make_shared<WellTracerProperties>(this->getTracerProperties());
            for (std::size_t tracer_index = 0; tracer_index < tracer_config.size(); tracer_index++) {
                const auto& name = tracer_config[tracer_index].name;
                const auto concentration = rst_well.tracer_concentration_injection[tracer_index];
                tracer->setConcentration(name, concentration);
            }
            this->updateTracer(tracer);
        }
    }
}



Well::Well(const std::string& wname_arg,
           const std::string& gname,
           std::size_t init_step_arg,
           std::size_t insert_index_arg,
           int headI_arg,
           int headJ_arg,
           const std::optional<double>& ref_depth_arg,
           const WellType& wtype_arg,
           ProducerCMode whistctl_cmode,
           Connection::Order ordering_arg,
           const UnitSystem& unit_system_arg,
           double udq_undefined_arg,
           double dr,
           bool allow_xflow,
           bool auto_shutin,
           int pvt_table_,
           GasInflowEquation inflow_eq):
    wname(wname_arg),
    group_name(gname),
    init_step(init_step_arg),
    insert_index(insert_index_arg),
    headI(headI_arg),
    headJ(headJ_arg),
    ref_depth(ref_depth_arg),
    drainage_radius(dr),
    allow_cross_flow(allow_xflow),
    automatic_shutin(auto_shutin),
    pvt_table(pvt_table_),
    gas_inflow(inflow_eq),
    unit_system(unit_system_arg),
    udq_undefined(udq_undefined_arg),
    wtype(wtype_arg),
    guide_rate({true, -1, Well::GuideRateTarget::UNDEFINED,ParserKeywords::WGRUPCON::SCALING_FACTOR::defaultValue}),
    efficiency_factor(1.0),
    solvent_fraction(0.0),
    econ_limits(std::make_shared<WellEconProductionLimits>()),
    foam_properties(std::make_shared<WellFoamProperties>()),
    polymer_properties(std::make_shared<WellPolymerProperties>()),
    micp_properties(std::make_shared<WellMICPProperties>()),
    brine_properties(std::make_shared<WellBrineProperties>()),
    tracer_properties(std::make_shared<WellTracerProperties>()),
    connections(std::make_shared<WellConnections>(ordering_arg, headI, headJ)),
    production(std::make_shared<WellProductionProperties>(unit_system, wname)),
    injection(std::make_shared<WellInjectionProperties>(unit_system, wname)),
    wvfpexp(std::make_shared<WVFPEXP>()),
    status(Status::SHUT)
{
    auto p = std::make_shared<WellProductionProperties>(this->unit_system, this->wname);
    p->whistctl_cmode = whistctl_cmode;
    this->updateProduction(p);
}

Well Well::serializeObject()
{
    Well result;
    result.wname = "test1";
    result.group_name = "test2";
    result.init_step = 1;
    result.insert_index = 2;
    result.headI = 3;
    result.headJ = 4;
    result.ref_depth = 5;
    result.unit_system = UnitSystem::serializeObject();
    result.udq_undefined = 6.0;
    result.status = Status::AUTO;
    result.drainage_radius = 7.0;
    result.allow_cross_flow = true;
    result.automatic_shutin = false;
    result.pvt_table = 77;
    result.gas_inflow = GasInflowEquation::GPP;
    result.wtype = WellType(Phase::WATER);
    result.guide_rate = WellGuideRate::serializeObject();
    result.efficiency_factor = 8.0;
    result.solvent_fraction = 9.0;
    result.prediction_mode = false;
    result.econ_limits = std::make_shared<Opm::WellEconProductionLimits>(Opm::WellEconProductionLimits::serializeObject());
    result.foam_properties = std::make_shared<WellFoamProperties>(WellFoamProperties::serializeObject());
    result.polymer_properties =  std::make_shared<WellPolymerProperties>(WellPolymerProperties::serializeObject());
    result.micp_properties =  std::make_shared<WellMICPProperties>(WellMICPProperties::serializeObject());
    result.brine_properties = std::make_shared<WellBrineProperties>(WellBrineProperties::serializeObject());
    result.tracer_properties = std::make_shared<WellTracerProperties>(WellTracerProperties::serializeObject());
    result.connections = std::make_shared<WellConnections>(WellConnections::serializeObject());
    result.production = std::make_shared<Well::WellProductionProperties>(Well::WellProductionProperties::serializeObject());
    result.injection = std::make_shared<Well::WellInjectionProperties>(Well::WellInjectionProperties::serializeObject());
    result.segments = std::make_shared<WellSegments>(WellSegments::serializeObject());
    result.wvfpexp = std::make_shared<WVFPEXP>(WVFPEXP::serializeObject());
    result.m_pavg = PAvg();

    return result;
}


bool Well::updateWPAVE(const PAvg& pavg) {
    if (this->m_pavg == pavg)
        return false;

    this->m_pavg = pavg;
    return true;
}


bool Well::updateEfficiencyFactor(double efficiency_factor_arg) {
    if (this->efficiency_factor != efficiency_factor_arg) {
        this->efficiency_factor = efficiency_factor_arg;
        return true;
    }

    return false;
}

bool Well::updateWellGuideRate(double guide_rate_arg) {
    if (this->guide_rate.guide_rate != guide_rate_arg) {
        this->guide_rate.guide_rate = guide_rate_arg;
        return true;
    }

    return false;
}


bool Well::updateFoamProperties(std::shared_ptr<WellFoamProperties> foam_properties_arg) {
    if (this->wtype.producer()) {
        throw std::runtime_error("Not allowed to set foam injection properties for well " + name()
                                 + " since it is a production well");
    }
    if (*this->foam_properties != *foam_properties_arg) {
        this->foam_properties = foam_properties_arg;
        return true;
    }

    return false;
}


bool Well::updatePolymerProperties(std::shared_ptr<WellPolymerProperties> polymer_properties_arg) {
    if (this->wtype.producer()) {
        throw std::runtime_error("Not allowed to set polymer injection properties for well " + name() +
                                 " since it is a production well");
    }
    if (*this->polymer_properties != *polymer_properties_arg) {
        this->polymer_properties = polymer_properties_arg;
        return true;
    }

    return false;
}

bool Well::updateMICPProperties(std::shared_ptr<WellMICPProperties> micp_properties_arg) {
    if (this->wtype.producer()) {
        throw std::runtime_error("Not allowed to set micp injection properties for well " + name() +
                                 " since it is a production well");
    }
    if (*this->micp_properties != *micp_properties_arg) {
        this->micp_properties = micp_properties_arg;
        return true;
    }

    return false;
}

bool Well::updateBrineProperties(std::shared_ptr<WellBrineProperties> brine_properties_arg) {
    if (this->wtype.producer()) {
        throw std::runtime_error("Not allowed to set brine injection properties for well " + name() +
                                 " since it is a production well");
    }
    if (*this->brine_properties != *brine_properties_arg) {
        this->brine_properties = brine_properties_arg;
        return true;
    }

    return false;
}


bool Well::updateEconLimits(std::shared_ptr<WellEconProductionLimits> econ_limits_arg) {
    if (*this->econ_limits != *econ_limits_arg) {
        this->econ_limits = econ_limits_arg;
        return true;
    }

    return false;
}

bool Well::updateWVFPEXP(std::shared_ptr<WVFPEXP> wvfpexp_arg) {
    if (*this->wvfpexp != *wvfpexp_arg) {
        this->wvfpexp = std::move(wvfpexp_arg);
        return true;
    }

    return false;
}

void Well::switchToProducer() {
    auto p = std::make_shared<WellInjectionProperties>(this->getInjectionProperties());

    p->BHPTarget.update(0);
    p->dropInjectionControl( Opm::Well::InjectorCMode::BHP );
    this->updateInjection( p );
    this->wtype.update(true);
}


void Well::switchToInjector() {
    auto p = std::make_shared<WellProductionProperties>(getProductionProperties());

    p->setBHPLimit(0);
    p->dropProductionControl( ProducerCMode::BHP );
    this->updateProduction( p );
}

bool Well::updateInjection(std::shared_ptr<WellInjectionProperties> injection_arg) {
    auto update = this->wtype.update(injection_arg->injectorType);
    if (this->wtype.producer()) {
        this->switchToInjector();
        update = true;
    }

    if (*this->injection != *injection_arg) {
        this->injection = injection_arg;
        update = true;
    }

    return update;
}

bool Well::updateWellProductivityIndex() {
    return this->connections->prepareWellPIScaling();
}

bool Well::updateHasProduced() {
    if (this->wtype.producer() && this->getStatus() == Status::OPEN) {
        if (this->has_produced)
            return false;

        this->has_produced = true;
        return true;
    }
    return false;
}

bool Well::updateHasInjected() {
    if (this->wtype.injector() && this->getStatus() == Status::OPEN) {
        if (this->has_injected)
            return false;

        this->has_injected= true;
        return true;
    }
    return false;
}

bool Well::updateProduction(std::shared_ptr<WellProductionProperties> production_arg) {
    if (!this->wtype.producer())
        this->switchToProducer( );

    if (*this->production != *production_arg) {
        this->production = production_arg;
        return true;
    }

    return false;
}

bool Well::updateTracer(std::shared_ptr<WellTracerProperties> tracer_properties_arg) {
    if (*this->tracer_properties != *tracer_properties_arg) {
        this->tracer_properties = tracer_properties_arg;
        return true;
    }

    return false;
}

bool Well::updateWellGuideRate(bool available, double guide_rate_arg, GuideRateTarget guide_phase, double scale_factor) {
    bool update = false;
    if (this->guide_rate.available != available) {
        this->guide_rate.available = available;
        update = true;
    }

    if(this->guide_rate.guide_rate != guide_rate_arg) {
        this->guide_rate.guide_rate = guide_rate_arg;
        update = true;
    }

    if(this->guide_rate.guide_phase != guide_phase) {
        this->guide_rate.guide_phase = guide_phase;
        update = true;
    }

    if(this->guide_rate.scale_factor != scale_factor) {
        this->guide_rate.scale_factor = scale_factor;
        update = true;
    }

    return update;
}




bool Well::updateGroup(const std::string& group_arg) {
    if (this->group_name != group_arg) {
        this->group_name = group_arg;
        return true;
    }
    return false;
}


bool Well::updateHead(int I, int J) {
    bool update = false;
    if (this->headI != I) {
        this->headI = I;
        update = true;
    }

    if (this->headJ != J) {
        this->headJ = J;
        update = true;
    }

    return update;
}



bool Well::updateStatus(Status well_state) {
    this->status = well_state;
    return true;
}





bool Well::updateRefDepth(const std::optional<double>& ref_depth_arg) {
    if (this->ref_depth != ref_depth_arg) {
        this->ref_depth = ref_depth_arg;
        return true;
    }

    return false;
}

bool Well::updateDrainageRadius(double drainage_radius_arg) {
    if (this->drainage_radius != drainage_radius_arg) {
        this->drainage_radius = drainage_radius_arg;
        return true;
    }

    return false;
}


bool Well::updateCrossFlow(bool allow_cross_flow_arg) {
    if (this->allow_cross_flow != allow_cross_flow_arg) {
        this->allow_cross_flow = allow_cross_flow_arg;
        return true;
    }

    return false;
}

bool Well::updateAutoShutin(bool auto_shutin) {
    if (this->automatic_shutin != auto_shutin) {
        this->automatic_shutin = auto_shutin;
        return true;
    }

    return false;
}


bool Well::updateConnections(std::shared_ptr<WellConnections> connections_arg, bool force) {
    connections_arg->order(  );
    if (force || *this->connections != *connections_arg) {
        this->connections = connections_arg;
        return true;
    }
    return false;
}

bool Well::updateConnections(std::shared_ptr<WellConnections> connections_arg, const ScheduleGrid& grid) {
    bool update = this->updateConnections(connections_arg, false);
    if (this->pvt_table == 0 && !this->connections->empty()) {
        const auto& lowest = this->connections->lowest();
        const auto& props = grid.get_cell(lowest.getI(), lowest.getJ(), lowest.getK()).props;
        this->pvt_table = props->pvtnum;
        update = true;
    }
    return update;
}

bool Well::updateSolventFraction(double solvent_fraction_arg) {
    if (this->solvent_fraction != solvent_fraction_arg) {
        this->solvent_fraction = solvent_fraction_arg;
        return true;
    }

    return false;
}


bool Well::handleCOMPSEGS(const DeckKeyword& keyword,
                          const ScheduleGrid& grid,
                          const ParseContext& parseContext,
                          ErrorGuard& errors) {
    auto [new_connections, new_segments] = Compsegs::processCOMPSEGS(
        keyword,
        *this->connections,
        *this->segments,
        grid,
        parseContext,
        errors
    );

    this->updateConnections( std::make_shared<WellConnections>(std::move(new_connections)), false );
    this->updateSegments( std::make_shared<WellSegments>( std::move(new_segments)) );
    return true;
}

const std::string& Well::groupName() const {
    return this->group_name;
}


bool Well::isMultiSegment() const {
    if (this->segments)
        return true;
    return false;
}

bool Well::isProducer() const {
    return this->wtype.producer();
}

bool Well::isInjector() const {
    return this->wtype.injector();
}

const WellType& Well::wellType() const {
    return this->wtype;
}

Well::InjectorCMode Well::injection_cmode() const {
    if (this->isInjector())
        return this->injection->controlMode;
    throw std::logic_error(fmt::format("Queried for INJECTION cmode for producer: {}", this->name()));
}

Well::ProducerCMode Well::production_cmode() const {
    if (this->isProducer())
        return this->production->controlMode;
    throw std::logic_error(fmt::format("Queried for PRODUCTION cmode for injector : {}", this->name()));
}

InjectorType Well::injectorType() const {
    if (this->wtype.producer())
        throw std::runtime_error("Can not access injectorType attribute of a producer");

    return this->injection->injectorType;
}



bool Well::isAvailableForGroupControl() const {
    return this->guide_rate.available;
}

double Well::getGuideRate() const {
    return this->guide_rate.guide_rate;
}

Well::GuideRateTarget Well::getGuideRatePhase() const
{
    const auto target = this->getRawGuideRatePhase();

    if (this->isInjector() && (target == GuideRateTarget::RAT)) {
        return this->preferredPhaseAsGuideRatePhase();
    }

    return target;
}

Well::GuideRateTarget Well::getRawGuideRatePhase() const
{
    return this->guide_rate.guide_phase;
}

Well::GuideRateTarget Well::preferredPhaseAsGuideRatePhase() const
{
    switch (this->getPreferredPhase()) {
    case Phase::OIL:   return GuideRateTarget::OIL;
    case Phase::GAS:   return GuideRateTarget::GAS;
    case Phase::WATER: return GuideRateTarget::WAT;

    default:
        throw std::logic_error {
            fmt::format("Unable to convert well preferred "
                        "phase {} to GuideRate target phase",
                        static_cast<int>(this->getPreferredPhase()))
        };
    }
}

double Well::getGuideRateScalingFactor() const {
    return this->guide_rate.scale_factor;
}


double Well::getEfficiencyFactor() const {
    return this->efficiency_factor;
}

double Well::getSolventFraction() const {
    return this->solvent_fraction;
}



std::size_t Well::seqIndex() const {
    return this->insert_index;
}

int Well::getHeadI() const {
    return this->headI;
}

int Well::getHeadJ() const {
    return this->headJ;
}

bool Well::getAutomaticShutIn() const {
    return this->automatic_shutin;
}

bool Well::getAllowCrossFlow() const {
    return this->allow_cross_flow;
}

bool Well::hasRefDepth() const
{
    return this->ref_depth.has_value();
}

double Well::getRefDepth() const {
    if (!this->hasRefDepth())
        throw std::logic_error(fmt::format("Well: {} - tried to access not initialized well reference depth", this->name()));
    return *this->ref_depth;
}

double Well::getWPaveRefDepth() const {
    return this->wpave_ref_depth.value_or( this->getRefDepth() );
}

void Well::updateRefDepth() {
    if( !this->ref_depth ) {
        // ref depth was defaulted and we get the depth of the first completion

        if( this->connections->empty() )
            throw std::invalid_argument( "No completions defined for well: "
                                         + name()
                                     + ". Can not infer reference depth" );
        this->ref_depth = this->connections->get(0).depth();
    }
}

void Well::updateWPaveRefDepth(double depth) {
    this->wpave_ref_depth = depth;
}

double Well::getDrainageRadius() const {
    return this->drainage_radius;
}

//const std::vector<std::string>& Well::wListNames() const {
//    return this->w_list_names;
//}


const std::string& Well::name() const {
    return this->wname;
}

bool Well::hasSameConnectionsPointers(const Well& other) const
{
    // Note: This is *supposed* to be a pointer comparison.  We need to know
    // if the two connection structures represent the exact same object, not
    // just if they have the same value.
    return this->connections == other.connections;
}

void Well::setInsertIndex(std::size_t index) {
    this->insert_index = index;
}


double Well::convertDeckPI(double deckPI) const {
    using M = UnitSystem::measure;

    // XXX: Should really have LIQUID here too, but the 'Phase' type does
    //      not provide that enumerator.
    switch (this->getPreferredPhase()) {
    case Phase::GAS:
        return this->unit_system.to_si(M::gas_productivity_index, deckPI);

    case Phase::OIL:
    case Phase::WATER:
        return this->unit_system.to_si(M::liquid_productivity_index, deckPI);

    default:
        throw std::invalid_argument {
            "Preferred phase " + std::to_string(static_cast<int>(this->getPreferredPhase())) +
            " is not supported. Must be one of 'OIL', 'GAS', or 'WATER'"
        };
    }
}



void Well::applyWellProdIndexScaling(const double scalingFactor, std::vector<bool>& scalingApplicable) {
    this->connections->applyWellPIScaling(scalingFactor, scalingApplicable);
}

const WellConnections& Well::getConnections() const {
    return *this->connections;
}

const std::vector<const Connection *> Well::getConnections(int completion) const {
    std::vector<const Connection *> connvector;
    for (const auto& conn : this->getConnections()) {
        if (conn.complnum() == completion)
            connvector.push_back( &conn );
    }
    return connvector;
}

const WellFoamProperties& Well::getFoamProperties() const {
    return *this->foam_properties;
}

const WellPolymerProperties& Well::getPolymerProperties() const {
    return *this->polymer_properties;
}

const WellMICPProperties& Well::getMICPProperties() const {
    return *this->micp_properties;
}

const WellBrineProperties& Well::getBrineProperties() const {
    return *this->brine_properties;
}

const WellTracerProperties& Well::getTracerProperties() const {
    return *this->tracer_properties;
}

const WVFPEXP& Well::getWVFPEXP() const {
    return *this->wvfpexp;
}

const WellEconProductionLimits& Well::getEconLimits() const {
    return *this->econ_limits;
}

const Well::WellProductionProperties& Well::getProductionProperties() const {
    return *this->production;
}

const WellSegments& Well::getSegments() const {
    if (this->segments)
        return *this->segments;
    else
        throw std::logic_error("Asked for segment information in not MSW well: " + this->name());
}

const Well::WellInjectionProperties& Well::getInjectionProperties() const {
    return *this->injection;
}


Well::Status Well::getStatus() const {
    return this->status;
}

const PAvg& Well::pavg() const {
    return this->m_pavg;
}


std::map<int, std::vector<Connection>> Well::getCompletions() const {
    std::map<int, std::vector<Connection>> completions;

    for (const auto& conn : *this->connections) {
        auto pair = completions.find( conn.complnum() );
        if (pair == completions.end())
            completions[conn.complnum()] = {};

        pair = completions.find(conn.complnum());
        pair->second.push_back(conn);
    }

    return completions;
}

bool Well::hasCompletion(int completion) const {
    for (const auto& conn : *this->connections) {
        if (conn.complnum() == completion)
            return true;
    }
    return false;
}



Phase Well::getPreferredPhase() const {
    return this->wtype.preferred_phase();
}


int Well::pvt_table_number() const {
    return this->pvt_table;
}

int Well::fip_region_number() const {
    return ParserKeywords::WELSPECS::FIP_REGION::defaultValue;
}

/*
  When all connections of a well are closed with the WELOPEN keywords, the well
  itself should also be SHUT. In the main parsing code this is handled by the
  function checkIfAllConnectionsIsShut() which is called at the end of every
  report step in Schedule::iterateScheduleSection(). This is done in this way
  because there is some twisted logic aggregating connection changes over a
  complete report step.

  However - when the WELOPEN is called runtime (typically as an ACTIONX action)
  the full Schedule::iterateScheduleSection() is not run and the check if all
  connections is closed is not done. Therefor we have a runtime flag here
  which makes sure to close the well in this case.
*/


bool Well::handleWELOPENConnections(const DeckRecord& record, Connection::State state_arg) {

    auto match = [=]( const Connection &c) -> bool {
        if (!match_eq(c.getI(), record, "I" , -1)) return false;
        if (!match_eq(c.getJ(), record, "J" , -1)) return false;
        if (!match_eq(c.getK(), record, "K", -1))  return false;
        if (!match_ge(c.complnum(), record, "C1")) return false;
        if (!match_le(c.complnum(), record, "C2")) return false;

        return true;
    };

    auto new_connections = std::make_shared<WellConnections>(this->connections->ordering(), this->headI, this->headJ);

    for (auto c : *this->connections) {
        if (match(c))
            c.setState( state_arg );

        new_connections->add(c);
    }
    return this->updateConnections(std::move(new_connections), false);
}





bool Well::handleCOMPLUMP(const DeckRecord& record) {

    auto match = [=]( const Connection &c) -> bool {
        if (!match_eq(c.getI(), record, "I" , -1)) return false;
        if (!match_eq(c.getJ(), record, "J" , -1)) return false;
        if (!match_ge(c.getK(), record, "K1", -1)) return false;
        if (!match_le(c.getK(), record, "K2", -1)) return false;

        return true;
    };

    auto new_connections = std::make_shared<WellConnections>(this->connections->ordering(), this->headI, this->headJ);
    const int complnum = record.getItem("N").get<int>(0);
    if (complnum <= 0)
        throw std::invalid_argument("Completion number must be >= 1. COMPLNUM=" + std::to_string(complnum) + "is invalid");

    for (auto c : *this->connections) {
        if (match(c))
            c.setComplnum( complnum );

        new_connections->add(c);
    }

    return this->updateConnections(std::move(new_connections), false);
}



bool Well::handleWPIMULT(const DeckRecord& record) {

    auto match = [=]( const Connection &c) -> bool {
        if (!match_ge(c.complnum(), record, "FIRST")) return false;
        if (!match_le(c.complnum(), record, "LAST"))  return false;
        if (!match_eq(c.getI()  , record, "I", -1)) return false;
        if (!match_eq(c.getJ()  , record, "J", -1)) return false;
        if (!match_eq(c.getK()  , record, "K", -1)) return false;

        return true;
    };

    auto new_connections = std::make_shared<WellConnections>(this->connections->ordering(), this->headI, this->headJ);
    double wellPi = record.getItem("WELLPI").get< double >(0);

    for (auto c : *this->connections) {
        if (match(c))
            c.scaleWellPi( wellPi );

        new_connections->add(c);
    }

    return this->updateConnections(std::move(new_connections), false);
}


void Well::updateSegments(std::shared_ptr<WellSegments> segments_arg) {
    this->segments = std::move(segments_arg);
    this->updateRefDepth( this->segments->depthTopSegment() );
}


bool Well::handleWELSEGS(const DeckKeyword& keyword) {
    if (this->segments) {
        auto new_segments = std::make_shared<WellSegments>( *this->segments );
        new_segments->loadWELSEGS(keyword);
        this->updateSegments(std::move(new_segments));
    } else
        this->updateSegments( std::make_shared<WellSegments>(keyword) );
    return true;
}

bool Well::updatePVTTable(int pvt_table_) {
    if (this->pvt_table != pvt_table_) {
        this->pvt_table = pvt_table_;
        return true;
    } else
        return false;
}


bool Well::updateWSEGSICD(const std::vector<std::pair<int, SICD> >& sicd_pairs) {
    auto new_segments = std::make_shared<WellSegments>(*this->segments);
    if (new_segments->updateWSEGSICD(sicd_pairs)) {
        this->segments = new_segments;
        return true;
    } else
        return false;
}


bool Well::updateWSEGAICD(const std::vector<std::pair<int, AutoICD> >& aicd_pairs, const KeywordLocation& location) {
    auto new_segments = std::make_shared<WellSegments>(*this->segments);
    if (new_segments->updateWSEGAICD(aicd_pairs, location)) {
        this->segments = new_segments;
        return true;
    } else
        return false;
}


bool Well::updateWSEGVALV(const std::vector<std::pair<int, Valve> >& valve_pairs) {
    auto new_segments = std::make_shared<WellSegments>(*this->segments);
    if (new_segments->updateWSEGVALV(valve_pairs)) {
        this->segments = new_segments;
        return true;
    } else
        return false;
}


void Well::filterConnections(const ActiveGridCells& grid) {
    this->connections->filter(grid);
}


std::size_t Well::firstTimeStep() const {
    return this->init_step;
}

bool Well::hasBeenDefined(size_t timeStep) const {
    if (timeStep < this->init_step)
        return false;
    else
        return true;
}

Well::GasInflowEquation Well::gas_inflow_equation() const {
    return this->gas_inflow;
}

const std::string Well::GasInflowEquation2String(GasInflowEquation enumValue) {
    switch(enumValue) {
    case GasInflowEquation::STD:
        return "STD";
    case GasInflowEquation::R_G:
        return "R-G";
    case GasInflowEquation::P_P:
        return "P-P";
    case GasInflowEquation::GPP:
        return "GPP";
    default:
        throw std::invalid_argument("Unhandled enum value");
    }
}

Well::GasInflowEquation Well::GasInflowEquationFromString(const std::string& stringValue) {
    if (stringValue == "STD" || stringValue == "NO")
        return GasInflowEquation::STD;

    if (stringValue == "R-G" || stringValue == "YES")
        return GasInflowEquation::R_G;

    if (stringValue == "P-P")
        return GasInflowEquation::P_P;

    if (stringValue == "GPP")
        return GasInflowEquation::GPP;

    throw std::invalid_argument("Gas inflow equation type: " + stringValue + " not recognized");
}


bool Well::canOpen() const {
    if (this->allow_cross_flow)
        return true;

    /*
      If the UDAValue is in string mode we return true unconditionally, without
      evaluating the internal UDA value.
    */
    if (this->wtype.producer()) {
        const auto& prod = *this->production;
        if (prod.OilRate.is<std::string>())
            return true;

        if (prod.GasRate.is<std::string>())
          return true;

        if (prod.WaterRate.is<std::string>())
          return true;

        if (!prod.OilRate.zero())
            return true;

        if (!prod.GasRate.zero())
            return true;

        if (!prod.WaterRate.zero())
            return true;

        return false;
    } else {
        const auto& inj = *this->injection;
        if (inj.surfaceInjectionRate.is<std::string>())
            return true;

        return !inj.surfaceInjectionRate.zero();
    }
}


bool Well::predictionMode() const {
    return this->prediction_mode;
}

bool Well::hasProduced( ) const {
    return this->has_produced;
}

bool Well::hasInjected( ) const {
    return this->has_injected;
}


bool Well::updatePrediction(bool prediction_mode_arg) {
    if (this->prediction_mode != prediction_mode_arg) {
        this->prediction_mode = prediction_mode_arg;
        return true;
    }

    return false;
}


double Well::production_rate(const SummaryState& st, Phase prod_phase) const {
    if( !this->isProducer() ) return 0.0;

    const auto controls = this->productionControls(st);

    switch( prod_phase ) {
        case Phase::WATER: return controls.water_rate;
        case Phase::OIL:   return controls.oil_rate;
        case Phase::GAS:   return controls.gas_rate;
        case Phase::SOLVENT:
            throw std::invalid_argument( "Production of 'SOLVENT' requested." );
        case Phase::POLYMER:
            throw std::invalid_argument( "Production of 'POLYMER' requested." );
        case Phase::ENERGY:
            throw std::invalid_argument( "Production of 'ENERGY' requested." );
        case Phase::POLYMW:
            throw std::invalid_argument( "Production of 'POLYMW' requested.");
        case Phase::FOAM:
            throw std::invalid_argument( "Production of 'FOAM' requested.");
        case Phase::BRINE:
            throw std::invalid_argument( "Production of 'BRINE' requested.");
        case Phase::ZFRACTION:
            throw std::invalid_argument( "Production of 'ZFRACTION' requested.");
    }

    throw std::logic_error( "Unreachable state. Invalid Phase value. "
                            "This is likely a programming error." );
}

double Well::injection_rate(const SummaryState& st, Phase phase_arg) const {
    if( !this->isInjector() ) return 0.0;
    const auto controls = this->injectionControls(st);

    const auto type = controls.injector_type;

    if( phase_arg == Phase::WATER && type != InjectorType::WATER ) return 0.0;
    if( phase_arg == Phase::OIL   && type != InjectorType::OIL   ) return 0.0;
    if( phase_arg == Phase::GAS   && type != InjectorType::GAS   ) return 0.0;

    return controls.surface_rate;
}


bool Well::wellNameInWellNamePattern(const std::string& wellName, const std::string& wellNamePattern) {
    bool wellNameInPattern = false;
    if (shmatch( wellNamePattern, wellName)) {
        wellNameInPattern = true;
    }
    return wellNameInPattern;
}


Well::ProductionControls Well::productionControls(const SummaryState& st) const {
    if (this->isProducer()) {
        auto controls = this->production->controls(st, this->udq_undefined);
        controls.prediction_mode = this->predictionMode();
        return controls;
    } else
        throw std::logic_error("Trying to get production data from an injector");
}

Well::InjectionControls Well::injectionControls(const SummaryState& st) const {
    if (!this->isProducer()) {
        auto controls = this->injection->controls(this->unit_system, st, this->udq_undefined);
        controls.prediction_mode = this->predictionMode();
        return controls;
    } else
        throw std::logic_error("Trying to get injection data from a producer");
}


/*
  These accessor functions are at the "wrong" level of abstraction; the same
  properties are part of the InjectionControls and ProductionControls structs.
  They are made available here to avoid passing a SummaryState instance in
  situations where it is not really needed.
*/


int Well::vfp_table_number() const {
    if (this->wtype.producer())
        return this->production->VFPTableNumber;
    else
        return this->injection->VFPTableNumber;
}

/*
  This short circuits the UDA and assumes the UDA contains a double.
*/
double Well::alq_value() const {
    if (this->wtype.producer())
        return this->production->ALQValue.getSI();

    throw std::runtime_error("Can not ask for ALQ value in an injector");
}

double Well::temperature() const {
    if (!this->wtype.producer())
        return this->injection->temperature;

    throw std::runtime_error("Can not ask for temperature in a producer");
}

std::ostream& operator<<(std::ostream& os, const Well::Status& st) {
    os << Well::Status2String(st);
    return os;
}

std::string Well::Status2String(Well::Status enumValue) {
    switch( enumValue ) {
    case Status::OPEN:
        return "OPEN";
    case Status::SHUT:
        return "SHUT";
    case Status::AUTO:
        return "AUTO";
    case Status::STOP:
        return "STOP";
    default:
        throw std::invalid_argument("unhandled enum value");
    }
}


Well::Status Well::StatusFromString(const std::string& stringValue) {
    if (stringValue == "OPEN")
        return Status::OPEN;
    else if (stringValue == "SHUT")
        return Status::SHUT;
    else if (stringValue == "STOP")
        return Status::STOP;
    else if (stringValue == "AUTO")
        return Status::AUTO;
    else
        throw std::invalid_argument("Unknown enum state string: " + stringValue );
}




const std::string Well::InjectorCMode2String( InjectorCMode enumValue ) {
    switch( enumValue ) {
    case InjectorCMode::RESV:
        return "RESV";
    case InjectorCMode::RATE:
        return "RATE";
    case InjectorCMode::BHP:
        return "BHP";
    case InjectorCMode::THP:
        return "THP";
    case InjectorCMode::GRUP:
        return "GRUP";
    default:
        throw std::invalid_argument("Unhandled enum value: " + std::to_string(static_cast<int>(enumValue)) + " in InjectorCMode2String");
    }
}


Well::InjectorCMode Well::InjectorCModeFromString(const std::string &stringValue) {
    if (stringValue == "RATE")
        return InjectorCMode::RATE;
    else if (stringValue == "RESV")
        return InjectorCMode::RESV;
    else if (stringValue == "BHP")
        return InjectorCMode::BHP;
    else if (stringValue == "THP")
        return InjectorCMode::THP;
    else if (stringValue == "GRUP")
        return InjectorCMode::GRUP;
    else
        throw std::invalid_argument("Unknown control mode string: " + stringValue);
}

std::ostream& operator<<(std::ostream& os, const Well::InjectorCMode& cm) {
    os << Well::InjectorCMode2String(cm);
    return os;
}

Well::WELTARGCMode Well::WELTARGCModeFromString(const std::string& string_value) {
    if (string_value == "ORAT")
        return WELTARGCMode::ORAT;

    if (string_value == "WRAT")
        return WELTARGCMode::WRAT;

    if (string_value == "GRAT")
        return WELTARGCMode::GRAT;

    if (string_value == "LRAT")
        return WELTARGCMode::LRAT;

    if (string_value == "CRAT")
        return WELTARGCMode::CRAT;

    if (string_value == "RESV")
        return WELTARGCMode::RESV;

    if (string_value == "BHP")
        return WELTARGCMode::BHP;

    if (string_value == "THP")
        return WELTARGCMode::THP;

    if (string_value == "VFP")
        return WELTARGCMode::VFP;

    if (string_value == "LIFT")
        return WELTARGCMode::LIFT;

    if (string_value == "GUID")
        return WELTARGCMode::GUID;

    throw std::invalid_argument("WELTARG control mode: " + string_value + " not recognized.");
}


std::ostream& operator<<(std::ostream& os, const Well::ProducerCMode& cm) {
    if (cm == Well::ProducerCMode::CMODE_UNDEFINED)
        os << "UNDEFINED";
    else
        os << Well::ProducerCMode2String(cm);
    return os;
}

const std::string Well::ProducerCMode2String( ProducerCMode enumValue ) {
    switch( enumValue ) {
    case ProducerCMode::ORAT:
        return "ORAT";
    case ProducerCMode::WRAT:
        return "WRAT";
    case ProducerCMode::GRAT:
        return "GRAT";
    case ProducerCMode::LRAT:
        return "LRAT";
    case ProducerCMode::CRAT:
        return "CRAT";
    case ProducerCMode::RESV:
        return "RESV";
    case ProducerCMode::BHP:
        return "BHP";
    case ProducerCMode::THP:
        return "THP";
    case ProducerCMode::GRUP:
        return "GRUP";
    default:
        throw std::invalid_argument("Unhandled enum value: " + std::to_string(static_cast<int>(enumValue)) + " in ProducerCMode2String");
    }
}

Well::ProducerCMode Well::ProducerCModeFromString( const std::string& stringValue ) {
    if (stringValue == "ORAT")
        return ProducerCMode::ORAT;
    else if (stringValue == "WRAT")
        return ProducerCMode::WRAT;
    else if (stringValue == "GRAT")
        return ProducerCMode::GRAT;
    else if (stringValue == "LRAT")
        return ProducerCMode::LRAT;
    else if (stringValue == "CRAT")
        return ProducerCMode::CRAT;
    else if (stringValue == "RESV")
        return ProducerCMode::RESV;
    else if (stringValue == "BHP")
        return ProducerCMode::BHP;
    else if (stringValue == "THP")
        return ProducerCMode::THP;
    else if (stringValue == "GRUP")
        return ProducerCMode::GRUP;
    else if (stringValue == "NONE")
        return ProducerCMode::NONE;
    else
        throw std::invalid_argument("Unknown enum state string: " + stringValue );
}


const std::string Well::GuideRateTarget2String( GuideRateTarget enumValue ) {
    switch( enumValue ) {
    case GuideRateTarget::OIL:
        return "OIL";
    case GuideRateTarget::WAT:
        return "WAT";
    case GuideRateTarget::GAS:
        return "GAS";
    case GuideRateTarget::LIQ:
        return "LIQ";
    case GuideRateTarget::COMB:
        return "COMB";
    case GuideRateTarget::WGA:
        return "WGA";
    case GuideRateTarget::CVAL:
        return "CVAL";
    case GuideRateTarget::RAT:
        return "RAT";
    case GuideRateTarget::RES:
        return "RES";
    case GuideRateTarget::UNDEFINED:
        return "UNDEFINED";
    default:
        throw std::invalid_argument("unhandled enum value");
    }
}

Well::GuideRateTarget Well::GuideRateTargetFromString( const std::string& stringValue ) {
    if (stringValue == "OIL")
        return GuideRateTarget::OIL;
    else if (stringValue == "WAT")
        return GuideRateTarget::WAT;
    else if (stringValue == "GAS")
        return GuideRateTarget::GAS;
    else if (stringValue == "LIQ")
        return GuideRateTarget::LIQ;
    else if (stringValue == "COMB")
        return GuideRateTarget::COMB;
    else if (stringValue == "WGA")
        return GuideRateTarget::WGA;
    else if (stringValue == "CVAL")
        return GuideRateTarget::CVAL;
    else if (stringValue == "RAT")
        return GuideRateTarget::RAT;
    else if (stringValue == "RES")
        return GuideRateTarget::RES;
    else if (stringValue == "UNDEFINED")
        return GuideRateTarget::UNDEFINED;
    else
        throw std::invalid_argument("Unknown enum state string: " + stringValue );
}


bool Well::cmp_structure(const Well& other) const {
    if ((this->segments && !other.segments) ||
        (!this->segments && other.segments))
    {
        return false;
    }

    if (this->segments && (this->getSegments() != other.getSegments())) {
        return false;
    }

    return (this->name() == other.name())
        && (this->groupName() == other.groupName())
        && (this->firstTimeStep() == other.firstTimeStep())
        && (this->seqIndex() == other.seqIndex())
        && (this->getHeadI() == other.getHeadI())
        && (this->getHeadJ() == other.getHeadJ())
        && (this->hasRefDepth() == other.hasRefDepth())
        && (!this->hasRefDepth() || (this->getRefDepth() == other.getRefDepth()))
        && (this->getPreferredPhase() == other.getPreferredPhase())
        && (this->unit_system == other.unit_system)
        && (this->udq_undefined == other.udq_undefined)
        && (this->getConnections() == other.getConnections())
        && (this->getDrainageRadius() == other.getDrainageRadius())
        && (this->getAllowCrossFlow() == other.getAllowCrossFlow())
        && (this->getAutomaticShutIn() == other.getAutomaticShutIn())
        && (this->getEfficiencyFactor() == other.getEfficiencyFactor())
        ;
}


bool Well::operator==(const Well& data) const {
    return this->cmp_structure(data)
        && (this->getSolventFraction() == data.getSolventFraction())
        && (this->getEconLimits() == data.getEconLimits())
        && (this->isProducer() == data.isProducer())
        && (this->getFoamProperties() == data.getFoamProperties())
        && (this->getStatus() == data.getStatus())
        && (this->guide_rate == data.guide_rate)
        && (this->solvent_fraction == data.solvent_fraction)
        && (this->hasProduced() == data.hasProduced())
        && (this->hasInjected() == data.hasInjected())
        && (this->predictionMode() == data.predictionMode())
        && (this->getTracerProperties() == data.getTracerProperties())
        && (this->getWVFPEXP() == data.getWVFPEXP())
        && (this->getProductionProperties() == data.getProductionProperties())
        && (this->m_pavg == data.m_pavg)
        && (this->getInjectionProperties() == data.getInjectionProperties())
        ;
}


PAvgCalculator Well::pavg_calculator(const EclipseGrid& grid, const std::vector<double>& porv) const {
    return PAvgCalculator(this->name(), this->getWPaveRefDepth(), grid, porv, this->getConnections(), this->m_pavg);
}


}

int Opm::Well::eclipseControlMode(const Well::InjectorCMode imode,
                             const InjectorType        itype)
{
    using IMode = ::Opm::Well::InjectorCMode;
    using Val   = ::Opm::RestartIO::Helpers::VectorItems::IWell::Value::WellCtrlMode;
    using IType = ::Opm::InjectorType;

    switch (imode) {
        case IMode::RATE: {
            switch (itype) {
            case IType::OIL:   return Val::OilRate;
            case IType::WATER: return Val::WatRate;
            case IType::GAS:   return Val::GasRate;
            case IType::MULTI: return Val::WMCtlUnk;
            }}
            break;

        case IMode::RESV: return Val::ResVRate;
        case IMode::THP:  return Val::THP;
        case IMode::BHP:  return Val::BHP;
        case IMode::GRUP: return Val::Group;
        default:
            return Val::WMCtlUnk;
    }
    return Val::WMCtlUnk;
}

int Opm::Well::eclipseControlMode(const Opm::Well::ProducerCMode pmode)
{
    using PMode = ::Opm::Well::ProducerCMode;
    using Val   = ::Opm::RestartIO::Helpers::VectorItems::IWell::Value::WellCtrlMode;

    switch (pmode) {
        case PMode::ORAT: return Val::OilRate;
        case PMode::WRAT: return Val::WatRate;
        case PMode::GRAT: return Val::GasRate;
        case PMode::LRAT: return Val::LiqRate;
        case PMode::RESV: return Val::ResVRate;
        case PMode::THP:  return Val::THP;
        case PMode::BHP:  return Val::BHP;
        case PMode::CRAT: return Val::CombRate;
        case PMode::GRUP: return Val::Group;
        default:
            return Val::WMCtlUnk;
    }
    return Val::WMCtlUnk;
}

/*
  The purpose of this function is to convert OPM well status to an integer value
  suitable for output in the eclipse restart file. In OPM we have different
  variables for the wells status and the active control, when this is written to
  a restart file they are combined to one integer. In OPM a well can have an
  active control while still being shut, when this is converted to an integer
  value suitable for the eclipse formatted restart file the value 0 will be used
  to signal a SHUT well and the active control will be lost.

  In the case of a well which is in state 'STOP' or 'AUTO' an integer
  corresponding to the currently active control is writte to the restart file.
*/

int Opm::Well::eclipseControlMode(const Well&         well,
                                  const SummaryState& st)
{
    if (well.isProducer()) {
        const auto& ctrl = well.productionControls(st);

        return eclipseControlMode(ctrl.cmode);
    }
    else { // Injector
        const auto& ctrl = well.injectionControls(st);

        return eclipseControlMode(ctrl.cmode, well.injectorType());
    }
}
