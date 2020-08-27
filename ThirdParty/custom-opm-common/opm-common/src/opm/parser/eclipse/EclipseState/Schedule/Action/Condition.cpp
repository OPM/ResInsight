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

#include <string>

#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionValue.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/Condition.hpp>

#include "ActionParser.hpp"


namespace Opm {
namespace Action {


namespace {

Condition::Comparator comparator(TokenType tt) {
    if (tt == TokenType::op_eq)
        return Condition::Comparator::EQUAL;

    if (tt == TokenType::op_gt)
        return Condition::Comparator::GREATER;

    if (tt == TokenType::op_lt)
        return Condition::Comparator::LESS;

    if (tt == TokenType::op_le)
        return Condition::Comparator::LESS_EQUAL;

    if (tt == TokenType::op_ge)
        return Condition::Comparator::GREATER_EQUAL;

    return Condition::Comparator::INVALID;
}


std::string cmp2string(Condition::Comparator cmp) {
    if (cmp == Condition::Comparator::EQUAL)
        return "=";

    if (cmp == Condition::Comparator::GREATER)
        return ">";

    if (cmp == Condition::Comparator::LESS)
        return "<";

    if (cmp == Condition::Comparator::LESS_EQUAL)
        return "<=";

    if (cmp == Condition::Comparator::GREATER_EQUAL)
        return ">=";

    throw std::logic_error("Bug in opm/flow - should not be here");
}

std::string strip_quotes(const std::string& s) {
    if (s[0] == '\'')
        return s.substr(1, s.size() - 2);
    else
        return s;
}

}

void Quantity::add_arg(const std::string& arg) {
    this->args.push_back(strip_quotes(arg));
}

Condition::Condition(const std::vector<std::string>& tokens, const Location& location) {
    this->lhs = Quantity(tokens[0]);
    std::size_t token_index = 1;

    while (true) {
        if (token_index >= tokens.size())
            break;

        auto comp = comparator( Parser::get_type(tokens[token_index]) );
        if (comp == Comparator::INVALID) {
            this->lhs.add_arg(tokens[token_index]);
            token_index += 1;
        } else {
            this->cmp = comp;
            this->cmp_string = cmp2string(this->cmp);
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
            this->logic = Condition::Logical::AND;
        else if (token_type == TokenType::op_or)
            this->logic = Condition::Logical::OR;
        else
            this->rhs.add_arg(tokens[token_index]);

        token_index++;
    }
}


bool Condition::operator==(const Condition& data) const {
    return lhs == data.lhs &&
           rhs == data.rhs &&
           logic == data.logic &&
           cmp == data.cmp &&
           cmp_string == data.cmp_string;
}


}
}
