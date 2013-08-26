/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_rft_cell.c' is part of ERT - Ensemble based Reservoir Tool. 
   
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

#include <ert/util/test_util.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_rft_node.h>
#include <ert/ecl/ecl_rft_cell.h>



void test_rft_cell() {
  const int i = 10;
  const int j = 11;
  const int k = 12;

  const double depth = 100;
  const double pressure = 200;
  const double swat = 0.25;
  const double sgas = 0.35;
  
  ecl_rft_cell_type * cell = ecl_rft_cell_alloc_RFT(i,j,k,depth,pressure,swat,sgas);
  
  test_assert_int_equal( i , ecl_rft_cell_get_i( cell ));
  test_assert_int_equal( j , ecl_rft_cell_get_j( cell ));
  test_assert_int_equal( k , ecl_rft_cell_get_k( cell ));

  {
    int ii,jj,kk;
    ecl_rft_cell_get_ijk( cell , &ii , &jj , &kk);
    test_assert_int_equal( i , ii);
    test_assert_int_equal( j , jj);
    test_assert_int_equal( k , kk);
  }

  test_assert_double_equal( depth , ecl_rft_cell_get_depth( cell ));
  test_assert_double_equal( pressure , ecl_rft_cell_get_pressure( cell ));
  test_assert_double_equal( swat  , ecl_rft_cell_get_swat( cell ));
  test_assert_double_equal( sgas  , ecl_rft_cell_get_sgas( cell ));
  test_assert_double_equal( 1 - (swat + sgas) , ecl_rft_cell_get_soil( cell ));

  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_orat( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_grat( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_wrat( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_flowrate( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_connection_start( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_connection_end( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_oil_flowrate( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_gas_flowrate( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_water_flowrate( cell ));

  ecl_rft_cell_free( cell );
  {
    ecl_rft_cell_type * cell = ecl_rft_cell_alloc_RFT(i,j,k,depth,pressure,swat,sgas);
    
    test_assert_true( ecl_rft_cell_ijk_equal( cell , i , j  , k  ));
    test_assert_false( ecl_rft_cell_ijk_equal( cell , i , j  , k + 1 ));
    
    ecl_rft_cell_free__( cell );
  }
}



void test_plt_cell() {
  const int i = 10;
  const int j = 11;
  const int k = 12;

  const double depth = 100;
  const double pressure = 200;
  const double orat = 0.25;
  const double grat = 0.35;
  const double wrat = 0.45;
  const double flowrate = 100.0;
  const double connection_start = 891;
  const double connection_end = 979; 
  const double oil_flowrate = 0.891;
  const double gas_flowrate = 7771;
  const double water_flowrate = 77614;

  ecl_rft_cell_type * cell = ecl_rft_cell_alloc_PLT(i,j,k,depth,pressure,orat,grat,wrat,connection_start,connection_end, flowrate,oil_flowrate , gas_flowrate , water_flowrate);
  
  test_assert_int_equal( i , ecl_rft_cell_get_i( cell ));
  test_assert_int_equal( j , ecl_rft_cell_get_j( cell ));
  test_assert_int_equal( k , ecl_rft_cell_get_k( cell ));

  {
    int ii,jj,kk;
    ecl_rft_cell_get_ijk( cell , &ii , &jj , &kk);
    test_assert_int_equal( i , ii);
    test_assert_int_equal( j , jj);
    test_assert_int_equal( k , kk);
  }

  test_assert_double_equal( depth , ecl_rft_cell_get_depth( cell ));
  test_assert_double_equal( pressure , ecl_rft_cell_get_pressure( cell ));

  test_assert_double_equal( orat , ecl_rft_cell_get_orat( cell ));
  test_assert_double_equal( grat , ecl_rft_cell_get_grat( cell ));
  test_assert_double_equal( wrat , ecl_rft_cell_get_wrat( cell ));
  test_assert_double_equal( connection_start, ecl_rft_cell_get_connection_start( cell ));
  test_assert_double_equal( connection_end, ecl_rft_cell_get_connection_end( cell ));
  test_assert_double_equal(flowrate , ecl_rft_cell_get_flowrate( cell ));
  
  test_assert_double_equal(oil_flowrate   , ecl_rft_cell_get_oil_flowrate( cell ));
  test_assert_double_equal(gas_flowrate   , ecl_rft_cell_get_gas_flowrate( cell ));
  test_assert_double_equal(water_flowrate , ecl_rft_cell_get_water_flowrate( cell ));
  
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_swat( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_sgas( cell ));
  test_assert_double_equal( ECL_RFT_CELL_INVALID_VALUE , ecl_rft_cell_get_soil( cell ));

  ecl_rft_cell_free( cell );
}





int main( int argc , char ** argv) {
  
  test_rft_cell();
  test_plt_cell();
  exit(0);
}
