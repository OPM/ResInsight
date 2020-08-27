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

#include <opm/parser/eclipse/EclipseState/Schedule/Schedule.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Group/GuideRate.hpp>

namespace Opm {

double GuideRate::RateVector::eval(Well::GuideRateTarget target) const {
    if (target == Well::GuideRateTarget::OIL)
        return this->oil_rat;

    if (target == Well::GuideRateTarget::GAS)
        return this->gas_rat;

    if (target == Well::GuideRateTarget::LIQ)
        return this->oil_rat + this->wat_rat;

    if (target == Well::GuideRateTarget::WAT)
        return this->wat_rat;

    throw std::logic_error("Don't know how to convert .... ");
}

double GuideRate::RateVector::eval(Group::GuideRateTarget target) const {
    if (target == Group::GuideRateTarget::OIL)
        return this->oil_rat;

    if (target == Group::GuideRateTarget::GAS)
        return this->gas_rat;

    if (target == Group::GuideRateTarget::LIQ)
        return this->oil_rat + this->wat_rat;

    if (target == Group::GuideRateTarget::WAT)
        return this->wat_rat;

    throw std::logic_error("Don't know how to convert .... ");
}

double GuideRate::RateVector::eval(GuideRateModel::Target target) const {
    if (target == GuideRateModel::Target::OIL)
        return this->oil_rat;

    if (target == GuideRateModel::Target::GAS)
        return this->gas_rat;

    if (target == GuideRateModel::Target::LIQ)
        return this->oil_rat + this->wat_rat;

    if (target == GuideRateModel::Target::WAT)
        return this->wat_rat;

    throw std::logic_error("Don't know how to convert .... ");
}


GuideRate::GuideRate(const Schedule& schedule_arg) :
    schedule(schedule_arg)
{}



double GuideRate::get(const std::string& well, Well::GuideRateTarget target, const RateVector& rates) const {
    auto model_target = GuideRateModel::convert_target(target);
    return get(well, model_target, rates);
}

double GuideRate::get(const std::string& group, Group::GuideRateTarget target, const RateVector& rates) const {
    auto model_target = GuideRateModel::convert_target(target);
    return get(group, model_target, rates);
}

double GuideRate::get(const std::string& name, GuideRateModel::Target model_target, const RateVector& rates) const {
    const auto iter = this->values.find(name);
    if (iter != this->values.end()) {
        const auto& value = iter->second;
        if (value.target == model_target)
            return value.value;
        else {
            double model_target_rate = rates.eval(model_target);
            double value_target_rate = rates.eval(value.target);        
                    
            if (model_target_rate < 1e-6)
                return value.value;
            if (value_target_rate < 1e-6)
                return value.value; 

            // scale with the current production ratio when the control target differs from the guide rate target.
            return value.value * model_target_rate / value_target_rate;
        }
    } else {
        const auto& pot = this->potentials.at(name);
        return pot.eval(model_target);
    }
}

bool GuideRate::has(const std::string& name) const
{
    return this->values.count(name) > 0;
}



void GuideRate::compute(const std::string& wgname, size_t report_step, double sim_time, double oil_pot, double gas_pot, double wat_pot) {
    const auto& config = this->schedule.guideRateConfig(report_step);
    this->potentials[wgname] = RateVector{oil_pot, gas_pot, wat_pot};

    if (config.has_group(wgname))
        this->group_compute(wgname, report_step, sim_time, oil_pot, gas_pot, wat_pot);
    else
        this->well_compute(wgname, report_step, sim_time, oil_pot, gas_pot, wat_pot);

}


void GuideRate::group_compute(const std::string& wgname, size_t report_step, double sim_time, double oil_pot, double gas_pot, double wat_pot) {
    const auto& config = this->schedule.guideRateConfig(report_step);
    const auto& group = config.group(wgname);

    if (group.guide_rate > 0) {
        auto model_target = GuideRateModel::convert_target(group.target);
        this->values[wgname] = GuideRateValue( sim_time, group.guide_rate, model_target );
    } else {
        auto iter = this->values.find(wgname);

        // If the FORM mode is used we check if the last computation is recent enough;
        // then we just return.
        if (iter != this->values.end()) {
            const auto& grv = iter->second;
            if (group.target == Group::GuideRateTarget::FORM) {
                if (!config.has_model())
                    throw std::logic_error("When specifying GUIDERATE target FORM you must enter a guiderate model with the GUIDERAT keyword");

                auto time_diff = sim_time - grv.sim_time;
                if (config.model().update_delay() > time_diff)
                    return;
            }
        }


        if (group.target == Group::GuideRateTarget::INJV)
            throw std::logic_error("Group guide rate mode: INJV not implemented");

        if (group.target == Group::GuideRateTarget::POTN)
            throw std::logic_error("Group guide rate mode: POTN not implemented");

        if (group.target == Group::GuideRateTarget::FORM) {
            double guide_rate;
            if (!config.has_model())
                throw std::logic_error("When specifying GUIDERATE target FORM you must enter a guiderate model with the GUIDERAT keyword");

            if (iter != this->values.end())
                guide_rate = this->eval_form(config.model(),  oil_pot,  gas_pot,  wat_pot, std::addressof(iter->second));
            else
                guide_rate = this->eval_form(config.model(),  oil_pot,  gas_pot,  wat_pot, nullptr);

            this->values[wgname] = GuideRateValue{sim_time, guide_rate, config.model().target()};
        }
    }
}


void GuideRate::well_compute(const std::string& wgname, size_t report_step, double sim_time, double oil_pot, double gas_pot, double wat_pot) {
    const auto& config = this->schedule.guideRateConfig(report_step);

    // guide rates spesified with WGRUPCON
    if (config.has_well(wgname)) {
        const auto& well = config.well(wgname);
        if (well.guide_rate > 0) {
            auto model_target = GuideRateModel::convert_target(well.target);
            this->values[wgname] = GuideRateValue( sim_time, well.guide_rate, model_target );
        }
    } else if (config.has_model()) { // GUIDERAT
        // only look for wells not groups
        if (!this->schedule.hasWell(wgname, report_step))
            return;

        const auto& well = this->schedule.getWell(wgname, report_step);

        // GUIDERAT does not apply to injectors
        if (well.isInjector())
            return;

        auto iter = this->values.find(wgname);

        if (iter != this->values.end()) {
            const auto& grv = iter->second;
            auto time_diff = sim_time - grv.sim_time;
            if (config.model().update_delay() > time_diff)
                return;
        }

        double guide_rate;
        if (iter == this->values.end())
            guide_rate = this->eval_form(config.model(),  oil_pot,  gas_pot,  wat_pot, nullptr);
        else
            guide_rate = this->eval_form(config.model(),  oil_pot,  gas_pot,  wat_pot, std::addressof(iter->second));

        this->values[wgname] = GuideRateValue{sim_time, guide_rate, config.model().target()};
    }
    // If neither WGRUPCON nor GUIDERAT is spesified potentials are used
}

double GuideRate::eval_form(const GuideRateModel& model, double oil_pot, double gas_pot, double wat_pot, const GuideRateValue * prev) const {
    double new_guide_rate = model.eval(oil_pot, gas_pot, wat_pot);
    if (!prev)
        return new_guide_rate;

    if (new_guide_rate > prev->value && !model.allow_increase())
        new_guide_rate = prev->value;

    auto damping_factor = model.damping_factor();

    return damping_factor * new_guide_rate + (1 - damping_factor) * prev->value;
}

double GuideRate::eval_group_pot() const {
    return 0;
}

double GuideRate::eval_group_resvinj() const {
    return 0;
}

}
