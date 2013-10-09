/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'scalar_config.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <util.h>
#include <scalar_config.h>
#include <enkf_util.h>
#include <enkf_macros.h>
#include <trans_func.h>
#include <active_list.h>

#define SCALAR_CONFIG_TYPE_ID 877065

struct scalar_config_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                data_size;
  active_list_type * active_list;

  trans_func_type ** transform;  
};




scalar_config_type * scalar_config_alloc_empty(int size) {
  scalar_config_type *scalar_config    = util_malloc(sizeof *scalar_config);
  UTIL_TYPE_ID_INIT( scalar_config , SCALAR_CONFIG_TYPE_ID );
  scalar_config->data_size             = size;
  scalar_config->active_list           = active_list_alloc(  );
  
  scalar_config->transform             = util_calloc(scalar_config->data_size ,  sizeof * scalar_config->transform  );
  return scalar_config;
}



void scalar_config_transform(const scalar_config_type * config , const double * input_data , double *output_data) {
  int index;
  for (index = 0; index < config->data_size; index++) 
    output_data[index] = trans_func_eval( config->transform[index] , input_data[index] );
}

 




void scalar_config_fscanf_line(scalar_config_type * config , int line_nr , FILE * stream) {
  config->transform[line_nr] = trans_func_fscanf_alloc( stream );
}



void scalar_config_free(scalar_config_type * scalar_config) {
  int i;
  active_list_free(scalar_config->active_list);
  for (i=0; i < scalar_config->data_size; i++) 
    trans_func_free( scalar_config->transform[i] );
                     
  util_safe_free( scalar_config->transform );
  free(scalar_config);
}



/*****************************************************************/

SAFE_CAST(scalar_config , SCALAR_CONFIG_TYPE_ID)
GET_DATA_SIZE(scalar);
GET_ACTIVE_LIST(scalar);
VOID_FREE(scalar_config);
