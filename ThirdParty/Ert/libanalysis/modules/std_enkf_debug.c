/*
   Copyright (C) 2016  Statoil ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdio.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/rng.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>
#include <ert/analysis/std_enkf.h>

#define PREFIX_KEY      "PREFIX"
#define DEFAULT_PREFIX  "debug"

/*
  A random 'magic' integer id which is used for run-time type checking
  of the input data.
*/
#define STD_ENKF_DEBUG_TYPE_ID 269923

typedef struct std_enkf_debug_data_struct std_enkf_debug_data_type;

struct std_enkf_debug_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  std_enkf_data_type * std_data;
  char * prefix;
  int    update_count;
};


static UTIL_SAFE_CAST_FUNCTION_CONST( std_enkf_debug_data , STD_ENKF_DEBUG_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION( std_enkf_debug_data , STD_ENKF_DEBUG_TYPE_ID )


void std_enkf_debug_data_set_prefix( std_enkf_debug_data_type * data , const char * prefix ) {
  data->prefix = util_realloc_string_copy( data->prefix , prefix );
}


void * std_enkf_debug_data_alloc( rng_type * rng) {
  std_enkf_debug_data_type * data = util_malloc( sizeof * data );
  UTIL_TYPE_ID_INIT( data , STD_ENKF_DEBUG_TYPE_ID );
  data->std_data = std_enkf_data_alloc( rng );

  data->update_count = 0;
  data->prefix = NULL;
  std_enkf_debug_data_set_prefix( data , DEFAULT_PREFIX );
  return data;
}


void std_enkf_debug_data_free( void * arg ) {
  std_enkf_debug_data_type * data = std_enkf_debug_data_safe_cast( arg );
  std_enkf_data_free( data->std_data );
  free( data );
}

void std_enkf_debug_save_matrix( const matrix_type * m , const char * path , const char * file, bool transpose) {
  char * filename = util_alloc_filename( path , file , NULL );

  if (transpose) {
    matrix_type * mt = matrix_alloc_transpose( m );
    matrix_dump_csv(mt,filename);
    matrix_free( mt );
  } else
    matrix_dump_csv(m , filename);


  free( filename );
}



void std_enkf_debug_initX(void * module_data ,
                          matrix_type * X ,
                          matrix_type * A ,
                          matrix_type * S ,
                          matrix_type * R ,
                          matrix_type * dObs ,
                          matrix_type * E ,
                          matrix_type * D) {

  std_enkf_debug_data_type * data = std_enkf_debug_data_safe_cast( module_data );
  char * debug_path = util_alloc_sprintf( "%s/%d" , data->prefix , data->update_count );
  std_enkf_data_type * std_data = data->std_data;
  util_make_path( debug_path );
  std_enkf_debug_save_matrix( A , debug_path ,  "prior_ert.csv" , true);

  std_enkf_debug_save_matrix( S , debug_path ,  "S.csv" , true);
  std_enkf_debug_save_matrix( S , debug_path ,  "simResponses.csv" , true);

  std_enkf_debug_save_matrix( E , debug_path ,  "E.csv" , true);
  std_enkf_debug_save_matrix( E , debug_path ,  "measurementErrors.csv" , true);

  std_enkf_debug_save_matrix( R , debug_path ,  "R.csv" , true);
  std_enkf_debug_save_matrix( D , debug_path ,  "D.csv" , true);
  {
    matrix_type * value = matrix_alloc_sub_copy( dObs , 0 , 0 , matrix_get_rows( dObs ) , 1 );
    matrix_type * std   = matrix_alloc_sub_copy( dObs , 0 , 1 , matrix_get_rows( dObs ) , 1 );

    std_enkf_debug_save_matrix( value , debug_path , "observations.csv", true);
    std_enkf_debug_save_matrix( std , debug_path , "observations_std.csv", true);

    matrix_free( value );
    matrix_free( std );
  }
  std_enkf_initX( std_data , X , A , S , R , dObs , E , D );
  {
    matrix_type * posterior = matrix_alloc_matmul( A , X );
    std_enkf_debug_save_matrix( posterior , debug_path , "posterior_ert.csv" , true);
    matrix_free( posterior );
  }

  data->update_count++;
}







bool std_enkf_debug_set_double( void * arg , const char * var_name , double value) {
  std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast( arg );
  return std_enkf_set_double( module_data->std_data , var_name , value );
}


bool std_enkf_debug_set_int( void * arg , const char * var_name , int value) {
  std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast( arg );
  return std_enkf_set_int( module_data->std_data , var_name , value );
}


bool std_enkf_debug_set_bool( void * arg , const char * var_name , bool value) {
  std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast( arg );
  return std_enkf_set_bool( module_data->std_data , var_name , value );
}


bool std_enkf_debug_set_string( void * arg , const char * var_name , const char * value) {
  std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast( arg );
  if (strcmp(var_name, PREFIX_KEY) == 0) {
    std_enkf_debug_data_set_prefix( module_data , value );
    return true;
  } else
    return false;
}



long std_enkf_debug_get_options( void * arg , long flag ) {
  const std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast_const( arg );
  long options = std_enkf_get_options( module_data->std_data , flag );
  options |= ANALYSIS_USE_A;
  return options;
}


bool std_enkf_debug_has_var( const void * arg, const char * var_name) {
  if (strcmp(var_name , PREFIX_KEY) == 0)
    return true;
  else {
    const std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast_const( arg );
    return std_enkf_has_var( module_data->std_data , var_name );
  }
}


double std_enkf_debug_get_double( const void * arg, const char * var_name) {
  const std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast_const( arg );
  return std_enkf_get_double( module_data->std_data , var_name );
}

int std_enkf_debug_get_int( const void * arg, const char * var_name) {
  const std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast_const( arg );
  return std_enkf_get_int( module_data->std_data , var_name  );
}


bool std_enkf_debug_get_bool( const void * arg, const char * var_name) {
  const std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast_const( arg );
  return std_enkf_get_bool( module_data->std_data , var_name );
}


void * std_enkf_debug_get_ptr( const void * arg, const char * var_name) {
  const std_enkf_debug_data_type * module_data = std_enkf_debug_data_safe_cast_const( arg );
  if (strcmp( var_name , PREFIX_KEY) == 0)
    return (void *) module_data->prefix;
  else
    return NULL;
}



#ifdef INTERNAL_LINK
#define LINK_NAME STD_ENKF_DEBUG
#else
#define LINK_NAME EXTERNAL_MODULE_SYMBOL
#endif


analysis_table_type LINK_NAME = {
    .name            = "STD_ENKF_DEBUG_DEBUG",
    .alloc           = std_enkf_debug_data_alloc,
    .freef           = std_enkf_debug_data_free,
    .set_int         = std_enkf_debug_set_int ,
    .set_double      = std_enkf_debug_set_double ,
    .set_bool        = std_enkf_debug_set_bool,
    .set_string      = std_enkf_debug_set_string,
    .get_options     = std_enkf_debug_get_options ,
    .initX           = std_enkf_debug_initX ,
    .updateA         = NULL,
    .init_update     = NULL,
    .complete_update = NULL,
    .has_var         = std_enkf_debug_has_var,
    .get_int         = std_enkf_debug_get_int,
    .get_double      = std_enkf_debug_get_double,
    .get_bool        = std_enkf_debug_get_bool,
    .get_ptr         = std_enkf_debug_get_ptr
};

