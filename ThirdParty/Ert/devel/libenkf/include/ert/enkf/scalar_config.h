/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'scalar_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCALAR_CONFIG_H__
#define __SCALAR_CONFIG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <enkf_util.h>
#include <enkf_macros.h>

typedef struct scalar_config_struct scalar_config_type;



scalar_config_type *  scalar_config_alloc_empty(int);
void                  scalar_config_free(scalar_config_type *);
const          char * scalar_config_get_ensfile_ref(const scalar_config_type * );
const          char * scalar_config_get_eclfile_ref(const scalar_config_type * );
void                  scalar_config_transform(const scalar_config_type * , const double * , double *);
double                scalar_config_transform_item(const scalar_config_type *, double, int);
void                  scalar_config_fscanf_line(scalar_config_type * , int , FILE * );
int                   scalar_config_get_active_size(const scalar_config_type *);

SAFE_CAST_HEADER(scalar_config)
GET_DATA_SIZE_HEADER(scalar);
VOID_FREE_HEADER(scalar_config);
GET_ACTIVE_LIST_HEADER(scalar);
#ifdef __cplusplus
}
#endif
#endif
