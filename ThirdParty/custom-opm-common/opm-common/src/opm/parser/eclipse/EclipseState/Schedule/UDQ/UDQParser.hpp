/*
  Copyright 2019  Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify it under the terms
  of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  OPM is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  OPM.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef UDQPARSER_HPP
#define UDQPARSER_HPP

#include <string>
#include <vector>

#ifdef _MSC_VER
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQASTNode.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQFunctionTable.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQParams.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>

namespace Opm {

class ParseContext;
class ErrorGuard;

struct UDQParseNode {
    UDQParseNode(UDQTokenType type_arg, const std::string& value_arg, const std::vector<std::string>& selector_arg) :
        type(type_arg),
        value(value_arg),
        selector(selector_arg)
    {
        if (type_arg == UDQTokenType::ecl_expr)
            this->var_type = UDQ::targetType(value_arg, selector_arg);
    }


    UDQParseNode(UDQTokenType type_arg, const std::string& value_arg) :
        UDQParseNode(type_arg, value_arg, {})
    {}


    // Implicit converting constructor.
    UDQParseNode(UDQTokenType type_arg) : UDQParseNode(type_arg, "")
    {}

    UDQTokenType type;
    std::string value;
    std::vector<std::string> selector;
    UDQVarType var_type = UDQVarType::NONE;
};


class UDQParser {
public:
  static UDQASTNode parse(const UDQParams& udq_params, UDQVarType target_type, const std::string& target_var, const std::vector<std::string>& tokens, const ParseContext& parseContext, ErrorGuard& errors);

private:
    UDQParser(const UDQParams& udq_params1, const std::vector<std::string>& tokens1) :
        udq_params(udq_params1),
        udqft(UDQFunctionTable(udq_params)),
        tokens(tokens1)
    {}

    UDQASTNode parse_cmp();
    UDQASTNode parse_add();
    UDQASTNode parse_factor();
    UDQASTNode parse_mul();
    UDQASTNode parse_pow();

    UDQParseNode current() const;
    UDQParseNode next();
    UDQTokenType get_type(const std::string& arg) const;
    std::size_t current_size() const;
    bool empty() const;

    const UDQParams& udq_params;
    UDQFunctionTable udqft;
    std::vector<std::string> tokens;
    ssize_t current_pos = -1;
};


}

#endif
