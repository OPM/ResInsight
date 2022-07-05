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
#include <optional>
#include <string>
#include <vector>

#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>
#include <opm/input/eclipse/EclipseState/Runspec.hpp>

namespace Opm {

class UDAValue;
class UDQConfig;
class UnitSystem;

namespace RestartIO {
    struct RstState;
}

class UDQActive {
public:

    struct RstRecord {

        RstRecord(UDAControl control_arg, UDAValue value_arg, std::string wgname_arg)
            : control(control_arg)
            , value(value_arg)
            , wgname(wgname_arg)
        {};

        RstRecord(UDAControl control_arg, UDAValue value_arg, std::string wgname_arg, Phase phase)
            : RstRecord(control_arg, value_arg, wgname_arg)
        {
            this->ig_phase = phase;
        };

        UDAControl control;
        UDAValue value;
        std::string wgname;
        std::optional<Phase> ig_phase;
    };



    class OutputRecord{
    public:
        OutputRecord() :
            input_index(0),
            control(UDAControl::WCONPROD_ORAT),
            uda_code(0),
            use_count(1)
        {}

        OutputRecord(const std::string& udq_arg, std::size_t input_index_arg, std::size_t use_index_arg, const std::string& wgname_arg, UDAControl control_arg) :
            udq(udq_arg),
            input_index(input_index_arg),
            use_index(use_index_arg),
            control(control_arg),
            uda_code(UDQ::udaCode(control_arg)),
            use_count(1),
            wgname(wgname_arg)
        {}

        bool operator==(const OutputRecord& other) const  {
            if ((this->udq == other.udq) &&
                (this->input_index == other.input_index) &&
                (this->use_index == other.use_index) &&
                (this->wgname == other.wgname) &&
                (this->control == other.control) &&
                (this->uda_code == other.uda_code) &&
                (this->use_count == other.use_count))
                return true;
            return false;
        }

        bool operator!=(const OutputRecord& other) const  {
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
            serializer(uda_code);
            serializer(use_count);
        }

        std::string udq;
        std::size_t input_index;
        std::size_t use_index = 0;
        UDAControl  control;
        int uda_code;
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
    UDQActive() = default;
    static std::vector<RstRecord> load_rst(const UnitSystem& units,
                                           const UDQConfig& udq_config,
                                           const RestartIO::RstState& rst_state,
                                           const std::vector<std::string>& well_names,
                                           const std::vector<std::string>& group_names);
    int update(const UDQConfig& udq_config, const UDAValue& uda, const std::string& wgname, UDAControl control);
    explicit operator bool() const;
    const std::vector<OutputRecord>& iuad() const;
    std::vector<InputRecord> iuap() const;

    bool operator==(const UDQActive& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.vector(input_data);
        serializer.vector(output_data);
    }

private:
    std::string udq_hash(const std::string& udq, UDAControl control);
    std::string wg_hash(const std::string& wgname, UDAControl control);
    int add(const UDQConfig& udq_config, const std::string& udq, const std::string& wgname, UDAControl control);
    int update_input(const UDQConfig& udq_config, const UDAValue& uda, const std::string& wgname, UDAControl control);
    int drop(const std::string& wgname, UDAControl control);

    std::vector<InputRecord> input_data;
    std::vector<OutputRecord> mutable output_data;
};

}

#endif
