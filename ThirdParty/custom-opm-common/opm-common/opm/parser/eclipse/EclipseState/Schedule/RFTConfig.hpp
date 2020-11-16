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
#ifndef RFT_CONFIG_HPP
#define RFT_CONFIG_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>

#include <opm/parser/eclipse/EclipseState/Schedule/Well/Connection.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/DynamicState.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/TimeMap.hpp>

namespace Opm {

class RFTConfig {
public:
    enum class RFT {
        YES = 1,
        REPT = 2,
        TIMESTEP = 3,
        FOPN = 4,
        NO = 5
    };
    static std::string RFT2String(RFT enumValue);
    static RFT RFTFromString(const std::string &stringValue);

    enum class PLT {
        YES      = 1,
        REPT     = 2,
        TIMESTEP = 3,
        NO       = 4
    };
    static std::string PLT2String(PLT enumValue);
    static PLT PLTFromString( const std::string& stringValue);

    template <typename Value>
    using ConfigMap = std::unordered_map<
        std::string, DynamicState<std::pair<Value, std::size_t>>
    >;

    using WellOpenTimeMap = std::unordered_map<std::string, std::size_t>;

    RFTConfig();
    explicit RFTConfig(const TimeMap& time_map);

    static RFTConfig serializeObject();

    bool rft(const std::string& well, std::size_t report_step) const;
    bool plt(const std::string& well, std::size_t report_step) const;
    bool getWellOpenRFT(const std::string& well_name, std::size_t report_step) const;
    void setWellOpenRFT(std::size_t report_step);
    void setWellOpenRFT(const std::string& well_name);

    bool active(std::size_t report_step) const;
    std::size_t firstRFTOutput() const { return this->first_rft_event; }
    void updateRFT(const std::string& well, std::size_t report_step, RFT value);
    void updatePLT(const std::string& well, std::size_t report_step, PLT value);
    void addWellOpen(const std::string& well, std::size_t report_step);

    const TimeMap& timeMap() const;

    bool operator==(const RFTConfig& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        tm.serializeOp(serializer);
        serializer(first_rft_event);
        serializer(well_open_rft_time);
        serializer(well_open_rft_name);
        serializer(well_open);
        serializer.template map<ConfigMap<RFT>,false>(rft_config);
        serializer.template map<ConfigMap<PLT>,false>(plt_config);
    }

private:
    TimeMap tm;
    std::size_t first_rft_event;
    std::pair<bool, std::size_t> well_open_rft_time;
    WellOpenTimeMap well_open_rft_name;
    WellOpenTimeMap well_open;
    ConfigMap<RFT> rft_config;
    ConfigMap<PLT> plt_config;

    bool outputRftAtWellopen(WellOpenTimeMap::const_iterator well, const std::size_t report_step) const;
    std::size_t firstWellopenStepNotBefore(const std::size_t report_step) const;
    void updateFirstIfNotShut(const std::string& well_name, const std::size_t report_step);
    void updateFirst(const std::size_t report_step);

    void setWellOpenRFT(const std::string& well_name, const std::size_t report_step);

    template <typename Value>
    void updateConfig(const std::string& well_name,
                      const std::size_t  report_step,
                      const Value        value,
                      ConfigMap<Value>&  cfgmap);
};

}

#endif
