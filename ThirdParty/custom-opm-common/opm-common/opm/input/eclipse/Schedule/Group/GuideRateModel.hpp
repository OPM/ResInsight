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

#ifndef GUIDE_RATE_MODEL_HPP
#define GUIDE_RATE_MODEL_HPP

#include <opm/input/eclipse/Deck/UDAValue.hpp>
#include <opm/input/eclipse/Schedule/Group/Group.hpp>
#include <opm/input/eclipse/Schedule/Well/Well.hpp>

namespace Opm {

class GuideRateModel {
public:

    enum class Target {
        OIL = 0,
        LIQ = 1,
        GAS = 2,
        WAT = 3,
        RES = 4,
        COMB = 5,
        NONE = 6
    };

    static Target TargetFromString(const std::string& s);
    static Target TargetFromRestart(const int nominated_phase);

    GuideRateModel(double time_interval_arg,
                   Target target_arg,
                   double A_arg,
                   double B_arg,
                   double C_arg,
                   double D_arg,
                   double E_arg,
                   double F_arg,
                   bool allow_increase_arg,
                   double damping_factor_arg,
                   bool use_free_gas_arg);
    GuideRateModel() = default;

    static bool rst_valid(double time_interval,
                          double A,
                          double B,
                          double C,
                          double D,
                          double E,
                          double F,
                          double damping_factor);


    static GuideRateModel serializeObject();

    double eval(double oil_pot, double gas_pot, double wat_pot) const;
    bool updateLINCOM(const UDAValue& alpha, const UDAValue& beta, const UDAValue& gamma) const;
    double update_delay() const;
    bool allow_increase() const;
    double damping_factor() const;
    bool operator==(const GuideRateModel& other) const;
    bool operator!=(const GuideRateModel& other) const;
    Target target() const;
    double getA() const;
    double getB() const;
    double getC() const;
    double getD() const;
    double getE() const;
    double getF() const;

    static Target convert_target(Well::GuideRateTarget well_target);
    static Target convert_target(Group::GuideRateProdTarget group_target);
    static Target convert_target(Phase injection_phase);
    static double pot(Target target, double oil_pot, double gas_pot, double wat_pot);

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(time_interval);
        serializer(m_target),
        serializer(A);
        serializer(B);
        serializer(C);
        serializer(D);
        serializer(E);
        serializer(F);
        serializer(allow_increase_);
        serializer(damping_factor_);
        serializer(use_free_gas);
        serializer(default_model);
        alpha.serializeOp(serializer);
        beta.serializeOp(serializer);
        gamma.serializeOp(serializer);
    }

private:
    double pot(double oil_pot, double gas_pot, double wat_pot) const;
    /*
      Unfortunately the default values will give a GuideRateModel which can not
      be evaluated, due to a division by zero problem.
    */
    double time_interval = 0;
    Target m_target = Target::NONE;
    double A = 0;
    double B = 0;
    double C = 0;
    double D = 0;
    double E = 0;
    double F = 0;
    bool allow_increase_ = true;
    double damping_factor_ = 1.0;
    bool use_free_gas = false;
    bool default_model = true;
    UDAValue alpha;
    UDAValue beta;
    UDAValue gamma;
};

}

#endif
