/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ert_util_rng.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/util.h>
#include <ert/util/rng.h>
#include <ert/util/mzran.h>


#define MAX_INT 666661

int main(int argc , char ** argv) {
  rng_type * rng = rng_alloc( MZRAN , INIT_DEFAULT ); 
  {
    int val1 = rng_get_int( rng , MAX_INT);
    int val2 = rng_get_int( rng , MAX_INT);

    test_assert_int_not_equal( val1 , val2 );
    
    rng_init( rng , INIT_DEFAULT );
    val2 = rng_get_int( rng , MAX_INT );
    test_assert_int_equal( val1 , val2 );
  }
  {
    int val2 , val1;
    int state_size = rng_state_size( rng );
    char * buffer1 = util_calloc( state_size , sizeof * buffer1 );
    char * buffer2 = util_calloc( state_size , sizeof * buffer2 );
    test_assert_int_not_equal( state_size , 0 );
    test_assert_int_equal( state_size , MZRAN_STATE_SIZE );
    
    rng_init( rng , INIT_DEFAULT );
    rng_get_state( rng , buffer1 );
    val1 = rng_get_int( rng , MAX_INT);
    val2 = rng_get_int( rng , MAX_INT);
    
    test_assert_int_not_equal( val1 , val2 );
    rng_set_state( rng , buffer1 );
    val2 = rng_get_int( rng , MAX_INT);
    test_assert_int_equal( val1 , val2 );

    rng_init( rng , INIT_DEFAULT );
    rng_get_state( rng , buffer2 );
    test_assert_mem_equal( buffer1 , buffer2 , state_size );
    val2 = rng_get_int( rng , MAX_INT);
    rng_get_state( rng , buffer2 );
    test_assert_mem_not_equal( buffer1 , buffer2 , state_size );
    
    free( buffer1 );
    free( buffer2 );
  }
  rng_free( rng );
  exit(0);
}
