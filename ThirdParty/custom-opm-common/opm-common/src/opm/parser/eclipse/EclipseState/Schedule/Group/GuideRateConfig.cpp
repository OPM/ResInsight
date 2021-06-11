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


#include <opm/parser/eclipse/EclipseState/Schedule/Group/GuideRateConfig.hpp>


namespace Opm {

GuideRateConfig GuideRateConfig::serializeObject()
{
    GuideRateConfig result;
    result.m_model = std::make_shared<GuideRateModel>(GuideRateModel::serializeObject());
    result.wells = {{"test1", WellTarget{1.0, Well::GuideRateTarget::COMB, 2.0}}};
    result.groups = {{"test2", GroupTarget{1.0, Group::GuideRateTarget::COMB}}};

    return result;
}


const GuideRateModel& GuideRateConfig::model() const {
    if (this->m_model)
        return *this->m_model;
    else
        throw std::logic_error("Tried to dereference empty GuideRateModel");
}


bool GuideRateConfig::has_model() const {
    if (this->m_model)
        return true;
    else
        return false;
}

bool GuideRateConfig::update_model(const GuideRateModel& new_model) {
    if (!this->m_model || *(this->m_model) != new_model) {
        this->m_model.reset( new GuideRateModel(new_model) );
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

void GuideRateConfig::update_group(const Group& group) {
    if (group.name() == "FIELD")
        return;

    const auto& properties = group.productionProperties();
    auto guide_target = properties.guide_rate_def;
    if (guide_target == Group::GuideRateTarget::NO_GUIDE_RATE) {
        this->groups.erase(group.name());
        return;
    }

    auto& group_node = this->groups[group.name()];
    group_node.guide_rate = properties.guide_rate;
    group_node.target = guide_target;
}

const GuideRateConfig::GroupTarget& GuideRateConfig::group(const std::string& group) const {
    return this->groups.at(group);
}

bool GuideRateConfig::has_well(const std::string& well) const {
    return (this->wells.count(well) > 0);
}

bool GuideRateConfig::has_group(const std::string& group) const {
    return (this->groups.count(group) > 0);
}

bool GuideRateConfig::operator==(const GuideRateConfig& data) const {
    if ((this->m_model && !data.m_model) || (!this->m_model && data.m_model))
        return false;

    if (this->m_model && !(*this->m_model == *data.m_model))
        return false;

    return this->wells == data.wells &&
           this->groups == data.groups;
}

}


