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
#include <string.h>
#include <stdio.h>

#include <ert/util/util.h>

#include <ert/config/config_root_path.h>

struct config_root_path_struct {
  char * input_path;
  char * abs_path;
  char * rel_path;
};


/**
   Input must be an existing directory, which will be used as the
   root; or NULL in which case cwd will be used as root. The input
   directory can be both realtive or absolute. 
*/

config_root_path_type * config_root_path_alloc( const char * input_path ) {
  if (input_path == NULL || util_is_directory( input_path )) {
    config_root_path_type * root_path = util_malloc( sizeof * root_path );
    {
      char * cwd = util_alloc_cwd();
      
      root_path->input_path = util_alloc_string_copy( input_path );
      if (input_path == NULL) {
        root_path->rel_path = NULL;
        root_path->abs_path = util_alloc_string_copy( cwd );
      } else {
        if (util_is_abs_path( input_path )) {
          root_path->abs_path = util_alloc_string_copy( input_path );
          root_path->rel_path = util_alloc_rel_path( cwd , root_path->abs_path);
        } else {
          root_path->rel_path = util_alloc_string_copy( input_path );
          {
            char * abs_path = util_alloc_filename( cwd , input_path , NULL );
            root_path->abs_path = util_alloc_realpath( abs_path );
            free( abs_path );
          }
        }
      }
      free( cwd );
    }
    return root_path;
  } else 
    return NULL;
}


void config_root_path_free( config_root_path_type * root_path ) {
  util_safe_free( root_path->rel_path );
  util_safe_free( root_path->abs_path );
  util_safe_free( root_path->input_path );
  free( root_path );
}

const char * config_root_path_get_input_path( const config_root_path_type * root_path ) {
  return root_path->input_path;
}


const char * config_root_path_get_rel_path( const config_root_path_type * root_path ) {
  return root_path->rel_path;
}


const char * config_root_path_get_abs_path( const config_root_path_type * root_path ) {
  return root_path->abs_path;
}





