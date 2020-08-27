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

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

#include <opm/parser/eclipse/EclipseState/SummaryConfig/SummaryConfig.hpp>

#include "ActionParser.hpp"

namespace Opm {
namespace Action {

Parser::Parser(const std::vector<std::string>& tokens_arg) :
    tokens(tokens_arg)
{}


TokenType Parser::get_type(const std::string& arg) {
    std::string lower_arg = arg;
    std::for_each(lower_arg.begin(),
                  lower_arg.end(),
                  [](char& c) {
                      c = std::tolower(static_cast<unsigned char>(c));
                  });

    if (lower_arg == "and")
        return TokenType::op_and;

    if (lower_arg == "or")
        return TokenType::op_or;

    if (lower_arg == "(")
        return TokenType::open_paren;

    if (lower_arg == ")")
        return TokenType::close_paren;

    if (lower_arg == ">" || lower_arg == ".gt.")
        return TokenType::op_gt;

    if (lower_arg == ">=" || lower_arg == ".ge.")
        return TokenType::op_ge;

    if (lower_arg == "<=" || lower_arg == ".le.")
        return TokenType::op_le;

    if (lower_arg == "<" || lower_arg == ".lt.")
        return TokenType::op_lt;

    if (lower_arg == "<=" || lower_arg == ".le.")
        return TokenType::op_le;

    if (lower_arg == "=" || lower_arg == ".eq.")
        return TokenType::op_eq;

    if (lower_arg == "!=" || lower_arg == ".ne.")
        return TokenType::op_ne;

    {
        char * end_ptr;
        std::strtod(lower_arg.c_str(), &end_ptr);
        if (std::strlen(end_ptr) == 0)
            return TokenType::number;
    }

    return TokenType::ecl_expr;
}

FuncType Parser::get_func(const std::string& arg) {

    if (arg == "YEAR") return FuncType::time;
    if (arg == "MNTH") return FuncType::time;
    if (arg == "DAY")  return FuncType::time;

    using Cat = SummaryConfigNode::Category;
    SummaryConfigNode::Category cat = parseKeywordCategory(arg);
    switch (cat) {
        case Cat::Aquifer:    return FuncType::aquifer;
        case Cat::Well:       return FuncType::well;
        case Cat::Group:      return FuncType::group;
        case Cat::Connection: return FuncType::well_connection;
        case Cat::Region:     return FuncType::region;
        case Cat::Block:      return FuncType::block;
        case Cat::Segment:    return FuncType::well_segment;
        default:              return FuncType::none;
    }
}


ParseNode Parser::next() {
    this->current_pos++;
    if (static_cast<size_t>(this->current_pos) == this->tokens.size())
        return TokenType::end;

    std::string arg = this->tokens[this->current_pos];
    return ParseNode(get_type(arg), arg);
}


ParseNode Parser::current() const {
    if (static_cast<size_t>(this->current_pos) == this->tokens.size())
        return TokenType::end;

    std::string arg = this->tokens[this->current_pos];
    return ParseNode(get_type(arg), arg);
}


Action::ASTNode Parser::parse_left() {
    auto current = this->current();
    if (current.type != TokenType::ecl_expr)
        return TokenType::error;

    std::string func = current.value;
    FuncType func_type = get_func(current.value);
    std::vector<std::string> arg_list;
    current = this->next();
    while (current.type == TokenType::ecl_expr || current.type == TokenType::number) {
        arg_list.push_back(current.value);
        current = this->next();
    }

    return Action::ASTNode(TokenType::ecl_expr, func_type, func, arg_list);
}

Action::ASTNode Parser::parse_op() {
    auto current = this->current();
    if (current.type == TokenType::op_gt ||
        current.type == TokenType::op_ge ||
        current.type == TokenType::op_lt ||
        current.type == TokenType::op_le ||
        current.type == TokenType::op_eq ||
        current.type == TokenType::op_ne) {
        this->next();
        return current.type;
    }
    return TokenType::error;
}


Action::ASTNode Parser::parse_right() {
    auto current = this->current();
    if (current.type == TokenType::number) {
        this->next();
        return Action::ASTNode( strtod(current.value.c_str(), nullptr) );
    }

    current = this->current();
    if (current.type != TokenType::ecl_expr)
        return TokenType::error;

    std::string func = current.value;
    FuncType func_type = FuncType::none;
    std::vector<std::string> arg_list;
    current = this->next();
    while (current.type == TokenType::ecl_expr || current.type == TokenType::number) {
        arg_list.push_back(current.value);
        current = this->next();
    }
    return Action::ASTNode(TokenType::ecl_expr, func_type, func, arg_list);
}



Action::ASTNode Parser::parse_cmp() {
    auto current = this->current();

    if (current.type == TokenType::open_paren) {
        this->next();
        auto inner_expr = this->parse_or();

        current = this->current();
        if (current.type != TokenType::close_paren)
            return TokenType::error;

        this->next();
        return inner_expr;
    } else {
        auto left_node = this->parse_left();
        if (left_node.type == TokenType::error)
            return TokenType::error;

        auto op_node = this->parse_op();
        if (op_node.type == TokenType::error)
            return TokenType::error;

        auto right_node = this->parse_right();
        if (right_node.type == TokenType::error)
            return TokenType::error;

        op_node.add_child(left_node);
        op_node.add_child(right_node);
        return op_node;
    }
}

Action::ASTNode Parser::parse_and() {
    auto left = this->parse_cmp();
    if (left.type == TokenType::error)
        return TokenType::error;

    auto current = this->current();
    if (current.type == TokenType::op_and) {
        Action::ASTNode and_node(TokenType::op_and);
        and_node.add_child(left);

        while (this->current().type == TokenType::op_and) {
            this->next();
            auto next_cmp = this->parse_cmp();
            if (next_cmp.type == TokenType::error)
                return TokenType::error;

            and_node.add_child(next_cmp);
        }
        return and_node;
    }

    return left;
}


Action::ASTNode Parser::parse_or() {
    auto left = this->parse_and();
    if (left.type == TokenType::error)
        return TokenType::error;

    auto current = this->current();
    if (current.type == TokenType::op_or) {
        Action::ASTNode or_node(TokenType::op_or);
        or_node.add_child(left);

        while (this->current().type == TokenType::op_or) {
            this->next();
            auto next_cmp = this->parse_or();
            if (next_cmp.type == TokenType::error)
                return TokenType::error;

            or_node.add_child(next_cmp);
        }
        return or_node;
    }

    return left;
}


Action::ASTNode Parser::parse(const std::vector<std::string>& tokens) {
    Parser parser(tokens);
    parser.next();

    auto tree = parser.parse_or();
    auto current = parser.current();
    if (current.type != TokenType::end) {
        size_t index = parser.current_pos;
        throw std::invalid_argument("Extra unhandled data starting with token[" + std::to_string(index) + "] = " + current.value);
    }

    if (tree.type == TokenType::error)
        throw std::invalid_argument("Failed to parse");

    return tree;
}
}
}

