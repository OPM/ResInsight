/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_updatestep.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __LOCAL_UPDATESTEP_H__
#define __LOCAL_UPDATESTEP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/enkf/local_ministep.h>

typedef struct local_updatestep_struct local_updatestep_type;

local_updatestep_type * local_updatestep_alloc( const char * name );
void                    local_updatestep_free__(void * arg);
void                    local_updatestep_add_ministep( local_updatestep_type * updatestep , local_ministep_type * ministep);
local_ministep_type   * local_updatestep_iget_ministep( const local_updatestep_type * updatestep , int index);
local_obsset_type     * local_updatestep_iget_obsset( const local_updatestep_type * updatestep , int index);
int                     local_updatestep_get_num_ministep( const local_updatestep_type * updatestep );
local_updatestep_type * local_updatestep_alloc_copy( const local_updatestep_type * src , const char * name );
void                    local_updatestep_fprintf( const local_updatestep_type * updatestep , FILE * stream);
const char            * local_updatestep_get_name( const local_updatestep_type * updatestep );

#ifdef __cplusplus
}
#endif
#endif
