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
#include <opm/input/eclipse/Schedule/UDQ/UDQASTNode.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQFunction.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQFunctionTable.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>

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


UDQASTNode::UDQASTNode(double numeric_value) :
    var_type(init_type(UDQTokenType::number)),
    type(UDQTokenType::number),
    value(numeric_value)
{}


UDQASTNode::UDQASTNode(UDQTokenType type_arg, const std::variant<std::string, double>& value_arg) :
    var_type(init_type(type_arg)),
    type(type_arg),
    value(value_arg)
{
}


UDQASTNode::UDQASTNode(UDQTokenType type_arg,
                       const std::variant<std::string, double>& value_arg,
                       const UDQASTNode& left_arg)
    : UDQASTNode(type_arg, value_arg)
{
    if (UDQ::scalarFunc(type_arg))
        this->var_type = UDQVarType::SCALAR;
    else
        this->var_type = left_arg.var_type;
    this->left = std::make_unique<UDQASTNode>(left_arg);
}


UDQASTNode::UDQASTNode(UDQTokenType type_arg,
                       const std::variant<std::string, double>& value_arg,
                       const UDQASTNode& left_arg,
                       const UDQASTNode& right_arg) :
    var_type(init_type(type_arg)),
    type(type_arg),
    value(value_arg)
{
    this->set_left(left_arg);
    this->set_right(right_arg);
}


UDQASTNode UDQASTNode::serializeObject()
{
    UDQASTNode result;
    result.var_type = UDQVarType::REGION_VAR;
    result.type = UDQTokenType::error;
    result.value = "test1";
    result.selector = {"test2"};
    result.sign = -1;
    UDQASTNode left = result;
    result.left = std::make_shared<UDQASTNode>(left);

    return result;
}

UDQASTNode::UDQASTNode(UDQTokenType type_arg,
                       const std::variant<std::string, double>& value_arg,
                       const std::vector<std::string>& selector_arg) :
    var_type(init_type(type_arg)),
    type(type_arg),
    value(value_arg),
    selector(selector_arg)
{
    if (type_arg == UDQTokenType::ecl_expr)
        this->var_type = UDQ::targetType(std::get<std::string>(this->value), this->selector);

    if (this->var_type == UDQVarType::CONNECTION_VAR ||
        this->var_type == UDQVarType::REGION_VAR ||
        this->var_type == UDQVarType::SEGMENT_VAR ||
        this->var_type == UDQVarType::AQUIFER_VAR ||
        this->var_type == UDQVarType::BLOCK_VAR)
        throw std::logic_error("UDQ variable of type: " + UDQ::typeName(this->var_type) + " not yet supported in flow");
}




UDQSet UDQASTNode::eval(UDQVarType target_type, const UDQContext& context) const {
    if (this->type == UDQTokenType::ecl_expr) {
        const auto& string_value = std::get<std::string>( this->value );
        auto data_type = UDQ::targetType(string_value);
        if (data_type == UDQVarType::WELL_VAR) {
            const auto& all_wells = context.wells();

            if (this->selector.empty()) {
                auto res = UDQSet::wells(string_value, all_wells);
                for (const auto& well : all_wells)
                    res.assign(well, context.get_well_var(well, string_value));
                return this->sign * res;
            } else {
                const std::string& well_pattern = this->selector[0];
                if (well_pattern.find('*') == std::string::npos)
                    /*
                      The right hand side is a fully qualified well name without
                      any '*', in this case the right hand side evaluates to a
                      *scalar* - and that scalar value is distributed among all
                      the wells in the result set.
                    */
                    return this->sign * UDQSet::scalar(string_value, context.get_well_var(well_pattern, string_value));
                else {
                    /*
                      The right hand side is a set of wells. The result set will
                      be updated for all wells in the right hand set, wells
                      missing in the right hand set will be undefined in the
                      result set.
                     */
                    auto res = UDQSet::wells(string_value, all_wells);
                    for (const auto& wname : context.wells(well_pattern))
                        res.assign(wname, context.get_well_var(wname, string_value));
                    return this->sign * res;
                }
            }
        }

        if (data_type == UDQVarType::GROUP_VAR) {
            if (this->selector.size() > 0) {
                const std::string& group_pattern = this->selector[0];
                if (group_pattern.find("*") == std::string::npos)
                    return UDQSet::scalar(string_value, context.get_group_var(group_pattern, string_value));
                else
                    throw std::logic_error("Group names with wildcards is not yet supported");
            } else {
                const auto& groups = context.groups();
                auto res = UDQSet::groups(string_value, groups);
                for (const auto& group : groups)
                    res.assign(group, context.get_group_var(group, string_value));
                return this->sign * res;
            }
        }

        if (data_type == UDQVarType::FIELD_VAR)
            return this->sign * UDQSet::scalar(string_value, context.get(string_value));

        auto scalar = context.get(string_value);
        if (scalar.has_value())
            return this->sign * UDQSet::scalar(string_value, scalar.value());

        throw std::logic_error("Should not be here: var_type: " + UDQ::typeName(data_type) + " stringvalue:" + string_value);
    }


    if (UDQ::scalarFunc(this->type)) {
        const auto& string_value = std::get<std::string>( this->value );
        const auto& udqft = context.function_table();
        const UDQScalarFunction& func = dynamic_cast<const UDQScalarFunction&>(udqft.get(string_value));
        return this->sign * func.eval( this->left->eval(target_type, context) );
    }


    if (UDQ::elementalUnaryFunc(this->type)) {
        const auto& string_value = std::get<std::string>( this->value );
        auto func_arg = this->left->eval(target_type, context);

        const auto& udqft = context.function_table();
        const UDQUnaryElementalFunction& func = dynamic_cast<const UDQUnaryElementalFunction&>(udqft.get(string_value));
        return this->sign * func.eval(func_arg);
    }

    if (UDQ::binaryFunc(this->type)) {
        auto left_arg = this->left->eval(target_type, context);
        auto right_arg = this->right->eval(target_type, context);
        const auto& string_value = std::get<std::string>( this->value );

        const auto& udqft = context.function_table();
        const UDQBinaryFunction& func = dynamic_cast<const UDQBinaryFunction&>(udqft.get(string_value));
        auto res = func.eval(left_arg, right_arg);
        return this->sign * res;
    }

    if (this->type == UDQTokenType::number) {
        const std::string dummy_name = "DUMMY";
        double numeric_value = std::get<double>(this->value);
        switch(target_type) {
        case UDQVarType::WELL_VAR:
            return this->sign * UDQSet::wells(dummy_name, context.wells(), numeric_value);
        case UDQVarType::GROUP_VAR:
            return this->sign * UDQSet::groups(dummy_name, context.groups(), numeric_value);
        case UDQVarType::SCALAR:
            return this->sign * UDQSet::scalar(dummy_name, numeric_value);
        case UDQVarType::FIELD_VAR:
            return this->sign * UDQSet::field(dummy_name, numeric_value);
        default:
            throw std::invalid_argument("Unsupported target_type: " + std::to_string(static_cast<int>(target_type)));
        }
    }

    throw std::invalid_argument("Should not be here ... this->type: " + std::to_string(static_cast<int>(this->type)));
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

void UDQASTNode::scale(double sign_factor) {
    this->sign *= sign_factor;
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
           value == data.value &&
           selector == data.selector;
}

namespace {

bool is_udq(const std::string& key) {
    if (key.size() < 2)
        return false;

    if (key[1] != 'U')
        return false;

    return true;
}

}

void UDQASTNode::required_summary(std::unordered_set<std::string>& summary_keys) const {
    if (this->type == UDQTokenType::ecl_expr) {
        if (std::holds_alternative<std::string>(this->value)) {
            const auto& keyword = std::get<std::string>(this->value);
            if (!is_udq(keyword))
                summary_keys.insert(keyword);
        }
    }

    if (this->left)
        this->left->required_summary(summary_keys);

    if (this->right)
        this->right->required_summary(summary_keys);
}

UDQASTNode operator*(const UDQASTNode&lhs, double sign_factor) {
    UDQASTNode prod = lhs;
    prod.scale(sign_factor);
    return prod;
}


UDQASTNode operator*(double lhs, const UDQASTNode& rhs) {
    return rhs * lhs;
}

}
