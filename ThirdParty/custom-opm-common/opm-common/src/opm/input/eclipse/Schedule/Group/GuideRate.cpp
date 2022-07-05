/*
  Copyright 2019, 2020 Equinor ASA.

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

#include <opm/input/eclipse/Schedule/Group/GuideRate.hpp>

#include <opm/input/eclipse/EclipseState/Runspec.hpp>
#include <opm/input/eclipse/Schedule/Schedule.hpp>

#include <opm/input/eclipse/Units/Units.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include <fmt/core.h>

double Opm::GuideRate::RateVector::eval(const Well::GuideRateTarget target) const
{
    return this->eval(GuideRateModel::convert_target(target));
}

double Opm::GuideRate::RateVector::eval(const Group::GuideRateProdTarget target) const
{
    return this->eval(GuideRateModel::convert_target(target));
}

double Opm::GuideRate::RateVector::eval(const GuideRateModel::Target target) const
{
    if (target == GuideRateModel::Target::OIL)
        return this->oil_rat;

    if (target == GuideRateModel::Target::GAS)
        return this->gas_rat;

    if (target == GuideRateModel::Target::LIQ)
        return this->oil_rat + this->wat_rat;

    if (target == GuideRateModel::Target::WAT)
        return this->wat_rat;

    if (target == GuideRateModel::Target::RES)
        return this->oil_rat + this->wat_rat + this->gas_rat;

    throw std::logic_error {
        "Don't know how to convert target type " + std::to_string(static_cast<int>(target))
    };
}

Opm::GuideRate::GuideRate(const Schedule& schedule_arg)
    : schedule(schedule_arg)
{}

double Opm::GuideRate::get(const std::string&          well,
                           const Well::GuideRateTarget target,
                           const RateVector&           rates) const
{
    return this->get(well, GuideRateModel::convert_target(target), rates);
}

double Opm::GuideRate::get(const std::string&               group,
                           const Group::GuideRateProdTarget target,
                           const RateVector&                rates) const
{
    return this->get(group, GuideRateModel::convert_target(target), rates);
}

double Opm::GuideRate::get(const std::string&           name,
                           const GuideRateModel::Target model_target,
                           const RateVector&            rates) const
{
    using namespace unit;
    using prefix::micro;

    auto iter = this->values.find(name);
    if (iter == this->values.end()) {
        return this->potentials.at(name).eval(model_target);
    }

    const auto& value = *iter->second;
    const auto grvalue = this->get_grvalue_result(value);
    if (value.curr.target == model_target) {
        return grvalue;
    }

    const auto value_target_rate = rates.eval(value.curr.target);
    if (value_target_rate < 1.0*micro*liter/day) {
        return grvalue;
    }

    // Scale with the current production ratio when the control target
    // differs from the guide rate target.
    const auto scale = rates.eval(model_target) / value_target_rate;
    return grvalue * scale;
}

double Opm::GuideRate::get(const std::string& name, const Phase& phase) const
{
    auto iter = this->injection_group_values.find(std::make_pair(phase, name));
    if (iter == this->injection_group_values.end()) {
        auto message = fmt::format("Did not find any guiderate values for injection group {}:{}", name, std::to_string(static_cast<int>(phase)));
        throw std::logic_error {message};
    }
    return iter->second;
}

double Opm::GuideRate::getSI(const std::string&          well,
                             const Well::GuideRateTarget target,
                             const RateVector&           rates) const
{
    return this->getSI(well, GuideRateModel::convert_target(target), rates);
}

double Opm::GuideRate::getSI(const std::string&               group,
                             const Group::GuideRateProdTarget target,
                             const RateVector&                rates) const
{
    return this->getSI(group, GuideRateModel::convert_target(target), rates);
}

double Opm::GuideRate::getSI(const std::string&           wgname,
                             const GuideRateModel::Target target,
                             const RateVector&            rates) const
{
    using M = UnitSystem::measure;

    const auto gr = this->get(wgname, target, rates);

    switch (target) {
    case GuideRateModel::Target::OIL:
    case GuideRateModel::Target::WAT:
    case GuideRateModel::Target::LIQ:
        return this->schedule.getUnits().to_si(M::liquid_surface_rate, gr);

    case GuideRateModel::Target::GAS:
        return this->schedule.getUnits().to_si(M::gas_surface_rate, gr);

    case GuideRateModel::Target::RES:
        return this->schedule.getUnits().to_si(M::rate, gr);

    case GuideRateModel::Target::NONE:
    case GuideRateModel::Target::COMB:
        return gr;
    }

    throw std::invalid_argument {
        fmt::format("Unsupported Guiderate Target '{}'",
                    static_cast<std::underlying_type_t<GuideRateModel::Target>>(target))
    };
}

double Opm::GuideRate::getSI(const std::string& group, const Phase& phase) const
{
    using M = UnitSystem::measure;

    const auto gr = this->get(group, phase);

    switch (phase) {
    case Phase::OIL:
    case Phase::WATER:
        return this->schedule.getUnits().to_si(M::liquid_surface_rate, gr);

    case Phase::GAS:
        return this->schedule.getUnits().to_si(M::gas_surface_rate, gr);

    default:
        break;
    }

    throw std::invalid_argument {
        fmt::format("Unsupported Injection Guiderate Phase '{}'", static_cast<int>(phase))
    };
}

bool Opm::GuideRate::has(const std::string& name) const
{
    return this->values.count(name) > 0;
}

bool Opm::GuideRate::hasPotentials(const std::string& name) const
{
    return this->potentials.find(name) != this->potentials.end();
}

bool Opm::GuideRate::has(const std::string& name, const Phase& phase) const
{
    return this->injection_group_values.count(std::pair(phase, name)) > 0;
}

void Opm::GuideRate::compute(const std::string& wgname,
                             const std::size_t  report_step,
                             const double       sim_time,
                             const double       oil_pot,
                             const double       gas_pot,
                             const double       wat_pot)
{
    this->potentials[wgname] = RateVector{oil_pot, gas_pot, wat_pot};

    const auto& config = this->schedule[report_step].guide_rate();
    if (config.has_production_group(wgname)) {
        this->group_compute(wgname, report_step, sim_time, oil_pot, gas_pot, wat_pot);
    }
    else {
        this->well_compute(wgname, report_step, sim_time, oil_pot, gas_pot, wat_pot);
    }
}

void Opm::GuideRate::group_compute(const std::string& wgname,
                                   const std::size_t  report_step,
                                   const double       sim_time,
                                   const double       oil_pot,
                                   const double       gas_pot,
                                   const double       wat_pot)
{
    const auto& config = this->schedule[report_step].guide_rate();
    const auto& group = config.production_group(wgname);
    if (group.guide_rate > 0.0) {
        auto model_target = GuideRateModel::convert_target(group.target);

        const auto& model = config.has_model() ? config.model() : GuideRateModel{};
        this->assign_grvalue(wgname, model, { sim_time, group.guide_rate, model_target });
    }
    else {
        const auto is_formula = group.target == Group::GuideRateProdTarget::FORM;

        if (is_formula && !config.has_model()) {
            throw std::logic_error {
                "When specifying GUIDERATE target FORM you must "
                "enter a guiderate model with the GUIDERAT keyword"
            };
        }

        auto iter = this->values.find(wgname);

        // Use existing GR value if sufficently recent.
        if ((iter != this->values.end()) && is_formula &&
            !this->guide_rates_expired &&
            (iter->second->curr.value > 0.0))
        {
            return;
        }

        if (group.target == Group::GuideRateProdTarget::INJV) {
            throw std::logic_error("Group guide rate mode: INJV not implemented");
        }

        if (group.target == Group::GuideRateProdTarget::POTN) {
            throw std::logic_error("Group guide rate mode: POTN not implemented");
        }

        if (is_formula) {
            const auto guide_rate = this->eval_form(config.model(), oil_pot, gas_pot, wat_pot);
            this->assign_grvalue(wgname, config.model(), { sim_time, guide_rate, config.model().target() });
        }
    }
}

void Opm::GuideRate::compute(const std::string& wgname,
                             const Phase&       phase,
                             const std::size_t  report_step,
                             const double       guide_rate)
{
    const auto& config = this->schedule[report_step].guide_rate();
    if (!config.has_injection_group(phase, wgname))
        return;

    if (guide_rate > 0) {
        this->injection_group_values[std::make_pair(phase, wgname)] = guide_rate;
        return;
    }

    const auto& group = config.injection_group(phase, wgname);
    if (group.target == Group::GuideRateInjTarget::POTN) {
        return;
    }
    this->injection_group_values[std::make_pair(phase, wgname)] = group.guide_rate;
}

void Opm::GuideRate::well_compute(const std::string& wgname,
                                  const std::size_t  report_step,
                                  const double       sim_time,
                                  const double       oil_pot,
                                  const double       gas_pot,
                                  const double       wat_pot)
{
    const auto& config = this->schedule[report_step].guide_rate();

    if (config.has_well(wgname)) { // WGRUPCON
        const auto& well = config.well(wgname);
        if (well.guide_rate > 0.0) {
            auto model_target = GuideRateModel::convert_target(well.target);

            const auto& model = config.has_model() ? config.model() : GuideRateModel{};
            this->assign_grvalue(wgname, model, { sim_time, well.guide_rate, model_target });
        }
    }
    else if (config.has_model()) { // GUIDERAT
        if (! this->schedule.hasWell(wgname, report_step)) {
            // 'wgname' might be a group or the well is not yet online.
            return;
        }

        // GUIDERAT does not apply to injectors
        if (this->schedule.getWell(wgname, report_step).isInjector()) {
            return;
        }

        // Use existing guide rate value if sufficiently recent.
        {
            auto existing = this->values.find(wgname);
            if ((existing != this->values.end()) &&
                !this->guide_rates_expired &&
                (existing->second->curr.value > 0.0))
            {
                return;
            }
        }

        const auto& model = config.model();
        const auto guide_rate = this->eval_form(model, oil_pot, gas_pot, wat_pot);
        this->assign_grvalue(wgname, model, { sim_time, guide_rate, model.target() });
    }
}

double Opm::GuideRate::eval_form(const GuideRateModel& model,
                                 const double          oil_pot,
                                 const double          gas_pot,
                                 const double          wat_pot) const
{
    return model.eval(oil_pot, gas_pot, wat_pot);
}

double Opm::GuideRate::eval_group_pot() const
{
    return 0.0;
}

double Opm::GuideRate::eval_group_resvinj() const
{
    return 0.0;
}

void Opm::GuideRate::assign_grvalue(const std::string&    wgname,
                                    const GuideRateModel& model,
                                    GuideRateValue&&      value)
{
    auto& v = this->values[wgname];
    if (v == nullptr) {
        v = std::make_unique<GRValState>();
    }

    if (value.sim_time > v->curr.sim_time) {
        // We've advanced in time since we previously calculated/stored this
        // guiderate value.  Push current value into the past and prepare to
        // capture new value.
        std::swap(v->prev, v->curr);
    }

    v->curr = std::move(value);

    if ((v->prev.sim_time < 0.0) || ! (v->prev.value > 0.0)) {
        // No previous non-zero guiderate exists.  No further actions.
        return;
    }

    // Incorporate damping &c.
    const auto new_guide_rate = model.allow_increase()
        ? v->curr.value : std::min(v->curr.value, v->prev.value);

    const auto damping_factor = model.damping_factor();
    v->curr.value = damping_factor*new_guide_rate + (1 - damping_factor)*v->prev.value;
}

void Opm::GuideRate::init_grvalue(const std::size_t  report_step,
                                  const std::string& wgname,
                                  GuideRateValue     value)
{
    const auto& model = this->schedule[report_step].guide_rate().model();
    this->assign_grvalue(wgname, model, std::move(value));
}

void Opm::GuideRate::init_grvalue_SI(const std::size_t  report_step,
                                     const std::string& wgname,
                                     GuideRateValue     value)
{
    if ((value.sim_time < 0.0) ||
        (value.target == GuideRateModel::Target::NONE) ||
        !(value.value > 0.0))
    {
        return;
    }

    using M = UnitSystem::measure;

    switch (value.target) {
    case GuideRateModel::Target::OIL:
    case GuideRateModel::Target::WAT:
    case GuideRateModel::Target::LIQ:
        value.value = this->schedule.getUnits().from_si(M::liquid_surface_rate, value.value);
        break;

    case GuideRateModel::Target::GAS:
        value.value = this->schedule.getUnits().from_si(M::gas_surface_rate, value.value);
        break;

    case GuideRateModel::Target::RES:
        value.value = this->schedule.getUnits().from_si(M::rate, value.value);
        break;

    default:
        break;
    }

    this->init_grvalue(report_step, wgname, std::move(value));
}

double Opm::GuideRate::get_grvalue_result(const GRValState& gr) const
{
    return (gr.curr.sim_time < 0.0)
        ? 0.0
        : std::max(gr.curr.value, 0.0);
}

void Opm::GuideRate::updateGuideRateExpiration(const double      sim_time,
                                               const std::size_t report_step)
{
    const auto& config = this->schedule[report_step].guide_rate();

    if (! config.has_model()) {
        this->guide_rates_expired = false;
        return;
    }

    if (this->values.empty()) {
        this->guide_rates_expired = true;
        return;
    }

    auto curr_sim_time = [](const auto& grMapElem)
    {
        return grMapElem.second->curr.sim_time;
    };

    // Get previous general update time--earliest 'curr.sim_time' in
    // existing collection.
    auto last_update = std::min_element(this->values.begin(), this->values.end(),
        [&curr_sim_time](const auto& gr1, const auto& gr2)
    {
        return curr_sim_time(gr1) < curr_sim_time(gr2);
    });

    const auto update_delay = config.model().update_delay();
    this->guide_rates_expired =
        ! (sim_time < curr_sim_time(*last_update) + update_delay);
}
