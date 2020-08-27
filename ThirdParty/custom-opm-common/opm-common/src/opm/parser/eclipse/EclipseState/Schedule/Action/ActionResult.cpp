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

#include <vector>
#include <algorithm>
#include <stdexcept>

#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionResult.hpp>

namespace Opm {
namespace Action {

Result::Result(bool result_arg) :
    result(result_arg)
{}

Result::Result(bool result_arg, const std::vector<std::string>& wells) :
    result(result_arg)
{
    this->matching_wells.reset( new WellSet(wells) );
}

Result::Result(bool result_arg, const WellSet& wells) :
    result(result_arg)
{
    this->matching_wells.reset( new WellSet(wells) );
}

Result::Result(const Result& src)
{
    this->result = src.result;
    if (src.matching_wells)
        this->matching_wells.reset( new WellSet(*src.matching_wells) );
}

Result::operator bool() const {
    return this->result;
}

std::vector<std::string> Result::wells() const {
    if (!this->result)
        throw std::logic_error("Programming error: trying to check wells in ActionResult which is false");

    if (this->matching_wells)
        return this->matching_wells->wells();
    else
        return {};
}

Result& Result::operator|=(const Result& other) {
    this->result = this->result || other.result;

    if (other.matching_wells) {
        if (this->matching_wells)
            this->matching_wells->add( *other.matching_wells );
        else
            this->matching_wells.reset( new WellSet(*other.matching_wells) );
    }
    return *this;
}

Result& Result::operator&=(const Result& other) {
    this->result = this->result && other.result;

    if (other.matching_wells) {
        if (this->matching_wells)
            this->matching_wells->intersect( *other.matching_wells );
        else
            this->matching_wells.reset( new WellSet(*other.matching_wells) );
    }
    return *this;
}

Result& Result::operator=(const Result& src)
{
    this->result = src.result;
    if (src.matching_wells) this->matching_wells.reset( new WellSet(*src.matching_wells) );

    return *this;
}

void Result::assign(bool value) {
    this->result = value;
}

void Result::add_well(const std::string& well) {
    if (!this->matching_wells)
        this->matching_wells.reset( new WellSet() );
    this->matching_wells->add(well);
}

bool Result::has_well(const std::string& well) const {
    if (!this->result)
        throw std::logic_error("Programming error: trying to check wells in ActionResult which is false");

    if (!this->matching_wells)
        return false;

    return this->matching_wells->contains(well);
}

/******************************************************************/

WellSet::WellSet(const std::vector<std::string>& wells)
{
    this->well_set.insert(wells.begin(), wells.end());
}


void WellSet::add(const std::string& well) {
    this->well_set.insert(well);
}


std::size_t WellSet::size() const {
    return this->well_set.size();
}

std::vector<std::string> WellSet::wells() const {
    return std::vector<std::string>( this->well_set.begin(), this->well_set.end() );
}


WellSet& WellSet::intersect(const WellSet& other) {
    auto well_iter = this->well_set.begin();
    while (well_iter != this->well_set.end()) {
        if (other.contains(*well_iter))
            well_iter++;
        else
            well_iter = this->well_set.erase(well_iter);
    }

    return *this;
}

WellSet& WellSet::add(const WellSet& other) {
    this->well_set.insert(other.well_set.begin(), other.well_set.end());
    return *this;
}


bool WellSet::contains(const std::string& well) const {
    return (this->well_set.find(well) != this->well_set.end());
}

}
}
