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

#ifdef _WIN32
#include "cross-platform/windows/Substitutes.hpp"
#endif

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQSet.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>

namespace Opm{
namespace {

    bool is_total(const std::string& key) {
        static const std::vector<std::string> totals = {"OPT"  , "GPT"  , "WPT" , "GIT", "WIT", "OPTF" , "OPTS" , "OIT"  , "OVPT" , "OVIT" , "MWT" ,
                                                        "WVPT" , "WVIT" , "GMT"  , "GPTF" , "SGT"  , "GST" , "FGT" , "GCT" , "GIMT" ,
                                                        "WGPT" , "WGIT" , "EGT"  , "EXGT" , "GVPT" , "GVIT" , "LPT" , "VPT" , "VIT" , "NPT" , "NIT",
                                                        "CPT", "CIT", "SPT", "SIT"};

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


    using map2 = std::unordered_map<std::string, std::unordered_map<std::string, double>>;

    bool has_var(const map2& values, const std::string& var1, const std::string var2) {
        const auto& var1_iter = values.find(var1);
        if (var1_iter == values.end())
            return false;

        const auto& var2_iter = var1_iter->second.find(var2);
        if (var2_iter == var1_iter->second.end())
            return false;

        return true;
    }

    bool erase_var(map2& values, const std::string& var1, const std::string var2) {
        const auto& var1_iter = values.find(var1);
        if (var1_iter == values.end())
            return false;

        return (var1_iter->second.erase(var2) > 0);
    }

    std::vector<std::string> var2_list(const map2& values, const std::string& var1) {
        const auto& var1_iter = values.find(var1);
        if (var1_iter == values.end())
            return {};

        std::vector<std::string> l;
        for (const auto& pair : var1_iter->second)
            l.push_back(pair.first);
        return l;
    }
}


    SummaryState::SummaryState(std::chrono::system_clock::time_point sim_start_arg):
        sim_start(sim_start_arg)
    {
        this->update_elapsed(0);
    }


    void SummaryState::update_elapsed(double delta) {
        this->elapsed += delta;
        std::time_t sim_time = std::chrono::system_clock::to_time_t( this->sim_start + std::chrono::microseconds(static_cast<std::size_t>(1000000*delta)));
        struct tm ts;
        gmtime_r(&sim_time, &ts);
        int year = ts.tm_year + 1900;
        int month = ts.tm_mon;
        int day = ts.tm_mday;

        this->update("YEAR", year);
        this->update("MNTH", month);
        this->update("DAY", day);
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

    void SummaryState::update_udq(const UDQSet& udq_set) {
        auto var_type = udq_set.var_type();
        if (var_type == UDQVarType::WELL_VAR) {
            const std::vector<std::string> wells = this->wells();
            for (const auto& well : wells) {
                const auto& udq_value = udq_set[well];
                if (udq_value)
                    this->update_well_var(well, udq_set.name(), udq_value.value());
                else
                    this->erase_well_var(well, udq_set.name());
            }
        } else if (var_type == UDQVarType::GROUP_VAR) {
            const std::vector<std::string> groups = this->groups();
            for (const auto& group : groups) {
                const auto& udq_value = udq_set[group];
                if (udq_value)
                    this->update_group_var(group, udq_set.name(), udq_value.value());
                else
                    this->erase_group_var(group, udq_set.name());
            }
        } else {
            const auto& udq_var = udq_set[0];
            if (udq_var)
                this->update(udq_set.name(), udq_var.value());
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

        erase_var(this->well_values, var, well);
        this->well_names.reset();
        return true;
    }

    bool SummaryState::erase_group_var(const std::string& group, const std::string& var) {
        std::string key = var + ":" + group;
        if (!this->erase(key))
            return false;

        erase_var(this->group_values, var, group);
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


namespace {
    class Serializer {
    public:
        Serializer() = default;
        explicit Serializer(const std::vector<char>& buffer_arg) :
            buffer(buffer_arg)
        {}


        template <typename T>
        void put(const T& value) {
            this->pack(std::addressof(value), sizeof(T));
        }



        template <typename T>
        T get() {
            T value;
            std::memcpy(&value, &this->buffer[pos], sizeof(T));
            this->pos += sizeof(T);
            return value;
        }

        std::vector<char> buffer;
    private:
        void pack(const void * ptr, std::size_t value_size) {
            std::size_t write_pos = this->buffer.size();
            std::size_t new_size = write_pos + value_size;
            this->buffer.resize( new_size );
            std::memcpy(&this->buffer[write_pos], ptr, value_size);
        }

        std::size_t pos = 0;
    };

    template <>
    void Serializer::put(const std::string& value) {
        this->put<std::string::size_type>(value.size());
        this->pack(value.c_str(), value.size());
    }

    template <>
    std::string Serializer::get() {
        std::string::size_type length = this->get<std::string::size_type>();
        this->pos += length;
        return {std::addressof(this->buffer[this->pos - length]), length};
    }

    void put_map(Serializer& ser, const std::unordered_map<std::string, double>& values) {
        ser.put(values.size());
        for (const auto& value_pair : values) {
            ser.put(value_pair.first);
            ser.put(value_pair.second);
        }
    }

}

    std::vector<char> SummaryState::serialize() const {
        Serializer ser;
        ser.put(this->elapsed);
        put_map(ser, values);

        ser.put(this->well_values.size());
        for (const auto& well_var_pair : this->well_values) {
            ser.put(well_var_pair.first);
            put_map(ser, well_var_pair.second);
        }

        ser.put(this->group_values.size());
        for (const auto& group_var_pair : this->group_values) {
            ser.put(group_var_pair.first);
            put_map(ser, group_var_pair.second);
        }

        return std::move(ser.buffer);
    }


    void  SummaryState::deserialize(const std::vector<char>& buffer) {
        this->values.clear();
        this->m_wells.clear();
        this->well_values.clear();
        this->m_groups.clear();
        this->group_values.clear();
        this->elapsed = 0;

        Serializer ser(buffer);
        this->elapsed = ser.get<double>();
        {
            std::size_t num_values = ser.get<std::size_t>();
            for (std::size_t index = 0; index < num_values; index++) {
                std::string key = ser.get<std::string>();
                double value = ser.get<double>();
                this->update(key, value);
            }
        }

        {
            std::size_t num_well_var = ser.get<std::size_t>();
            for (std::size_t var_index = 0; var_index < num_well_var; var_index++) {
                std::string var = ser.get<std::string>();

                std::size_t num_well = ser.get<std::size_t>();
                for (std::size_t well_index=0; well_index < num_well; well_index++) {
                    std::string well = ser.get<std::string>();
                    double value = ser.get<double>();
                    this->update_well_var(well, var, value);
                }
            }
        }

        {
            std::size_t num_group_var = ser.get<std::size_t>();
            for (std::size_t var_index = 0; var_index < num_group_var; var_index++) {
                std::string var = ser.get<std::string>();

                std::size_t num_group = ser.get<std::size_t>();
                for (std::size_t group_index=0; group_index < num_group; group_index++) {
                    std::string group = ser.get<std::string>();
                    double value = ser.get<double>();
                    this->update_group_var(group, var, value);
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
}
