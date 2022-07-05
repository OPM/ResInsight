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

#ifndef UDQ_ENUMS_HPP
#define UDQ_ENUMS_HPP

#include <string>
#include <vector>

namespace Opm {

/*
  The UDQ variables can be of of many different types. In addition they can be
  either scalars or vector sets. The arch example of a vector set is well
  variables - in the expressions:

    UDQ
       DEFINE WUBHP WBHP * 1.15 /
       DEFINE WUORAT 1000 /
    /

  we define two UDQ values 'WUBHP' and 'WUORAT'. Both of these UDQ values will
  apply to all wells; the WUBHP vector will correspond to the normal BHP scaled
  up 15%, the WUORAT has the scalar value 1000 for all the wells. The well sets
  can be qualified with a wellname, if the wellname has a wildcard we will get a
  well set, if the wellname is fully qualified we have a scalar:

  UDQ
     DEFINE WUWCT   WWCT 'OP*' /
     DEFINE FUORAT  WOPR 'OPX' * 100 /
  /

  Here the UDQ WUCWT corresponds to the well WWCT for all wells matching the
  name 'OP*', and it is undefined for the remaing wells. The UDQ FUORAT is a
  scalar, given by the WOPR of well 'OPX' - multiplied by 100.

  There are clearly rules for how the different variable types can be combined
  in expressions, and what will be resulting type from an expression -
  unfortunately that is not yet very well implemented in the opm codebase. In
  UDQParser.cpp there is a function static_type_check and in UDQDefine there is
  a function dynamic_type_check - these functions try to verfiy that the type
  conversions are legitimate, but currently they are woefully inadequate.
*/

enum class UDQVarType {
    NONE = 0,
    SCALAR = 1,
    CONNECTION_VAR = 2,
    FIELD_VAR = 3,
    REGION_VAR = 4,
    SEGMENT_VAR = 5,
    AQUIFER_VAR = 6,
    BLOCK_VAR = 7,
    WELL_VAR = 8,
    GROUP_VAR = 9
};

enum class UDQTokenType{
    error = 0,
    number = 1,
    open_paren = 2,
    close_paren = 3,
    comp_expr = 6,
    ecl_expr = 7,
    //
    binary_op_add = 8,
    binary_op_sub = 9,
    binary_op_div = 10,
    binary_op_mul = 11,
    binary_op_pow = 12,
    binary_op_uadd = 13,
    binary_op_umul = 14,
    binary_op_umin = 15,
    binary_op_umax = 16,
    binary_cmp_eq  = 17,
    binary_cmp_ne  = 18,
    binary_cmp_le  = 19,
    binary_cmp_ge  = 20,
    binary_cmp_lt  = 21,
    binary_cmp_gt  = 22,
    //
    elemental_func_randn = 23,
    elemental_func_randu = 24,
    elemental_func_rrandn = 25,
    elemental_func_rrandu = 26,
    elemental_func_abs = 27,
    elemental_func_def = 28,
    elemental_func_exp = 29,
    elemental_func_idv = 30,
    elemental_func_ln = 31,
    elemental_func_log = 32,
    elemental_func_nint = 33,
    elemental_func_sorta = 34,
    elemental_func_sortd = 35,
    elemental_func_undef = 36,
    //
    scalar_func_sum = 37,
    scalar_func_avea = 38,
    scalar_func_aveg = 39,
    scalar_func_aveh = 40,
    scalar_func_max = 41,
    scalar_func_min = 42,
    scalar_func_norm1 = 43,
    scalar_func_norm2 = 44,
    scalar_func_normi = 45,
    scalar_func_prod = 46,
    //
    table_lookup = 47,
    //
    end = 100
};

enum class UDQAction {
    ASSIGN,
    DEFINE,
    UNITS,
    UPDATE
};

enum class UDQUpdate {
    ON,
    OFF,
    NEXT
};

enum class UDAControl {
    WCONPROD_ORAT,
    WCONPROD_WRAT,
    WCONPROD_GRAT,
    WCONPROD_LRAT,
    WCONPROD_RESV,
    WCONPROD_BHP,
    WCONPROD_THP,
    //
    WCONINJE_RATE,
    WCONINJE_RESV,
    WCONINJE_BHP,
    WCONINJE_THP,
    //
    GCONPROD_OIL_TARGET,
    GCONPROD_WATER_TARGET,
    GCONPROD_GAS_TARGET,
    GCONPROD_LIQUID_TARGET,
    //
    GCONINJE_SURFACE_MAX_RATE,
    GCONINJE_RESV_MAX_RATE,
    GCONINJE_TARGET_REINJ_FRACTION,
    GCONINJE_TARGET_VOID_FRACTION,
    //
    WELTARG_ORAT,
    WELTARG_WRAT,
    WELTARG_GRAT,
    WELTARG_LRAT,
    WELTARG_RESV,
    WELTARG_BHP,
    WELTARG_THP,
    WELTARG_LIFT,
};

enum class UDAKeyword {
    WCONPROD,
    WCONINJE,
    WELTARG,
    GCONINJE,
    GCONPROD,
};

namespace UDQ {

    UDQVarType targetType(const std::string& keyword, const std::vector<std::string>& selector);
    UDQVarType targetType(const std::string& keyword);
    UDQVarType varType(const std::string& keyword);
    UDQVarType coerce(UDQVarType t1, UDQVarType t2);
    UDQAction actionType(const std::string& action_string);
    UDQUpdate updateType(const std::string& update_string);
    UDQUpdate updateType(int int_value);
    UDQTokenType tokenType(const std::string& func_name);
    UDQTokenType funcType(const std::string& func_name);
    bool binaryFunc(UDQTokenType token_type);
    bool elementalUnaryFunc(UDQTokenType token_type);
    bool scalarFunc(UDQTokenType token_type);
    bool cmpFunc(UDQTokenType token_type);
    bool setFunc(UDQTokenType token_type);
    bool trailingSpace(UDQTokenType token_type);
    bool leadingSpace(UDQTokenType token_type);
    bool group_control(UDAControl control);
    bool well_control(UDAControl control);
    bool is_well_injection_control(UDAControl control, const bool isInjector);
    bool is_well_production_control(UDAControl control, const bool isProducer);
    bool is_group_injection_control(UDAControl control);
    bool is_group_production_control(UDAControl control);

    std::string typeName(UDQVarType var_type);
    std::string controlName(UDAControl control);
    UDAKeyword keyword(UDAControl control);
    int udaCode(UDAControl control);
    UDAControl udaControl(int uda_code);

    constexpr double restart_default = -0.3E+21;
} // UDQ
} // Opm

#endif  // UDQ_ENUMS_HPP
