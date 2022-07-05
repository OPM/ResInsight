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
#include <stdexcept>
#include <cmath>
#include <string>
#include <unordered_map>

#include <opm/input/eclipse/Parser/ParserKeywords/L.hpp>
#include <opm/input/eclipse/Schedule/Group/GuideRateModel.hpp>

namespace Opm {

GuideRateModel::GuideRateModel(double time_interval_arg,
                               Target target_arg,
                               double A_arg,
                               double B_arg,
                               double C_arg,
                               double D_arg,
                               double E_arg,
                               double F_arg,
                               bool allow_increase_arg,
                               double damping_factor_arg,
                               bool use_free_gas_arg) :
    time_interval(time_interval_arg),
    m_target(target_arg),
    A(A_arg),
    B(B_arg),
    C(C_arg),
    D(D_arg),
    E(E_arg),
    F(F_arg),
    allow_increase_(allow_increase_arg),
    damping_factor_(damping_factor_arg),
    use_free_gas(use_free_gas_arg),
    default_model(false),
    alpha(UDAValue(ParserKeywords::LINCOM::ALPHA::defaultValue)),
    beta(UDAValue(ParserKeywords::LINCOM::BETA::defaultValue)),
    gamma(UDAValue(ParserKeywords::LINCOM::GAMMA::defaultValue))
{
    if (this->A > 3 || this->A < -3)
        throw std::invalid_argument("Invalid value for A must be in interval [-3,3]");

    if (this->B < 0)
        throw std::invalid_argument("Invalid value for B must be > 0");

    if (this->D > 3 || this->D < -3)
        throw std::invalid_argument("Invalid value for D must be in interval [-3,3]");

    if (this->F > 3 || this->F < -3)
        throw std::invalid_argument("Invalid value for F must be in interval [-3,3]");
}

GuideRateModel GuideRateModel::serializeObject()
{
    GuideRateModel result;
    result.time_interval = 1.0;
    result.m_target = Target::WAT;
    result.A = 2.0;
    result.B = 3.0;
    result.C = 4.0;
    result.D = 5.0;
    result.E = 6.0;
    result.F = 7.0;
    result.allow_increase_ = false;
    result.damping_factor_ = 8.0;
    result.use_free_gas = true;
    result.default_model = false;
    result.alpha = UDAValue(9.0);
    result.beta = UDAValue(10.0);
    result.gamma = UDAValue(11.0);

    return result;
}

bool GuideRateModel::rst_valid(double time_interval,
                               double A,
                               double B,
                               double C,
                               double D,
                               double E,
                               double F,
                               double damping_factor) {
    if (time_interval == 0 &&
        A == 0 &&
        B == 0 &&
        C == 0 &&
        D == 0 &&
        E == 0 &&
        F == 0 &&
        damping_factor == 0)
        return false;

    return true;
}



double GuideRateModel::pot(double oil_pot, double gas_pot, double wat_pot) const {
    return pot(this->target(), oil_pot, gas_pot, wat_pot);
}

double GuideRateModel::pot(Target target, double oil_pot, double gas_pot, double wat_pot) {
    switch (target) {
    case Target::OIL:
        return oil_pot;

    case Target::LIQ:
        return oil_pot + wat_pot;

    case Target::GAS:
        return gas_pot;

    case Target::WAT:
        return wat_pot;

    case Target::COMB:
        throw std::logic_error("Not implemented - don't have a clue?");

    case Target::RES:
        return oil_pot + wat_pot + gas_pot;

    default:
        throw std::logic_error("Hmmm - should not be here?");
    }
}


double GuideRateModel::eval(double oil_pot, double gas_pot, double wat_pot) const {
    if (this->default_model)
        throw std::invalid_argument("The default GuideRateModel can not be evaluated - must enter GUIDERAT information explicitly.");

    if (this->m_target == Target::COMB)
        throw std::logic_error("Sorry the COMB target model is not supported");

    double pot = this->pot(oil_pot, gas_pot, wat_pot);
    if (pot == 0)
        return 0;


    double R1;
    double R2;
    switch (this->m_target) {
    case Target::OIL:
        R1 = wat_pot / oil_pot;
        R2 = gas_pot / oil_pot;
        break;

    case Target::LIQ:
        R1 = wat_pot / (oil_pot + wat_pot);
        R2 = gas_pot / (oil_pot + wat_pot);
        break;

    case Target::GAS:
        R1 = wat_pot / gas_pot;
        R2 = oil_pot / gas_pot;
        break;

    case Target::COMB:
        throw std::logic_error("Not implemented - don't have a clue?");

    case Target::RES:
        R1 = wat_pot / oil_pot;
        R2 = gas_pot / oil_pot;
        break;

    default:
        throw std::logic_error("Hmmm - should not be here?");
    }


    double denom = this->B + this->C*std::pow(R1, this->D) + this->E*std::pow(R2, this->F);
    /*
      The values pot, R1 and R2 are runtime simulation results, so here
      basically anything could happen. Quite dangerous to have hard error
      handling here?
    */
    if (denom <= 0)
        throw std::range_error("Invalid denominator: " + std::to_string(denom));

    return std::pow(pot, this->A) / denom;
}

bool GuideRateModel::operator==(const GuideRateModel& other) const {
    return (this->time_interval == other.time_interval) &&
        (this->m_target == other.m_target) &&
        (this->A == other.A) &&
        (this->B == other.B) &&
        (this->C == other.C) &&
        (this->D == other.D) &&
        (this->E == other.E) &&
        (this->F == other.F) &&
        (this->allow_increase_ == other.allow_increase_) &&
        (this->damping_factor_ == other.damping_factor_) &&
        (this->use_free_gas == other.use_free_gas);
}

bool GuideRateModel::operator!=(const GuideRateModel& other) const {
    return !(*this == other);
}


double GuideRateModel::update_delay() const {
    return this->time_interval;
}

double GuideRateModel::damping_factor() const {
    return this->damping_factor_;
}

double GuideRateModel::getA() const {
    return this->A;
}

double GuideRateModel::getB() const {
    return this->B;
}

double GuideRateModel::getC() const {
    return this->C;
}

double GuideRateModel::getD() const {
    return this->D;
}

double GuideRateModel::getE() const {
    return this->E;
}

double GuideRateModel::getF() const {
    return this->F;
}

bool GuideRateModel::allow_increase() const {
    return this->allow_increase_;
}

GuideRateModel::Target GuideRateModel::target() const {
    return this->m_target;
}


GuideRateModel::Target GuideRateModel::TargetFromString(const std::string& s) {
    if (s == "OIL")
        return Target::OIL;

    if (s == "LIQ")
        return Target::LIQ;

    if (s == "GAS")
        return Target::GAS;

    if (s == "RES")
        return Target::RES;

    if (s == "COMB")
        return Target::COMB;

    if (s == "NONE")
        return Target::NONE;

    throw std::invalid_argument("Could not convert: " + s + " to a valid Target enum value");
}

GuideRateModel::Target GuideRateModel::TargetFromRestart(const int nominated_phase) {
    static const auto int_to_target = std::unordered_map<int, Target> {
        {0, Target::NONE},
        {1, Target::OIL },
        {3, Target::GAS },
        {4, Target::LIQ },
        {6, Target::RES },
        {9, Target::COMB},
    };

    auto t = int_to_target.find(nominated_phase);
    return (t == int_to_target.end()) ? Target::NONE : t->second;
}

/*
  The COMB target - which requires parameters from the LINCOM keyword, is not
  supported. There are at least two pieces of missing functionality:

    1. The parameters in the LINCOM come with unit specified by the LCUNIT
       keyword; seemingly decoupled from the unit system in the rest of deck.
       This functionality is not supported.

    2. The values in the LINCOM kewyords are UDA values, have not yet integrated
       the necessary SummaryState into this.
*/

bool GuideRateModel::updateLINCOM(const UDAValue& , const UDAValue& , const UDAValue& ) const {
    if (this->m_target == Target::COMB)
        throw std::logic_error("The LINCOM keyword is not supported - at all!");

    return true;
}


GuideRateModel::Target GuideRateModel::convert_target(Well::GuideRateTarget well_target) {
    if (well_target == Well::GuideRateTarget::OIL)
        return Target::OIL;

    if (well_target == Well::GuideRateTarget::GAS)
        return Target::GAS;

    if (well_target == Well::GuideRateTarget::LIQ)
        return Target::LIQ;

    if (well_target == Well::GuideRateTarget::WAT)
        return Target::WAT;

    if (well_target == Well::GuideRateTarget::RES)
        return Target::RES;

    throw std::logic_error("Can not convert this .... ");
}

GuideRateModel::Target GuideRateModel::convert_target(Group::GuideRateProdTarget group_target) {
    if (group_target == Group::GuideRateProdTarget::OIL)
        return Target::OIL;

    if (group_target == Group::GuideRateProdTarget::GAS)
        return Target::GAS;

    if (group_target == Group::GuideRateProdTarget::LIQ)
        return Target::LIQ;

    if (group_target == Group::GuideRateProdTarget::WAT)
        return Target::WAT;

    if (group_target == Group::GuideRateProdTarget::RES)
        return Target::RES;

    throw std::logic_error("Can not convert this .... ");
}

GuideRateModel::Target GuideRateModel::convert_target(Phase injection_phase) {
    if (injection_phase == Phase::OIL)
        return Target::OIL;

    if (injection_phase == Phase::GAS)
        return Target::GAS;

    if (injection_phase == Phase::WATER)
        return Target::WAT;

    throw std::logic_error("Can not convert this .... ");
}

}
