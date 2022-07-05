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

#include <opm/common/utility/Serializer.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQState.hpp>
#include <opm/io/eclipse/rst/state.hpp>

namespace Opm {

namespace {

bool is_udq(const std::string& key) {
    if (key.size() < 2)
        return false;

    if (key[1] != 'U')
        return false;

    return true;
}

bool has_var(const std::unordered_map<std::string, std::unordered_map<std::string, double>>& values,
             const std::string& wgname,
             const std::string udq_key) {
    auto res_iter = values.find(udq_key);
    if (res_iter == values.end())
        return false;

    return res_iter->second.count(wgname);
}

void add_results(std::unordered_map<std::string, std::unordered_map<std::string, double>>& values,
                 const std::string& udq_key,
                 const UDQSet& result) {

    auto& udq_values = values[udq_key];
    for (const auto& res1 : result) {
        auto iter = udq_values.find(res1.wgname());
        if (iter == udq_values.end()) {
            if (res1.defined())
                udq_values.emplace( res1.wgname(), res1.get() );
        } else {
            if (res1.defined())
                iter->second = res1.get();
            else
                udq_values.erase(iter);
        }
    }
}

double get_scalar(const std::unordered_map<std::string, double>& values,
                  const std::string& udq_key,
                  double undef_value) {
    auto iter = values.find(udq_key);
    if (iter == values.end())
        return undef_value;
    return iter->second;
}


double get_wg(const std::unordered_map<std::string, std::unordered_map<std::string, double>>& values,
              const std::string& wgname,
              const std::string& udq_key,
              double undef_value) {

    auto res_iter = values.find(udq_key);
    if (res_iter == values.end()) {
        if (is_udq(udq_key))
            throw std::out_of_range("No such UDQ variable: " + udq_key);
        else
            throw std::logic_error("No such UDQ variable: " + udq_key);
    }
    const auto& result_set = res_iter->second;
    return get_scalar(result_set, wgname, undef_value);
}

}

void UDQState::load_rst(const RestartIO::RstState& rst_state) {
    for (const auto& udq : rst_state.udqs) {
        if (udq.is_define()) {
            if (udq.var_type == UDQVarType::WELL_VAR) {
                for (const auto& [wname, value] : udq.values())
                    this->well_values[udq.name][wname] = value;
            }

            if (udq.var_type == UDQVarType::GROUP_VAR) {
                for (const auto& [gname, value] : udq.values())
                    this->group_values[udq.name][gname] = value;
            }

            const auto& field_value = udq.field_value();
            if (field_value.has_value())
                this->scalar_values[udq.name] = field_value.value();
        } else {
            auto value = udq.assign_value();
            if (udq.var_type == UDQVarType::WELL_VAR) {
                for (const auto& wname : udq.assign_selector())
                    this->well_values[udq.name][wname] = value;
            }

            if (udq.var_type == UDQVarType::GROUP_VAR) {
                for (const auto& gname : udq.assign_selector())
                    this->group_values[udq.name][gname] = value;
            }

            if (udq.var_type == UDQVarType::FIELD_VAR)
                this->scalar_values[udq.name] = value;
        }
    }
}

double UDQState::undefined_value() const {
    return this->undef_value;
}


UDQState::UDQState(double undefined) :
    undef_value(undefined)
{}

bool UDQState::has(const std::string& key)  const {
    return this->scalar_values.count(key);
}



bool UDQState::has_well_var(const std::string& well, const std::string& key) const {
    return has_var( this->well_values, well, key);
}

bool UDQState::has_group_var(const std::string& group, const std::string& key) const {
    return has_var( this->group_values, group, key);
}


void UDQState::add(const std::string& udq_key, const UDQSet& result) {
    if (!is_udq(udq_key))
        throw std::logic_error("Key is not a UDQ variable:" + udq_key);

    auto var_type = result.var_type();
    if (var_type == UDQVarType::WELL_VAR)
        add_results(this->well_values, udq_key, result);
    else if (var_type == UDQVarType::GROUP_VAR)
        add_results(this->group_values, udq_key, result);
    else {
        auto scalar = result[0];
        auto iter = this->scalar_values.find(udq_key);
        if (iter == this->scalar_values.end()) {
            if (scalar.defined())
                this->scalar_values.emplace(udq_key, scalar.get());
        } else {
            if (scalar.defined())
                iter->second = scalar.get();
            else
                this->scalar_values.erase(iter);
        }
    }
}


void UDQState::add_define(std::size_t report_step, const std::string& udq_key, const UDQSet& result) {
    this->defines[udq_key] = report_step;
    this->add(udq_key, result);
}

void UDQState::add_assign(std::size_t report_step, const std::string& udq_key, const UDQSet& result) {
    this->assignments[udq_key] = report_step;
    this->add(udq_key, result);
}

double UDQState::get(const std::string& key) const {
    if (!is_udq(key))
        throw std::logic_error("Key is not a UDQ variable:" + key);

    auto iter = this->scalar_values.find(key);
    if (iter == this->scalar_values.end())
        throw std::out_of_range("Invalid key: " + key);
    return iter->second;
}

double UDQState::get_well_var(const std::string& well, const std::string& key) const {
    return get_wg(this->well_values, well, key, this->undef_value);
}

double UDQState::get_group_var(const std::string& group, const std::string& key) const {
    return get_wg(this->group_values, group, key, this->undef_value);
}

bool UDQState::operator==(const UDQState& other) const {
    return this->undef_value == other.undef_value &&
           this->scalar_values == other.scalar_values &&
           this->group_values == other.group_values &&
           this->well_values == other.well_values &&
           this->assignments == other.assignments &&
           this->defines == other.defines;
}



bool UDQState::assign(std::size_t report_step, const std::string& udq_key) const {
    auto assign_iter = this->assignments.find(udq_key);
    if (assign_iter == this->assignments.end())
        return true;
    else
        return report_step > assign_iter->second;
}

bool UDQState::define(const std::string& udq_key, std::pair<UDQUpdate, std::size_t> update_status) const {
    if (update_status.first == UDQUpdate::ON)
        return true;

    if (update_status.first == UDQUpdate::OFF)
        return false;

    auto define_iter = this->defines.find(udq_key);
    if (define_iter == this->defines.end())
        return true;

    return define_iter->second < update_status.second;
}

std::vector<char> UDQState::serialize() const {
    Serializer ser;
    ser.put(this->undef_value);

    ser.put(this->well_values.size());
    for (const auto& [udq_key, values] : this->well_values) {
        ser.put(udq_key);
        ser.put_map(values);
    }

    ser.put(this->group_values.size());
    for (const auto& [udq_key, values] : this->group_values) {
        ser.put(udq_key);
        ser.put_map(values);
    }

    ser.put_map(this->scalar_values);
    ser.put_map(this->assignments);
    ser.put_map(this->defines);
    return ser.buffer;
}


void UDQState::deserialize(const std::vector<char>& buffer) {
    Serializer ser(buffer);
    this->undef_value = ser.get<double>();

    this->well_values.clear();
    std::size_t size = ser.get<std::size_t>();
    for (std::size_t index = 0; index < size; index++) {
        auto udq_key = ser.get<std::string>();
        this->well_values.insert( std::make_pair(udq_key, ser.get_map<std::string, double>()));;
    }

    this->group_values.clear();
    size = ser.get<std::size_t>();
    for (std::size_t index = 0; index < size; index++) {
        auto udq_key = ser.get<std::string>();
        this->group_values.insert( std::make_pair(udq_key, ser.get_map<std::string, double>()));;
    }

    this->scalar_values = ser.get_map<std::string, double>();
    this->assignments = ser.get_map<std::string, std::size_t>();
    this->defines = ser.get_map<std::string, std::size_t>();
}
}


