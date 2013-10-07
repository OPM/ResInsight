/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_local_obsdata.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/enkf/local_obsdata.h>
#include <ert/enkf/local_obsdata_node.h>


void test_wrapper() {
  local_obsdata_node_type * node = local_obsdata_node_alloc("KEY");
  local_obsdata_type * data = local_obsdata_alloc_wrapper( node );
  test_assert_true( local_obsdata_is_instance( data ));
  test_assert_int_equal( 1 , local_obsdata_get_size( data ));
  test_assert_ptr_equal( node , local_obsdata_iget( data , 0 ));
  test_assert_true( local_obsdata_has_node( data , "KEY" ));
  test_assert_false( local_obsdata_has_node( data , "KEYX" ));
  test_assert_string_equal( local_obsdata_node_get_key( node ) , local_obsdata_get_name( data ));
  local_obsdata_free( data );
}


int main(int argc , char ** argv) {
  local_obsdata_type * obsdata;
  
  obsdata = local_obsdata_alloc( "KEY");
  test_assert_true( local_obsdata_is_instance( obsdata ));
  test_assert_int_equal( 0 , local_obsdata_get_size( obsdata ));
  test_assert_string_equal( "KEY" , local_obsdata_get_name( obsdata ));
  
  {
    local_obsdata_node_type * obsnode = local_obsdata_node_alloc( "KEY" );
    test_assert_true( local_obsdata_add_node( obsdata , obsnode ) );
    test_assert_false( local_obsdata_add_node( obsdata , obsnode ) );
    test_assert_int_equal( 1 , local_obsdata_get_size( obsdata ));
    test_assert_ptr_equal( obsnode , local_obsdata_iget( obsdata , 0));
  }

  local_obsdata_free( obsdata );

  test_wrapper();
  exit(0);
}

