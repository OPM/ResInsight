/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ecl_grid.h>
#include <local_config.h>
#include <util.h>




int main( int argc , char ** argv) {
  if (argc != 4) {
    fprintf(stderr,"Usage:\n\nbash%% local_config  GRID_FILE   NEW_CONFIG_FILE  OLD_CONFIG_FILE");
    exit(1);
  } else {
    ecl_grid_type * ecl_grid = ecl_grid_alloc( argv[1] );
    const char * src_file    = argv[2];
    const char * target_file = argv[3]; 

    local_config_type * local_config = local_config_alloc( 100 );
    local_config_add_config_file( local_config , src_file );
    local_config_reload( local_config , ecl_grid , NULL , NULL , NULL );
    
    local_config_fprintf( local_config , target_file );
    local_config_free( local_config );
  }
}
