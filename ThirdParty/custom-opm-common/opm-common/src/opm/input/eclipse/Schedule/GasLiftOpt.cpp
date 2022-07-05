/*
  Copyright 2020 Equinor ASA.

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

#include <stdexcept>

#include <opm/input/eclipse/Schedule/GasLiftOpt.hpp>

namespace Opm {

bool GasLiftOpt::active() const {
    return (this->m_increment > 0);
}

void GasLiftOpt::gaslift_increment(double gaslift_increment) {
    this->m_increment = gaslift_increment;
}

double GasLiftOpt::gaslift_increment() const {
    return this->m_increment;
}

void GasLiftOpt::min_eco_gradient(double min_eco_gradient) {
    this->m_min_eco_gradient = min_eco_gradient;
}

double GasLiftOpt::min_eco_gradient() const {
    return this->m_min_eco_gradient;
}

void GasLiftOpt::min_wait(double min_wait) {
    this->m_min_wait = min_wait;
}

double GasLiftOpt::min_wait() const {
    return this->m_min_wait;
}

void GasLiftOpt::all_newton(double all_newton) {
    this->m_all_newton = all_newton;
}

bool GasLiftOpt::all_newton() const {
    return this->m_all_newton;
}

const GasLiftOpt::Group& GasLiftOpt::group(const std::string& gname) const {
    const auto iter = this->m_groups.find(gname);
    if (iter == this->m_groups.end())
        throw std::out_of_range("No such group: " + gname + " configured for gas lift optimization");

    return iter->second;
}

bool GasLiftOpt::has_well(const std::string& wname) const {
    const auto iter = this->m_wells.find(wname);
    return (iter != this->m_wells.end());
}

std::size_t GasLiftOpt::num_wells() const {
    return this->m_wells.size();
}

bool GasLiftOpt::has_group(const std::string& gname) const {
    const auto iter = this->m_groups.find(gname);
    return (iter != this->m_groups.end());
}


void GasLiftOpt::add_group(const Group& group) {
    this->m_groups.insert_or_assign(group.name(), group);
}


void GasLiftOpt::add_well(const Well& well) {
    this->m_wells.insert_or_assign(well.name(), well);
}


const GasLiftOpt::Well& GasLiftOpt::well(const std::string& wname) const {
    const auto iter = this->m_wells.find(wname);
    if (iter == this->m_wells.end())
        throw std::out_of_range("No such well: " + wname + " configured for gas lift optimization");

    return iter->second;
}

GasLiftOpt GasLiftOpt::serializeObject() {
    GasLiftOpt glo;
    glo.m_increment = 0;
    glo.m_min_eco_gradient = 100;
    glo.m_min_wait = 1;
    glo.m_all_newton = true;
    return glo;
}

bool GasLiftOpt::operator==(const GasLiftOpt& other) const {
    return this->m_increment == other.m_increment &&
           this->m_min_eco_gradient == other.m_min_eco_gradient &&
           this->m_min_wait == other.m_min_wait &&
           this->m_all_newton == other.m_all_newton &&
           this->m_groups == other.m_groups &&
           this->m_wells == other.m_wells;
}

}

