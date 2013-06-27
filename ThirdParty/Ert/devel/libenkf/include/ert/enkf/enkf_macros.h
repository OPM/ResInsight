/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_macros.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ENKF_MACROS_H__
#define __ENKF_MACROS_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <ert/util/matrix.h>
#include <ert/util/log.h>
#include <ert/util/rng.h>
#include <ert/util/double_vector.h>

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/active_list.h>
#include <ert/enkf/meas_data.h>


#define CONFIG_STD_FIELDS \
int __type_id;            \
int data_size;



/*****************************************************************/

#define IS_INSTANCE(prefix,ID) \
bool prefix ## _is_instance__(const void * __arg) {                        \
  prefix ## _type * arg = (prefix ## _type *) __arg;               \
  if (arg->__type_id != ID)                                        \
     return false;                                                 \
  else                                                             \
     return true;                                                  \
}

#define IS_INSTANCE_HEADER(prefix)  bool prefix ## _is_instance__(const void * );

/******************************************************************/



/*****************************************************************/

#define VOID_CONFIG_FREE(prefix)            void prefix ## _config_free__(void *void_arg) { prefix ## _config_free((prefix ## _config_type *) void_arg); }
#define VOID_CONFIG_FREE_HEADER(prefix)     void prefix ## _config_free__(void *);

/*****************************************************************/

#define GET_DATA_SIZE(prefix)               int prefix ## _config_get_data_size (const prefix ## _config_type *arg) { return arg->data_size; }
#define GET_DATA_SIZE_HEADER(prefix)        int prefix ## _config_get_data_size (const prefix ## _config_type *arg);

#define VOID_GET_DATA_SIZE(prefix)               int prefix ## _config_get_data_size__ (const void * arg) {\
   const prefix ## _config_type * config = prefix ## _config_safe_cast_const( arg ); \
   return prefix ## _config_get_data_size( config );                     \
}
#define VOID_GET_DATA_SIZE_HEADER(prefix)        int prefix ## _config_get_data_size__ (const void * arg);

/*****************************************************************/



#define VOID_ALLOC(prefix)                                                            \
void * prefix ## _alloc__(const void *void_config) {                                  \
  const prefix ## _config_type * config = prefix ## _config_safe_cast_const( void_config ); \
  return prefix ## _alloc(config);                                                    \
}

#define VOID_ALLOC_HEADER(prefix) void * prefix ## _alloc__(const void *);

/*****************************************************************/

#define VOID_HAS_DATA(prefix) \
  bool prefix ##_has_data__(const void * void_arg , int report_step , state_enum state) { \
  const prefix ## _type * arg = prefix ## _safe_cast_const( void_arg ); \
  return prefix ## _has_data(arg , report_step , state);                \
}

#define VOID_HAS_DATA_HEADER(prefix) bool prefix ##_has_data__(const void *  , int , state_enum );

/*****************************************************************/
#define VOID_WRITE_TO_BUFFER(prefix)                                        \
  bool prefix ## _write_to_buffer__(const void * void_arg , buffer_type * buffer , int report_step , state_enum state) { \
   const prefix ## _type * arg = prefix ## _safe_cast_const( void_arg ); \
   return prefix ## _write_to_buffer(arg , buffer , report_step , state);     \
}


#define VOID_READ_FROM_BUFFER(prefix)                                              \
  void prefix ## _read_from_buffer__(void * void_arg , buffer_type * buffer , int report_step, state_enum state) { \
   prefix ## _type * arg = prefix ## _safe_cast( void_arg );                       \
   prefix ## _read_from_buffer(arg , buffer , report_step, state);            \
}

#define VOID_WRITE_TO_BUFFER_HEADER(prefix) bool prefix ## _write_to_buffer__(const void * , buffer_type * , int , state_enum);
#define VOID_READ_FROM_BUFFER_HEADER(prefix) void prefix ## _read_from_buffer__(void * , buffer_type * , int, state_enum);

#define VOID_FLOAD(prefix)                                                         \
bool prefix ## _fload__(void * void_arg , const char * filename) {                 \
   prefix ## _type * arg = prefix ## _safe_cast( void_arg );                       \
   return prefix ## _fload(arg , filename);                                               \
}
#define VOID_FLOAD_HEADER(prefix) bool prefix ## _fload__(void * , const char * );


/*****************************************************************/

#define VOID_ECL_WRITE(prefix) \
void prefix ## _ecl_write__(const void * void_arg , const char * path , const char * file , fortio_type * restart_fortio) { \
   const prefix ## _type * arg = prefix ## _safe_cast_const( void_arg );       \
   prefix ## _ecl_write(arg , path , file , restart_fortio);                    \
}

#define VOID_ECL_WRITE_HEADER(prefix) void prefix ## _ecl_write__(const void * , const char * , const char * , fortio_type *);

/*****************************************************************/

#define VOID_FORWARD_LOAD(prefix) \
bool prefix ## _forward_load__(void * void_arg , const char * ecl_file  , const ecl_sum_type * ecl_sum, const ecl_file_type * restart_file, int report_step) { \
   prefix ## _type * arg = prefix ## _safe_cast( void_arg );                         \
   return prefix ## _forward_load(arg , ecl_file , ecl_sum , restart_file , report_step);      \
}

#define VOID_FORWARD_LOAD_HEADER(prefix) bool prefix ## _forward_load__(void * , const char * , const ecl_sum_type *, const ecl_file_type * , int);

  /*****************************************************************/

#define VOID_FORWARD_LOAD_VECTOR(prefix) \
  bool prefix ## _forward_load_vector__(void * void_arg , const char * ecl_file  , const ecl_sum_type * ecl_sum, const ecl_file_type * restart_file, int report_step1 , int report_step2) { \
   prefix ## _type * arg = prefix ## _safe_cast( void_arg );                         \
   return prefix ## _forward_load_vector(arg , ecl_file , ecl_sum , restart_file , report_step1 , report_step2); \
}

#define VOID_FORWARD_LOAD_VECTOR_HEADER(prefix) bool prefix ## _forward_load_vector__(void * , const char * , const ecl_sum_type *, const ecl_file_type * , int , int);


/*****************************************************************/

#define VOID_FREE(prefix)                        \
void prefix ## _free__(void * void_arg) {         \
   prefix ## _type * arg = prefix ## _safe_cast( void_arg ); \
   prefix ## _free( arg ); \
}

#define VOID_FREE_HEADER(prefix) void prefix ## _free__(void * );


/*****************************************************************/

#define VOID_USER_GET(prefix)                                                     \
bool prefix ## _user_get__(void * void_arg , const char * key , int report_step , state_enum state , double * value) { \
   prefix ## _type * arg = prefix ## _safe_cast( void_arg );                      \
   return prefix ## _user_get(arg , key , report_step, state , value);   \
}

#define VOID_USER_GET_HEADER(prefix) bool prefix ## _user_get__(void * , const char * , int, state_enum , double *);


/*****************************************************************/

#define VOID_USER_GET_VECTOR(prefix)                                                     \
void prefix ## _user_get_vector__(void * void_arg , const char * key , state_enum state , double_vector_type * value) { \
   prefix ## _type * arg = prefix ## _safe_cast( void_arg );                      \
   prefix ## _user_get_vector(arg , key , state , value);   \
}

#define VOID_USER_GET_VECTOR_HEADER(prefix) void prefix ## _user_get_vector__(void * , const char * , state_enum , double_vector_type *);

/*****************************************************************/

#define VOID_USER_GET_OBS(prefix)                                                     \
void prefix ## _user_get__(void * void_arg , const char * key , double * value, double * std, bool * valid) { \
   prefix ## _user_get((prefix ## _type *) void_arg , key , value , std , valid);               \
}

#define VOID_USER_GET_OBS_HEADER(prefix) void prefix ## _user_get__(void * , const char * , double * , double * , bool *);


/*****************************************************************/

#define VOID_FREE_DATA(prefix)                               \
void prefix ## _free_data__(void * void_arg) {               \
   prefix ## _type * arg = prefix ## _safe_cast( void_arg ); \
   prefix ## _free_data( arg );                              \
}

#define VOID_FREE_DATA_HEADER(prefix) void prefix ## _free_data__(void * );

/*****************************************************************/

#define VOID_COPY(prefix)                                             \
void prefix ## _copy__(const void * void_src, void * void_target) {   \
   const prefix ## _type * src = prefix ## _safe_cast_const( void_src );    \
   prefix ## _type * target = prefix ## _safe_cast( void_target );    \
   prefix ## _copy( src , target );                                   \
}
#define VOID_COPY_HEADER(prefix) void prefix ## _copy__(const void * , void * );

/*****************************************************************/


#define CONFIG_GET_ECL_KW_NAME(prefix)        const char * prefix ## _config_get_ecl_kw_name(const prefix ## _config_type * config) { return config->ecl_kw_name; }
#define CONFIG_GET_ECL_KW_NAME_HEADER(prefix) const char * prefix ## _config_get_ecl_kw_name(const prefix ## _config_type * )


/*****************************************************************/

#define VOID_SERIALIZE(prefix)     \
  void prefix ## _serialize__(const void *void_arg, node_id_type node_id , const active_list_type * active_list , matrix_type * A , int row_offset , int column) { \
   const prefix ## _type  *arg = prefix ## _safe_cast_const( void_arg );                                                              \
   prefix ## _serialize (arg , node_id , active_list , A , row_offset , column); \
}
#define VOID_SERIALIZE_HEADER(prefix) void prefix ## _serialize__(const void * , node_id_type , const active_list_type * , matrix_type *  , int , int);


#define VOID_DESERIALIZE(prefix)     \
  void prefix ## _deserialize__(void *void_arg, node_id_type node_id , const active_list_type * active_list , const matrix_type * A , int row_offset , int column) { \
   prefix ## _type  *arg = prefix ## _safe_cast( void_arg );                                                          \
   prefix ## _deserialize (arg , node_id , active_list , A , row_offset , column); \
}
#define VOID_DESERIALIZE_HEADER(prefix) void prefix ## _deserialize__(void * , node_id_type , const active_list_type * , const matrix_type *  , int , int);




/*****************************************************************/

#define VOID_INITIALIZE(prefix)     \
  bool prefix ## _initialize__(void *void_arg, int iens , const char * init_file, rng_type * rng) { \
   prefix ## _type  *arg = prefix ## _safe_cast(void_arg);       \
   return prefix ## _initialize (arg , iens , init_file , rng);  \
}
#define VOID_INITIALIZE_HEADER(prefix) bool prefix ## _initialize__(void *, int , const char * , rng_type * );

/*****************************************************************/

#define VOID_SET_INFLATION(prefix) \
void prefix ## _set_inflation__( void * void_inflation , const void * void_std , const void * void_min_std) {                                               \
   prefix ## _set_inflation( prefix ## _safe_cast( void_inflation ) , prefix ## _safe_cast_const( void_std ) , prefix ## _safe_cast_const( void_min_std )); \
}
#define VOID_SET_INFLATION_HEADER(prefix) void prefix ## _set_inflation__( void * void_inflation , const void * void_std , const void * void_min_std );


/*****************************************************************/

#define VOID_GET_OBS(prefix)   \
void prefix ## _get_observations__(const void * void_arg , obs_data_type * obs_data, int report_step , const active_list_type * __active_list) { \
  prefix ## _get_observations((prefix ## _type *) void_arg , obs_data , report_step , __active_list); \
}

#define VOID_GET_OBS_HEADER(prefix) void prefix ## _get_observations__(const void * , obs_data_type * , int , const active_list_type * )

/*****************************************************************/

#define VOID_MEASURE(obs_prefix, state_prefix) \
void obs_prefix ## _measure__(const void * void_obs ,  const void * void_state , node_id_type node_id , meas_data_type * meas_data , const active_list_type * __active_list) { \
   const obs_prefix ## _type   * obs   = obs_prefix ## _safe_cast_const( void_obs );     \
   const state_prefix ## _type * state = state_prefix ## _safe_cast_const( void_state );       \
   obs_prefix ## _measure(obs , state , node_id , meas_data , __active_list); \
}

#define VOID_MEASURE_UNSAFE(obs_prefix, state_prefix) \
void obs_prefix ## _measure__(const void * void_obs ,  const void * state , node_id_type node_id , meas_data_type * meas_data , const active_list_type * __active_list) { \
   const obs_prefix ## _type   * obs   = obs_prefix ## _safe_cast_const( void_obs );     \
   obs_prefix ## _measure(obs , state , node_id , meas_data , __active_list); \
}


#define VOID_MEASURE_HEADER(obs_prefix) void obs_prefix ## _measure__(const void * ,  const void * , node_id_type , meas_data_type * , const active_list_type *)


/*****************************************************************/

#define VOID_CHI2(obs_prefix, state_prefix) \
  double obs_prefix ## _chi2__(const void * void_obs ,  const void * void_state, node_id_type node_id) { \
   const obs_prefix ## _type   * obs   = obs_prefix ## _safe_cast_const( void_obs );     \
   const state_prefix ## _type * state = state_prefix ## _safe_cast_const( void_state ); \
   return obs_prefix ## _chi2(obs , state , node_id);                           \
}

#define VOID_CHI2_HEADER(obs_prefix) double obs_prefix ## _chi2__(const void * ,  const void *, node_id_type);


/*****************************************************************/

#define VOID_TRUNCATE(prefix)         void prefix ## _truncate__(void * void_arg) { prefix ## _truncate( (prefix ## _type *) void_arg); }
#define VOID_TRUNCATE_HEADER(prefix)  void prefix ## _truncate__(void * )

/*****************************************************************/
#define VOID_SCALE(prefix)        void prefix ## _scale__(void * void_arg , double scale_factor) { prefix ## _scale( prefix ## _safe_cast( void_arg ) , scale_factor ); }
#define VOID_SCALE_HEADER(prefix) void prefix ## _scale__(void *  , double );

/*****************************************************************/

#define VOID_CLEAR(prefix)         void prefix ## _clear__(void * void_arg) { prefix ## _clear( prefix ## _safe_cast( void_arg )); }
#define VOID_CLEAR_HEADER(prefix)  void prefix ## _clear__(void * )

/*****************************************************************/


#define VOID_ISQRT(prefix)         void prefix ## _isqrt__(void * void_arg) { prefix ## _isqrt( prefix ## _safe_cast( void_arg )); }
#define VOID_ISQRT_HEADER(prefix)  void prefix ## _isqrt__(void * )

/*****************************************************************/

#define VOID_IADD(prefix)   void prefix ## _iadd__( void * void_arg , const void * void_delta ) { \
   prefix ## _iadd( prefix ## _safe_cast( void_arg ) , prefix ## _safe_cast_const( void_delta ) ); \
} 

#define VOID_IADD_HEADER(prefix)   void prefix ## _iadd__( void * void_arg , const void * void_delta );

/*****************************************************************/

#define VOID_IMUL(prefix)   void prefix ## _imul__( void * void_arg , const void * void_delta ) { \
   prefix ## _imul( prefix ## _safe_cast( void_arg ) , prefix ## _safe_cast_const( void_delta ) ); \
} 

#define VOID_IMUL_HEADER(prefix)   void prefix ## _imul__( void * void_arg , const void * void_delta );

/*****************************************************************/

#define VOID_IADDSQR(prefix)   void prefix ## _iaddsqr__( void * void_arg , const void * void_delta ) { \
   prefix ## _iaddsqr( prefix ## _safe_cast( void_arg ) , prefix ## _safe_cast_const( void_delta ) ); \
} 

#define VOID_IADDSQR_HEADER(prefix)   void prefix ## _iaddsqr__( void * void_arg , const void * void_delta );

/*****************************************************************/

#define CONFIG_GET_ENSFILE(prefix)                   const char * prefix ## _config_get_ensfile_ref(const prefix ## _config_type * config) { return config->ensfile; }
#define CONFIG_GET_ECLFILE(prefix)                   const char * prefix ## _config_get_eclfile_ref(const prefix ## _config_type * config) { return config->eclfile; }
#define CONFIG_GET_ENSFILE_HEADER(prefix)       const char * prefix ## _config_get_ensfile_ref(const prefix ## _config_type * )
#define CONFIG_GET_ECLFILE_HEADER(prefix)       const char * prefix ## _config_get_eclfile_ref(const prefix ## _config_type * )

/*****************************************************************/

#define VOID_IGET(prefix)        double prefix ## _iget__(const void * void_arg, int index) { return prefix ## _iget((const prefix ## _type *) void_arg , index); }
#define VOID_IGET_HEADER(prefix) double prefix ## _iget__(const void * , int ) 

#ifdef __cplusplus
}
#endif
#endif
