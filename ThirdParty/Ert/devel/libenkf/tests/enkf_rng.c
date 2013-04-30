/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_rng.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <unistd.h>

#include <ert/util/test_util.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/rng_config.h>


int main(int argc , char ** argv) {
  unsigned int rand1,rand2;
  {
    {
      enkf_main_type * enkf_main = enkf_main_alloc_empty();
      enkf_main_resize_ensemble( enkf_main , 10 );
      {
        enkf_state_type * state = enkf_main_iget_state( enkf_main , 9 );
        rand1 = enkf_state_get_random( state );
      }
      enkf_main_free( enkf_main );
    }
    
    {
      enkf_main_type * enkf_main = enkf_main_alloc_empty();
      enkf_main_resize_ensemble( enkf_main , 10 );
      {
        enkf_state_type * state = enkf_main_iget_state( enkf_main , 9 );
        rand2 = enkf_state_get_random( state );
      }
      enkf_main_free( enkf_main );
    }
    test_assert_uint_not_equal( rand1 , rand2 );
  }

  /*****************************************************************/

  {
    const char * seed_file = "/tmp/seed";
    {
      enkf_main_type * enkf_main = enkf_main_alloc_empty();
      {
        rng_config_type * rng_config = enkf_main_get_rng_config( enkf_main );
        rng_config_set_seed_store_file( rng_config , seed_file );
      }
      enkf_main_rng_init( enkf_main );

      enkf_main_resize_ensemble( enkf_main , 10 );
      {
        enkf_state_type * state = enkf_main_iget_state( enkf_main , 9 );
        rand1 = enkf_state_get_random( state );
      }
      enkf_main_free( enkf_main );
    }
    
    {
      enkf_main_type * enkf_main = enkf_main_alloc_empty();
      {
        rng_config_type * rng_config = enkf_main_get_rng_config( enkf_main );
        rng_config_set_seed_load_file( rng_config , seed_file );
      }
      enkf_main_rng_init( enkf_main );
      
      enkf_main_resize_ensemble( enkf_main , 10 );
      {
        enkf_state_type * state = enkf_main_iget_state( enkf_main , 9 );
        rand2 = enkf_state_get_random( state );
      }
      enkf_main_free( enkf_main );
    }
    test_assert_uint_equal( rand1 , rand2 );
    util_unlink_existing( seed_file );
  }
  /*****************************************************************/
  {
    {
      enkf_main_type * enkf_main = enkf_main_bootstrap( NULL , argv[1] , true , true );
      enkf_state_type * state = enkf_main_iget_state( enkf_main , 9 );
      rand1 = enkf_state_get_random( state );
      enkf_main_free( enkf_main );
    }

    {
      enkf_main_type * enkf_main = enkf_main_bootstrap( NULL , argv[1] , true , true );
      enkf_state_type * state = enkf_main_iget_state( enkf_main , 9 );
      rand2 = enkf_state_get_random( state );
      enkf_main_free( enkf_main );
    }
    test_assert_uint_equal( rand1 , rand2 );
    
    {
      enkf_main_type * enkf_main = enkf_main_bootstrap( NULL , argv[1] , true , true );
      enkf_state_type * state = enkf_main_iget_state( enkf_main , 9 );
      rand2 = enkf_state_get_random( state );
      enkf_main_free( enkf_main );
    }
    test_assert_uint_equal( rand1 , rand2 );
  }
  exit(0);
}

