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

#include <opm/parser/eclipse/EclipseState/Schedule/SummaryState.hpp>

namespace Opm{
namespace {

    bool is_total(const std::string& key) {
        static const std::vector<std::string> totals = {"OPT"  , "GPT"  , "WPT" , "GIT", "WIT", "OPTF" , "OPTS" , "OIT"  , "OVPT" , "OVIT" , "MWT" ,
                                                        "WVPT" , "WVIT" , "GMT"  , "GPTF" , "SGT"  , "GST" , "FGT" , "GCT" , "GIMT" ,
                                                        "WGPT" , "WGIT" , "EGT"  , "EXGT" , "GVPT" , "GVIT" , "LPT" , "VPT" , "VIT" , "NPT" , "NIT",
                                                        "CPT", "CIT"};

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
        this->m_groups.insert(group);
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
        this->m_wells.insert(well);
    }


    void SummaryState::set(const std::string& key, double value) {
        this->values[key] = value;
    }


    bool SummaryState::has(const std::string& key) const {
        return (this->values.find(key) != this->values.end());
    }


    double SummaryState::get(const std::string& key) const {
        const auto iter = this->values.find(key);
        if (iter == this->values.end())
            throw std::out_of_range("No such key: " + key);

        return iter->second;
    }

    bool SummaryState::has_well_var(const std::string& well, const std::string& var) const {
        const auto& var_iter = this->well_values.find(var);
        if (var_iter == this->well_values.end())
            return false;

        const auto& well_iter = var_iter->second.find(well);
        if (well_iter == var_iter->second.end())
            return false;

        return true;
    }

    double SummaryState::get_well_var(const std::string& well, const std::string& var) const {
        return this->well_values.at(var).at(well);
    }

    bool SummaryState::has_group_var(const std::string& group, const std::string& var) const {
        const auto& var_iter = this->group_values.find(var);
        if (var_iter == this->group_values.end())
            return false;

        const auto& group_iter = var_iter->second.find(group);
        if (group_iter == var_iter->second.end())
            return false;

        return true;
    }

    double SummaryState::get_group_var(const std::string& group, const std::string& var) const {
        return this->group_values.at(var).at(group);
    }

    SummaryState::const_iterator SummaryState::begin() const {
        return this->values.begin();
    }


    SummaryState::const_iterator SummaryState::end() const {
        return this->values.end();
    }


    std::vector<std::string> SummaryState::wells(const std::string& var) const {
        const auto& var_iter = this->well_values.find(var);
        if (var_iter == this->well_values.end())
            return {};

        std::vector<std::string> wells;
        for (const auto& pair : var_iter->second)
            wells.push_back(pair.first);
        return wells;
    }


    std::vector<std::string> SummaryState::wells() const {
        return std::vector<std::string>(this->m_wells.begin(), this->m_wells.end());
    }


    std::vector<std::string> SummaryState::groups(const std::string& var) const {
        const auto& var_iter = this->group_values.find(var);
        if (var_iter == this->group_values.end())
            return {};

        std::vector<std::string> groups;
        for (const auto& pair : var_iter->second)
            groups.push_back(pair.first);
        return groups;
    }


    std::vector<std::string> SummaryState::groups() const {
        return std::vector<std::string>(this->m_groups.begin(), this->m_groups.end());
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
