/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'surface.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <math.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/log.h>

#include <ert/geometry/geo_surface.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/surface.h>
#include <ert/enkf/surface_config.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_serialize.h>


/*****************************************************************/


struct surface_struct {
  int                          __type_id;     /* Only used for run_time checking. */
  surface_config_type        * config;        /* Can not be NULL - var_type is set on first load. */
  double                     * data;          /* Size is always one - but what the fuck ... */
};




void surface_clear(surface_type * surface) {
  const int data_size = surface_config_get_data_size( surface->config );
  for (int k=0; k < data_size; k++)
    surface->data[k] = 0;
}

static void surface_fload( surface_type * surface , const char * filename ) {
  const geo_surface_type * base_surface = surface_config_get_base_surface( surface->config );
  geo_surface_fload_irap_zcoord( base_surface , filename , surface->data );
}



bool surface_initialize(surface_type *surface , int iens , const char * filename , rng_type * rng) {
  surface_fload(surface , filename );
  return true;
}


surface_type * surface_alloc(const surface_config_type * surface_config) {
  surface_type * surface  = util_malloc(sizeof *surface);
  surface->__type_id      = SURFACE;
  surface->config         = (surface_config_type *) surface_config;
  {
    const int data_size = surface_config_get_data_size( surface_config );
    surface->data       = util_calloc( data_size , sizeof * surface->data );
  }
  return surface;
}




void surface_copy(const surface_type *src , surface_type * target) {
  if (src->config == target->config) {
    const int data_size = surface_config_get_data_size( src->config );
    for (int k=0; k < data_size; k++)
      target->data[k] = src->data[k];
  } else
    util_abort("%s: do not share config objects \n",__func__);
}




void surface_read_from_buffer(surface_type * surface , buffer_type * buffer, int report_step, state_enum state) {
  int  size = surface_config_get_data_size( surface->config );
  enkf_util_assert_buffer_type( buffer , SURFACE );
  buffer_fread( buffer , surface->data , sizeof * surface->data , size);
}






bool surface_write_to_buffer(const surface_type * surface , buffer_type * buffer, int report_step , state_enum state) {
  int  size = surface_config_get_data_size( surface->config );
  buffer_fwrite_int( buffer , SURFACE );
  buffer_fwrite( buffer , surface->data , sizeof * surface->data , size);
  return true;
}


void surface_free(surface_type *surface) {
  free(surface->data);
  free(surface);
}




void surface_serialize(const surface_type * surface , node_id_type node_id , const active_list_type * active_list , matrix_type * A , int row_offset , int column) {
  const surface_config_type *config  = surface->config;
  const int                data_size = surface_config_get_data_size(config );
  
  enkf_matrix_serialize( surface->data , data_size , ECL_DOUBLE_TYPE , active_list , A , row_offset , column);
}



void surface_deserialize(surface_type * surface , node_id_type node_id , const active_list_type * active_list , const matrix_type * A , int row_offset , int column) {
  const surface_config_type *config  = surface->config;
  const int                data_size = surface_config_get_data_size(config );
  
  enkf_matrix_deserialize( surface->data , data_size , ECL_DOUBLE_TYPE , active_list , A , row_offset , column);
}


void surface_ecl_write(const surface_type * surface , const char * run_path , const char * base_file , fortio_type * fortio) {
  char * target_file = util_alloc_filename( run_path , base_file  , NULL);
  surface_config_ecl_write( surface->config , target_file , surface->data );
  free( target_file );
}


bool surface_user_get(const surface_type * surface , const char * index_key , int report_step , state_enum state, double * value) {
  const int                data_size = surface_config_get_data_size( surface->config );
  int index;

  *value = 0.0;

  if (util_sscanf_int( index_key , &index)) 
    if ((index >= 0) && (index < data_size)) {
      *value = surface->data[index];
      return true;
    }
  
  // Not valid
  return false;
}



void surface_set_inflation(surface_type * inflation , const surface_type * std , const surface_type * min_std) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    inflation->data[i] = util_double_max( 1.0 , min_std->data[i] / std->data[i]);
}


void surface_iadd( surface_type * surface , const surface_type * delta) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    surface->data[i] += delta->data[i];
}


void surface_iaddsqr( surface_type * surface , const surface_type * delta) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    surface->data[i] += delta->data[i] * delta->data[i];
}


void surface_imul( surface_type * surface , const surface_type * delta) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    surface->data[i] *= delta->data[i];
}

void surface_scale( surface_type * surface , double scale_factor) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    surface->data[i] *= scale_factor;
}

void surface_isqrt( surface_type * surface ) {
  int size = 1;
  for (int i = 0; i < size; i++) 
    surface->data[i] = sqrt( surface->data[i] );
}





/******************************************************************/
/* Anonumously generated functions used by the enkf_node object   */
/******************************************************************/
UTIL_SAFE_CAST_FUNCTION(surface , SURFACE)
UTIL_SAFE_CAST_FUNCTION_CONST(surface , SURFACE)
VOID_ALLOC(surface)
VOID_FREE(surface)
VOID_ECL_WRITE(surface)
VOID_COPY(surface)
VOID_USER_GET(surface)
VOID_WRITE_TO_BUFFER(surface)
VOID_READ_FROM_BUFFER(surface)
VOID_SERIALIZE(surface)
VOID_DESERIALIZE(surface)
VOID_INITIALIZE(surface)
VOID_SET_INFLATION(surface)
VOID_CLEAR(surface)
VOID_IADD(surface)
VOID_SCALE(surface)
VOID_IMUL(surface)
VOID_IADDSQR(surface)
VOID_ISQRT(surface)
     
