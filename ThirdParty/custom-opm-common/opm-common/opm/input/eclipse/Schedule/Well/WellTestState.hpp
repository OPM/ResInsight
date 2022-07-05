/*
  Copyright 2018 Statoil ASA.

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
#ifndef WELLTEST_STATE_H
#define WELLTEST_STATE_H

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <opm/io/eclipse/rst/state.hpp>

#include <opm/input/eclipse/Schedule/Well/WellTestConfig.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>

namespace Opm {

namespace {

template<class BufferType, class M>
void pack_map(BufferType& buffer, const M& m) {
    buffer.write(m.size());
    for (const auto& [k,v] : m) {
        buffer.write(k);
        v.pack(buffer);
    }
}

template<class BufferType, class M>
void unpack_map(BufferType& buffer, M& m) {
    typename M::size_type size;
    buffer.read(size);
    for (std::size_t i = 0; i < size; i++) {
        typename M::key_type k;
        typename M::mapped_type v;
        buffer.read(k);
        v.unpack(buffer);
        m.emplace(std::move(k), std::move(v));
    }
}

}



class WellTestState {
public:
    /*
      This class implements a small mutable state object which keeps track of
      which wells have been automatically closed by the simulator through the
      WTEST mechanism.

      The default behavior of the container is to manage *closed* wells, but
      since the counter mechanism for the maximum number of opening attempts
      should maintain the same counter through open/close events we need to
      manage the well object also after they have been opened up again.
    */
    struct RestartWell {
        std::string name;
        double test_interval;
        int num_test;
        double startup_time;

        int config_reasons;
        int close_reason;

        RestartWell(const std::string& wname, double ti, int num, double st, int r1, int r2)
            : name(wname)
            , test_interval(ti)
            , num_test(num)
            , startup_time(st)
            , config_reasons(r1)
            , close_reason(r2)
        {};
    };

    struct WTestWell {
        std::string name;
        WellTestConfig::Reason reason;
        double last_test;

        int num_attempt{0};
        bool closed{true};
        std::optional<int> wtest_report_step;

        WTestWell() = default;
        WTestWell(const std::string& wname, WellTestConfig::Reason reason_, double last_test);

        int int_reason() const;
        static WellTestConfig::Reason inverse_ecl_reason(int ecl_reason);

        bool operator==(const WTestWell& other) const {
            return this->name == other.name &&
                   this->reason == other.reason &&
                   this->last_test == other.last_test &&
                   this->num_attempt == other.num_attempt &&
                   this->closed == other.closed &&
                   this->wtest_report_step == other.wtest_report_step;
        }

        static WTestWell serializeObject();

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(this->name);
            serializer(this->reason);
            serializer(this->last_test);
            serializer(this->num_attempt);
            serializer(this->closed);
            serializer(this->wtest_report_step);
        }

        template<class BufferType>
        void pack(BufferType& buffer) const {
            buffer.write(this->name);
            buffer.write(this->reason);
            buffer.write(this->last_test);
            buffer.write(this->num_attempt);
            buffer.write(this->closed);
            buffer.write(this->wtest_report_step);
        }

        template<class BufferType>
        void unpack(BufferType& buffer) {
            buffer.read(this->name);
            buffer.read(this->reason);
            buffer.read(this->last_test);
            buffer.read(this->num_attempt);
            buffer.read(this->closed);
            buffer.read(this->wtest_report_step);
        }
    };


    struct ClosedCompletion {
        std::string wellName;
        int complnum;
        double last_test;
        int num_attempt;

        bool operator==(const ClosedCompletion& other) const {
            return this->wellName == other.wellName &&
                   this->complnum == other.complnum &&
                   this->last_test == other.last_test &&
                   this->num_attempt == other.num_attempt;
        }

        static ClosedCompletion serializeObject();

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(this->wellName);
            serializer(this->complnum);
            serializer(this->last_test);
            serializer(this->num_attempt);
        }

        template<class BufferType>
        void pack(BufferType& buffer) const {
            buffer.write(this->wellName);
            buffer.write(this->complnum);
            buffer.write(this->last_test);
            buffer.write(this->num_attempt);
        }

        template<class BufferType>
        void unpack(BufferType& buffer) {
            buffer.read(this->wellName);
            buffer.read(this->complnum);
            buffer.read(this->last_test);
            buffer.read(this->num_attempt);
        }
    };

    WellTestState() = default;
    WellTestState(std::time_t start_time, const RestartIO::RstState& rst_state);


    std::vector<std::string> test_wells(const WellTestConfig& config, double sim_time);
    void filter_wells(const std::vector<std::string>& existing_wells);
    /*
      The purpose of this container is to manage explicitly *closed wells*,
      since the class has no relation to the set of of wells defined in the
      Schedule section the concept of open wells and totally unknown wells is
      slightly murky:

         well_is_closed("UNKOWN_WELL") -> false

      This implies that we have not explicitly closed a well with name
      'UNKNOWN_WELL', but since we do not know whether the well is at all
      defined it does not make sense to extrapolate to:

         well_is_open("UNKNOWN_WELL") -> true.

      That is the reason we do not have any xxx_is_open() predicates.
    */
    void close_well(const std::string& well_name, WellTestConfig::Reason reason, double sim_time);
    bool well_is_closed(const std::string& well_name) const;
    void open_well(const std::string& well_name);
    std::size_t num_closed_wells() const;
    double lastTestTime(const std::string& well_name) const;

    void close_completion(const std::string& well_name, int complnum, double sim_time);
    void open_completion(const std::string& well_name, int complnum);
    void open_completions(const std::string& well_name);
    bool completion_is_closed(const std::string& well_name, const int complnum) const;
    std::size_t num_closed_completions() const;

    void clear();


    template<class BufferType>
    void pack(BufferType& buffer) const {
        pack_map(buffer, this->wells);

        buffer.write(this->completions.size());
        for (const auto& [well, cmap] : this->completions) {
            buffer.write(well);
            pack_map(buffer, cmap);
        }
    }

    template<class BufferType>
    void unpack(BufferType& buffer) {
        unpack_map(buffer, this->wells);
        std::size_t size;
        buffer.read(size);
        for (std::size_t i = 0; i < size; i++) {
            std::string well;
            std::unordered_map<int, ClosedCompletion> cmap;

            buffer.read(well);
            unpack_map(buffer, cmap);
            this->completions.emplace(std::move(well), std::move(cmap));
        }
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.map(this->wells);
        if (serializer.isSerializing()) {
            std::size_t size = this->completions.size();
            serializer(size);
            for (auto& [well, comp_map] : this->completions) {
                serializer(well);
                serializer.map(comp_map);
            }
        } else {
            std::size_t size = 0;
            serializer(size);
            for (std::size_t i=0; i < size; i++) {
                std::string well;
                std::unordered_map<int, ClosedCompletion> comp_map;

                serializer(well);
                serializer.map(comp_map);
                this->completions.emplace(well, std::move(comp_map));
            }
        }
    }


    static WellTestState serializeObject();
    bool operator==(const WellTestState& other) const;

    std::optional<WellTestState::RestartWell> restart_well(const Opm::WellTestConfig& config, const std::string& wname) const;

private:
    std::unordered_map<std::string, WTestWell> wells;
    std::unordered_map<std::string, std::unordered_map<int, ClosedCompletion>> completions;

    std::vector<std::pair<std::string, int>> updateCompletion(const WellTestConfig& config, double sim_time);
};


}

#endif

