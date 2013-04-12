/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'config_path_elm.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __CONFIG_PATH_ELM_H__
#define __CONFIG_PATH_ELM_H__

#ifdef __cplusplus
extern "C" 
#endif

#include <ert/config/config_root_path.h>

typedef struct config_path_elm_struct config_path_elm_type;

void                   config_path_elm_free( config_path_elm_type * path_elm );
void                   config_path_elm_free__( void * arg );
config_path_elm_type * config_path_elm_alloc( const config_root_path_type * root_path , const char * path);
const char *           config_path_elm_get_abspath( const config_path_elm_type * path_elm );
const char *           config_path_elm_get_relpath( const config_path_elm_type * path_elm );
const config_root_path_type * config_path_elm_get_rootpath( const config_path_elm_type * path_elm );
char *                 config_path_elm_alloc_abspath(const config_path_elm_type * path_elm , const char * input_path);
char *                 config_path_elm_alloc_relpath(const config_path_elm_type * path_elm , const char * input_path);
char *                 config_path_elm_alloc_path(const config_path_elm_type * path_elm , const char * input_path);

#ifdef __cplusplus
}
#endif

#endif
