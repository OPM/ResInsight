/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'well_conn_collection.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


void test_empty() {
  well_conn_collection_type * wellcc = well_conn_collection_alloc( );
  test_assert_not_NULL( wellcc );
  test_assert_true( well_conn_collection_is_instance( wellcc ));
  
  test_assert_int_equal( 0 , well_conn_collection_get_size( wellcc ));
  {
    well_conn_type * conn = well_conn_collection_iget( wellcc , 0 );
    test_assert_NULL( conn );
  }
  {
    const well_conn_type * conn = well_conn_collection_iget_const( wellcc , 10 );
    test_assert_NULL( conn );
  }
  
  well_conn_collection_free( wellcc );
}



int main(int argc , char ** argv) {
  test_empty();
  exit(0);
}
