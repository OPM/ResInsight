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

#include <opm/parser/eclipse/Deck/UDAValue.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQActive.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQConfig.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>
#include <iostream>

namespace Opm {

UDQActive UDQActive::serializeObject()
{
    UDQActive result;
    result.input_data = {{1, "test1", "test2", UDAControl::WCONPROD_ORAT}};
    result.output_data = {{"test1", 1, 2, "test2", UDAControl::WCONPROD_ORAT}};
    result.udq_keys = {{"test1", 1}};
    result.wg_keys = {{"test2", 2}};

    return result;
}

std::size_t UDQActive::IUAD_size() const {
    const auto& output = this->get_iuad();
    return output.size();
}

std::size_t UDQActive::IUAP_size() const {
    const auto& output = this->get_iuap();
    return output.size();
}

UDQActive::operator bool() const {
    return this->input_data.size() > 0;
}


std::string UDQActive::Record::wg_name()  const {
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


const std::vector<UDQActive::Record>& UDQActive::get_iuad() const {
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

        if (!output_data.empty()) {
            for (std::size_t index = 1; index < output_data.size(); index++) {
                const auto& prev_record = this->output_data[index - 1];
                this->output_data[index].use_index = prev_record.use_index + prev_record.use_count;
            }
        }
    }
    
    return this->output_data;
}

std::vector<UDQActive::InputRecord> UDQActive::get_iuap() const {
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

UDQActive::Record UDQActive::operator[](std::size_t index) const {
    const auto& output_record = this->get_iuad()[index];
    return output_record;
}   

bool UDQActive::operator==(const UDQActive& data) const {
    return this->input_data == data.input_data &&
           this->output_data == data.output_data &&
           this->udq_keys == data.udq_keys &&
           this->wg_keys == data.wg_keys;
}


}

