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
#include <unordered_map>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include <opm/parser/eclipse/EclipseState/Schedule/UDQ/UDQEnums.hpp>


namespace Opm {
namespace UDQ {
namespace {

    const std::set<UDQTokenType> cmp_func = {UDQTokenType::binary_cmp_eq,
                                             UDQTokenType::binary_cmp_ne,
                                             UDQTokenType::binary_cmp_le,
                                             UDQTokenType::binary_cmp_ge,
                                             UDQTokenType::binary_cmp_lt,
                                             UDQTokenType::binary_cmp_gt};

    const std::set<UDQTokenType> binary_func  = {UDQTokenType::binary_op_add,
                                                 UDQTokenType::binary_op_mul,
                                                 UDQTokenType::binary_op_sub,
                                                 UDQTokenType::binary_op_div,
                                                 UDQTokenType::binary_op_pow,
                                                 UDQTokenType::binary_op_uadd,
                                                 UDQTokenType::binary_op_umul,
                                                 UDQTokenType::binary_op_umin,
                                                 UDQTokenType::binary_op_umax,
                                                 UDQTokenType::binary_cmp_eq,
                                                 UDQTokenType::binary_cmp_ne,
                                                 UDQTokenType::binary_cmp_le,
                                                 UDQTokenType::binary_cmp_ge,
                                                 UDQTokenType::binary_cmp_lt,
                                                 UDQTokenType::binary_cmp_gt};


    const std::set<UDQTokenType> scalar_func = {UDQTokenType::scalar_func_sum,
                                                UDQTokenType::scalar_func_avea,
                                                UDQTokenType::scalar_func_aveg,
                                                UDQTokenType::scalar_func_aveh,
                                                UDQTokenType::scalar_func_max,
                                                UDQTokenType::scalar_func_min,
                                                UDQTokenType::scalar_func_norm1,
                                                UDQTokenType::scalar_func_norm2,
                                                UDQTokenType::scalar_func_normi,
                                                UDQTokenType::scalar_func_prod};


    const std::set<UDQTokenType> unary_elemental_func = {UDQTokenType::elemental_func_randn,
                                                         UDQTokenType::elemental_func_randu,
                                                         UDQTokenType::elemental_func_rrandn,
                                                         UDQTokenType::elemental_func_rrandu,
                                                         UDQTokenType::elemental_func_abs,
                                                         UDQTokenType::elemental_func_def,
                                                         UDQTokenType::elemental_func_exp,
                                                         UDQTokenType::elemental_func_idv,
                                                         UDQTokenType::elemental_func_ln,
                                                         UDQTokenType::elemental_func_log,
                                                         UDQTokenType::elemental_func_nint,
                                                         UDQTokenType::elemental_func_sorta,
                                                         UDQTokenType::elemental_func_sortd,
                                                         UDQTokenType::elemental_func_undef};


    const std::unordered_map<std::string, UDQTokenType> func_type = {{"+", UDQTokenType::binary_op_add},
                                                                     {"-", UDQTokenType::binary_op_sub},
                                                                     {"/", UDQTokenType::binary_op_div},
                                                                     {"DIV", UDQTokenType::binary_op_div},
                                                                     {"*", UDQTokenType::binary_op_mul},
                                                                     {"^", UDQTokenType::binary_op_pow},
                                                                     {"UADD", UDQTokenType::binary_op_uadd},
                                                                     {"UMUL", UDQTokenType::binary_op_umul},
                                                                     {"UMIN", UDQTokenType::binary_op_umin},
                                                                     {"UMAX", UDQTokenType::binary_op_umax},
                                                                     {"==", UDQTokenType::binary_cmp_eq},
                                                                     {"!=", UDQTokenType::binary_cmp_ne},
                                                                     {"<=", UDQTokenType::binary_cmp_le},
                                                                     {">=", UDQTokenType::binary_cmp_ge},
                                                                     {"<", UDQTokenType::binary_cmp_lt},
                                                                     {">", UDQTokenType::binary_cmp_gt},
                                                                     {"RANDN", UDQTokenType::elemental_func_randn},
                                                                     {"RANDU", UDQTokenType::elemental_func_randu},
                                                                     {"RRNDN", UDQTokenType::elemental_func_rrandn},
                                                                     {"RRNDU", UDQTokenType::elemental_func_rrandu},
                                                                     {"ABS", UDQTokenType::elemental_func_abs},
                                                                     {"DEF", UDQTokenType::elemental_func_def},
                                                                     {"EXP", UDQTokenType::elemental_func_exp},
                                                                     {"IDV", UDQTokenType::elemental_func_idv},
                                                                     {"LN", UDQTokenType::elemental_func_ln},
                                                                     {"LOG", UDQTokenType::elemental_func_log},
                                                                     {"NINT", UDQTokenType::elemental_func_nint},
                                                                     {"SORTA", UDQTokenType::elemental_func_sorta},
                                                                     {"SORTD", UDQTokenType::elemental_func_sortd},
                                                                     {"UNDEF", UDQTokenType::elemental_func_undef},
                                                                     {"SUM", UDQTokenType::scalar_func_sum},
                                                                     {"AVEA", UDQTokenType::scalar_func_avea},
                                                                     {"AVEG", UDQTokenType::scalar_func_aveg},
                                                                     {"AVEH", UDQTokenType::scalar_func_aveh},
                                                                     {"MAX", UDQTokenType::scalar_func_max},
                                                                     {"MIN", UDQTokenType::scalar_func_min},
                                                                     {"NORM1", UDQTokenType::scalar_func_norm1},
                                                                     {"NORM2", UDQTokenType::scalar_func_norm2},
                                                                     {"NORMI", UDQTokenType::scalar_func_normi},
                                                                     {"PROD", UDQTokenType::scalar_func_prod}};


}

UDQVarType targetType(const std::string& keyword) {

    char first_char =  keyword[0];
    switch(first_char) {
    case 'C':
        return UDQVarType::CONNECTION_VAR;
    case 'R':
        return UDQVarType::REGION_VAR;
    case 'F':
        return UDQVarType::FIELD_VAR;
    case 'S':
        return UDQVarType::SEGMENT_VAR;
    case 'A':
        return UDQVarType::AQUIFER_VAR;
    case 'B':
        return UDQVarType::BLOCK_VAR;
    case 'W':
        return UDQVarType::WELL_VAR;
    case 'G':
        return UDQVarType::GROUP_VAR;
    default:
        try {
            std::stod(keyword);
            return UDQVarType::SCALAR;
        } catch(const std::invalid_argument& exc) {
            return UDQVarType::NONE;
        }
    }

}

UDQVarType targetType(const std::string& keyword, const std::vector<std::string>& selector) {
    auto tt = targetType(keyword);

    if (tt == UDQVarType::WELL_VAR || tt == UDQVarType::GROUP_VAR) {
        if (selector.empty())
            return tt;
        else {
            const auto& wgname = selector[0];
            if (wgname.find("*") != std::string::npos)
                return tt;
        }
    }

    return UDQVarType::SCALAR;
}



UDQVarType varType(const std::string& keyword) {
    if (keyword[1] != 'U')
        throw std::invalid_argument("Keyword: " + keyword + " is not of UDQ type");

    char first_char =  keyword[0];
    switch(first_char) {
    case 'W':
        return UDQVarType::WELL_VAR;
    case 'G':
        return UDQVarType::GROUP_VAR;
    case 'C':
        return UDQVarType::CONNECTION_VAR;
    case 'R':
        return UDQVarType::REGION_VAR;
    case 'F':
        return UDQVarType::FIELD_VAR;
    case 'S':
        return UDQVarType::SEGMENT_VAR;
    case 'A':
        return UDQVarType::AQUIFER_VAR;
    case 'B':
        return UDQVarType::BLOCK_VAR;
    default:
        throw std::invalid_argument("Keyword: " + keyword + " is not of UDQ type");
    }

}


UDQAction actionType(const std::string& action_string) {
    if (action_string == "ASSIGN")
        return UDQAction::ASSIGN;

    if (action_string == "DEFINE")
        return UDQAction::DEFINE;

    if (action_string == "UNITS")
        return UDQAction::UNITS;

    if (action_string == "UPDATE")
        return UDQAction::UPDATE;

    throw std::invalid_argument("Invalid action string " + action_string);
}


bool binaryFunc(UDQTokenType token_type) {
    return (binary_func.count(token_type) > 0);
}

bool scalarFunc(UDQTokenType token_type) {
    return (scalar_func.count(token_type) > 0);
}

bool elementalUnaryFunc(UDQTokenType token_type) {
    return (unary_elemental_func.count(token_type) > 0);
}

bool cmpFunc(UDQTokenType token_type) {
    return (cmp_func.count(token_type) > 0);
}


UDQTokenType funcType(const std::string& func_name) {
    if (func_type.count(func_name) > 0)
        return func_type.at(func_name);

    if (func_name.substr(0,2) == "TU") {
        return UDQTokenType::table_lookup;
    }

    return UDQTokenType::error;
}


UDQVarType coerce(UDQVarType t1, UDQVarType t2) {
    if (t1 == t2)
        return t1;

    if (t1 == UDQVarType::WELL_VAR ) {
        if (t2 == UDQVarType::GROUP_VAR)
            throw std::logic_error("Can not coerce well variable and group variable");

        return t1;
    }

    if (t1 == UDQVarType::GROUP_VAR ) {
        if (t2 == UDQVarType::WELL_VAR)
            throw std::logic_error("Can not coerce well variable and group variable");

        return t1;
    }

    if (t2 == UDQVarType::WELL_VAR ) {
        if (t1 == UDQVarType::GROUP_VAR)
            throw std::logic_error("Can not coerce well variable and group variable");

        return t2;
    }

    if (t2 == UDQVarType::GROUP_VAR ) {
        if (t1 == UDQVarType::WELL_VAR)
            throw std::logic_error("Can not coerce well variable and group variable");

        return t2;
    }

    if (t1 == UDQVarType::NONE)
        return t2;

    if (t2 == UDQVarType::NONE)
        return t1;

    return t1;
}




std::string typeName(UDQVarType var_type) {
    switch (var_type) {
    case UDQVarType::NONE:
        return "NONE";
    case UDQVarType::SCALAR:
        return "SCALAR";
    case UDQVarType::WELL_VAR:
        return "WELL_VAR";
    case UDQVarType::CONNECTION_VAR:
        return "CONNECTION_VAR";
    case UDQVarType::FIELD_VAR:
        return "FIELD_VAR";
    case UDQVarType::GROUP_VAR:
        return "GROUP_VAR";
    case UDQVarType::REGION_VAR:
        return "REGION_VAR";
    case UDQVarType::SEGMENT_VAR:
        return "SEGMENT_VAR";
    case UDQVarType::AQUIFER_VAR:
        return "AQUIFER_VAR";
    case UDQVarType::BLOCK_VAR:
        return "BLOCK_VAR";
    default:
        throw std::runtime_error("Should not be here: " + std::to_string(static_cast<int>(var_type)));
    }
}


UDAKeyword keyword(UDAControl control) {
    const std::map<UDAControl, UDAKeyword> c2k = {{UDAControl::WCONPROD_ORAT, UDAKeyword::WCONPROD},
                                                  {UDAControl::WCONPROD_GRAT, UDAKeyword::WCONPROD},
                                                  {UDAControl::WCONPROD_WRAT, UDAKeyword::WCONPROD},
                                                  {UDAControl::WCONPROD_LRAT, UDAKeyword::WCONPROD},
                                                  {UDAControl::WCONPROD_RESV, UDAKeyword::WCONPROD},
                                                  {UDAControl::WCONPROD_BHP,  UDAKeyword::WCONPROD},
                                                  {UDAControl::WCONPROD_THP,  UDAKeyword::WCONPROD},
                                                  {UDAControl::WCONINJE_RATE, UDAKeyword::WCONINJE},
                                                  {UDAControl::WCONINJE_RESV, UDAKeyword::WCONINJE},
                                                  {UDAControl::WCONINJE_BHP,  UDAKeyword::WCONINJE},
                                                  {UDAControl::WCONINJE_THP,  UDAKeyword::WCONINJE},
                                                  {UDAControl::GCONPROD_OIL_TARGET,    UDAKeyword::GCONPROD},
                                                  {UDAControl::GCONPROD_WATER_TARGET,  UDAKeyword::GCONPROD},
                                                  {UDAControl::GCONPROD_GAS_TARGET,    UDAKeyword::GCONPROD},
                                                  {UDAControl::GCONPROD_LIQUID_TARGET, UDAKeyword::GCONPROD}};

    auto it = c2k.find(control);
    if (it != c2k.end())
      return it->second;

    throw std::logic_error("Unrecognized enum type - internal error");
}


int uadCode(UDAControl control) {
    const std::map<UDAControl, int> c2uad = {{UDAControl::WCONPROD_ORAT,  300004},
                                             {UDAControl::WCONPROD_GRAT,  500004},
                                             {UDAControl::WCONPROD_WRAT,  400004},
                                             {UDAControl::WCONPROD_LRAT,  600004},
                                             {UDAControl::WCONPROD_RESV,  999999},
                                             {UDAControl::WCONPROD_BHP,   999999},
                                             {UDAControl::WCONPROD_THP,   999999},
                                             {UDAControl::WCONINJE_RATE,  400003},
                                             {UDAControl::WCONINJE_RESV,  500003},
                                             {UDAControl::WCONINJE_BHP,   999999},
                                             {UDAControl::WCONINJE_THP,   999999},
                                             {UDAControl::GCONPROD_OIL_TARGET,    200019},
                                             {UDAControl::GCONPROD_WATER_TARGET,  300019},
                                             {UDAControl::GCONPROD_GAS_TARGET,    400019},
                                             {UDAControl::GCONPROD_LIQUID_TARGET, 500019}};

    auto it = c2uad.find(control);
    if (it != c2uad.end())
      return it->second;

    throw std::logic_error("Unrecognized enum type - internal error");
}
}
}
