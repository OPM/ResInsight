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
#include <ert/util/test_work_area.h>

#include <ert/config/config.h>
#include <ert/config/config_path_elm.h>
#include <ert/config/config_root_path.h>

int main(int argc , char ** argv) {
#ifdef ERT_LINUX
  const char * rel_path = "rel/path";
  const char * rel_true = "rel/path/XXX";
  const char * path_true1 = "rel/path/XXX";
  
  
#endif
  test_work_area_type * work_area = test_work_area_alloc( "config_path_elm" , true );
  const char * root = test_work_area_get_cwd( work_area );
  char * abs_path = util_alloc_filename( root , "rel/path" , NULL);
  char * abs_true = util_alloc_filename( root , "rel/path/XXX" , NULL);
  char * path_true2 = util_alloc_filename( root , "rel/path/XXX" , NULL);

  chdir( test_work_area_get_original_cwd( work_area ));
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
  {
    config_path_elm_type * path_elm = config_path_elm_alloc( root_path , abs_path );
  
    test_assert_string_equal( config_path_elm_get_relpath( path_elm ) , rel_path );
    test_assert_string_equal( config_path_elm_get_abspath( path_elm ) , abs_path );

    test_assert_string_equal( config_path_elm_alloc_relpath( path_elm , "XXX" ) , rel_true);
    test_assert_string_equal( config_path_elm_alloc_abspath( path_elm , "XXX" ) , abs_true);
    test_assert_string_equal( config_path_elm_alloc_path( path_elm , "XXX" ) , path_true2 );
    
    config_path_elm_free( path_elm );
  }
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
  
  exit(0);
}

