/*
  Copyright 2021 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <cmath>

#include <opm/io/eclipse/rst/action.hpp>
#include <opm/output/eclipse/VectorItems/action.hpp>
#include <opm/common/utility/String.hpp>
#include <opm/common/utility/TimeService.hpp>

namespace Opm {
namespace RestartIO {

namespace IACN = Helpers::VectorItems::IACN;
namespace SACN = Helpers::VectorItems::SACN;
namespace ZACN = Helpers::VectorItems::ZACN;


RstAction::RstAction(const std::string& name_arg, int max_run_arg, int run_count_arg, double min_wait_arg, std::time_t start_time_arg, std::time_t last_run_arg, std::vector<RstAction::Condition> conditions_arg)
    : name(name_arg)
    , max_run(max_run_arg)
    , run_count(run_count_arg)
    , min_wait(min_wait_arg)
    , start_time(start_time_arg)
    , conditions(conditions_arg)
{
    if (this->run_count > 0)
        this->last_run = last_run_arg;
}


RstAction::Quantity::Quantity(const std::string * zacn, double sacn_value) {
    auto str_quantity = trim_copy(zacn[ZACN::Quantity]);
    if (str_quantity.empty()) {
        this->quantity = sacn_value;
        return;
    }

    this->quantity = str_quantity;
    if (str_quantity[0] == 'W')
        this->wgname = trim_copy(zacn[ZACN::Well]);
    else if (str_quantity[0] == 'G')
        this->wgname = trim_copy(zacn[ZACN::Group]);
}

RstAction::Quantity::Quantity(const std::string& q)
    : quantity(q)
{
}

RstAction::Quantity::Quantity(double value)
    : quantity(value)
{
}



RstAction::Condition::Condition(const std::string * zacn, const int * iacn, const double * sacn)
    : logic(Action::logic_from_int(iacn[IACN::TerminalLogic]))
    , cmp_op(Action::comparator_from_int(iacn[IACN::Comparator]))
{
    auto type_index = IACN::LHSQuantityType;
    auto rhs_value = sacn[SACN::RHSValue0];
    if (iacn[type_index] == IACN::Value::Day) {
        this->lhs = RstAction::Quantity("DAY");
        this->rhs = RstAction::Quantity(rhs_value);
        return;
    }

    if (iacn[type_index] == IACN::Value::Month) {
        this->lhs = RstAction::Quantity("MNTH");
        this->rhs = RstAction::Quantity(TimeService::eclipseMonthNames().at(static_cast<int>(rhs_value)));
        return;
    }

    if (iacn[type_index] == IACN::Value::Year) {
        this->lhs = RstAction::Quantity("YEAR");
        this->rhs = RstAction::Quantity(rhs_value);
        return;
    }

    this->lhs = RstAction::Quantity(zacn, sacn[SACN::LHSValue0]);
    this->rhs = RstAction::Quantity(&zacn[ZACN::RHSOffset], sacn[SACN::RHSValue0]);
    this->left_paren = (iacn[IACN::Paren] == IACN::Value::Open);
    this->right_paren = (iacn[IACN::Paren] == IACN::Value::Close);
}

bool RstAction::Condition::valid(const std::string * zacn, const int * iacn) {
    auto type_index = IACN::LHSQuantityType;
    if (iacn[type_index] == IACN::Value::Day)
        return true;

    if (iacn[type_index] == IACN::Value::Month)
        return true;

    if (iacn[type_index] == IACN::Value::Year)
        return true;

    auto str_quantity = trim_copy(zacn[ZACN::LHSQuantity]);
    return !str_quantity.empty();
}

std::vector<std::string> RstAction::Condition::tokens() const {
    std::vector<std::string> tokens;
    if (this->left_paren)
        tokens.push_back("(");

    tokens.push_back(std::get<std::string>(this->lhs.quantity));
    if (this->lhs.wgname.has_value())
        tokens.push_back(this->lhs.wgname.value());

    tokens.push_back(Action::comparator_as_string(this->cmp_op));

    if (std::holds_alternative<std::string>(this->rhs.quantity))
        tokens.push_back(std::get<std::string>(this->rhs.quantity));
    else
        tokens.push_back(format_double(std::get<double>(this->rhs.quantity)));

    if (this->rhs.wgname.has_value())
        tokens.push_back(this->rhs.wgname.value());

    if (this->right_paren)
        tokens.push_back(")");

    if (this->logic == Action::Logical::AND)
        tokens.push_back("AND");

    if (this->logic == Action::Logical::OR)
        tokens.push_back("OR");

    return tokens;
}

}
}
