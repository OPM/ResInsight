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

#ifndef UDQASTNODE_HPP
#define UDQASTNODE_HPP

#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include <opm/input/eclipse/Schedule/UDQ/UDQSet.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQContext.hpp>
#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>



namespace Opm {

class UDQASTNode {
public:
    UDQASTNode();
    explicit UDQASTNode(UDQTokenType type_arg);
    explicit UDQASTNode(double scalar_value);
    UDQASTNode(UDQTokenType type_arg, const std::variant<std::string, double>& value_arg, const UDQASTNode& left_arg);
    UDQASTNode(UDQTokenType type_arg, const std::variant<std::string, double>& value_arg, const UDQASTNode& left, const UDQASTNode& right);
    UDQASTNode(UDQTokenType type_arg, const std::variant<std::string, double>& value_arg);
    UDQASTNode(UDQTokenType type_arg, const std::variant<std::string, double>& value_arg, const std::vector<std::string>& selector);

    static UDQASTNode serializeObject();

    UDQSet eval(UDQVarType eval_target, const UDQContext& context) const;

    bool valid() const;
    UDQVarType var_type = UDQVarType::NONE;
    std::set<UDQTokenType> func_tokens() const;
    void update_type(const UDQASTNode& arg);
    void set_left(const UDQASTNode& arg);
    void set_right(const UDQASTNode& arg);
    UDQASTNode* get_left() const;
    UDQASTNode* get_right() const;
    void scale(double sign_factor);

    bool operator==(const UDQASTNode& data) const;
    void required_summary(std::unordered_set<std::string>& summary_keys) const;

    template<class Serializer>
    void serializeOp(Serializer& serializer)
    {
        serializer(var_type);
        serializer(type);
        serializer(value);
        serializer(sign);
        serializer(selector);
        serializer(left);
        serializer(right);
    }

private:
    UDQTokenType type;
    void func_tokens(std::set<UDQTokenType>& tokens) const;

    std::variant<std::string, double> value;
    double sign = 1.0;
    std::vector<std::string> selector;
    std::shared_ptr<UDQASTNode> left;
    std::shared_ptr<UDQASTNode> right;
};

UDQASTNode operator*(const UDQASTNode&lhs, double rhs);
UDQASTNode operator*(double lhs, const UDQASTNode& rhs);

}

#endif
