/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'config_content_node.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/hash.h>

#include <ert/config/config_content_node.h>
#include <ert/config/config_schema_item.h>
#include <ert/config/config_path_elm.h>


int main(int argc , char ** argv) {
  config_schema_item_type * schema = config_schema_item_alloc("TEST" , true);
  config_root_path_type * root_path = config_root_path_alloc( NULL );
  config_path_elm_type * cwd = config_path_elm_alloc( root_path , NULL );
  {
    config_content_node_type * node = config_content_node_alloc( schema , cwd );
    config_content_node_add_value( node , "KEY1:VALUE1" );
    config_content_node_add_value( node , "KEY2:VALUE2" );
    config_content_node_add_value( node , "KEY3:VALUE3" );
    config_content_node_add_value( node , "KEYVALUE" );
    
    test_assert_int_equal( config_content_node_get_size( node ) , 4 );
    test_assert_string_equal( config_content_node_iget( node , 0 ) , "KEY1:VALUE1" );
    test_assert_string_equal( config_content_node_iget( node , 2 ) , "KEY3:VALUE3" );
    
    test_assert_string_equal( config_content_node_get_full_string( node , ",") , "KEY1:VALUE1,KEY2:VALUE2,KEY3:VALUE3,KEYVALUE");
    
    {
      hash_type * opt_hash = hash_alloc( );
      {
        config_content_node_init_opt_hash( node , opt_hash , 0 );
        test_assert_int_equal( hash_get_size( opt_hash ) , 3 );
        test_assert_string_equal( hash_get( opt_hash , "KEY1" ) , "VALUE1" );
        test_assert_string_equal( hash_get( opt_hash , "KEY3" ) , "VALUE3" );
      }

      hash_clear( opt_hash );
      test_assert_int_equal( hash_get_size( opt_hash ) , 0 );
      config_content_node_init_opt_hash( node , opt_hash , 1 );
      test_assert_int_equal( hash_get_size( opt_hash ) , 2 );
      test_assert_string_equal( hash_get( opt_hash , "KEY2" ) , "VALUE2" );
      test_assert_string_equal( hash_get( opt_hash , "KEY3" ) , "VALUE3" );      
      test_assert_false( hash_has_key( opt_hash , "KEY1" ) );
      test_assert_false( hash_has_key( opt_hash , "KEYVALUE" ) );
      hash_free( opt_hash );
    }
    

    config_content_node_free( node );
  }
  config_path_elm_free( cwd );
  config_root_path_free( root_path );
  config_schema_item_free( schema );
  exit(0);
}

