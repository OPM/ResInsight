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


#ifndef UDQ_USAGE_HPP
#define UDQ_USAGE_HPP

#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>

namespace Opm {

class UDAValue;
class UDQConfig;
class UDQActive {
public:

    class Record{
    public:
        Record() :
            input_index(0),
            control(UDAControl::WCONPROD_ORAT),
            uad_code(0),
            use_count(1)
        {}

        Record(const std::string& udq_arg, std::size_t input_index_arg, std::size_t use_index_arg, const std::string& wgname_arg, UDAControl control_arg) :
            udq(udq_arg),
            input_index(input_index_arg),
            use_index(use_index_arg),
            control(control_arg),
            uad_code(UDQ::uadCode(control_arg)),
            use_count(1),
            wgname(wgname_arg)
        {}

        bool operator==(const Record& other) const  {
            if ((this->udq == other.udq) &&
                (this->input_index == other.input_index) &&
                (this->use_index == other.use_index) &&
                (this->wgname == other.wgname) &&
                (this->control == other.control) &&
                (this->uad_code == other.uad_code) &&
                (this->use_count == other.use_count))
                return true;
            return false;
        }

        bool operator!=(const Record& other) const  {
            return !(*this == other);
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(udq);
            serializer(input_index);
            serializer(use_index);
            serializer(wgname);
            serializer(control);
            serializer(uad_code);
            serializer(use_count);
        }

        std::string udq;
        std::size_t input_index;
        std::size_t use_index = 0;
        UDAControl  control;
        int uad_code;
        std::string wg_name() const;
        std::size_t use_count;
    private:
        // The wgname is need in the update process, but it should
        // not be exported out.
        std::string wgname;
    };

    class InputRecord {
    public:
        InputRecord() :
            input_index(0),
            control(UDAControl::WCONPROD_ORAT)
        {}

        InputRecord(std::size_t input_index_arg, const std::string& udq_arg, const std::string& wgname_arg, UDAControl control_arg) :
            input_index(input_index_arg),
            udq(udq_arg),
            wgname(wgname_arg),
            control(control_arg)
        {}

        bool operator==(const InputRecord& other) const {
            return this->input_index == other.input_index &&
                   this->udq == other.udq &&
                   this->wgname == other.wgname &&
                   this->control == other.control;
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(input_index);
            serializer(udq);
            serializer(wgname);
            serializer(control);
        }

        std::size_t input_index;
        std::string udq;
        std::string wgname;
        UDAControl control;
    };

    static UDQActive serializeObject();

    int update(const UDQConfig& udq_config, const UDAValue& uda, const std::string& wgname, UDAControl control);
    std::size_t IUAD_size() const;
    std::size_t IUAP_size() const;
    explicit operator bool() const;
    Record operator[](std::size_t index) const;
    const std::vector<Record>& get_iuad() const;
    std::vector<InputRecord> get_iuap() const;

    bool operator==(const UDQActive& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(input_data);
        serializer.vector(output_data);
        serializer(udq_keys);
        serializer(wg_keys);
    }

private:
    std::string udq_hash(const std::string& udq, UDAControl control);
    std::string wg_hash(const std::string& wgname, UDAControl control);
    int add(const UDQConfig& udq_config, const std::string& udq, const std::string& wgname, UDAControl control);
    int update_input(const UDQConfig& udq_config, const UDAValue& uda, const std::string& wgname, UDAControl control);
    int drop(const std::string& wgname, UDAControl control);

    std::vector<InputRecord> input_data;
    std::vector<Record> mutable output_data;
    std::unordered_map<std::string, std::size_t> udq_keys;
    std::unordered_map<std::string, std::size_t> wg_keys;
};

}

#endif
