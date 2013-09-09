/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_meas_data.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/int_vector.h>

#include <ert/enkf/meas_data.h>



void create_test() {
  int_vector_type * ens_active_list = int_vector_alloc(0 , false);
  int_vector_append( ens_active_list , 10 );
  int_vector_append( ens_active_list , 20 );
  int_vector_append( ens_active_list , 30 );

  {
    meas_data_type * meas_data = meas_data_alloc( ens_active_list );
    test_assert_int_equal( 3 , meas_data_get_ens_size( meas_data ));
    
    meas_data_free( meas_data );
  }
  
  int_vector_free( ens_active_list );
}



int main(int argc , char ** argv) {
  create_test();
  exit(0);
}

