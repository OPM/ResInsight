/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_subsidence.c' is part of ERT - Ensemble based
   Reservoir Tool.

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
#define _USE_MATH_DEFINES // for C WINDOWS
#include <math.h>
#include <stdbool.h>

#include <ert/util/hash.hpp>
#include <ert/util/util.h>
#include <ert/util/vector.hpp>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_region.hpp>
#include <ert/ecl/ecl_subsidence.hpp>
#include <ert/ecl/ecl_grid_cache.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_grav_common.hpp>


/**
   This file contains datastructures for calculating changes in
   subsidence from compression of reservoirs. The main datastructure is the
   ecl_subsidence_type structure (which is the only structure which is
   exported).
*/


/**
   The ecl_subsidence_struct datastructure is the main structure for
   calculating the subsidence from time lapse ECLIPSE simulations.
*/

struct ecl_subsidence_struct {
  const ecl_file_type  * init_file;    /* The init file - a shared reference owned by calling scope. */
  ecl_grid_cache_type  * grid_cache;   /* An internal specialized structure to facilitate fast grid lookup. */
  bool                 * aquifer_cell;
  hash_type            * surveys;      /* A hash table containg ecl_subsidence_survey_type instances; one instance
                                          for each interesting time. */
  double               * compressibility; /*total compressibility*/
  double               * poisson_ratio;
};




/**
   Data structure representing one subsidence survey.
*/

#define ECL_SUBSIDENCE_SURVEY_ID 88517
struct ecl_subsidence_survey_struct {
  UTIL_TYPE_ID_DECLARATION;
  const ecl_grid_cache_type * grid_cache;
  const bool                * aquifer_cell;   /* Is this cell a numerical aquifer cell - must be disregarded. */
  char                      * name;           /* Name of the survey - arbitrary string. */
  double                    * porv;           /* Reference pore volume */
  double                    * pressure;              /* Pressure in each grid cell at survey time */
};



/*****************************************************************/


static ecl_subsidence_survey_type * ecl_subsidence_survey_alloc_empty(const ecl_subsidence_type * sub,
                                                                      const char * name) {
  ecl_subsidence_survey_type * survey = (ecl_subsidence_survey_type*)util_malloc( sizeof * survey );
  UTIL_TYPE_ID_INIT( survey , ECL_SUBSIDENCE_SURVEY_ID );
  survey->grid_cache   = sub->grid_cache;
  survey->aquifer_cell = sub->aquifer_cell;
  survey->name         = util_alloc_string_copy( name );

  survey->porv     = (double*)util_calloc( ecl_grid_cache_get_size( sub->grid_cache ) , sizeof * survey->porv     );
  survey->pressure = (double*)util_calloc( ecl_grid_cache_get_size( sub->grid_cache ) , sizeof * survey->pressure );

  return survey;
}

static UTIL_SAFE_CAST_FUNCTION( ecl_subsidence_survey , ECL_SUBSIDENCE_SURVEY_ID )

static ecl_subsidence_survey_type * ecl_subsidence_survey_alloc_PRESSURE(ecl_subsidence_type * ecl_subsidence ,
                                                                         const ecl_file_view_type * restart_view ,
                                                                         const char * name ) {

  ecl_subsidence_survey_type * survey = ecl_subsidence_survey_alloc_empty( ecl_subsidence , name );
  ecl_grid_cache_type * grid_cache = ecl_subsidence->grid_cache;
  const int * global_index = ecl_grid_cache_get_global_index( grid_cache );
  const int size = ecl_grid_cache_get_size( grid_cache );
  int active_index;
  ecl_kw_type * init_porv_kw = ecl_file_iget_named_kw( ecl_subsidence->init_file , PORV_KW , 0); /*Global indexing*/
  ecl_kw_type * pressure_kw = ecl_file_view_iget_named_kw( restart_view , PRESSURE_KW , 0); /*Active indexing*/

  for (active_index = 0; active_index < size; active_index++){
    survey->porv[ active_index ] = ecl_kw_iget_float( init_porv_kw , global_index[active_index] );
    survey->pressure[ active_index ] = ecl_kw_iget_float( pressure_kw , active_index );
  }
  return survey;
}





static void ecl_subsidence_survey_free( ecl_subsidence_survey_type * subsidence_survey ) {
  free( subsidence_survey->name );
  free( subsidence_survey->porv );
  free( subsidence_survey->pressure );
  free( subsidence_survey );
}


static void ecl_subsidence_survey_free__( void * __subsidence_survey ) {
  ecl_subsidence_survey_type * subsidence_survey = ecl_subsidence_survey_safe_cast( __subsidence_survey );
  ecl_subsidence_survey_free( subsidence_survey );
}

/*****************************************************************/

static double ecl_subsidence_survey_eval( const ecl_subsidence_survey_type * base_survey ,
                                          const ecl_subsidence_survey_type * monitor_survey,
                                          ecl_region_type * region ,
                                          double utm_x , double utm_y , double depth,
                                          double compressibility, double poisson_ratio) {

  const ecl_grid_cache_type * grid_cache = base_survey->grid_cache;
  const int size  = ecl_grid_cache_get_size( grid_cache );
  double * weight = (double*)util_calloc( size , sizeof * weight );
  double deltaz;
  int index;

  if (monitor_survey != NULL) {
    for (index = 0; index < size; index++)
      weight[index] = base_survey->porv[index] * (base_survey->pressure[index] - monitor_survey->pressure[index]);
  } else {
    for (index = 0; index < size; index++)
      weight[index] = base_survey->porv[index] * base_survey->pressure[index];
  }

  deltaz = compressibility * 31.83099*(1-poisson_ratio) *
    ecl_grav_common_eval_biot_savart( grid_cache , region , base_survey->aquifer_cell , weight , utm_x , utm_y , depth );

  free( weight );
  return deltaz;
}


static double ecl_subsidence_survey_eval_geertsma( const ecl_subsidence_survey_type * base_survey ,
                                                   const ecl_subsidence_survey_type * monitor_survey,
                                                   ecl_region_type * region ,
                                                   double utm_x , double utm_y , double depth,
                                                   double youngs_modulus, double poisson_ratio, double seabed) {

  const ecl_grid_cache_type * grid_cache = base_survey->grid_cache;
  const double * cell_volume = ecl_grid_cache_get_volume( grid_cache );
  const int size  = ecl_grid_cache_get_size( grid_cache );
  double scale_factor = 1e4 *(1 + poisson_ratio) * ( 1 - 2*poisson_ratio) / ( 4*M_PI*( 1 - poisson_ratio)  * youngs_modulus );
  double * weight = (double*)util_calloc( size , sizeof * weight );
  double deltaz;

  for (int index = 0; index < size; index++) {
    if (monitor_survey) {
        weight[index] = scale_factor * cell_volume[index] * (base_survey->pressure[index] - monitor_survey->pressure[index]);
    } else {
        weight[index] = scale_factor * cell_volume[index] * (base_survey->pressure[index] );
    }
  }

  deltaz = ecl_grav_common_eval_geertsma( grid_cache , region , base_survey->aquifer_cell , weight , utm_x , utm_y , depth , poisson_ratio, seabed);

  free( weight );
  return deltaz;
}



/*****************************************************************/
/**
   The grid instance is only used during the construction phase. The
   @init_file object is used by the ecl_subsidence_add_survey_XXX()
   functions; and calling scope must NOT destroy this object before
   all surveys have been added.
*/

ecl_subsidence_type * ecl_subsidence_alloc( const ecl_grid_type * ecl_grid, const ecl_file_type * init_file) {
  ecl_subsidence_type * ecl_subsidence = (ecl_subsidence_type*)util_malloc( sizeof * ecl_subsidence );
  ecl_subsidence->init_file      = init_file;
  ecl_subsidence->grid_cache     = ecl_grid_cache_alloc( ecl_grid );
  ecl_subsidence->aquifer_cell   = ecl_grav_common_alloc_aquifer_cell( ecl_subsidence->grid_cache , init_file );

  ecl_subsidence->surveys        = hash_alloc();
  return ecl_subsidence;
}



static void ecl_subsidence_add_survey__( ecl_subsidence_type * subsidence , const char * name , ecl_subsidence_survey_type * survey) {
  hash_insert_hash_owned_ref( subsidence->surveys , name , survey , ecl_subsidence_survey_free__ );
}

ecl_subsidence_survey_type * ecl_subsidence_add_survey_PRESSURE( ecl_subsidence_type * subsidence , const char * name , const ecl_file_view_type * restart_view ) {
  ecl_subsidence_survey_type * survey = ecl_subsidence_survey_alloc_PRESSURE( subsidence , restart_view , name );
  ecl_subsidence_add_survey__( subsidence , name , survey );
  return survey;
}


bool ecl_subsidence_has_survey( const ecl_subsidence_type * subsidence , const char * name) {
  return hash_has_key( subsidence->surveys , name );
}

static ecl_subsidence_survey_type * ecl_subsidence_get_survey( const ecl_subsidence_type * subsidence , const char * name) {
  if (name == NULL)
    return NULL;  // Calling scope must determine if this is OK?
  else
    return (ecl_subsidence_survey_type*)hash_get( subsidence->surveys , name );
}


double ecl_subsidence_eval( const ecl_subsidence_type * subsidence , const char * base, const char * monitor , ecl_region_type * region ,
                            double utm_x, double utm_y , double depth,
                            double compressibility, double poisson_ratio) {
  ecl_subsidence_survey_type * base_survey    = ecl_subsidence_get_survey( subsidence , base );
  ecl_subsidence_survey_type * monitor_survey = ecl_subsidence_get_survey( subsidence , monitor );
  return ecl_subsidence_survey_eval( base_survey , monitor_survey , region , utm_x , utm_y , depth , compressibility, poisson_ratio);
}


double ecl_subsidence_eval_geertsma( const ecl_subsidence_type * subsidence , const char * base, const char * monitor , ecl_region_type * region ,
                                     double utm_x, double utm_y , double depth,
                                     double youngs_modulus, double poisson_ratio, double seabed) {
  ecl_subsidence_survey_type * base_survey    = ecl_subsidence_get_survey( subsidence , base );
  ecl_subsidence_survey_type * monitor_survey = ecl_subsidence_get_survey( subsidence , monitor );
  return ecl_subsidence_survey_eval_geertsma( base_survey , monitor_survey , region , utm_x , utm_y , depth , youngs_modulus, poisson_ratio, seabed);
}

void ecl_subsidence_free( ecl_subsidence_type * ecl_subsidence ) {
  ecl_grid_cache_free( ecl_subsidence->grid_cache );
  free( ecl_subsidence->aquifer_cell );
  hash_free( ecl_subsidence->surveys );
  free( ecl_subsidence );
}

