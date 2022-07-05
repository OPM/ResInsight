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

#ifndef GUIDE_RATE_CONFIG_HPP
#define GUIDE_RATE_CONFIG_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

#include <opm/input/eclipse/Schedule/Group/GuideRateModel.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>

namespace Opm {


class GuideRateConfig {
public:

 struct WellTarget {
    double guide_rate;
    Well::GuideRateTarget target;
    double scaling_factor;

    bool operator==(const WellTarget& data) const {
        return guide_rate == data.guide_rate &&
               target == data.target &&
               scaling_factor == data.scaling_factor;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(guide_rate);
        serializer(target);
        serializer(scaling_factor);
    }
};

struct GroupProdTarget {
    double guide_rate;
    Group::GuideRateProdTarget target;

    bool operator==(const GroupProdTarget& data) const {
        return guide_rate == data.guide_rate &&
               target == data.target;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(guide_rate);
        serializer(target);
    }
};

struct GroupInjTarget {
    double guide_rate;
    Group::GuideRateInjTarget target;

    bool operator==(const GroupInjTarget& data) const {
        return guide_rate == data.guide_rate &&
               target == data.target;
    }

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(guide_rate);
        serializer(target);
    }
};

    static GuideRateConfig serializeObject();

    const GuideRateModel& model() const;
    bool has_model() const;
    bool update_model(const GuideRateModel& model);
    void update_well(const Well& well);
    void update_injection_group(const std::string& group_name, const Group::GroupInjectionProperties& properties);
    void update_production_group(const Group& group);
    const WellTarget& well(const std::string& well) const;
    const GroupProdTarget& production_group(const std::string& group) const;
    const GroupInjTarget& injection_group(const Phase& phase, const std::string& group) const;

    bool has_well(const std::string& well) const;
    bool has_injection_group(const Phase& phase, const std::string& group) const;
    bool has_production_group(const std::string& group) const;

    bool operator==(const GuideRateConfig& data) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_model);
        serializer.map(wells);
        serializer.map(production_groups);
        serializer.map(injection_groups);
    }

private:

    typedef std::pair<Phase,std::string> pair;

    struct pair_hash
    {
        template <class T1, class T2>
        std::size_t operator() (const std::pair<T1, T2> &pair) const
        {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };

    std::optional<GuideRateModel> m_model;
    std::unordered_map<std::string, WellTarget> wells;
    std::unordered_map<std::string, GroupProdTarget> production_groups;
    std::unordered_map<pair, GroupInjTarget, pair_hash> injection_groups;

};

}

#endif
