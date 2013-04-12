/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'config_root_path.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/config/config.h>
#include <ert/config/config_root_path.h>


int main(int argc , char ** argv) {
#ifdef ERT_LINUX
  const char * root = "/tmp/root";
  const char * rel_path = "rel/path";
  const char * abs_path = "/tmp/root/abs/path";
  const char * rel_true = "rel/path/XXX";
  const char * abs_true = "/tmp/root/rel/path/XXX";
  const char * path_true1 = "rel/path/XXX";
  const char * path_true2 = "/tmp/root/rel/path/XXX";
#endif
  
  char * cwd = util_alloc_cwd();

  {
    config_root_path_type * root_path = config_root_path_alloc( NULL );
    
    if (!test_string_equal( config_root_path_get_abs_path( root_path ) , cwd ))
      test_error_exit("abs:path:%s   expeceted:%s \n",config_root_path_get_abs_path( root_path ) , cwd );

    if (!test_string_equal( config_root_path_get_input_path( root_path ) , NULL ))
      test_error_exit("input:path:%s   expeceted:%s \n",config_root_path_get_input_path( root_path ) , NULL );
    
    if (!test_string_equal( config_root_path_get_rel_path( root_path ) , NULL ))
      test_error_exit("rel:path:%s   expeceted:%s \n",config_root_path_get_rel_path( root_path ) , NULL );

    
    config_root_path_free( root_path );
  }


  {
    config_root_path_type * root_path = config_root_path_alloc( "/does/not/exist" );
    if (root_path != NULL)
      test_error_exit("Created root_path instance for not-existing input \n");
  }
  

  
  {
    const char * input_path = argv[1];
    char * cwd      = util_alloc_cwd();
    char * rel_path = util_alloc_rel_path( cwd , input_path );

    config_root_path_type * root_path1 = config_root_path_alloc( input_path );
    config_root_path_type * root_path2 = config_root_path_alloc( rel_path );

    if (!test_string_equal( config_root_path_get_rel_path( root_path1 ) , config_root_path_get_rel_path( root_path2 )))
      test_error_exit("Rel: %s != %s \n",config_root_path_get_rel_path( root_path1 ) , config_root_path_get_rel_path( root_path2));
    
    if (!test_string_equal( config_root_path_get_abs_path( root_path1 ) , config_root_path_get_abs_path( root_path2 )))
      test_error_exit("Abs: %s != %s \n",config_root_path_get_abs_path( root_path1 ) , config_root_path_get_abs_path( root_path2 ));
    
    config_root_path_free( root_path1 );
    config_root_path_free( root_path2 );
  }


  exit(0);
}

