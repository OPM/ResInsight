/*
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

#include <unordered_map>
#include <cstring>
#include <ctime>
#include <iostream>
#include <iomanip>

#include <opm/common/utility/Serializer.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQSet.hpp>
#include <opm/input/eclipse/Schedule/SummaryState.hpp>

namespace Opm{
namespace {

    bool is_total(const std::string& key) {
        static const std::vector<std::string> totals = {
            "OPT"  , "GPT"  , "WPT" , "GIT", "WIT", "OPTF" , "OPTS" , "OIT"  , "OVPT" , "OVIT" , "MWT" ,
            "WVPT" , "WVIT" , "GMT"  , "GPTF" , "SGT"  , "GST" , "FGT" , "GCT" , "GIMT" ,
            "WGPT" , "WGIT" , "EGT"  , "EXGT" , "GVPT" , "GVIT" , "LPT" , "VPT" , "VIT" , "NPT" , "NIT",
            "TPT", "TIT", "CPT", "CIT", "SPT", "SIT", "EPT", "EIT", "TPTHEA", "TITHEA",
            "OFT", "OFT+", "OFT-", "OFTG", "OFTL",
            "GFT", "GFT+", "GFT-", "GFTG", "GFTL",
            "WFT", "WFT+", "WFT-",
        };

        auto sep_pos = key.find(':');

        // Starting with ':' - that is probably broken?!
        if (sep_pos == 0)
            return false;

        if (sep_pos == std::string::npos) {
            for (const auto& total : totals) {
                if (key.compare(1, total.size(), total) == 0)
                    return true;
            }

            return false;
        } else
            return is_total(key.substr(0,sep_pos));
    }

    template <class T>
    using map2 = std::unordered_map<std::string, std::unordered_map<std::string, T>>;

    template <class T>
    bool has_var(const map2<T>& values, const std::string& var1, const std::string& var2) {
        const auto& var1_iter = values.find(var1);
        if (var1_iter == values.end())
            return false;

        const auto& var2_iter = var1_iter->second.find(var2);
        if (var2_iter == var1_iter->second.end())
            return false;

        return true;
    }

    template <class T>
    void erase_var(map2<T>& values, std::set<std::string>& var2_set, const std::string& var1, const std::string& var2) {
        const auto& var1_iter = values.find(var1);
        if (var1_iter == values.end())
            return;

        var1_iter->second.erase(var2);
        var2_set.clear();
        for (const auto& [_, var2_map] : values) {
            (void)_;
            for (const auto& [v2, __] : var2_map) {
                (void)__;
                var2_set.insert(v2);
            }
        }
    }

    template <class T>
    std::vector<std::string> var2_list(const map2<T>& values, const std::string& var1) {
        const auto& var1_iter = values.find(var1);
        if (var1_iter == values.end())
            return {};

        std::vector<std::string> l;
        for (const auto& pair : var1_iter->second)
            l.push_back(pair.first);
        return l;
    }
}


    SummaryState::SummaryState(time_point sim_start_arg):
        sim_start(sim_start_arg)
    {
        this->update_elapsed(0);
    }

    SummaryState::SummaryState(std::time_t sim_start_arg):
        SummaryState(TimeService::from_time_t(sim_start_arg))
    {}


    void SummaryState::update_elapsed(double delta) {
        this->elapsed += delta;
    }


    double SummaryState::get_elapsed() const {
        return this->elapsed;
    }


    void SummaryState::update(const std::string& key, double value) {
        if (is_total(key))
            this->values[key] += value;
        else
            this->values[key] = value;
    }


    void SummaryState::update_group_var(const std::string& group, const std::string& var, double value) {
        std::string key = var + ":" + group;
        if (is_total(var)) {
            this->values[key] += value;
            this->group_values[var][group] += value;
        } else {
            this->values[key] = value;
            this->group_values[var][group] = value;
        }
        if (this->m_groups.count(group) == 0) {
            this->m_groups.insert(group);
            this->group_names.reset();
        }
    }

    void SummaryState::update_well_var(const std::string& well, const std::string& var, double value) {
        std::string key = var + ":" + well;
        if (is_total(var)) {
            this->values[key] += value;
            this->well_values[var][well] += value;
        } else {
            this->values[key] = value;
            this->well_values[var][well] = value;
        }
        if (this->m_wells.count(well) == 0) {
            this->m_wells.insert(well);
            this->well_names.reset();
        }
    }

    bool SummaryState::has_conn_var(const std::string& well, const std::string& var, std::size_t global_index) const {
        if (!has_var(this->conn_values, var, well))
            return false;

        const auto& index_map = this->conn_values.at(var).at(well);
        return (index_map.count(global_index) > 0);
    }

    void SummaryState::update_conn_var(const std::string& well, const std::string& var, std::size_t global_index, double value) {
        std::string key = var + ":" + well + ":" + std::to_string(global_index);
        if (is_total(var)) {
            this->values[key] += value;
            this->conn_values[var][well][global_index] += value;
        } else {
            this->values[key] = value;
            this->conn_values[var][well][global_index] = value;
        }
    }

    double SummaryState::get_conn_var(const std::string& well, const std::string& var, std::size_t global_index) const {
        return this->conn_values.at(var).at(well).at(global_index);
    }

    double SummaryState::get_conn_var(const std::string& well, const std::string& var, std::size_t global_index, double default_value) const {
        if (this->has_conn_var(well, var, global_index))
            return this->get_conn_var(well, var, global_index);
        return default_value;
    }


    void SummaryState::update_udq(const UDQSet& udq_set, double undefined_value) {
        auto var_type = udq_set.var_type();
        if (var_type == UDQVarType::WELL_VAR) {
            const std::vector<std::string> wells = this->wells();
            for (const auto& well : wells) {
                const auto& udq_value = udq_set[well].value();
                this->update_well_var(well, udq_set.name(), udq_value.value_or(undefined_value));
            }
        } else if (var_type == UDQVarType::GROUP_VAR) {
            const std::vector<std::string> groups = this->groups();
            for (const auto& group : groups) {
                const auto& udq_value = udq_set[group].value();
                this->update_group_var(group, udq_set.name(), udq_value.value_or(undefined_value));
            }
        } else {
            const auto& udq_var = udq_set[0].value();
            this->update(udq_set.name(), udq_var.value_or(undefined_value));
        }
    }


    void SummaryState::set(const std::string& key, double value) {
        this->values[key] = value;
    }

    bool SummaryState::erase(const std::string& key) {
        return (this->values.erase(key) > 0);
    }

    bool SummaryState::erase_well_var(const std::string& well, const std::string& var) {
        std::string key = var + ":" + well;
        if (!this->erase(key))
            return false;

        erase_var(this->well_values, this->m_wells, var, well);
        this->well_names.reset();
        return true;
    }

    bool SummaryState::erase_group_var(const std::string& group, const std::string& var) {
        std::string key = var + ":" + group;
        if (!this->erase(key))
            return false;

        erase_var(this->group_values, this->m_groups, var, group);
        this->group_names.reset();
        return true;
    }

    bool SummaryState::has(const std::string& key) const {
        return (this->values.find(key) != this->values.end());
    }

    double SummaryState::get(const std::string& key, double default_value) const {
        const auto iter = this->values.find(key);
        if (iter == this->values.end())
            return default_value;

        return iter->second;
    }

    double SummaryState::get(const std::string& key) const {
        const auto iter = this->values.find(key);
        if (iter == this->values.end())
            throw std::out_of_range("No such key: " + key);

        return iter->second;
    }

    bool SummaryState::has_well_var(const std::string& well, const std::string& var) const {
        return has_var(this->well_values, var, well);
    }

    bool SummaryState::has_well_var(const std::string& var) const {
        return this->well_values.count(var) != 0;
    }

    double SummaryState::get_well_var(const std::string& well, const std::string& var) const {
        return this->well_values.at(var).at(well);
    }

    double SummaryState::get_well_var(const std::string& well, const std::string& var, double default_value) const {
        if (this->has_well_var(well, var))
            return this->get_well_var(well, var);

        return default_value;
    }

    bool SummaryState::has_group_var(const std::string& group, const std::string& var) const {
        return has_var(this->group_values, var, group);
    }

    bool SummaryState::has_group_var(const std::string& var) const {
        return this->group_values.count(var) != 0;
    }

    double SummaryState::get_group_var(const std::string& group, const std::string& var) const {
        return this->group_values.at(var).at(group);
    }

    double SummaryState::get_group_var(const std::string& group, const std::string& var, double default_value) const {
        if (this->has_group_var(group, var))
            return this->get_group_var(group, var);

        return default_value;
    }

    SummaryState::const_iterator SummaryState::begin() const {
        return this->values.begin();
    }


    SummaryState::const_iterator SummaryState::end() const {
        return this->values.end();
    }


    std::vector<std::string> SummaryState::wells(const std::string& var) const {
        return var2_list(this->well_values, var);
    }


    const std::vector<std::string>& SummaryState::wells() const {
        if (!this->well_names)
            this->well_names = std::vector<std::string>(this->m_wells.begin(), this->m_wells.end());

        return *this->well_names;
    }


    std::vector<std::string> SummaryState::groups(const std::string& var) const {
        return var2_list(this->group_values, var);
    }


    const std::vector<std::string>& SummaryState::groups() const {
        if (!this->group_names)
            this->group_names = std::vector<std::string>(this->m_groups.begin(), this->m_groups.end());

        return *this->group_names;
    }

    std::size_t SummaryState::num_wells() const {
        return this->m_wells.size();
    }

    std::size_t SummaryState::size() const {
        return this->values.size();
    }




    std::vector<char> SummaryState::serialize() const {
        Serializer ser;
        ser.put(this->sim_start);
        ser.put(this->elapsed);
        ser.put_map(this->values);


        ser.put(this->well_values.size());
        for (const auto& [var, well_map] : this->well_values) {
            ser.put(var);
            ser.put_map(well_map);
        }

        ser.put(this->group_values.size());
        for (const auto& [var, group_map] : this->group_values) {
            ser.put(var);
            ser.put_map(group_map);
        }

        ser.put(this->conn_values.size());
        for (const auto& [var, well_conn_map] : this->conn_values) {
            ser.put(var);
            ser.put(well_conn_map.size());
            for (const auto& [well, conn_map] : well_conn_map) {
                ser.put(well);
                ser.put_map(conn_map);
            }
        }

        return std::move(ser.buffer);
    }


    void  SummaryState::deserialize(const std::vector<char>& buffer) {
        Serializer ser(buffer);
        this->sim_start = ser.get<time_point>();
        this->elapsed = ser.get<double>();
        this->values = ser.get_map<std::string, double>();

        {
            std::size_t num_well_var = ser.get<std::size_t>();
            for (std::size_t var_index = 0; var_index < num_well_var; var_index++) {
                std::string var = ser.get<std::string>();
                auto v = ser.get_map<std::string, double>();
                for (const auto& [well, _] : v) {
                    (void)_;
                    this->m_wells.insert(well);
                }
                this->well_values[var] = std::move(v);
            }
            this->well_names.reset();
        }

        {
            std::size_t num_group_var = ser.get<std::size_t>();
            for (std::size_t var_index = 0; var_index < num_group_var; var_index++) {
                std::string var = ser.get<std::string>();
                auto v= ser.get_map<std::string, double>();
                for (const auto& [group, _] : v) {
                    (void)_;
                    this->m_groups.insert(group);
                }
                this->group_values[var] = std::move(v);
            }
            this->group_names.reset();
        }

        {
            std::size_t num_conn_var = ser.get<std::size_t>();
            for (std::size_t var_index = 0; var_index < num_conn_var; var_index++) {
                std::string var = ser.get<std::string>();
                std::size_t num_wells = ser.get<std::size_t>();
                for (std::size_t well_index = 0; well_index < num_wells; well_index++) {
                    std::string well = ser.get<std::string>();
                    auto conn_map = ser.get_map<std::size_t, double>();
                    this->conn_values[var][well] = std::move(conn_map);
                }
            }
        }
    }

    std::ostream& operator<<(std::ostream& stream, const SummaryState& st) {
        stream << "Simulated seconds: " << st.get_elapsed() << std::endl;
        for (const auto& value_pair : st)
            stream << std::setw(17) << value_pair.first << ": " << value_pair.second << std::endl;

        return stream;
    }


    bool SummaryState::operator==(const SummaryState& other) const {
        return this->sim_start == other.sim_start &&
               this->elapsed == other.elapsed &&
               this->values == other.values &&
               this->well_values == other.well_values &&
               this->m_wells == other.m_wells &&
               this->wells() == other.wells() &&
               this->group_values == other.group_values &&
               this->m_groups == other.m_groups &&
               this->groups() == other.groups() &&
               this->conn_values == other.conn_values;
    }
}
