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
#ifndef WELLTEST_CONFIG_H
#define WELLTEST_CONFIG_H

#include <cstddef>
#include <string>
#include <unordered_map>


namespace Opm {

namespace RestartIO {
struct RstState;
}

namespace WTest {
/*
  Different numerical values are used in the restart file to enumarate the
  possible WTEST modes and the actual reason a well has been closed.
*/

namespace EclConfigReason {
constexpr int PHYSICAL   = 2;
constexpr int ECONOMIC   = 3;
constexpr int GCON       = 5;
constexpr int THPLimit   = 7;
constexpr int CONNECTION = 11;
}

namespace EclCloseReason {
constexpr int PHYSICAL = 3;
constexpr int ECONOMIC = 5;
constexpr int GCON     = 6;
constexpr int THPLimit = 9;
}

}

class WellTestConfig {

public:
    enum class Reason {
        PHYSICAL = 1,
        ECONOMIC = 2,
        GROUP = 4,
        THP_DESIGN=8,
        COMPLETION=16,
    };

    struct WTESTWell {
        std::string name;
        int reasons;
        double test_interval;
        int num_test;
        double startup_time;
        // the related WTEST keywords is entered and will begin
        // taking effects since this report step
        int begin_report_step;

        bool operator==(const WTESTWell& data) const {
            return name == data.name &&
                   reasons == data.reasons &&
                   test_interval == data.test_interval &&
                   num_test == data.num_test &&
                   startup_time == data.startup_time &&
                   begin_report_step == data.begin_report_step;
        }

        WTESTWell() = default;
        WTESTWell(const std::string& name, int reasons, double test_interval, int num_test, double startup_time, int begin_report_step);
        bool test_well(int num_attempt, double elapsed) const;

        static int inverse_ecl_reasons(int ecl_reasons);
        static WTESTWell serializeObject();
        int ecl_reasons() const;

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(name);
            serializer(reasons);
            serializer(test_interval);
            serializer(num_test);
            serializer(startup_time);
            serializer(begin_report_step);
        }
    };

    static WellTestConfig serializeObject();

    WellTestConfig() = default;
    WellTestConfig(const RestartIO::RstState& rst_state, int report_step);
    void add_well(const std::string& well, int reasons, double test_interval,
                  int num_test, double startup_time, int current_step);
    void add_well(const std::string& well, const std::string& reasons, double test_interval,
                  int num_test, double startup_time, int current_step);
    void drop_well(const std::string& well);
    bool has(const std::string& well) const;
    bool has(const std::string& well, Reason reason) const;
    const WTESTWell& get(const std::string& well) const;

    static std::string reasonToString(const Reason reason);
    bool empty() const;

    bool operator==(const WellTestConfig& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer.map(wells);
    }

private:
    std::unordered_map<std::string, WTESTWell> wells;
};
}

#endif

