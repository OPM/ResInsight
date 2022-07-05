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


#include <opm/input/eclipse/Schedule/Group/GuideRateConfig.hpp>


namespace Opm {

GuideRateConfig GuideRateConfig::serializeObject()
{
    GuideRateConfig result;
    result.m_model = GuideRateModel::serializeObject();
    result.wells = {{"test1", WellTarget{1.0, Well::GuideRateTarget::COMB, 2.0}}};
    result.production_groups = {{"test2", GroupProdTarget{1.0, Group::GuideRateProdTarget::COMB}}};
    result.injection_groups = {{{Phase::OIL, "test3"}, GroupInjTarget{1.0, Group::GuideRateInjTarget::NETV}}};
    return result;
}


const GuideRateModel& GuideRateConfig::model() const {
    if (this->m_model.has_value())
        return this->m_model.value();
    else
        throw std::logic_error("Tried to dereference empty GuideRateModel");
}


bool GuideRateConfig::has_model() const {
    return this->m_model.has_value();
}

bool GuideRateConfig::update_model(const GuideRateModel& new_model) {
    if (!this->m_model.has_value() || this->m_model.value() != new_model) {
        this->m_model = new_model;
        return true;
    }

    return false;
}

void GuideRateConfig::update_well(const Well& well) {
    if (well.isAvailableForGroupControl()) {
        auto& well_node = this->wells[well.name()];
        well_node.guide_rate = well.getGuideRate();
        well_node.target= well.getGuideRatePhase();
        well_node.scaling_factor = well.getGuideRateScalingFactor();
    } else
        this->wells.erase(well.name());
}

const GuideRateConfig::WellTarget& GuideRateConfig::well(const std::string& well) const {
    return this->wells.at(well);
}

void GuideRateConfig::update_production_group(const Group& group) {
    if (group.name() == "FIELD")
        return;

    const auto& properties = group.productionProperties();
    auto guide_target = properties.guide_rate_def;
    if (guide_target == Group::GuideRateProdTarget::NO_GUIDE_RATE) {
        this->production_groups.erase(group.name());
        return;
    }

    auto& group_node = this->production_groups[group.name()];
    group_node.guide_rate = properties.guide_rate;
    group_node.target = guide_target;
}

void GuideRateConfig::update_injection_group(const std::string& group_name, const Group::GroupInjectionProperties& properties) {
    if (group_name == "FIELD")
        return;

    auto guide_target = properties.guide_rate_def;
    if (guide_target == Group::GuideRateInjTarget::NO_GUIDE_RATE) {
        this->injection_groups.erase(std::make_pair(properties.phase, group_name));
        return;
    }

    auto& group_node = this->injection_groups[std::make_pair(properties.phase, group_name)];
    group_node.guide_rate = properties.guide_rate;
    group_node.target = guide_target;
}

const GuideRateConfig::GroupProdTarget& GuideRateConfig::production_group(const std::string& group) const {
    return this->production_groups.at(group);
}

const GuideRateConfig::GroupInjTarget& GuideRateConfig::injection_group(const Phase& phase, const std::string& group) const {
    return this->injection_groups.at(std::make_pair(phase, group));
}

bool GuideRateConfig::has_well(const std::string& well) const {
    return (this->wells.count(well) > 0);
}

bool GuideRateConfig::has_injection_group(const Phase& phase, const std::string& name) const {
    return (this->injection_groups.count(std::make_pair(phase, name)) > 0);
}

bool GuideRateConfig::has_production_group(const std::string& name) const {
    return (this->production_groups.count(name) > 0);
}

bool GuideRateConfig::operator==(const GuideRateConfig& data) const {
    return this->wells == data.wells &&
           this->m_model == data.m_model &&
           this->production_groups == data.production_groups &&
           this->injection_groups == data.injection_groups;
}

}


