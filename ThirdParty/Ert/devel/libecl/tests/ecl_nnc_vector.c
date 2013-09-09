/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_nnc_vector.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/int_vector.h>

#include <ert/ecl/nnc_vector.h>
 

int main(int argc , char ** argv) {
  int lgr_nr = 100;
  nnc_vector_type * vector = nnc_vector_alloc( lgr_nr );
  
  test_assert_true( nnc_vector_is_instance( vector ));
  test_assert_int_equal( lgr_nr , nnc_vector_get_lgr_nr( vector ));

  nnc_vector_add_nnc( vector , 100 );
  nnc_vector_add_nnc( vector , 200 );
  nnc_vector_add_nnc( vector , 300 );

  nnc_vector_add_nnc( vector , 100 );
  nnc_vector_add_nnc( vector , 200 );
  nnc_vector_add_nnc( vector , 300 );


  {
    const int_vector_type * index_list = nnc_vector_get_index_list( vector );
    
    test_assert_int_equal( 3 , int_vector_size( index_list ));
    test_assert_int_equal( 100 , int_vector_iget( index_list , 0 ));
    test_assert_int_equal( 200 , int_vector_iget( index_list , 1 ));
    test_assert_int_equal( 300 , int_vector_iget( index_list , 2 ));
  }
  
  nnc_vector_free( vector );
  exit(0);
}
