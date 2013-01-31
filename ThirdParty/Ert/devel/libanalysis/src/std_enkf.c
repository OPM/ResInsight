/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'std_enkf.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/matrix.h>
#include <ert/util/matrix_blas.h>
#include <ert/util/rng.h>

#include <ert/analysis/analysis_module.h>
#include <ert/analysis/analysis_table.h>
#include <ert/analysis/enkf_linalg.h>
#include <ert/analysis/std_enkf.h>


/*
  A random 'magic' integer id which is used for run-time type checking
  of the input data. 
*/
#define STD_ENKF_TYPE_ID 261123




/*
  Observe that only one of the settings subspace_dimension and
  truncation can be valid at a time; otherwise the svd routine will
  fail. This implies that the set_truncation() and
  set_subspace_dimension() routines will set one variable, AND
  INVALIDATE THE OTHER. For most situations this will be OK, but if
  you have repeated calls to both of these functions the end result
  might be a surprise.  
*/
#define INVALID_SUBSPACE_DIMENSION  -1
#define INVALID_TRUNCATION          -1
#define DEFAULT_SUBSPACE_DIMENSION  INVALID_SUBSPACE_DIMENSION





/*
  The configuration data used by the std_enkf module is contained in a
  std_enkf_data_struct instance. The data type used for the std_enkf
  module is quite simple; with only a few scalar variables, but there
  are essentially no limits to what you can pack into such a datatype.

  All the functions in the module have a void pointer as the first
  argument, this will immediately be casted to a std_enkf_data_type
  instance, to get some type safety the UTIL_TYPE_ID system should be
  used (see documentation in util.h)

  The data structure holding the data for your analysis module should
  be created and initialized by a constructor, which should be
  registered with the '.alloc' element of the analysis table; in the
  same manner the desctruction of this data should be handled by a
  destructor or free() function registered with the .freef field of
  the analysis table.
*/




struct std_enkf_data_struct {
  UTIL_TYPE_ID_DECLARATION;
  double    truncation;            // Controlled by config key: ENKF_TRUNCATION_KEY
  int       subspace_dimension;    // Controlled by config key: ENKF_NCOMP_KEY (-1: use Truncation instead)
  long      option_flags;
};


/*
  This is a macro which will expand to generate a function:

     std_enkf_data_type * std_enkf_data_safe_cast( void * arg ) {}

  which is used for runtime type checking of all the functions which
  accept a void pointer as first argument. 
*/
static UTIL_SAFE_CAST_FUNCTION( std_enkf_data , STD_ENKF_TYPE_ID )


double std_enkf_get_truncation( std_enkf_data_type * data ) {
  return data->truncation;
}

int std_enkf_get_subspace_dimension( std_enkf_data_type * data ) {
  return data->subspace_dimension;
}

void std_enkf_set_truncation( std_enkf_data_type * data , double truncation ) {
  data->truncation = truncation;
  if (truncation > 0.0)
    data->subspace_dimension = INVALID_SUBSPACE_DIMENSION;
}

void std_enkf_set_subspace_dimension( std_enkf_data_type * data , int subspace_dimension) {
  data->subspace_dimension = subspace_dimension;
  if (subspace_dimension > 0)
    data->truncation = INVALID_TRUNCATION;
}



void * std_enkf_data_alloc( rng_type * rng) {
  std_enkf_data_type * data = util_malloc( sizeof * data );
  UTIL_TYPE_ID_INIT( data , STD_ENKF_TYPE_ID );
  
  std_enkf_set_truncation( data , DEFAULT_ENKF_TRUNCATION_ );
  std_enkf_set_subspace_dimension( data , DEFAULT_SUBSPACE_DIMENSION );
  data->option_flags = ANALYSIS_NEED_ED + ANALYSIS_SCALE_DATA;
  return data;
}


void std_enkf_data_free( void * data ) { 
  free( data );
}




void std_enkf_initX__( matrix_type * X , 
                       matrix_type * S , 
                       matrix_type * R , 
                       matrix_type * E , 
                       matrix_type * D ,
                       double truncation,
                       int    ncomp,
                       bool   bootstrap ) {

  int nrobs         = matrix_get_rows( S );
  int ens_size      = matrix_get_columns( S );
  int nrmin         = util_int_min( ens_size , nrobs); 
  
  matrix_type * W   = matrix_alloc(nrobs , nrmin);                      
  double      * eig = util_calloc( nrmin , sizeof * eig);    
  
  matrix_subtract_row_mean( S );           /* Shift away the mean */
  enkf_linalg_lowrankCinv( S , R , W , eig , truncation , ncomp);    
  enkf_linalg_init_stdX( X , S , D , W , eig , bootstrap);
  
  matrix_free( W );
  free( eig );
  enkf_linalg_checkX( X , bootstrap );
}





void std_enkf_initX(void * module_data , 
                    matrix_type * X , 
                    matrix_type * A , 
                    matrix_type * S , 
                    matrix_type * R , 
                    matrix_type * dObs , 
                    matrix_type * E , 
                    matrix_type * D) {


  std_enkf_data_type * data = std_enkf_data_safe_cast( module_data );
  {
    int ncomp         = data->subspace_dimension;
    double truncation = data->truncation;

    std_enkf_initX__(X,S,R,E,D,truncation,ncomp,false);
  }
}







bool std_enkf_set_double( void * arg , const char * var_name , double value) {
  std_enkf_data_type * module_data = std_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;

    if (strcmp( var_name , ENKF_TRUNCATION_KEY_) == 0)
      std_enkf_set_truncation( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}


bool std_enkf_set_int( void * arg , const char * var_name , int value) {
  std_enkf_data_type * module_data = std_enkf_data_safe_cast( arg );
  {
    bool name_recognized = true;
    
    if (strcmp( var_name , ENKF_NCOMP_KEY_) == 0)
      std_enkf_set_subspace_dimension( module_data , value );
    else
      name_recognized = false;

    return name_recognized;
  }
}


long std_enkf_get_options( void * arg , long flag ) {
  std_enkf_data_type * module_data = std_enkf_data_safe_cast( arg );
  {
    return module_data->option_flags;
  }
}



/**
   gcc -fpic -c <object_file> -I??  <src_file>
   gcc -shared -o <lib_file> <object_files>
*/



#ifdef INTERNAL_LINK
#define SYMBOL_TABLE std_enkf_symbol_table
#else
#define SYMBOL_TABLE EXTERNAL_MODULE_SYMBOL
#endif


analysis_table_type SYMBOL_TABLE = {
    .alloc           = std_enkf_data_alloc,
    .freef           = std_enkf_data_free,
    .set_int         = std_enkf_set_int , 
    .set_double      = std_enkf_set_double , 
    .set_bool        = NULL , 
    .set_string      = NULL , 
    .get_options     = std_enkf_get_options , 
    .initX           = std_enkf_initX , 
    .updateA         = NULL,
    .init_update     = NULL,
    .complete_update = NULL,
    .has_var         = NULL,
    .get_int         = NULL,
    .get_double      = NULL,
    .get_ptr         = NULL, 
};

