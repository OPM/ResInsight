/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_nnc_export.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    ll
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

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_nnc_export.h>
#include <ert/ecl/ecl_kw_magic.h>

void test_get_tran(const char * name) {
  char * grid_file_name = ecl_util_alloc_filename(NULL , name , ECL_EGRID_FILE , false  , -1);
  char * init_file_name = ecl_util_alloc_filename(NULL , name , ECL_INIT_FILE , false  , -1);
  ecl_grid_type * grid = ecl_grid_alloc( grid_file_name );
  ecl_file_type * grid_file = ecl_file_open( grid_file_name , 0 );
  ecl_file_type * init_file = ecl_file_open( init_file_name , 0 );

  /* Get global */
  {
    ecl_kw_type * tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANNNC_KW , 0 );
    test_assert_true( ecl_kw_is_instance( tran_kw ));
    test_assert_double_equal( 0.85582769 , ecl_kw_iget_as_double( tran_kw , 0 ));
    test_assert_double_equal( 0.24635284 , ecl_kw_iget_as_double( tran_kw , 7184 ));
  }
  test_assert_NULL( ecl_nnc_export_get_tran_kw( init_file , TRANGL_KW , 0 ));
  test_assert_NULL( ecl_nnc_export_get_tran_kw( init_file , TRANLL_KW , 0 ));
  test_assert_NULL( ecl_nnc_export_get_tran_kw( init_file , "INVALID" , 1));


  /* Get lgr_nr: 48 */
  {
    ecl_kw_type * tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANNNC_KW , 48 );
    test_assert_true( ecl_kw_is_instance( tran_kw ));
    test_assert_int_equal( 0 , ecl_kw_get_size( tran_kw ));
    
    tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANGL_KW , 48 );
    test_assert_int_equal( 282 , ecl_kw_get_size( tran_kw ));
    test_assert_double_equal( 22.922695 , ecl_kw_iget_as_double( tran_kw , 0 ));
    test_assert_double_equal( 16.720325 , ecl_kw_iget_as_double( tran_kw , 281 ));
  }

  /* Get lgr_nr: 99 */
  {
    ecl_kw_type * tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANNNC_KW , 99 );
    test_assert_true( ecl_kw_is_instance( tran_kw ));
    test_assert_int_equal( 0 , ecl_kw_get_size( tran_kw ));
    
    tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANGL_KW , 99 );
    test_assert_int_equal( 693 , ecl_kw_get_size( tran_kw ));
    test_assert_double_equal( 0.25534782 , ecl_kw_iget_as_double( tran_kw , 0 ));
    test_assert_double_equal( 0.12677453 , ecl_kw_iget_as_double( tran_kw ,  692 ));
  }


  /* Get lgr_nr: 10 */
  {
    ecl_kw_type * tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANNNC_KW , 10 );
    test_assert_true( ecl_kw_is_instance( tran_kw ));
    test_assert_int_equal( 0 , ecl_kw_get_size( tran_kw ));
    
    tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANGL_KW , 10 );
    test_assert_int_equal( 260 , ecl_kw_get_size( tran_kw ));
    test_assert_double_equal( 0.87355447 , ecl_kw_iget_as_double( tran_kw , 0 ));
    test_assert_double_equal( 26.921568 , ecl_kw_iget_as_double( tran_kw ,  259 ));
  }


  /* Get lgr_nr: 110 */
  {
    ecl_kw_type * tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANNNC_KW , 110 );
    test_assert_true( ecl_kw_is_instance( tran_kw ));
    test_assert_int_equal( 0 , ecl_kw_get_size( tran_kw ));
    
    tran_kw = ecl_nnc_export_get_tran_kw( init_file , TRANGL_KW , 110 );
    test_assert_int_equal( 208 , ecl_kw_get_size( tran_kw ));
    test_assert_double_equal( 17.287283 , ecl_kw_iget_as_double( tran_kw , 0 ));
    test_assert_double_equal( 569.26312 , ecl_kw_iget_as_double( tran_kw ,  207 ));
  }
  

  free( init_file_name );
  free(grid_file_name);
  ecl_grid_free( grid );
  ecl_file_close( grid_file );
  ecl_file_close( init_file );
}



void test_tranLL( const ecl_grid_type * grid , const ecl_file_type * init_file , int lgr_nr1 , int lgr_nr2, 
                  int size,
                  double first , 
                  double last) {
  
  ecl_kw_type * ecl_kw = ecl_nnc_export_get_tranll_kw(grid , init_file , lgr_nr1 , lgr_nr2 );
  
  printf("lgr: %d -> %d \n",lgr_nr1 , lgr_nr2);
  test_assert_not_NULL(ecl_kw);
  test_assert_true(ecl_kw_is_instance( ecl_kw ));
  test_assert_int_equal( size , ecl_kw_get_size( ecl_kw ));
  test_assert_double_equal( first , ecl_kw_iget_as_double( ecl_kw , 0 ));
  test_assert_double_equal( last , ecl_kw_iget_as_double( ecl_kw , size - 1 ));
}



void test_get_tranLL(const char * name) {
  char * grid_file_name = ecl_util_alloc_filename(NULL , name , ECL_EGRID_FILE , false  , -1);
  char * init_file_name = ecl_util_alloc_filename(NULL , name , ECL_INIT_FILE , false  , -1);
  ecl_grid_type * grid = ecl_grid_alloc( grid_file_name );
  ecl_file_type * grid_file = ecl_file_open( grid_file_name , 0 );
  ecl_file_type * init_file = ecl_file_open( init_file_name , 0 );
  
  test_tranLL( grid , init_file , ecl_grid_get_lgr_nr_from_name( grid , "LG003017" ), ecl_grid_get_lgr_nr_from_name( grid , "LG003018" ),
               172 , 5.3957253 , 1.0099934);

  test_tranLL( grid , init_file , ecl_grid_get_lgr_nr_from_name( grid , "LG002016" ), ecl_grid_get_lgr_nr_from_name( grid , "LG002017" ),
               93 , 1.4638059 , 0.36407200 );
  
  test_tranLL( grid , init_file , ecl_grid_get_lgr_nr_from_name( grid , "LG002016" ), ecl_grid_get_lgr_nr_from_name( grid , "LG003016" ),
               56 , 2.7360380 , 10.053267);

  test_tranLL( grid , init_file , ecl_grid_get_lgr_nr_from_name( grid , "LG009027" ), ecl_grid_get_lgr_nr_from_name( grid , "LG009026" ),
               152 , 155.47754, 219.23553);

  test_tranLL( grid , init_file , ecl_grid_get_lgr_nr_from_name( grid , "LG009027" ), ecl_grid_get_lgr_nr_from_name( grid , "LG008027" ),
               317 , 0.040260997 , 0.0066288318);
  

  free( init_file_name );
  free(grid_file_name);
  ecl_grid_free( grid );
  ecl_file_close( grid_file );
  ecl_file_close( init_file );
}




int main( int argc , char ** argv) {
  test_get_tran( argv[1] );
  test_get_tranLL( argv[1] );
  exit(0);
}

