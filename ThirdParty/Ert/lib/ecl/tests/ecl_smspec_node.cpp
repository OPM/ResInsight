/*
   Copyright (C) 2017  Equinor ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.

   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#include <stdlib.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/test_util.hpp>

#include <ert/ecl/smspec_node.hpp>


static void test_identify_rate_variable() {
  const char* rate_vars[] = {
    "WOPR" , "GGPR" , "FWPR" , "WLPR" , "WOIR" , "FGIR" , "GWIR" ,
    "WLIR" , "GGOR" , "FWCT" , "SOFR" , "SGFR" , "SWFR" ,
  };

  const auto n_var = sizeof(rate_vars) / sizeof(rate_vars[0]);

  for (auto var = rate_vars, end = rate_vars + n_var; var != end; ++var)
    test_assert_true(smspec_node_identify_rate(*var));

  test_assert_false(smspec_node_identify_rate("SPR"));
  test_assert_false(smspec_node_identify_rate("SOFT"));
  test_assert_false(smspec_node_identify_rate("SGFT"));
  test_assert_false(smspec_node_identify_rate("SWFT"));
}

static void test_identify_total_variable() {
  test_assert_true(smspec_node_identify_total("WOPT", ECL_SMSPEC_WELL_VAR));
  test_assert_true(smspec_node_identify_total("GGPT", ECL_SMSPEC_GROUP_VAR));
  test_assert_true(smspec_node_identify_total("FWPT", ECL_SMSPEC_FIELD_VAR));
  test_assert_true(smspec_node_identify_total("RGIT", ECL_SMSPEC_REGION_VAR));
  test_assert_true(smspec_node_identify_total("CWIT", ECL_SMSPEC_COMPLETION_VAR));

  test_assert_true(smspec_node_identify_total("WOPTF", ECL_SMSPEC_WELL_VAR));
  test_assert_true(smspec_node_identify_total("GOPTS", ECL_SMSPEC_GROUP_VAR));
  test_assert_true(smspec_node_identify_total("FOIT", ECL_SMSPEC_FIELD_VAR));
  test_assert_true(smspec_node_identify_total("ROVPT", ECL_SMSPEC_REGION_VAR));
  test_assert_true(smspec_node_identify_total("COVIT", ECL_SMSPEC_COMPLETION_VAR));

  test_assert_true(smspec_node_identify_total("WMWT", ECL_SMSPEC_WELL_VAR));
  test_assert_true(smspec_node_identify_total("GWVPT", ECL_SMSPEC_GROUP_VAR));
  test_assert_true(smspec_node_identify_total("FWVIT", ECL_SMSPEC_FIELD_VAR));
  test_assert_true(smspec_node_identify_total("RGMT", ECL_SMSPEC_REGION_VAR));
  test_assert_true(smspec_node_identify_total("CGPTF", ECL_SMSPEC_COMPLETION_VAR));

  test_assert_true(smspec_node_identify_total("WSGT", ECL_SMSPEC_WELL_VAR));
  test_assert_true(smspec_node_identify_total("GGST", ECL_SMSPEC_GROUP_VAR));
  test_assert_true(smspec_node_identify_total("FFGT", ECL_SMSPEC_FIELD_VAR));
  test_assert_true(smspec_node_identify_total("RGCT", ECL_SMSPEC_REGION_VAR));
  test_assert_true(smspec_node_identify_total("CGIMT", ECL_SMSPEC_COMPLETION_VAR));

  test_assert_true(smspec_node_identify_total("WWGPT", ECL_SMSPEC_WELL_VAR));
  test_assert_true(smspec_node_identify_total("GWGIT", ECL_SMSPEC_GROUP_VAR));
  test_assert_true(smspec_node_identify_total("FEGT", ECL_SMSPEC_FIELD_VAR));
  test_assert_true(smspec_node_identify_total("REXGT", ECL_SMSPEC_REGION_VAR));
  test_assert_true(smspec_node_identify_total("CGVPT", ECL_SMSPEC_COMPLETION_VAR));

  test_assert_true(smspec_node_identify_total("WGVIT", ECL_SMSPEC_WELL_VAR));
  test_assert_true(smspec_node_identify_total("GLPT", ECL_SMSPEC_GROUP_VAR));
  test_assert_true(smspec_node_identify_total("FVPT", ECL_SMSPEC_FIELD_VAR));
  test_assert_true(smspec_node_identify_total("RVIT", ECL_SMSPEC_REGION_VAR));
  test_assert_true(smspec_node_identify_total("CNPT", ECL_SMSPEC_COMPLETION_VAR));

  test_assert_true(smspec_node_identify_total("WNIT", ECL_SMSPEC_WELL_VAR));
  test_assert_true(smspec_node_identify_total("GCPT", ECL_SMSPEC_GROUP_VAR));
  test_assert_true(smspec_node_identify_total("FCIT", ECL_SMSPEC_FIELD_VAR));

  test_assert_true(smspec_node_identify_total("SOFT", ECL_SMSPEC_SEGMENT_VAR));
  test_assert_true(smspec_node_identify_total("SGFT", ECL_SMSPEC_SEGMENT_VAR));
  test_assert_true(smspec_node_identify_total("SWFT", ECL_SMSPEC_SEGMENT_VAR));

  test_assert_false(smspec_node_identify_total("SOPT", ECL_SMSPEC_SEGMENT_VAR));
  test_assert_false(smspec_node_identify_total("HEI!", ECL_SMSPEC_SEGMENT_VAR));
  test_assert_false(smspec_node_identify_total("xXx", ECL_SMSPEC_SEGMENT_VAR));
  test_assert_false(smspec_node_identify_total("SPR", ECL_SMSPEC_SEGMENT_VAR));
}


void test_nums_default() {
  ecl::smspec_node field_node( 0, "FOPT" , "UNIT" , 0);
  ecl::smspec_node group_node( 0, "GOPR" , "G1", "UNIT" , 0, ":");
  ecl::smspec_node well_node( 0, "WOPR" , "W1", "UNIT" , 0, ":");

  int default_nums = 0;
  /*
    The integer constant default nums corresponds to the symbol nums_unused
    in smspec_node.cpp. It is duplicated here to avoid exporting it - it should
    not really be a publically available symbol.
  */

  test_assert_int_equal( field_node.get_num(), default_nums);
  test_assert_int_equal( group_node.get_num(), default_nums);
  test_assert_int_equal( well_node.get_num(), default_nums);
}


void test_cmp_types() {
  const int dims[3] = {10,10,10};
  ecl::smspec_node field_node( 0, "FOPT" , "UNIT" , 0);
  ecl::smspec_node region_node( 0, "RPR" , 10, "UNIT" , dims , 0 , ":");
  ecl::smspec_node group_node( 0, "GOPR" , "G1", "UNIT" , 0, ":");
  ecl::smspec_node well_node( 0, "WOPR" , "W1", "UNIT" , 0, ":");
  ecl::smspec_node block_node( 0, "BPR", 10, "UNIT", dims, 0, ":");
  ecl::smspec_node aquifer_node( 0, "AAQP" , 10, "UNIT" , dims, 0 , ":");
  ecl::smspec_node segment_node( 0, "SGOR" , "W1" , 10, "UNIT" , dims , 0 , ":");
  ecl::smspec_node misc_node1( 0, "TIME" , "UNIT", 0 );
  ecl::smspec_node misc_node2( 0, "TCPU", "UNIT", 0);

  test_assert_int_equal( field_node.cmp(field_node), 0);
  test_assert_int_equal( region_node.cmp(region_node), 0);
  test_assert_int_equal( well_node.cmp(well_node), 0);
  test_assert_int_equal( group_node.cmp(group_node), 0);
  test_assert_int_equal( block_node.cmp(block_node), 0);

  test_assert_true( misc_node1.cmp(field_node) < 0 );
  test_assert_true( field_node.cmp(region_node) < 0 );
  test_assert_true( region_node.cmp(group_node) < 0 );
  test_assert_true( group_node.cmp(well_node) < 0 );
  test_assert_true( well_node.cmp(segment_node) < 0 );
  test_assert_true( segment_node.cmp(block_node) < 0 );
  test_assert_true( block_node.cmp(aquifer_node)< 0 );
  test_assert_true( aquifer_node.cmp(misc_node2) < 0 );

  test_assert_true( field_node.cmp(misc_node1) > 0 );
  test_assert_true( misc_node2.cmp(aquifer_node) > 0 );
  test_assert_true( misc_node1.cmp(misc_node2) < 0 );
  test_assert_true( misc_node2.cmp(misc_node1) > 0 );
}

void test_cmp_well() {
  ecl::smspec_node well_node1_1( 0 ,  "WOPR" ,"W1" , "UNIT" , 10 ,":");
  ecl::smspec_node well_node1_2( 0 ,  "WOPR" ,"W2" , "UNIT" , 10 ,":");
  ecl::smspec_node well_node2_1( 0 ,  "WWCT" ,"W1" , "UNIT" , 10 ,":");
  ecl::smspec_node well_node2_2( 0 ,  "WWWT" ,"W2" , "UNIT" , 10 ,":");

  test_assert_int_equal( well_node1_1.cmp(well_node1_1), 0);
  test_assert_int_equal( well_node2_2.cmp(well_node2_2), 0);

  test_assert_true( well_node1_1.cmp(well_node1_2)< 0 );
  test_assert_true( well_node1_1.cmp(well_node2_1)< 0 );
  test_assert_true( well_node1_1.cmp(well_node2_2)< 0 );

  test_assert_true( well_node1_2.cmp(well_node1_1)> 0 );
  test_assert_true( well_node1_2.cmp(well_node2_1)< 0 );
}



void test_cmp_region() {
  const int dims[3] = {10,10,10};
  ecl::smspec_node region_node1_1( 0, "ROIP" ,  10 ,"UNIT" ,  dims , 0 , ":" );
  ecl::smspec_node region_node1_2( 0, "ROIP" ,  11 ,"UNIT" ,  dims , 0 , ":" );
  ecl::smspec_node region_node2_1( 0, "RPR" ,   10 ,"UNIT" ,  dims , 0 , ":" );
  ecl::smspec_node region_node2_2( 0, "RPR" ,   12 ,"UNIT" ,  dims , 0 , ":" );

  test_assert_true( region_node1_1.cmp(region_node1_2)< 0 );
  test_assert_true( region_node1_1.cmp(region_node2_1)< 0 );
  test_assert_true( region_node1_1.cmp(region_node2_2)< 0 );

  test_assert_true( region_node1_2.cmp(region_node1_1)> 0 );
  test_assert_true( region_node1_2.cmp(region_node2_1)< 0 );
}


int main(int argc, char ** argv) {
  util_install_signals();
  test_cmp_types();
  test_cmp_well();
  test_cmp_region( );
  test_identify_rate_variable();
  test_identify_total_variable();
  test_nums_default();
}
