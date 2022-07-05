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
#include <fmt/format.h>
#include <iostream>

#include <opm/io/eclipse/rst/state.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQActive.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQConfig.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>
#include <opm/input/eclipse/Units/UnitSystem.hpp>

namespace Opm {

std::vector<UDQActive::RstRecord> UDQActive::load_rst(const UnitSystem& units,
                                                      const UDQConfig& udq_config,
                                                      const RestartIO::RstState& rst_state,
                                                      const std::vector<std::string>& well_names,
                                                      const std::vector<std::string>& group_names)
{
    std::vector<RstRecord> records;
    const auto& rst_active = rst_state.udq_active;

    for (const auto& record : rst_active.iuad) {
        const auto& udq_input = udq_config[record.input_index];
        UDAValue uda(udq_input.keyword(), units.uda_dim(record.control));
        for (std::size_t use_index = 0; use_index < record.use_count; use_index++) {
            std::size_t wg_index = rst_active.wg_index[record.wg_offset + use_index];

            if (UDQ::well_control(record.control))
                records.emplace_back(record.control, uda, well_names[wg_index]);
            else {
                const auto& group = group_names[wg_index + 1];
                if (UDQ::is_group_production_control(record.control))
                    records.emplace_back(record.control, uda, group);
                else
                    records.emplace_back(record.control, uda, group, rst_active.ig_phase[wg_index]);
            }
        }
    }
    return records;
}

UDQActive UDQActive::serializeObject()
{
    UDQActive result;
    result.input_data = {{1, "test1", "test2", UDAControl::WCONPROD_ORAT}};
    result.output_data = {{"test1", 1, 2, "test2", UDAControl::WCONPROD_ORAT}};

    return result;
}

UDQActive::operator bool() const {
    return this->input_data.size() > 0;
}


std::string UDQActive::OutputRecord::wg_name()  const {
    return this->wgname;
}

std::string UDQActive::udq_hash(const std::string& udq, UDAControl control) {
  return udq + std::to_string(static_cast<int64_t>(control));
}

std::string UDQActive::wg_hash(const std::string& wgname, UDAControl control) {
    return wgname + std::to_string(static_cast<int64_t>(control));
}

/*
  We go through the current list of input records and compare with the supplied
  arguments (uda, wgnamem, control). There are six possible outcomes:

    1. The uda variable is a double and no uda usage has been registered so far:
       fast return.

    2. The uda variable is a double, and the (wgname,control) combination is
       found in the input data; this implies that uda has been used previously
       for this particular (wgname, control) combination: We remove that record
       from the input_data.

    3. The uda variable is a string and we find that particular (udq, wgname,
       control) combination in the input data: No changes

    4. The uda variable is a string; but another udq was used for this (wgname,
       control) combination: We erase the previous entry and add a new entry.

    5. The uda ariable is a string and we do not find this (wgname, control)
       combination in the previous records: We add a new record.

    6. The uda variable is a double, and the (wgname, control) combination has
       not been encountered before: return 0

*/
int UDQActive::update(const UDQConfig& udq_config, const UDAValue& uda, const std::string& wgname, UDAControl control) {
    // Alternative 1
    if (uda.is<double>() && this->input_data.empty())
        return 0;

    if (uda.is<std::string>()) {
        if (!udq_config.has_keyword(uda.get<std::string>()))
            throw std::logic_error(fmt::format("Missing ASSIGN/DEFINE for UDQ {} can not be used as UDA for {} for {}", uda.get<std::string>(), UDQ::controlName(control), wgname));
    }

    for (auto iter = this->input_data.begin(); iter != this->input_data.end(); ++iter) {
        auto& record = *iter;
        if ((record.wgname == wgname) && (record.control == control)) {
            if (uda.is<double>()) {
                // Alternative 2
                iter = this->input_data.erase(iter);
                this->output_data.clear();
                return 1;
            } else {
                const std::string& udq = uda.get<std::string>();
                if (record.udq == udq)
                    // Alternative 3
                    return 0;
                else {
                    // Alternative 4
                    iter = this->input_data.erase(iter);
                    this->output_data.clear();
                    break;
                }
            }
        }
    }

    // Alternative 4 & 5
    if (uda.is<std::string>()) {
        const std::string& udq = uda.get<std::string>();
        const auto& udq_input = udq_config[udq];
        auto udq_index = udq_input.index.insert_index;
        this->input_data.emplace_back( udq_index, udq, wgname, control );
        this->output_data.clear();
        return 1;
    }

    // Alternative 6
    return 0;
}


const std::vector<UDQActive::OutputRecord>& UDQActive::iuad() const {
    if (this->output_data.empty()) {
        for (const auto& input_record : this->input_data) {
            const auto& udq = input_record.udq;
            const auto& control = input_record.control;
            bool found = false;
            for (auto& output_record : this->output_data) {
                if ((output_record.udq == udq) && (output_record.control == control)) {
                    output_record.use_count += 1;
                    found = true;
                    break;
                }
            }

            if (!found)
                this->output_data.emplace_back(input_record.udq, input_record.input_index, 0, input_record.wgname, input_record.control);
        }

        if (!this->output_data.empty()) {
            for (std::size_t index = 1; index < output_data.size(); index++) {
                const auto& prev_record = this->output_data[index - 1];
                this->output_data[index].use_index = prev_record.use_index + prev_record.use_count;
            }
        }
    }

    return this->output_data;
}

std::vector<UDQActive::InputRecord> UDQActive::iuap() const {
    std::vector<UDQActive::InputRecord> iuap_data;
    auto input_rcpy = this->input_data;
    while (!input_rcpy.empty()) {
        //store next active control (new control)
        auto inp_rec = input_rcpy.begin();
        auto cur_rec = *inp_rec;
        iuap_data.push_back(*inp_rec);
        auto it = input_rcpy.erase(input_rcpy.begin());
        //find and store active controls with same control and udq
        //auto it = input_rcpy.begin();
        while (it != input_rcpy.end()) {
            if ((it->control == cur_rec.control) && (it->udq == cur_rec.udq)) {
                iuap_data.push_back(*it);
                it = input_rcpy.erase(it);
            }
            else {
                it++;
            }
        }
    }
    return iuap_data;
}


bool UDQActive::operator==(const UDQActive& data) const {
    return this->input_data == data.input_data &&
           this->output_data == data.output_data;
}


}

