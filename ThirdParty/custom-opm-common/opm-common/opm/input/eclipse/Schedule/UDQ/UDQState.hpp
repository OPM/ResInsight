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

#ifndef UDQSTATE_HPP_
#define UDQSTATE_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include <opm/input/eclipse/Schedule/UDQ/UDQSet.hpp>


namespace Opm {

namespace RestartIO {
    struct RstState;
}

class UDQState {
public:
    UDQState() = default;
    UDQState(double undefined);

    bool has(const std::string& key) const;
    void load_rst(const RestartIO::RstState& rst_state);

    bool has_well_var(const std::string& well, const std::string& key) const;
    bool has_group_var(const std::string& group, const std::string& key) const;

    double get(const std::string& key) const;
    double get_group_var(const std::string& well, const std::string& var) const;
    double get_well_var(const std::string& well, const std::string& var) const;
    void add_define(std::size_t report_step, const std::string& udq_key, const UDQSet& result);
    void add_assign(std::size_t report_step, const std::string& udq_key, const UDQSet& result);
    bool assign(std::size_t report_step, const std::string& udq_key) const;
    bool define(const std::string& udq_key, std::pair<UDQUpdate, std::size_t> update_status) const;
    double undefined_value() const;

    std::vector<char> serialize() const;
    void deserialize(const std::vector<char>& buffer);
    bool operator==(const UDQState& other) const;

    static UDQState serializeObject() {
        UDQState st;
        st.undef_value = 78;
        st.scalar_values = {{"FU1", 100}, {"FU2", 200}};
        st.assignments = {{"GU1", 99}, {"GU2", 199}};
        st.defines = {{"DU1", 299}, {"DU2", 399}};

        st.well_values.emplace("W1", std::unordered_map<std::string, double>{{"U1", 100}, {"U2", 200}});
        st.well_values.emplace("W2", std::unordered_map<std::string, double>{{"U1", 700}, {"32", 600}});

        st.group_values.emplace("G1", std::unordered_map<std::string, double>{{"U1", 100}, {"U2", 200}});
        st.group_values.emplace("G2", std::unordered_map<std::string, double>{{"U1", 700}, {"32", 600}});
        return st;
    }


    template<class Serializer>
    void pack_unpack_wgmap(Serializer& serializer, std::unordered_map<std::string, std::unordered_map<std::string, double>>& wgmap) {
        std::size_t map_size = wgmap.size();
        serializer(map_size);
        if (serializer.isSerializing()) {
            for (auto& [udq_key, values] : wgmap) {
                serializer(udq_key);
                serializer.template map<std::unordered_map<std::string, double>, false>(values);
            }
        } else {
            for (std::size_t index=0; index < map_size; index++) {
                std::string udq_key;
                std::unordered_map<std::string, double> inner_map;
                serializer(udq_key);
                serializer.template map<std::unordered_map<std::string, double>, false>(inner_map);

                wgmap.emplace(udq_key, inner_map);
            }
        }
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(this->undef_value);
        serializer.template map<std::unordered_map<std::string, double>, false>(this->scalar_values);
        serializer.template map<std::unordered_map<std::string, std::size_t>, false>(this->assignments);
        serializer.template map<std::unordered_map<std::string, std::size_t>, false>(this->defines);

        pack_unpack_wgmap(serializer, this->well_values);
        pack_unpack_wgmap(serializer, this->group_values);
    }


private:
    void add(const std::string& udq_key, const UDQSet& result);
    double get_wg_var(const std::string& well, const std::string& key, UDQVarType var_type) const;
    double undef_value;
    std::unordered_map<std::string, double> scalar_values;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> well_values;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> group_values;
    std::unordered_map<std::string, std::size_t> assignments;
    std::unordered_map<std::string, std::size_t> defines;
};
}

#endif
