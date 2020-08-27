/*
  Copyright 2020 Equinor ASA.

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
#ifndef GAS_LIFT_OPT_HPP
#define GAS_LIFT_OPT_HPP

#include <optional>
#include <string>
#include <map>

namespace Opm {

class GasLiftOpt {
public:

    class Group {
    public:
        Group() = default;

        Group(const std::string& name) :
            m_name(name)
        {}

        const std::optional<double>& max_lift_gas() const {
            return this->m_max_lift_gas;
        }

        void max_lift_gas(double value) {
            if (value >= 0)
                this->m_max_lift_gas = value;
        }

        const std::optional<double>& max_total_gas() const {
            return this->m_max_total_gas;
        }

        void max_total_gas(double value) {
            if (value >= 0)
                this->m_max_total_gas = value;
        }

        const std::string& name() const {
            return this->m_name;
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_name);
            serializer(m_max_lift_gas);
            serializer(m_max_total_gas);
        }


        static Group serializeObject() {
            Group group;
            group.m_name = "GR";
            group.m_max_lift_gas  = 100;
            group.m_max_total_gas = 200;
            return group;
        }


        bool operator==(const Group& other) const {
            return this->m_name == other.m_name &&
                   this->m_max_lift_gas == other.m_max_lift_gas &&
                   this->m_max_total_gas == other.m_max_total_gas;
        }

    private:
        std::string m_name;
        std::optional<double> m_max_lift_gas;
        std::optional<double> m_max_total_gas;
    };


    class Well {
    public:
        Well() = default;
        Well(const std::string& name, bool use_glo) :
            m_name(name),
            m_use_glo(use_glo)
        {}

        const std::string& name() const {
            return this->m_name;
        }

        bool use_glo() const {
            return this->m_use_glo;
        }

        void max_rate(double value) {
            this->m_max_rate = value;
        }


        /*
          The semantics of the max_rate is quite complicated:

            1. If the std::optional<double> has a value that value should be
               used as the maximum rate and all is fine.

            2. If the std::optional<double> does not a have well we must check
               the value of Well::use_glo():

               False: The maximum gas lift should have been set with WCONPROD /
                  WELTARG - this code does not provide a value in that case.

               True: If the well should be controlled with gas lift optimization
                  the value to use should be the largest ALQ value in the wells
                  VFP table.
        */
        const std::optional<double>& max_rate() const {
            return this->m_max_rate;
        }

        void weight_factor(double value) {
            if (this->m_use_glo)
                this->m_weight = value;
        }

        double weight_factor() const {
            return this->m_weight;
        }

        void inc_weight_factor(double value) {
            if (this->m_use_glo)
                this->m_inc_weight = value;
        }

        double inc_weight_factor() const {
            return this->m_inc_weight;
        }

        void min_rate(double value) {
            if (this->m_use_glo)
                this->m_min_rate = value;
        }

        double min_rate() const {
            return this->m_min_rate;
        }

        void alloc_extra_gas(bool value) {
            if (this->m_use_glo)
                this->m_alloc_extra_gas = value;
        }

        bool alloc_extra_gas() const {
            return this->m_alloc_extra_gas;
        }

        template<class Serializer>
        void serializeOp(Serializer& serializer)
        {
            serializer(m_name);
            serializer(m_use_glo);
            serializer(m_max_rate);
            serializer(m_min_rate);
            serializer(m_weight);
            serializer(m_inc_weight);
            serializer(m_alloc_extra_gas);
        }

        static Well serializeObject() {
            Well well;
            well.m_name = "WELL";
            well.m_max_rate = 2000;
            well.m_min_rate = 56;
            well.m_use_glo = true;
            well.m_weight = 1.25;
            well.m_inc_weight = 0.25;
            well.m_alloc_extra_gas = false;
            return well;
        }

        bool operator==(const Well& other) const {
            return this->m_name == other.m_name &&
                   this->m_max_rate == other.m_max_rate &&
                   this->m_min_rate == other.m_min_rate &&
                   this->m_use_glo  == other.m_use_glo &&
                   this->m_weight   == other.m_weight &&
                   this->m_inc_weight == other.m_inc_weight &&
                   this->m_alloc_extra_gas == other.m_alloc_extra_gas;
        }

    private:
        std::string m_name;
        std::optional<double> m_max_rate;
        double m_min_rate = 0;
        bool m_use_glo;
        double m_weight = 1;
        double m_inc_weight = 0;
        bool m_alloc_extra_gas = false;
    };

    GasLiftOpt() = default;

    const Group& group(const std::string& gname) const;
    const Well& well(const std::string& wname) const;

    double gaslift_increment() const;
    void gaslift_increment(double gaslift_increment);
    double min_eco_gradient() const;
    void min_eco_gradient(double min_eco_gradient);
    void min_wait(double min_wait);
    void all_newton(double all_newton);
    void add_group(const Group& group);
    void add_well(const Well& well);
    bool active() const;

    static GasLiftOpt serializeObject();
    bool operator==(const GasLiftOpt& other) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(m_increment);
        serializer(m_min_eco_gradient);
        serializer(m_min_wait);
        serializer(m_all_newton);
        serializer.map(m_groups);
        serializer.map(m_wells);
    }
private:
    double m_increment = 0;
    double m_min_eco_gradient;
    double m_min_wait;
    bool   m_all_newton = true;

    std::map<std::string, GasLiftOpt::Group> m_groups;
    std::map<std::string, GasLiftOpt::Well> m_wells;
};

}

#endif
