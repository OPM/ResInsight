/*
  Copyright 2020 Equinor AS.

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

#ifndef FIELD_DATA_HPP
#define FIELD_DATA_HPP

#include <opm/input/eclipse/EclipseState/Grid/Box.hpp>
#include <opm/input/eclipse/EclipseState/Grid/Keywords.hpp>
#include <opm/input/eclipse/Deck/value_status.hpp>

#include <string>
#include <vector>
#include <optional>
#include <array>
#include <algorithm>
#include <stdexcept>

namespace Opm
{
namespace Fieldprops
{
   template<typename T>
    static void compress(std::vector<T>& data, const std::vector<bool>& active_map) {
        std::size_t shift = 0;
        for (std::size_t g = 0; g < active_map.size(); g++) {
            if (active_map[g] && shift > 0) {
                data[g - shift] = data[g];
                continue;
            }

            if (!active_map[g])
                shift += 1;
        }

        data.resize(data.size() - shift);
    }

    template<typename T>
    struct FieldData {
        std::vector<T> data;
        std::vector<value::status> value_status;
        keywords::keyword_info<T> kw_info;
        std::optional<std::vector<T>> global_data;
        std::optional<std::vector<value::status>> global_value_status;
        mutable bool all_set;

        bool operator==(const FieldData& other) const {
            return this->data == other.data &&
                   this->value_status == other.value_status &&
                   this->kw_info == other.kw_info &&
                   this->global_data == other.global_data &&
                   this->global_value_status == other.global_value_status;
        }


        FieldData() = default;

        FieldData(const keywords::keyword_info<T>& info, std::size_t active_size, std::size_t global_size) :
            data(std::vector<T>(active_size)),
            value_status(active_size, value::status::uninitialized),
            kw_info(info),
            all_set(false)
        {
            if (global_size != 0) {
                this->global_data = std::vector<T>(global_size);
                this->global_value_status = std::vector<value::status>(global_size, value::status::uninitialized);
            }

            if (info.scalar_init)
                this->default_assign( *info.scalar_init );
        }


        std::size_t size() const {
            return this->data.size();
        }

        bool valid() const {
            if (this->all_set)
                return true;

            static const std::array<value::status,2> invalid_value = {value::status::uninitialized, value::status::empty_default};
            const auto& it = std::find_first_of(this->value_status.begin(), this->value_status.end(), invalid_value.begin(), invalid_value.end());
            this->all_set = (it == this->value_status.end());

            return this->all_set;
        }

        bool valid_default() const {
            return std::all_of( this->value_status.begin(), this->value_status.end(), [] (const value::status& status) {return status == value::status::valid_default; });
        }


        void compress(const std::vector<bool>& active_map) {
            Fieldprops::compress(this->data, active_map);
            Fieldprops::compress(this->value_status, active_map);
        }

        void copy(const FieldData<T>& src, const std::vector<Box::cell_index>& index_list) {
            for (const auto& ci : index_list) {
                this->data[ci.active_index] = src.data[ci.active_index];
                this->value_status[ci.active_index] = src.value_status[ci.active_index];
            }
        }

        void default_assign(T value) {
            std::fill(this->data.begin(), this->data.end(), value);
            std::fill(this->value_status.begin(), this->value_status.end(), value::status::valid_default);

            if (this->global_data) {
                std::fill(this->global_data->begin(), this->global_data->end(), value);
                std::fill(this->global_value_status->begin(), this->global_value_status->end(), value::status::valid_default);
            }
        }

        void default_assign(const std::vector<T>& src) {
            if (src.size() != this->size())
                throw std::invalid_argument("Size mismatch got: " + std::to_string(src.size()) + " expected: " + std::to_string(this->size()));

            std::copy(src.begin(), src.end(), this->data.begin());
            std::fill(this->value_status.begin(), this->value_status.end(), value::status::valid_default);
        }

        void default_update(const std::vector<T>& src) {
            if (src.size() != this->size())
                throw std::invalid_argument("Size mismatch got: " + std::to_string(src.size()) + " expected: " + std::to_string(this->size()));

            for (std::size_t i = 0; i < src.size(); i++) {
                if (!value::has_value(this->value_status[i])) {
                    this->value_status[i] = value::status::valid_default;
                    this->data[i] = src[i];
                }
            }
        }

        void update(std::size_t index, T value, value::status status) {
            this->data[index] = value;
            this->value_status[index] = status;
        }

    };
} // end namespace Fieldprops
} // end namespace Opm
#endif // FIELD_DATA_HPP
