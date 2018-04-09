/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'well_conn.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/stringlist.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>


#include <ert/ecl_well/well_conn_collection.h>
#include <ert/ecl_well/well_conn.h>

void test_conn_rate(){
  int i = 10;
  int j = 5;
  int k = 16;
  double CF = 0;
  bool open = true;

  well_conn_dir_enum dir = well_conn_dirX;
  well_conn_type * conn = well_conn_alloc(i,j,k,CF,dir,open);

  test_assert_double_equal(0.0, well_conn_get_oil_rate(conn));
  test_assert_double_equal(0.0, well_conn_get_gas_rate(conn));
  test_assert_double_equal(0.0, well_conn_get_water_rate(conn));
  test_assert_double_equal(0.0, well_conn_get_volume_rate(conn));

  test_assert_double_equal(0.0, well_conn_get_oil_rate_si(conn));
  test_assert_double_equal(0.0, well_conn_get_gas_rate_si(conn));
  test_assert_double_equal(0.0, well_conn_get_water_rate_si(conn));
  test_assert_double_equal(0.0, well_conn_get_volume_rate_si(conn));

  well_conn_free( conn );
}


int main(int argc , char ** argv) {
  int i = 10;
  int j = 5;
  int k = 16;
  double CF = 0;
  bool open = true;
  test_install_SIGNALS();

  {
    well_conn_dir_enum dir = well_conn_dirX;
    well_conn_type * conn = well_conn_alloc(i,j,k,CF,dir,open);
    well_conn_type * conn2 = well_conn_alloc(i,j,k,CF,dir,open);
    well_conn_type * conn3 = well_conn_alloc(i,j,k+1,CF,dir,open);
    test_assert_not_NULL( conn );
    test_assert_true( well_conn_is_instance( conn ));
    test_assert_int_equal( i , well_conn_get_i( conn ));
    test_assert_int_equal( j , well_conn_get_j( conn ));
    test_assert_int_equal( k , well_conn_get_k( conn ));
    test_assert_int_equal( dir , well_conn_get_dir( conn ));
    test_assert_bool_equal( open , well_conn_open( conn ));
    test_assert_false( well_conn_MSW( conn ));
    test_assert_true( well_conn_matrix_connection( conn ));
    test_assert_true( well_conn_equal( conn , conn2 ));
    test_assert_false( well_conn_equal( conn , conn3 ));
    test_assert_double_equal( CF , well_conn_get_connection_factor( conn ));
    well_conn_free( conn3 );
    well_conn_free( conn2 );
    well_conn_free( conn );
  }

  {
    well_conn_dir_enum dir = well_conn_fracX;
    well_conn_type * conn = well_conn_alloc(i,j,k,CF,dir,open);
    test_assert_NULL( conn );
  }


  {
    well_conn_dir_enum dir = well_conn_fracX;
    well_conn_type * conn = well_conn_alloc_fracture(i,j,k,CF,dir,open);
    test_assert_not_NULL( conn );
    test_assert_int_equal( i , well_conn_get_i( conn ));
    test_assert_int_equal( j , well_conn_get_j( conn ));
    test_assert_int_equal( k , well_conn_get_k( conn ));
    test_assert_bool_equal( open , well_conn_open( conn ));
    test_assert_int_equal( dir , well_conn_get_dir( conn ));
    test_assert_false( well_conn_MSW( conn ));
    test_assert_false( well_conn_matrix_connection( conn ));
    test_assert_true( well_conn_fracture_connection( conn ));
    well_conn_free( conn );
  }


  {
    well_conn_dir_enum dir = well_conn_dirX;
    well_conn_type * conn = well_conn_alloc_fracture(i,j,k,CF,dir,open);
    test_assert_not_NULL( conn );
    well_conn_free( conn );
  }

  {
    int segment = 16;
    well_conn_dir_enum dir = well_conn_dirX;
    well_conn_type * conn = well_conn_alloc_MSW(i,j,k,CF,dir,open,segment);
    test_assert_not_NULL( conn );
    test_assert_int_equal( i , well_conn_get_i( conn ));
    test_assert_int_equal( j , well_conn_get_j( conn ));
    test_assert_int_equal( k , well_conn_get_k( conn ));
    test_assert_int_equal( segment , well_conn_get_segment_id( conn ));
    test_assert_bool_equal( open , well_conn_open( conn ));
    test_assert_int_equal( dir , well_conn_get_dir( conn ));
    test_assert_true( well_conn_MSW( conn ));
    test_assert_true( well_conn_matrix_connection( conn ));
    well_conn_free( conn );
  }


  {
    int segment = 16;
    well_conn_dir_enum dir = well_conn_fracX;
    well_conn_type * conn = well_conn_alloc_fracture_MSW(i,j,k,CF,dir,open,segment);
    test_assert_not_NULL( conn );
    test_assert_int_equal( i , well_conn_get_i( conn ));
    test_assert_int_equal( j , well_conn_get_j( conn ));
    test_assert_int_equal( k , well_conn_get_k( conn ));
    test_assert_int_equal( segment , well_conn_get_segment_id( conn ));
    test_assert_bool_equal( open , well_conn_open( conn ));
    test_assert_int_equal( dir , well_conn_get_dir( conn ));
    test_assert_true( well_conn_MSW( conn ));
    test_assert_false( well_conn_matrix_connection( conn ));
    well_conn_free( conn );
  }

  test_conn_rate();

}
