/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'enkf_obs.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef __ENKF_OBS_H__
#define __ENKF_OBS_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>

#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/int_vector.h>
#include <ert/util/type_macros.h>

#include <ert/sched/history.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_state.h>
#include <ert/enkf/meas_data.h>
#include <ert/enkf/obs_data.h>
#include <ert/enkf/time_map.h>
#include <ert/enkf/obs_vector.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/local_obsdata_node.h>
#include <ert/enkf/local_obsdata.h>

  bool            enkf_obs_have_obs( const enkf_obs_type * enkf_obs );
  enkf_obs_type * enkf_obs_alloc( const history_type * history ,
                                  time_map_type * external_time_map ,
                                  const ecl_grid_type * grid ,
                                  const ecl_sum_type * refcase,
                                  ensemble_config_type * ensemble_config );

  void            enkf_obs_free(  enkf_obs_type * enkf_obs);

  obs_vector_type * enkf_obs_iget_vector(const enkf_obs_type * obs, int index);
  obs_vector_type * enkf_obs_get_vector(const enkf_obs_type * , const char * );
  void enkf_obs_add_obs_vector(enkf_obs_type * enkf_obs,
                               const obs_vector_type * vector);

  bool              enkf_obs_load(enkf_obs_type * enkf_obs,
                                  const char           * config_file,
                                  double std_cutoff);
  void enkf_obs_clear( enkf_obs_type * enkf_obs );

  void enkf_obs_get_obs_and_measure_node( const enkf_obs_type      * enkf_obs,
                                          enkf_fs_type             * fs,
                                          const local_obsdata_node_type * obs_node ,
                                          state_enum                 state,
                                          const int_vector_type    * ens_active_list ,
                                          meas_data_type           * meas_data,
                                          obs_data_type            * obs_data);


  void enkf_obs_get_obs_and_measure_data(const enkf_obs_type      * enkf_obs,
                                         enkf_fs_type             * fs,
                                         const local_obsdata_type * local_obsdata ,
                                         state_enum                 state,
                                         const int_vector_type    * ens_active_list ,
                                         meas_data_type           * meas_data,
                                         obs_data_type            * obs_data);


  stringlist_type * enkf_obs_alloc_typed_keylist( enkf_obs_type * enkf_obs , obs_impl_type );
  hash_type * enkf_obs_alloc_data_map(enkf_obs_type * enkf_obs);

  const obs_vector_type * enkf_obs_user_get_vector(const enkf_obs_type * obs , const char  * full_key, char ** index_key );
  bool              enkf_obs_has_key(const enkf_obs_type * , const char * );
  int               enkf_obs_get_size( const enkf_obs_type * obs );

  hash_iter_type  * enkf_obs_alloc_iter( const enkf_obs_type * enkf_obs );

  stringlist_type * enkf_obs_alloc_keylist(enkf_obs_type * enkf_obs );
  stringlist_type * enkf_obs_alloc_matching_keylist(const enkf_obs_type * enkf_obs , const char * input_string);
  time_t            enkf_obs_iget_obs_time(const enkf_obs_type * enkf_obs , int report_step);
  void              enkf_obs_scale_std(enkf_obs_type * enkf_obs, double scale_factor);
  void enkf_obs_local_scale_std( const enkf_obs_type * enkf_obs , const local_obsdata_type * local_obsdata, double scale_factor);
  void              enkf_obs_add_local_nodes_with_data(const enkf_obs_type * enkf_obs , local_obsdata_type * local_obs , enkf_fs_type *fs , const bool_vector_type * ens_mask);
  double            enkf_obs_scale_correlated_std(const enkf_obs_type * enkf_obs , enkf_fs_type * fs , const int_vector_type * ens_active_list , const local_obsdata_type * local_obsdata);
  local_obsdata_type * enkf_obs_alloc_all_active_local_obs( const enkf_obs_type * enkf_obs , const char * key);

  UTIL_IS_INSTANCE_HEADER( enkf_obs );

#ifdef __cplusplus
}
#endif
#endif
