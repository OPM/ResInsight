/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'rml_enkf_config.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/analysis/std_enkf.h>
#include <ert/analysis/analysis_module.h>

#include <rml_enkf_config.h>


#define INVALID_SUBSPACE_DIMENSION     -1
#define INVALID_TRUNCATION             -1

#define DEFAULT_SUBSPACE_DIMENSION     INVALID_SUBSPACE_DIMENSION
#define DEFAULT_USE_PRIOR              true

#define DEFAULT_LAMBDA_INCREASE_FACTOR 4
#define DEFAULT_LAMBDA_REDUCE_FACTOR   0.1
#define DEFAULT_LAMBDA0                -1
#define DEFAULT_LAMBDA_MIN             0.01
#define DEFAULT_LAMBDA_RECALCULATE     false



#define RML_ENKF_CONFIG_TYPE_ID 61400061

struct rml_enkf_config_struct {
  UTIL_TYPE_ID_DECLARATION;
  double    truncation;            // Controlled by config key: ENKF_TRUNCATION_KEY
  int       subspace_dimension;    // Controlled by config key: ENKF_NCOMP_KEY (-1: use Truncation instead)
  long      option_flags;
  bool      use_prior;             // Use exact/approximate scheme? Approximate scheme drops the "prior" term in the LM step.


  double    lambda0;
  double    lambda_min;
  double    lambda_decrease_factor;
  double    lambda_increase_factor;
  bool      lambda_recalculate;
};







rml_enkf_config_type * rml_enkf_config_alloc() {
  rml_enkf_config_type * config = util_malloc( sizeof * config );
  UTIL_TYPE_ID_INIT( config , RML_ENKF_CONFIG_TYPE_ID );

  rml_enkf_config_set_truncation( config , DEFAULT_ENKF_TRUNCATION_);
  rml_enkf_config_set_subspace_dimension( config , DEFAULT_SUBSPACE_DIMENSION);
  rml_enkf_config_set_use_prior( config , DEFAULT_USE_PRIOR );
  rml_enkf_config_set_option_flags( config , ANALYSIS_NEED_ED + ANALYSIS_UPDATE_A + ANALYSIS_ITERABLE + ANALYSIS_SCALE_DATA);

  rml_enkf_config_set_lambda_min( config , DEFAULT_LAMBDA_MIN );
  rml_enkf_config_set_lambda0( config , DEFAULT_LAMBDA0 );
  rml_enkf_config_set_lambda_decrease_factor( config , DEFAULT_LAMBDA_REDUCE_FACTOR );
  rml_enkf_config_set_lambda_increase_factor( config , DEFAULT_LAMBDA_INCREASE_FACTOR );
  rml_enkf_config_set_lambda_recalculate( config , DEFAULT_LAMBDA_RECALCULATE );

  return config;
}


bool rml_enkf_config_get_use_prior( const rml_enkf_config_type * config ) {
   return config->use_prior;
}

void rml_enkf_config_set_use_prior( rml_enkf_config_type * config , bool use_prior) {
  config->use_prior = use_prior;
}


double rml_enkf_config_get_truncation( rml_enkf_config_type * config ) {
  return config->truncation;
}

void rml_enkf_config_set_truncation( rml_enkf_config_type * config , double truncation) {
  config->truncation = truncation;
  if (truncation > 0.0)
    config->subspace_dimension = INVALID_SUBSPACE_DIMENSION;
}

int rml_enkf_config_get_subspace_dimension( rml_enkf_config_type * config ) {
  return config->subspace_dimension;
}

void rml_enkf_config_set_subspace_dimension( rml_enkf_config_type * config , int subspace_dimension) {
  config->subspace_dimension = subspace_dimension;
  if (subspace_dimension > 0)
    config->truncation = INVALID_TRUNCATION;
}

void rml_enkf_config_set_option_flags( rml_enkf_config_type * config , long flags) {
  config->option_flags = flags;
}

long rml_enkf_config_get_option_flags( const rml_enkf_config_type * config ) {
  return config->option_flags;
}

double rml_enkf_config_get_lambda0( rml_enkf_config_type * config ) {
   return config->lambda0;
}

void rml_enkf_config_set_lambda0( rml_enkf_config_type * config , double lambda0) {
   config->lambda0 = lambda0;
}


double rml_enkf_config_get_lambda_min( rml_enkf_config_type * config ) {
  return config->lambda_min;
}

void rml_enkf_config_set_lambda_min( rml_enkf_config_type * config , double lambda_min) {
  config->lambda_min = lambda_min;
}


double rml_enkf_config_get_lambda_increase_factor( rml_enkf_config_type * config ) {
  return config->lambda_increase_factor;
}

void rml_enkf_config_set_lambda_increase_factor( rml_enkf_config_type * config , double lambda_increase_factor) {
  config->lambda_increase_factor = lambda_increase_factor;
}

double rml_enkf_config_get_lambda_decrease_factor( rml_enkf_config_type * config ) {
  return config->lambda_decrease_factor;
}

void rml_enkf_config_set_lambda_decrease_factor( rml_enkf_config_type * config , double lambda_decrease_factor) {
  config->lambda_decrease_factor = lambda_decrease_factor;
}


bool rml_enkf_config_get_lambda_recalculate( const rml_enkf_config_type * config ) {
  return config->lambda_recalculate;
}

void rml_enkf_config_set_lambda_recalculate( rml_enkf_config_type * config , bool lambda_recalculate) {
  config->lambda_recalculate = lambda_recalculate;
}


void rml_enkf_config_free(rml_enkf_config_type * config) {
  free( config );
}
