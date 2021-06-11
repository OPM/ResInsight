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

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQASTNode.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQFunction.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQFunctionTable.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>

namespace Opm {

UDQASTNode::UDQASTNode() :
    UDQASTNode(UDQTokenType::error)
{}


UDQASTNode::UDQASTNode(UDQTokenType type_arg) :
    var_type(UDQVarType::NONE),
    type(type_arg)
{
    if (type == UDQTokenType::error)
        return;

    if (type == UDQTokenType::binary_op_add)
        return;

    if (type == UDQTokenType::binary_op_sub)
        return;
    throw std::invalid_argument("The one argument constructor is only available for error and end");
}


namespace {

UDQVarType init_type(UDQTokenType token_type)
{
    if (token_type == UDQTokenType::number)
        return UDQVarType::SCALAR;

    if (UDQ::scalarFunc(token_type))
        return UDQVarType::SCALAR;

    return UDQVarType::NONE;
}


}


UDQASTNode::UDQASTNode(double scalar_value_arg) :
    var_type(init_type(UDQTokenType::number)),
    type(UDQTokenType::number),
    scalar_value(scalar_value_arg)
{}


UDQASTNode::UDQASTNode(UDQTokenType type_arg, const std::string& func_name) :
    var_type(init_type(type_arg)),
    type(type_arg),
    string_value(func_name)
{
}


UDQASTNode::UDQASTNode(UDQTokenType type_arg,
                       const std::string& func_name,
                       const UDQASTNode& left_arg)
    : UDQASTNode(type_arg, func_name)
{
    if (UDQ::scalarFunc(type_arg))
        this->var_type = UDQVarType::SCALAR;
    else
        this->var_type = left_arg.var_type;
    this->left = std::make_unique<UDQASTNode>(left_arg);
}


UDQASTNode::UDQASTNode(UDQTokenType type_arg,
                       const std::string& func_name,
                       const UDQASTNode& left_arg,
                       const UDQASTNode& right_arg) :
    var_type(init_type(type_arg)),
    type(type_arg),
    string_value(func_name)
{
    this->set_left(left_arg);
    this->set_right(right_arg);
}


UDQASTNode UDQASTNode::serializeObject()
{
    UDQASTNode result;
    result.var_type = UDQVarType::REGION_VAR;
    result.type = UDQTokenType::error;
    result.string_value = "test1";
    result.selector = {"test2"};
    result.scalar_value = 1.0;
    UDQASTNode left = result;
    result.left = std::make_shared<UDQASTNode>(left);

    return result;
}

UDQASTNode::UDQASTNode(UDQTokenType type_arg,
                       const std::string& string_value_arg,
                       const std::vector<std::string>& selector_arg) :
    var_type(init_type(type_arg)),
    type(type_arg),
    string_value(string_value_arg),
    selector(selector_arg)
{
    if (type_arg == UDQTokenType::number)
        this->scalar_value = std::stod(this->string_value);

    if (type_arg == UDQTokenType::ecl_expr) {
        this->var_type = UDQ::targetType(this->string_value, this->selector);

    }

    if (this->var_type == UDQVarType::CONNECTION_VAR ||
        this->var_type == UDQVarType::REGION_VAR ||
        this->var_type == UDQVarType::SEGMENT_VAR ||
        this->var_type == UDQVarType::AQUIFER_VAR ||
        this->var_type == UDQVarType::BLOCK_VAR)
        throw std::logic_error("UDQ variable of type: " + UDQ::typeName(this->var_type) + " not yet supported in flow");
}




UDQSet UDQASTNode::eval(UDQVarType target_type, const UDQContext& context) const {
    if (this->type == UDQTokenType::ecl_expr) {
        auto data_type = UDQ::targetType(this->string_value);
        if (data_type == UDQVarType::WELL_VAR) {
            const auto& wells = context.wells();

            if (this->selector.size() > 0) {
                const std::string& well_pattern = this->selector[0];
                if (well_pattern.find("*") == std::string::npos)
                    return UDQSet::scalar(this->string_value, context.get_well_var(well_pattern, this->string_value));
                else {
                    auto res = UDQSet::wells(this->string_value, wells);
                    int fnmatch_flags = 0;
                    for (const auto& well : wells) {
                        if (fnmatch(well_pattern.c_str(), well.c_str(), fnmatch_flags) == 0) {
                            if (context.has_well_var(well, this->string_value))
                                res.assign(well, context.get_well_var(well, this->string_value));
                        }
                    }
                    return res;
                }
            } else {
                auto res = UDQSet::wells(this->string_value, wells);
                for (const auto& well : wells) {
                    if (context.has_well_var(well, this->string_value))
                        res.assign(well, context.get_well_var(well, this->string_value));
                }
                return res;
            }
        }

        if (data_type == UDQVarType::GROUP_VAR) {
            if (this->selector.size() > 0) {
                const std::string& group_pattern = this->selector[0];
                if (group_pattern.find("*") == std::string::npos)
                    return UDQSet::scalar(this->string_value, context.get_group_var(group_pattern, this->string_value));
                else
                    throw std::logic_error("Group names with wildcards is not yet supported");
            } else {
                const auto& groups = context.groups();
                auto res = UDQSet::groups(this->string_value, groups);
                for (const auto& group : groups) {
                    if (context.has_group_var(group, this->string_value))
                        res.assign(group, context.get_group_var(group, this->string_value));
                }
                return res;
            }
        }

        if (data_type == UDQVarType::FIELD_VAR)
            return UDQSet::scalar(this->string_value, context.get(this->string_value));

        throw std::logic_error("Should not be here: var_type: " + UDQ::typeName(data_type));
    }


    if (UDQ::scalarFunc(this->type)) {
        const auto& udqft = context.function_table();
        const UDQScalarFunction& func = dynamic_cast<const UDQScalarFunction&>(udqft.get(this->string_value));
        return func.eval( this->left->eval(target_type, context) );
    }


    if (UDQ::elementalUnaryFunc(this->type)) {
        auto func_arg = this->left->eval(target_type, context);

        const auto& udqft = context.function_table();
        const UDQUnaryElementalFunction& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get(this->string_value));
        return func.eval(func_arg);
    }

    if (UDQ::binaryFunc(this->type)) {
        auto left_arg = this->left->eval(target_type, context);
        auto right_arg = this->right->eval(target_type, context);

        const auto& udqft = context.function_table();
        const UDQBinaryFunction& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get(this->string_value));
        auto res = func.eval(left_arg, right_arg);
        return res;
    }

    if (this->type == UDQTokenType::number) {
        switch(target_type) {
        case UDQVarType::WELL_VAR:
            return UDQSet::wells(this->string_value, context.wells(), this->scalar_value);
        case UDQVarType::GROUP_VAR:
            return UDQSet::groups(this->string_value, context.groups(), this->scalar_value);
        case UDQVarType::SCALAR:
            return UDQSet::scalar(this->string_value, this->scalar_value);
        case UDQVarType::FIELD_VAR:
            return UDQSet::field(this->string_value, this->scalar_value);
        default:
            throw std::invalid_argument("Unsupported target_type: " + std::to_string(static_cast<int>(target_type)));
        }
    }

    throw std::invalid_argument("Should not be here ... this->type: " + std::to_string(static_cast<int>(this->type)) + " string_value: <" + this->string_value + ">");
}

void UDQASTNode::func_tokens(std::set<UDQTokenType>& tokens) const {
    tokens.insert( this->type );
    if (this->left)
        this->left->func_tokens(tokens);
    if (this->right)
        this->right->func_tokens(tokens);
}

std::set<UDQTokenType> UDQASTNode::func_tokens() const {
    std::set<UDQTokenType> tokens;
    this->func_tokens(tokens);
    return tokens;
}


UDQASTNode* UDQASTNode::get_left() const {
    return this->left.get();
}

UDQASTNode* UDQASTNode::get_right() const {
    return this->right.get();
}


void UDQASTNode::update_type(const UDQASTNode& arg) {
    if (this->var_type == UDQVarType::NONE)
        this->var_type = arg.var_type;
    else
        this->var_type = UDQ::coerce(this->var_type, arg.var_type);
}


bool UDQASTNode::valid() const {
    return (this->type != UDQTokenType::error);
}


void UDQASTNode::set_left(const UDQASTNode& arg) {
    this->left = std::make_unique<UDQASTNode>(arg);
    this->update_type(arg);
}

void UDQASTNode::set_right(const UDQASTNode& arg) {
    this->right = std::make_unique<UDQASTNode>(arg);
    this->update_type(arg);
}

bool UDQASTNode::operator==(const UDQASTNode& data) const {
    if ((this->left && !data.left) ||
        (!this->left && data.left))
        return false;

    if (this->left && !(*this->left == *data.left))
        return false;

    if ((this->right && !data.right) ||
        (!this->right && data.right))
        return false;

    if (this->right && !(*this->right == *data.right))
        return false;

    return type == data.type &&
           var_type == data.var_type &&
           string_value == data.string_value &&
           scalar_value == data.scalar_value &&
           selector == data.selector;
}

}
