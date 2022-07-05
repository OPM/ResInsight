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

#include <fmt/format.h>
#include <string>


#include <opm/output/eclipse/VectorItems/action.hpp>
#include <opm/common/utility/String.hpp>
#include <opm/input/eclipse/Deck/DeckKeyword.hpp>
#include <opm/input/eclipse/Schedule/Action/ActionValue.hpp>
#include <opm/input/eclipse/Schedule/Action/Condition.hpp>
#include <opm/input/eclipse/Schedule/Action/Enums.hpp>
#include <opm/io/eclipse/rst/action.hpp>

#include "ActionParser.hpp"


namespace Opm {
namespace Action {


namespace {

Comparator comparator(TokenType tt) {
    if (tt == TokenType::op_eq)
        return Comparator::EQUAL;

    if (tt == TokenType::op_gt)
        return Comparator::GREATER;

    if (tt == TokenType::op_lt)
        return Comparator::LESS;

    if (tt == TokenType::op_le)
        return Comparator::LESS_EQUAL;

    if (tt == TokenType::op_ge)
        return Comparator::GREATER_EQUAL;

    return Comparator::INVALID;
}



std::string strip_quotes(const std::string& s) {
    if (s[0] == '\'')
        return s.substr(1, s.size() - 2);
    else
        return s;
}

}

Quantity::Quantity(const std::string& arg) :
    quantity(strip_quotes(arg))
{}


Quantity::Quantity(const RestartIO::RstAction::Quantity& rst_quantity)
{
    if (std::holds_alternative<std::string>(rst_quantity.quantity))
        this->quantity = std::get<std::string>(rst_quantity.quantity);
    else
        this->quantity = format_double(std::get<double>(rst_quantity.quantity));

    if (rst_quantity.wgname.has_value())
        this->add_arg(rst_quantity.wgname.value());
}


void Quantity::add_arg(const std::string& arg) {
    this->args.push_back(strip_quotes(arg));
}

bool Quantity::date() const {
    if (this->quantity == "DAY")
        return true;

    if (this->quantity == "MNTH")
        return true;

    if (this->quantity == "MONTH")
        return true;

    if (this->quantity == "YEAR")
        return true;

    return false;
}

int Quantity::int_type() const {
    namespace QuantityType = Opm::RestartIO::Helpers::VectorItems::IACN::Value;
    const auto& first_char = this->quantity[0];
    if (first_char == 'W')
        return QuantityType::Well;

    if (first_char == 'F')
        return QuantityType::Field;

    if (first_char == 'G')
        return QuantityType::Group;

    if (first_char == 'D')
        return QuantityType::Day;

    if (first_char == 'M')
        return QuantityType::Month;

    if (first_char == 'Y')
        return QuantityType::Year;

    return QuantityType::Const;
}


Condition::Condition(const RestartIO::RstAction::Condition& rst_condition)
    : lhs(rst_condition.lhs)
    , rhs(rst_condition.rhs)
    , logic(rst_condition.logic)
    , cmp(rst_condition.cmp_op)
    , left_paren(rst_condition.left_paren)
    , right_paren(rst_condition.right_paren)
{
}


Condition::Condition(const std::vector<std::string>& tokens, const KeywordLocation& location) {
    std::size_t token_index = 0;
    if (tokens[0] == "(") {
        this->left_paren = true;
        token_index += 1;
    }
    this->lhs = Quantity(tokens[token_index]);
    token_index += 1;

    while (true) {
        if (token_index >= tokens.size())
            break;

        auto comp = comparator( Parser::get_type(tokens[token_index]) );
        if (comp == Comparator::INVALID) {
            this->lhs.add_arg(tokens[token_index]);
            token_index += 1;
        } else {
            this->cmp = comp;
            this->cmp_string = comparator_as_string(this->cmp);
            token_index += 1;
            break;
        }
    }

    if (token_index >= tokens.size())
        throw std::invalid_argument("Could not determine right hand side / comparator for ACTIONX keyword at " + location.filename + ":" + std::to_string(location.lineno));

    this->rhs = Quantity(tokens[token_index]);
    token_index++;

    while (true) {
        if (token_index >= tokens.size())
            break;

        auto token_type = Parser::get_type(tokens[token_index]);
        if (token_type == TokenType::op_and)
            this->logic = Logical::AND;
        else if (token_type == TokenType::op_or)
            this->logic = Logical::OR;
        else if (token_type == TokenType::close_paren)
            this->right_paren = true;
        else
            this->rhs.add_arg(tokens[token_index]);

        token_index++;
    }
}


bool Condition::operator==(const Condition& data) const {
    return lhs == data.lhs &&
           left_paren == data.left_paren &&
           right_paren == data.right_paren &&
           rhs == data.rhs &&
           logic == data.logic &&
           cmp == data.cmp &&
           cmp_string == data.cmp_string;
}

bool Condition::open_paren() const {
    return this->left_paren && !this->right_paren;
}

bool Condition::close_paren() const {
    return !this->left_paren && this->right_paren;
}

int Condition::paren_as_int() const {
    namespace ParenType = Opm::RestartIO::Helpers::VectorItems::IACN::Value;

    if (this->open_paren())
        return ParenType::Open;
    else if (this->close_paren())
        return ParenType::Close;

    return ParenType::None;
}


int Condition::logic_as_int() const {
    switch (this->logic) {
    case Logical::END:
        return 0;
    case Logical::AND:
        return 1;
    case Logical::OR:
        return 2;
    default:
        throw std::logic_error("What the f...?");
    }
}


int Condition::comparator_as_int() const {
    switch (this->cmp) {
    case Comparator::GREATER:
        return 1;
    case Comparator::LESS:
        return 2;
    case Comparator::GREATER_EQUAL:
        return 3;
    case Comparator::LESS_EQUAL:
        return 4;
    case Comparator::EQUAL:
        return 5;
    default:
        throw std::logic_error(fmt::format("Unhandeled value: {} in enum comparison", static_cast<int>(this->cmp)));
    }
}

}
}
