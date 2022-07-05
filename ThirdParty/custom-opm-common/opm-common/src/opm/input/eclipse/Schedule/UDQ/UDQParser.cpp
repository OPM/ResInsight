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

#include <iostream>
#include <cstring>
#include <cassert>

#include <fmt/format.h>

#include <opm/input/eclipse/Parser/ParseContext.hpp>
#include <opm/common/OpmLog/KeywordLocation.hpp>

#include "UDQParser.hpp"

namespace Opm {

UDQTokenType UDQParser::get_type(const std::string& arg) const {
    auto func_type = UDQ::funcType(arg);
    if (func_type == UDQTokenType::table_lookup)
        throw std::invalid_argument("Table lookup function TU*[] is not supported in UDQ");

    if (func_type != UDQTokenType::error)
        return func_type;

    if (arg == "(")
        return UDQTokenType::open_paren;

    if (arg == ")")
        return UDQTokenType::close_paren;

    {
        char * end_ptr;
        std::strtod(arg.c_str(), &end_ptr);
        if (std::strlen(end_ptr) == 0)
            return UDQTokenType::number;
    }

    return UDQTokenType::ecl_expr;
}



bool UDQParser::empty() const {
    return (static_cast<size_t>(this->current_pos) == this->tokens.size());
}

UDQParseNode UDQParser::next() {
    this->current_pos += 1;
    return this->current();
}



UDQParseNode UDQParser::current() const {
    if (this->empty())
        return UDQTokenType::end;

    const auto& token = this->tokens[current_pos];
    if (token.type() == UDQTokenType::number)
        return UDQParseNode(UDQTokenType::number, token.value());

    if (token.type() == UDQTokenType::ecl_expr)
        return UDQParseNode(UDQTokenType::ecl_expr, token.value(), token.selector());

    return UDQParseNode(this->get_type(std::get<std::string>(token.value())), token.value());
}


UDQASTNode UDQParser::parse_factor() {
    double sign = 1.0;
    auto current = this->current();
    if (current.type == UDQTokenType::binary_op_add || current.type == UDQTokenType::binary_op_sub) {
        if (current.type == UDQTokenType::binary_op_sub)
            sign = -1.0;
        this->next();
        current = this->current();
    }


    if (current.type == UDQTokenType::open_paren) {
        this->next();
        auto inner_expr = this->parse_set();

        current = this->current();
        if (current.type != UDQTokenType::close_paren)
            return UDQASTNode(UDQTokenType::error);

        this->next();
        return sign * inner_expr;
    }

    if (UDQ::scalarFunc(current.type) || UDQ::elementalUnaryFunc(current.type)) {
        auto func_node = current;
        auto next = this->next();
        if (next.type == UDQTokenType::open_paren) {
            this->next();
            auto arg_expr = this->parse_set();

            current = this->current();
            if (current.type != UDQTokenType::close_paren)
                return UDQASTNode(UDQTokenType::error);

            this->next();
            return sign * UDQASTNode(func_node.type, func_node.value, arg_expr);
        } else
            return UDQASTNode(UDQTokenType::error);
    }

    UDQASTNode node(current.type, current.value, current.selector);
    this->next();
    return sign * node;
}

UDQASTNode UDQParser::parse_pow() {
    auto left = this->parse_factor();
    if (this->empty())
        return left;

    auto current = this->current();
    if (current.type == UDQTokenType::binary_op_pow) {
        this->next();
        if (this->empty())
            return UDQASTNode(UDQTokenType::error);

        auto right = this->parse_mul();
        return UDQASTNode(current.type, current.value, left, right);
    }

    return left;
}



UDQASTNode UDQParser::parse_mul() {
    std::vector<UDQASTNode> nodes;
    {
        std::unique_ptr<UDQASTNode> current_node;
        while (true) {
            auto node = this->parse_pow();
            if (current_node) {
                current_node->set_right(node);
                nodes.push_back(*current_node);
            } else
                nodes.push_back(node);

            if (this->empty())
                break;

            auto current_token = this->current();
            if (current_token.type == UDQTokenType::binary_op_mul || current_token.type == UDQTokenType::binary_op_div) {
                current_node.reset( new UDQASTNode(current_token.type, current_token.value) );
                this->next();
                if (this->empty())
                    return UDQASTNode( UDQTokenType::error );
            } else break;
        }
    }

    UDQASTNode top_node = nodes.back();
    if (nodes.size() > 1) {
        UDQASTNode * current = &top_node;
        for (std::size_t index = nodes.size() - 1; index > 0; index--) {
            current->set_left(nodes[index - 1]);
            current = current->get_left();
        }
    }
    return top_node;
}


UDQASTNode UDQParser::parse_add() {
    std::vector<UDQASTNode> nodes;
    {
        std::unique_ptr<UDQASTNode> current_node;
        while (true) {
            auto node = this->parse_mul();
            if (current_node) {
                current_node->set_right(node);
                nodes.push_back(*current_node);
            } else
                nodes.push_back(node);

            if (this->empty())
                break;

            auto current_token = this->current();
            if (current_token.type == UDQTokenType::binary_op_add || current_token.type == UDQTokenType::binary_op_sub) {
                current_node.reset( new UDQASTNode(current_token.type, current_token.value) );
                this->next();
                if (this->empty())
                    return UDQASTNode( UDQTokenType::error );
            } else if (current_token.type == UDQTokenType::close_paren || UDQ::cmpFunc(current_token.type) || UDQ::setFunc(current_token.type))
                break;
            else
                return UDQASTNode( UDQTokenType::error );
        }
    }

    UDQASTNode top_node = nodes.back();
    if (nodes.size() > 1) {
        UDQASTNode * current = &top_node;
        for (std::size_t index = nodes.size() - 1; index > 0; index--) {
            current->set_left(nodes[index - 1]);
            current = current->get_left();
        }
    }
    return top_node;
}


/*
  A bit uncertain on the presedence of the comparison operators. In normal C the
  comparion operators bind weaker than addition, i.e. for the assignment:

     auto cmp = a + b < c;

  The sum (a+b) is evaluated and then compared with c, that is the order of
  presedence implemented here. But reading the eclipse UDQ manual one can get
  the imporession that the relation operators should bind "very strong", i.e.
  that (b < c) should be evaluated first, and then the result of the comparison
  added to a.
*/

UDQASTNode UDQParser::parse_cmp() {
    auto left = this->parse_add();
    if (this->empty())
        return left;

    auto current = this->current();
    if (UDQ::cmpFunc(current.type)) {
        auto func_node = current;
        this->next();
        if (this->empty())
            return UDQASTNode(UDQTokenType::error);

        auto right = this->parse_cmp();
        return UDQASTNode(current.type, current.value, left, right);
    }
    return left;
}


UDQASTNode UDQParser::parse_set() {
    auto left = this->parse_cmp();
    if (this->empty())
        return left;

    auto current = this->current();
    if (UDQ::setFunc(current.type)) {
        auto func_node = current;
        this->next();
        if (this->empty())
            return UDQASTNode(UDQTokenType::error);

        auto right = this->parse_set();
        return UDQASTNode(current.type, current.value, left, right);
    }
    return left;
}


namespace {
    void dump_tokens(const std::string& target_var, const std::vector<UDQToken>& tokens) {
        std::cout << target_var << " = ";
        for (const auto& token : tokens)
            std::cout << token.str();
        std::cout << std::endl;
    }

/*
  This function is extremely weak - hopefully it can be improved in the future.
  See the comment in UDQEnums.hpp about 'UDQ type system'.
*/
bool static_type_check(UDQVarType lhs, UDQVarType rhs) {
    if (lhs == rhs)
        return true;

    if (rhs == UDQVarType::SCALAR)
        return true;

    /*
      This does not check if the rhs evaluates to a scalar.
    */
    if (rhs == UDQVarType::WELL_VAR)
        return (lhs == UDQVarType::WELL_VAR);

    return false;
}
}


UDQASTNode UDQParser::parse(const UDQParams& udq_params, UDQVarType target_type, const std::string& target_var, const KeywordLocation& location, const std::vector<UDQToken>& tokens, const ParseContext& parseContext, ErrorGuard& errors)
{
    UDQParser parser(udq_params, tokens);
    parser.next();
    auto tree = parser.parse_set();

    if (!parser.empty()) {
        auto current = parser.current();
        std::string msg_fmt = fmt::format("Problem parsing UDQ {}\n"
                                          "In {{file}} line {{line}}.\n"
                                          "Extra unhandled data starting with item {}.", target_var, current.string());
        parseContext.handleError(ParseContext::UDQ_PARSE_ERROR, msg_fmt, location, errors);
        return UDQASTNode( udq_params.undefinedValue() );
    }

    if (!tree.valid()) {
        std::string token_string;
        for (const auto& token : tokens)
            token_string += token.str() + " ";

        std::string msg_fmt = fmt::format("Failed to parse UDQ {}\n"
                                          "In {{file}} line {{line}}.\n"
                                          "This can be a bug in flow or a bug in the UDQ input string.\n"
                                          "UDQ input: '{}'", target_var, token_string);
        parseContext.handleError(ParseContext::UDQ_PARSE_ERROR, msg_fmt, location, errors);
        return UDQASTNode( udq_params.undefinedValue() );
    }

    if (!static_type_check(target_type, tree.var_type)) {
        std::string msg_fmt = fmt::format("Failed to parse UDQ {}\n"
                                          "In {{file}} line {{line}}.\n"
                                          "Invalid type conversion detected in UDQ expression expected: {}  got: {}", target_var, UDQ::typeName(target_type), UDQ::typeName(tree.var_type));

        parseContext.handleError(ParseContext::UDQ_TYPE_ERROR, msg_fmt, location, errors);
        if (parseContext.get(ParseContext::UDQ_TYPE_ERROR) != InputError::IGNORE)
            dump_tokens(target_var, tokens);

        return UDQASTNode( udq_params.undefinedValue() );
    }

    if (tree.var_type == UDQVarType::NONE) {
        std::string msg_fmt = fmt::format("Failed to parse UDQ {}\n"
                                          "In {{file}} line {{line}}.\n"
                                          "Could not determine expression type.", target_var);
        parseContext.handleError(ParseContext::UDQ_TYPE_ERROR, msg_fmt, location, errors);
        if (parseContext.get(ParseContext::UDQ_TYPE_ERROR) != InputError::IGNORE)
            dump_tokens(target_var, tokens);

        return UDQASTNode( udq_params.undefinedValue() );
    }

    return tree;
}

}
