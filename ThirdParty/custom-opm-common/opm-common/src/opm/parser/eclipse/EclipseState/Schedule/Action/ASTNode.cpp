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

#ifdef _WIN32
#include "cross-platform/windows/Substitutes.hpp"
#else
#include <fnmatch.h>
#endif

#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionContext.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ActionValue.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Action/ASTNode.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WList.hpp>

#include <stdexcept>

namespace {
    std::string strip_quotes(const std::string& s) {
        if (s[0] == '\'')
            return s.substr(1, s.size() - 2);
        else
            return s;
    }

    std::vector<std::string> strip_quotes(const std::vector<std::string>& quoted_strings) {
        std::vector<std::string> strings;
        for (const auto& qs : quoted_strings)
            strings.push_back(strip_quotes(qs));

        return strings;
    }

}

namespace Opm {
namespace Action {

ASTNode::ASTNode() :
    type(TokenType::error)
{}


ASTNode::ASTNode(TokenType type_arg)
    :
    type(type_arg)
{}


ASTNode::ASTNode(double value) :
    type(TokenType::number),
    number(value)
{}


ASTNode::ASTNode(TokenType type_arg,
                 FuncType func_type_arg,
                 const std::string& func_arg,
                 const std::vector<std::string>& arg_list_arg)
    :
    type(type_arg),
    func_type(func_type_arg),
    func(func_arg),
    arg_list(strip_quotes(arg_list_arg))
{}

ASTNode ASTNode::serializeObject()
{
    ASTNode result;
    result.type = Opm::TokenType::number;
    result.func_type = Opm::FuncType::field;
    result.func = "test1";
    result.arg_list = {"test2"};
    result.number = 1.0;
    ASTNode child = result;
    result.children = {child};

    return result;
}

size_t ASTNode::size() const {
    return this->children.size();
}


void ASTNode::add_child(const ASTNode& child) {
    this->children.push_back(child);
}

Action::Value ASTNode::value(const Action::Context& context) const {
    if (this->children.size() != 0)
        throw std::invalid_argument("value() method should only reach leafnodes");

    if (this->type == TokenType::number)
        return Action::Value(this->number);

    if (this->arg_list.size() == 0)
        return Action::Value(context.get(this->func));
    else {
        /*
          The matching code is special case to handle one-argument cases with
          well patterns like 'P*'.
        */
        if ((this->arg_list.size() == 1) && (this->arg_list[0].find("*") != std::string::npos)) {
            if (this->func_type != FuncType::well)
                throw std::logic_error(": attempted to action-evaluate list not of type well.");

            const auto& well_arg = this->arg_list[0];
            Action::Value well_values;
            std::vector<std::string> wnames;

            if (well_arg[0] == '*' && well_arg.size() > 1) {
                const auto& wlm = context.wlist_manager();
                wnames = wlm.wells(well_arg);
            } else {
                int fnmatch_flags = 0;
                for (const auto& well : context.wells(this->func)) {
                    if (fnmatch(well_arg.c_str(), well.c_str(), fnmatch_flags) == 0)
                        wnames.push_back(well);
                }
            }
            for (const auto& wname : wnames)
                well_values.add_well(wname, context.get(this->func, wname));

            return well_values;
        } else {
            std::string arg_key = this->arg_list[0];
            for (size_t index = 1; index < this->arg_list.size(); index++)
                arg_key += ":" + this->arg_list[index];
            return Action::Value(context.get(this->func, arg_key));
        }
    }
}


Action::Result ASTNode::eval(const Action::Context& context) const {
    if (this->children.size() == 0)
        throw std::invalid_argument("ASTNode::eval() should not reach leafnodes");

    if (this->type == TokenType::op_or || this->type == TokenType::op_and) {
        Action::Result result(this->type == TokenType::op_and);
        for (const auto& child : this->children) {
            if (this->type == TokenType::op_or)
                result |= child.eval(context);
            else
                result &= child.eval(context);
        }
        return result;
    }

    auto v1 = this->children[0].value(context);
    auto v2 = this->children[1].value(context);
    return v1.eval_cmp(this->type, v2);
}


bool ASTNode::operator==(const ASTNode& data) const {
    return type == data.type &&
           func_type == data.func_type &&
           func == data.func &&
           arg_list == data.arg_list &&
           number == data.number &&
           children == data.children;
}

}
}
