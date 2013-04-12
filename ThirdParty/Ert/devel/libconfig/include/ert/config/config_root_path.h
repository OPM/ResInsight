/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'config_root_path.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __CONFIG_ROOT_PATH_H__
#define __CONFIG_ROOT_PATH_H__

#ifdef __cplusplus
extern "C" 
#endif


typedef struct config_root_path_struct config_root_path_type;

void                    config_root_path_free( config_root_path_type * root_path );
config_root_path_type * config_root_path_alloc( const char * input_path );

const char *            config_root_path_get_input_path( const config_root_path_type * root_path );
const char *            config_root_path_get_rel_path( const config_root_path_type * root_path );
const char *            config_root_path_get_abs_path( const config_root_path_type * root_path );

#ifdef __cplusplus
}
#endif

#endif
