/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'config_path_elm.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/util.h>

#include <ert/config/config.h>
#include <ert/config/config_path_elm.h>
#include <ert/config/config_root_path.h>

int main(int argc , char ** argv) {
#ifdef ERT_LINUX
  const char * root = "/tmp/root";
  const char * rel_path = "rel/path";
  const char * abs_path = "/tmp/root/rel/path";
  const char * rel_true = "rel/path/XXX";
  const char * abs_true = "/tmp/root/rel/path/XXX";
  const char * path_true1 = "rel/path/XXX";
  const char * path_true2 = "/tmp/root/rel/path/XXX";
  
#endif
  
  util_make_path( root );
  config_root_path_type * root_path = config_root_path_alloc( root );
  {
    config_path_elm_type * path_elm = config_path_elm_alloc( root_path , rel_path );
    
    test_assert_string_equal( config_path_elm_get_relpath( path_elm ) , rel_path );
    test_assert_string_equal( config_path_elm_get_abspath( path_elm ) , abs_path );

    test_assert_string_equal( config_path_elm_alloc_relpath( path_elm , "XXX" ) , rel_true);
    test_assert_string_equal( config_path_elm_alloc_abspath( path_elm , "XXX" ) , abs_true);
    test_assert_string_equal( config_path_elm_alloc_path( path_elm , "XXX" ) , path_true2 );

    
    config_path_elm_free( path_elm );
  }
  printf("test1 OK \n");
  {
    config_path_elm_type * path_elm = config_path_elm_alloc( root_path , abs_path );
  
    test_assert_string_equal( config_path_elm_get_relpath( path_elm ) , rel_path );
    test_assert_string_equal( config_path_elm_get_abspath( path_elm ) , abs_path );

    test_assert_string_equal( config_path_elm_alloc_relpath( path_elm , "XXX" ) , rel_true);
    test_assert_string_equal( config_path_elm_alloc_abspath( path_elm , "XXX" ) , abs_true);
    test_assert_string_equal( config_path_elm_alloc_path( path_elm , "XXX" ) , path_true2 );
    
    config_path_elm_free( path_elm );
  }
  printf("test2 OK \n");
  config_root_path_free( root_path );

  chdir( root );
  root_path = config_root_path_alloc( NULL );
  {
    config_path_elm_type * path_elm = config_path_elm_alloc( root_path , rel_path );
    
    test_assert_string_equal( config_path_elm_get_relpath( path_elm ) , rel_path );
    test_assert_string_equal( config_path_elm_get_abspath( path_elm ) , abs_path );

    test_assert_string_equal( config_path_elm_alloc_relpath( path_elm , "XXX" ) , rel_true);
    test_assert_string_equal( config_path_elm_alloc_abspath( path_elm , "XXX" ) , abs_true);
    test_assert_string_equal( config_path_elm_alloc_path( path_elm , "XXX" ) , path_true1 );

    
    config_path_elm_free( path_elm );
  }
  printf("test3 OK \n");
  
  exit(0);
}

