/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'rml_enkf_config.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef RML_ENKF_CONFIG_H
#define RML_ENKF_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct rml_enkf_config_struct rml_enkf_config_type;


  rml_enkf_config_type * rml_enkf_config_alloc();
  void                        rml_enkf_config_free(rml_enkf_config_type * config);

  int    rml_enkf_config_get_subspace_dimension( rml_enkf_config_type * config );
  void   rml_enkf_config_set_subspace_dimension( rml_enkf_config_type * config , int subspace_dimension);

  double rml_enkf_config_get_truncation( rml_enkf_config_type * config );
  void   rml_enkf_config_set_truncation( rml_enkf_config_type * config , double truncation);

  bool   rml_enkf_config_get_use_prior( const rml_enkf_config_type * config );
  void   rml_enkf_config_set_use_prior( rml_enkf_config_type * config , bool use_prior);

  void rml_enkf_config_set_option_flags( rml_enkf_config_type * config , long flags);
  long rml_enkf_config_get_option_flags( const rml_enkf_config_type * config );

  double rml_enkf_config_get_lambda0( rml_enkf_config_type * config );
  void   rml_enkf_config_set_lambda0( rml_enkf_config_type * config , double lambda0);

  double rml_enkf_config_get_lambda_min( rml_enkf_config_type * config );
  void   rml_enkf_config_set_lambda_min( rml_enkf_config_type * config , double lambda_min);

  double rml_enkf_config_get_lambda_increase_factor( rml_enkf_config_type * config );
  void   rml_enkf_config_set_lambda_increase_factor( rml_enkf_config_type * config , double lambda_increase_factor);

  double rml_enkf_config_get_lambda_decrease_factor( rml_enkf_config_type * config );
  void   rml_enkf_config_set_lambda_decrease_factor( rml_enkf_config_type * config , double lambda_decrease_factor);

  bool   rml_enkf_config_get_lambda_recalculate( const rml_enkf_config_type * config );
  void   rml_enkf_config_set_lambda_recalculate( rml_enkf_config_type * config , bool lambda_recalculate);


#ifdef __cplusplus
}
#endif
#endif
