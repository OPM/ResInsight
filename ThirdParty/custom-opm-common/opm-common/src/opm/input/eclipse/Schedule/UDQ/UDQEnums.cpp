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

#include <opm/input/eclipse/Schedule/UDQ/UDQEnums.hpp>

#include <opm/common/utility/String.hpp>

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace Opm { namespace UDQ {

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

    const std::set<UDQTokenType> set_func = {UDQTokenType::binary_op_uadd,
                                             UDQTokenType::binary_op_umul,
                                             UDQTokenType::binary_op_umin,
                                             UDQTokenType::binary_op_umax};

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
        const auto& double_value = try_parse_double(keyword);
        if (double_value.has_value())
            return UDQVarType::SCALAR;

        return UDQVarType::NONE;
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


UDQUpdate updateType(const std::string& update_string) {
    if (update_string == "ON")
        return UDQUpdate::ON;

    if (update_string == "OFF")
        return UDQUpdate::OFF;

    if (update_string == "NEXT")
        return UDQUpdate::NEXT;

    throw std::invalid_argument("Invalid status update string " + update_string);
}

UDQUpdate updateType(int int_value) {
    switch (int_value) {
    case 0: return UDQUpdate::OFF;
    case 1: return UDQUpdate::NEXT;
    case 2: return UDQUpdate::ON;
    default:
        throw std::logic_error("Invalid integer for UDQUpdate type");
    }
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

bool setFunc(UDQTokenType token_type) {
    return (set_func.count(token_type) > 0);
}


UDQTokenType funcType(const std::string& func_name) {
    if (func_type.count(func_name) > 0)
        return func_type.at(func_name);

    if (func_name.substr(0,2) == "TU") {
        return UDQTokenType::table_lookup;
    }

    return UDQTokenType::error;
}

UDQTokenType tokenType(const std::string& token) {
    auto token_type = funcType(token);
    if (token_type == UDQTokenType::error) {
        if (token == "(")
            token_type = UDQTokenType::open_paren;
        else if (token == ")")
            token_type = UDQTokenType::close_paren;
        else {
            auto value = try_parse_double(token);
            if (value.has_value())
                token_type = UDQTokenType::number;
            else
                token_type = UDQTokenType::ecl_expr;
        }
    }
    return token_type;
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

bool trailingSpace(UDQTokenType token_type) {
    if (binaryFunc(token_type))
        return true;

    if (cmpFunc(token_type))
        return true;

    return false;
}

bool leadingSpace(UDQTokenType token_type) {
    if (binaryFunc(token_type))
        return true;

    if (cmpFunc(token_type))
        return true;

    return false;
}

namespace {
    template <typename Value>
    Value lookup_control_map_value(const std::map<UDAControl, Value>& map,
                                   const UDAControl                   control)
    {
        auto pos = map.find(control);
        if (pos == map.end()) {
            throw std::logic_error {
                "Unrecognized enum type (" +
                std::to_string(static_cast<int>(control)) +
                "- internal error"
            };
        }

        return pos->second;
    }
} // Anonymous

UDAKeyword keyword(const UDAControl control)
{
    static const auto c2k = std::map<UDAControl, UDAKeyword> {
        {UDAControl::WCONPROD_ORAT, UDAKeyword::WCONPROD},
        {UDAControl::WCONPROD_WRAT, UDAKeyword::WCONPROD},
        {UDAControl::WCONPROD_GRAT, UDAKeyword::WCONPROD},
        {UDAControl::WCONPROD_LRAT, UDAKeyword::WCONPROD},
        {UDAControl::WCONPROD_RESV, UDAKeyword::WCONPROD},
        {UDAControl::WCONPROD_BHP,  UDAKeyword::WCONPROD},
        {UDAControl::WCONPROD_THP,  UDAKeyword::WCONPROD},

        // --------------------------------------------------------------
        {UDAControl::WCONINJE_RATE, UDAKeyword::WCONINJE},
        {UDAControl::WCONINJE_RESV, UDAKeyword::WCONINJE},
        {UDAControl::WCONINJE_BHP,  UDAKeyword::WCONINJE},
        {UDAControl::WCONINJE_THP,  UDAKeyword::WCONINJE},

        // --------------------------------------------------------------
        {UDAControl::WELTARG_ORAT, UDAKeyword::WELTARG},
        {UDAControl::WELTARG_WRAT, UDAKeyword::WELTARG},
        {UDAControl::WELTARG_GRAT, UDAKeyword::WELTARG},
        {UDAControl::WELTARG_LRAT, UDAKeyword::WELTARG},
        {UDAControl::WELTARG_RESV, UDAKeyword::WELTARG},
        {UDAControl::WELTARG_BHP,  UDAKeyword::WELTARG},
        {UDAControl::WELTARG_THP,  UDAKeyword::WELTARG},
        {UDAControl::WELTARG_LIFT, UDAKeyword::WELTARG},

        // --------------------------------------------------------------
        {UDAControl::GCONPROD_OIL_TARGET,    UDAKeyword::GCONPROD},
        {UDAControl::GCONPROD_WATER_TARGET,  UDAKeyword::GCONPROD},
        {UDAControl::GCONPROD_GAS_TARGET,    UDAKeyword::GCONPROD},
        {UDAControl::GCONPROD_LIQUID_TARGET, UDAKeyword::GCONPROD},

        // --------------------------------------------------------------
        {UDAControl::GCONINJE_SURFACE_MAX_RATE,      UDAKeyword::GCONINJE},
        {UDAControl::GCONINJE_RESV_MAX_RATE,         UDAKeyword::GCONINJE},
        {UDAControl::GCONINJE_TARGET_REINJ_FRACTION, UDAKeyword::GCONINJE},
        {UDAControl::GCONINJE_TARGET_VOID_FRACTION,  UDAKeyword::GCONINJE},
    };

    return lookup_control_map_value(c2k, control);
}

int udaCode(const UDAControl control)
{
    static const auto c2uda = std::map<UDAControl, int> {
        {UDAControl::WCONPROD_ORAT,  300'004},
        {UDAControl::WCONPROD_WRAT,  400'004},
        {UDAControl::WCONPROD_GRAT,  500'004},
        {UDAControl::WCONPROD_LRAT,  600'004},
        {UDAControl::WCONPROD_RESV,  700'004},
        {UDAControl::WCONPROD_BHP,   800'004},
        {UDAControl::WCONPROD_THP,   900'004},

        // --------------------------------------------------------------
        {UDAControl::WCONINJE_RATE,  400'003},
        {UDAControl::WCONINJE_RESV,  500'003},
        {UDAControl::WCONINJE_BHP,   600'003},
        {UDAControl::WCONINJE_THP,   700'003},

        // --------------------------------------------------------------
        {UDAControl::GCONPROD_OIL_TARGET,    200'019},
        {UDAControl::GCONPROD_WATER_TARGET,  300'019},
        {UDAControl::GCONPROD_GAS_TARGET,    400'019},
        {UDAControl::GCONPROD_LIQUID_TARGET, 500'019},

        // --------------------------------------------------------------
        {UDAControl::GCONINJE_SURFACE_MAX_RATE,      300'017}, // Surface injection rate
        {UDAControl::GCONINJE_RESV_MAX_RATE,         400'017}, // Reservoir volume injection rate
        {UDAControl::GCONINJE_TARGET_REINJ_FRACTION, 500'017}, // Reinjection fraction
        {UDAControl::GCONINJE_TARGET_VOID_FRACTION,  600'017}, // Voidage replacement fraction

        // --------------------------------------------------------------
        {UDAControl::WELTARG_ORAT,        16},
        {UDAControl::WELTARG_WRAT,   100'016},
        {UDAControl::WELTARG_GRAT,   200'016},
        {UDAControl::WELTARG_LRAT,   300'016},
        {UDAControl::WELTARG_RESV,   400'016},
        {UDAControl::WELTARG_BHP,    500'016},
        {UDAControl::WELTARG_THP,    600'016},
        {UDAControl::WELTARG_LIFT, 1'000'016},
    };

    return lookup_control_map_value(c2uda, control);
}

bool group_control(const UDAControl control)
{
    try {
        const auto kw = keyword(control);
        return (kw == UDAKeyword::GCONPROD)
            || (kw == UDAKeyword::GCONINJE);
    }
    catch (const std::logic_error&) {
        return false;
    }
}

bool well_control(const UDAControl control)
{
    try {
        const auto kw = keyword(control);
        return (kw == UDAKeyword::WCONPROD)
            || (kw == UDAKeyword::WCONINJE)
            || (kw == UDAKeyword::WELTARG);
    }
    catch (const std::logic_error&) {
        return false;
    }
}

bool is_well_injection_control(const UDAControl control,
                               const bool       isInjector)
{
    try {
        const auto kw = keyword(control);
        return (kw == UDAKeyword::WCONINJE)
            || (isInjector && (kw == UDAKeyword::WELTARG));
    }
    catch (const std::logic_error&) {
        return false;
    }
}

bool is_well_production_control(const UDAControl control,
                                const bool       isProducer)
{
    try {
        const auto kw = keyword(control);
        return (kw == UDAKeyword::WCONPROD)
            || (isProducer && (kw == UDAKeyword::WELTARG));
    }
    catch (const std::logic_error&) {
        return false;
    }
}

bool is_group_injection_control(const UDAControl control)
{
    try {
        return keyword(control) == UDAKeyword::GCONINJE;
    }
    catch (const std::logic_error&) {
        return false;
    }
}

bool is_group_production_control(const UDAControl control)
{
    try {
        return keyword(control) == UDAKeyword::GCONPROD;
    }
    catch (const std::logic_error&) {
        return false;
    }
}

UDAControl udaControl(const int uda_code)
{
    switch (uda_code) {
    case   300'004: return UDAControl::WCONPROD_ORAT;
    case   400'004: return UDAControl::WCONPROD_WRAT;
    case   500'004: return UDAControl::WCONPROD_GRAT;
    case   600'004: return UDAControl::WCONPROD_LRAT;
    case   700'004: return UDAControl::WCONPROD_RESV;
    case   800'004: return UDAControl::WCONPROD_BHP;
    case   900'004: return UDAControl::WCONPROD_THP;

    case   400'003: return UDAControl::WCONINJE_RATE;
    case   500'003: return UDAControl::WCONINJE_RESV;
    case   600'003: return UDAControl::WCONINJE_BHP;
    case   700'003: return UDAControl::WCONINJE_THP;

    case   200'019: return UDAControl::GCONPROD_OIL_TARGET;
    case   300'019: return UDAControl::GCONPROD_WATER_TARGET;
    case   400'019: return UDAControl::GCONPROD_GAS_TARGET;
    case   500'019: return UDAControl::GCONPROD_LIQUID_TARGET;

    case   300'017: return UDAControl::GCONINJE_SURFACE_MAX_RATE;
    case   400'017: return UDAControl::GCONINJE_RESV_MAX_RATE;
    case   500'017: return UDAControl::GCONINJE_TARGET_REINJ_FRACTION;
    case   600'017: return UDAControl::GCONINJE_TARGET_VOID_FRACTION;

    case        16: return UDAControl::WELTARG_ORAT;
    case   100'016: return UDAControl::WELTARG_WRAT;
    case   200'016: return UDAControl::WELTARG_GRAT;
    case   300'016: return UDAControl::WELTARG_LRAT;
    case   400'016: return UDAControl::WELTARG_RESV;
    case   500'016: return UDAControl::WELTARG_BHP;
    case   600'016: return UDAControl::WELTARG_THP;
    case 1'000'016: return UDAControl::WELTARG_LIFT;
    }

    throw std::logic_error {
        "Unknown UDA integer control code " + std::to_string(uda_code)
    };
}

std::string controlName(const UDAControl control)
{
    switch (control) {
    case UDAControl::GCONPROD_OIL_TARGET:
        return "GCONPROD_ORAT";

    case UDAControl::GCONPROD_WATER_TARGET:
        return "GCONPROD_WRAT";

    case UDAControl::GCONPROD_GAS_TARGET:
        return "GCONPROD_GRAT";

    case UDAControl::GCONPROD_LIQUID_TARGET:
        return "GCONPROD_LRAT";

    case UDAControl::GCONINJE_SURFACE_MAX_RATE:
        return "GCONINJE_SURFACE_RATE";

    case UDAControl::GCONINJE_RESV_MAX_RATE:
        return "GCONINJE_RESERVOIR_RATE";

    case UDAControl::GCONINJE_TARGET_REINJ_FRACTION:
        return "GCONINJE_REINJ_FRACTION";

    case UDAControl::GCONINJE_TARGET_VOID_FRACTION:
        return "GCONINJE_VOID_FRACTION";

    case UDAControl::WCONPROD_ORAT:
        return "WCONPROD_ORAT";

    case UDAControl::WCONPROD_GRAT:
        return "WCONPROD_GRAT";

    case UDAControl::WCONPROD_WRAT:
        return "WCONPROD_WRAT";

    case UDAControl::WCONPROD_LRAT:
        return "WCONPROD_LRAT";

    case UDAControl::WCONPROD_RESV:
        return "WCONPROD_RESV";

    case UDAControl::WCONPROD_BHP:
        return "WCONPROD_BHP";

    case UDAControl::WCONPROD_THP:
        return "WCONPROD_THP";

    case UDAControl::WCONINJE_RATE:
        return "WCONINJE_RATE";

    case UDAControl::WCONINJE_RESV:
        return "WCONINJE_RESV";

    case UDAControl::WCONINJE_BHP:
        return "WCONINJE_BHP";

    case UDAControl::WCONINJE_THP:
        return "WCONINJE_THP";

    case UDAControl::WELTARG_ORAT: return "WELTARG_ORAT";
    case UDAControl::WELTARG_WRAT: return "WELTARG_WRAT";
    case UDAControl::WELTARG_GRAT: return "WELTARG_GRAT";
    case UDAControl::WELTARG_LRAT: return "WELTARG_LRAT";
    case UDAControl::WELTARG_RESV: return "WELTARG_RESV";
    case UDAControl::WELTARG_BHP:  return "WELTARG_BHP";
    case UDAControl::WELTARG_THP:  return "WELTARG_THP";
    case UDAControl::WELTARG_LIFT: return "WELTARG_LIFT";
    }

    throw std::logic_error {
        "Unknown UDA control keyword '" +
        std::to_string(static_cast<int>(control)) + '\''
    };
}


}} // Opm::UDQ
