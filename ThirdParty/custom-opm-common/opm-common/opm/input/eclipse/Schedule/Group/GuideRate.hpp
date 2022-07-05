/*
  Copyright 2019, 2020 Equinor ASA.

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

#ifndef GUIDE_RATE_HPP
#define GUIDE_RATE_HPP

#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Group/GuideRateModel.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>

#include <cstddef>
#include <ctime>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace Opm {

class Schedule;

} // namespace Opm

namespace Opm {

class GuideRate
{
public:
    // used for potentials and well rates
    struct RateVector {
        RateVector() = default;
        RateVector(const double orat, const double grat, const double wrat)
            : oil_rat(orat)
            , gas_rat(grat)
            , wat_rat(wrat)
        {}

        double eval(const Well::GuideRateTarget target) const;
        double eval(const Group::GuideRateProdTarget target) const;
        double eval(const GuideRateModel::Target target) const;

        double oil_rat{0.0};
        double gas_rat{0.0};
        double wat_rat{0.0};
    };

    struct GuideRateValue {
        GuideRateValue() = default;
        GuideRateValue(const double t, const double v, const GuideRateModel::Target tg)
            : sim_time(t)
            , value   (v)
            , target  (tg)
        {}

        bool operator==(const GuideRateValue& other) const
        {
            return (this->sim_time == other.sim_time)
                && (this->value == other.value);
        }

        bool operator!=(const GuideRateValue& other) const
        {
            return !(*this == other);
        }

        double sim_time { std::numeric_limits<double>::lowest() };
        double value { std::numeric_limits<double>::lowest() };
        GuideRateModel::Target target { GuideRateModel::Target::NONE };
    };

    GuideRate(const Schedule& schedule);

    void compute(const std::string& wgname,
                 const std::size_t  report_step,
                 const double       sim_time,
                 const double       oil_pot,
                 const double       gas_pot,
                 const double       wat_pot);

    void compute(const std::string& wgname,
                 const Phase&       phase,
                 const std::size_t  report_step,
                 const double       guide_rate);

    bool has(const std::string& name) const;
    bool hasPotentials(const std::string& name) const;
    bool has(const std::string& name, const Phase& phase) const;

    double get(const std::string& well, const Well::GuideRateTarget target, const RateVector& rates) const;
    double get(const std::string& group, const Group::GuideRateProdTarget target, const RateVector& rates) const;
    double get(const std::string& name, const GuideRateModel::Target model_target, const RateVector& rates) const;
    double get(const std::string& group, const Phase& phase) const;

    double getSI(const std::string& well, const Well::GuideRateTarget target, const RateVector& rates) const;
    double getSI(const std::string& group, const Group::GuideRateProdTarget target, const RateVector& rates) const;
    double getSI(const std::string& wgname, const GuideRateModel::Target target, const RateVector& rates) const;
    double getSI(const std::string& group, const Phase& phase) const;

    void init_grvalue(const std::size_t report_step, const std::string& wgname, GuideRateValue value);
    void init_grvalue_SI(const std::size_t report_step, const std::string& wgname, GuideRateValue value);

    void updateGuideRateExpiration(const double      sim_time,
                                   const std::size_t report_step);

private:
    struct GRValState
    {
        GuideRateValue curr{};
        GuideRateValue prev{};
    };

    struct pair_hash
    {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2>& pair) const
        {
            return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
        }
    };

    using GRValPtr = std::unique_ptr<GRValState>;
    using pair = std::pair<Phase, std::string>;

    void well_compute(const std::string& wgname,
                      const std::size_t  report_step,
                      const double       sim_time,
                      const double       oil_pot,
                      const double       gas_pot,
                      const double       wat_pot);

    void group_compute(const std::string& wgname,
                       const std::size_t  report_step,
                       const double       sim_time,
                       const double       oil_pot,
                       const double       gas_pot,
                       const double       wat_pot);

    double eval_form(const GuideRateModel& model,
                     const double          oil_pot,
                     const double          gas_pot,
                     const double          wat_pot) const;
    double eval_group_pot() const;
    double eval_group_resvinj() const;

    void assign_grvalue(const std::string&    wgname,
                        const GuideRateModel& model,
                        GuideRateValue&&      value);
    double get_grvalue_result(const GRValState& gr) const;

    const Schedule& schedule;

    std::unordered_map<std::string, GRValPtr> values{};
    std::unordered_map<pair, double, pair_hash> injection_group_values{};
    std::unordered_map<std::string, RateVector> potentials{};
    bool guide_rates_expired {false};
};

} // namespace Opm

#endif
