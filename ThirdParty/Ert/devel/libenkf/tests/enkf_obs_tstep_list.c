/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_obs_tstep_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>

#include <ert/enkf/obs_tstep_list.h>
#include <ert/util/test_util.h>


void test_range( ) {
  obs_tstep_list_type * tstep_list = obs_tstep_list_alloc();
  
  obs_tstep_list_add_range( tstep_list , 10 , 12 );
  test_assert_int_equal( 3, obs_tstep_list_get_size( tstep_list ));
  obs_tstep_list_free( tstep_list );
}

void test_contains() {
  obs_tstep_list_type * tstep_list = obs_tstep_list_alloc();
  
  test_assert_false( obs_tstep_list_contains( tstep_list , 0 ));
  test_assert_false( obs_tstep_list_contains( tstep_list , 10 ));
  test_assert_false( obs_tstep_list_contains( tstep_list , 12 ));
  test_assert_false( obs_tstep_list_contains( tstep_list , 15 ));

  obs_tstep_list_add_range( tstep_list , 10 , 12 );
  test_assert_false( obs_tstep_list_contains( tstep_list , 0 ));
  test_assert_true(  obs_tstep_list_contains( tstep_list , 10 ));
  test_assert_true(  obs_tstep_list_contains( tstep_list , 12 ));
  test_assert_false( obs_tstep_list_contains( tstep_list , 15 ));

  obs_tstep_list_free( tstep_list );
}


int main(int argc , char ** argv) {
  obs_tstep_list_type * tstep_list;

  tstep_list = obs_tstep_list_alloc();
  test_assert_true( obs_tstep_list_is_instance( tstep_list ));
  test_assert_true( obs_tstep_list_all_active( tstep_list ));
  test_assert_int_equal( 0 , obs_tstep_list_get_size( tstep_list ));
  
  obs_tstep_list_add_tstep( tstep_list , 100 );
  test_assert_int_equal( 1 , obs_tstep_list_get_size( tstep_list ));
  test_assert_false( obs_tstep_list_all_active( tstep_list ));

  obs_tstep_list_add_tstep( tstep_list , 101 );
  test_assert_int_equal( 2 , obs_tstep_list_get_size( tstep_list ));

  obs_tstep_list_add_tstep( tstep_list , 101 );
  test_assert_int_equal( 2 , obs_tstep_list_get_size( tstep_list ));

  obs_tstep_list_add_tstep( tstep_list , 1 );
  test_assert_int_equal( 3 , obs_tstep_list_get_size( tstep_list ));
  
  test_assert_int_equal( 1   , obs_tstep_list_iget( tstep_list , 0 ));
  test_assert_int_equal( 100 , obs_tstep_list_iget( tstep_list , 1 ));
  test_assert_int_equal( 101 , obs_tstep_list_iget( tstep_list , 2 ));

  test_assert_int_equal( 101 , obs_tstep_list_get_last( tstep_list ));
  obs_tstep_list_free( tstep_list );


  test_range();
  test_contains();
  exit(0);
}

