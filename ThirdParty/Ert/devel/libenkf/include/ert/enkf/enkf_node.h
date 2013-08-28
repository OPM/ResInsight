/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'enkf_node.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ENKF_NODE_H__
#define __ENKF_NODE_H__
#include <stdlib.h>
#include <stdbool.h>

#include <ert/util/arg_pack.h>
#include <ert/util/buffer.h>
#include <ert/util/msg.h>
#include <ert/util/matrix.h>
#include <ert/util/rng.h>
#include <ert/util/hash.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/fortio.h>

#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/active_list.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_config_node.h>
#include <ert/enkf/enkf_fs.h>

#ifdef __cplusplus
extern "C" {
#endif


/**********************************/


  typedef void (serialize_ftype)   (const void * , node_id_type , const active_list_type *  ,       matrix_type * , int , int);
  typedef void (deserialize_ftype) (      void * , node_id_type , const active_list_type *  , const matrix_type * , int , int);




  typedef void          (ecl_write_ftype)         (const void *  ,   /* Node object */
                                                   const char *  ,   /* Directory to write to. */
                                                   const char *  ,   /* Filename - can be NULL. */
                                                   fortio_type *);   /* fortio inistance for writing elements in restart files. */
  
  typedef bool          (fload_ftype)                     (      void *  , const char *);
  typedef void          (read_from_buffer_ftype)          (      void *  , buffer_type * , int, state_enum );
  typedef bool          (write_to_buffer_ftype)           (const void *  , buffer_type * , int, state_enum );
  typedef bool          (has_data_ftype)                  (const void *  , int , state_enum); 
  

  typedef void          (set_inflation_ftype)             (void *       ,   
                                                           const void * ,      /* Node object with the ensemble standard deviation. */
                                                           const void * );     /* Node object with the minimum standard deviation - supplied by the user. */


  typedef void          (user_get_vector_ftype)           (void * , const char * , state_enum , double_vector_type *);
  typedef bool          (user_get_ftype)                  (void * , const char * , int , state_enum , double *);
  typedef void *        (alloc_ftype)                     (const void *);
  typedef bool          (initialize_ftype)                (      void *  , int , const char * , rng_type * );
  typedef bool          (forward_load_ftype)              (void *  , const char * , const ecl_sum_type * , const ecl_file_type * , int);
  typedef bool          (forward_load_vector_ftype)       (void *  , const char * , const ecl_sum_type * , const ecl_file_type * , int , int);
  typedef void          (realloc_data_ftype)              (void * );
  typedef void          (free_data_ftype)                 (void * );
  typedef void          (node_free_ftype)                 (      void *);
  typedef void          (clear_ftype)                     (      void *);
  typedef void          (node_copy_ftype)                 (const void * , void *);
  typedef void          (isqrt_ftype)                     (      void *);
  typedef void          (scale_ftype)                     (      void *  , double);
  typedef void          (iadd_ftype)                      (      void *  , const void *);
  typedef void          (imul_ftype)                      (      void *  , const void *);
  typedef void          (iaddsqr_ftype)                   (      void *  , const void *);
  typedef void          (ensemble_mulX_vector_ftype)      (      void *  , int , const void ** , const double *);


  typedef enum {alloc_func                    =  0, 
                ecl_write_func                =  1,
                forward_load_func             =  2,
                fread_func                    =  3,
                fwrite_func                   =  4,
                copy_func                     =  5,
                initialize_func               =  6,
                free_func                     =  7,
                free_data_func                =  8,    
                clear_serial_state_func       =  9,  
                serialize                     = 10,
                deserialize                   = 11} node_function_type;
              

  typedef void          (enkf_node_ftype1)           (enkf_node_type *);
  typedef void          (enkf_node_ftype_NEW)        (enkf_node_type * , arg_pack_type * );


  bool             enkf_node_user_get_vector( enkf_node_type * enkf_node , enkf_fs_type * fs , const char * key , int iens , state_enum state , double_vector_type * values);
  bool             enkf_node_user_get_no_id(enkf_node_type * enkf_node , enkf_fs_type * fs , const char * key , int report_step, int iens, state_enum state , double * value);
  bool             enkf_node_user_get(enkf_node_type *  , enkf_fs_type * , const char * , node_id_type , double * );
  enkf_node_type * enkf_node_alloc(const enkf_config_node_type *);
  enkf_node_type * enkf_node_copyc(const enkf_node_type * );
  /*
    The enkf_node_free() function declaration is in the enkf_config_node.h header,
    because the enkf_config_node needs to know how to free the min_std node.
    
    void             enkf_node_free(enkf_node_type *enkf_node);
  */

  bool             enkf_node_forward_init(enkf_node_type * enkf_node , const char * run_path , int iens);
  bool             enkf_node_has_data( enkf_node_type * enkf_node , enkf_fs_type * fs , node_id_type node_id);
  void             enkf_node_free_data(enkf_node_type * );
  void             enkf_node_free__(void *);
  void             enkf_initialize(enkf_node_type * , int);
  bool             enkf_node_include_type(const enkf_node_type * , int );
  void           * enkf_node_value_ptr(const enkf_node_type * );
  ert_impl_type    enkf_node_get_impl_type(const enkf_node_type * );
  enkf_var_type    enkf_node_get_var_type(const enkf_node_type * );
  bool             enkf_node_use_forward_init( const enkf_node_type * enkf_node );
  void             enkf_node_clear_serial_state(enkf_node_type * );
  void             enkf_node_serialize(enkf_node_type * enkf_node , enkf_fs_type * fs , node_id_type node_id , const active_list_type * active_list , matrix_type * A , int row_offset , int column);
  void             enkf_node_deserialize(enkf_node_type *enkf_node , enkf_fs_type * fs , node_id_type node_id , const active_list_type * active_list , const matrix_type * A , int row_offset , int column);
  
  bool             enkf_node_forward_load_vector(enkf_node_type *enkf_node , const char * run_path , const ecl_sum_type * ecl_sum, const ecl_file_type * restart_block , int report_step1, int report_step2 , int iens );
  bool             enkf_node_forward_load  (enkf_node_type *, const char * , const ecl_sum_type * , const ecl_file_type * , int, int );
  void             enkf_node_ecl_load_static  (enkf_node_type *, const ecl_kw_type * , int , int);
  void             enkf_node_ecl_write (const enkf_node_type *, const char * , fortio_type * , int);
  bool             enkf_node_initialize(enkf_node_type *enkf_node , int , rng_type * );
  void             enkf_node_printf(const enkf_node_type *);
  bool             enkf_node_fwrite (enkf_node_type * , FILE * stream, bool , int , int , state_enum);
  void             enkf_node_clear     (enkf_node_type *);
  void             enkf_node_fread  (enkf_node_type * , FILE * stream , int , int , state_enum);
  
  void enkf_node_copy_ensemble(const enkf_config_node_type * config_node , 
                               enkf_fs_type * src_case,
                               enkf_fs_type * target_case , 
                               int report_step_from, state_enum state_from,    /* src state */
                               int report_step_to  , state_enum state_to,      /* target state */
                               int ens_size, 
                               const int * permutations);
  
  void enkf_node_copy(const enkf_config_node_type * config_node , 
                      enkf_fs_type * src_case ,
                      enkf_fs_type * target_case ,
                      node_id_type src_id , 
                      node_id_type target_id );
  enkf_node_type ** enkf_node_load_alloc_ensemble( const enkf_config_node_type * config_node , enkf_fs_type * fs , 
                                                   int report_step , int iens1 , int iens2 , state_enum state);
  enkf_node_type *  enkf_node_load_alloc( const enkf_config_node_type * config_node , enkf_fs_type * fs , node_id_type node_id);
  bool              enkf_node_fload( enkf_node_type * enkf_node , const char * filename );
  void              enkf_node_load(enkf_node_type * enkf_node , enkf_fs_type * fs , node_id_type node_id );
  bool              enkf_node_store(enkf_node_type * enkf_node , enkf_fs_type * fs , bool force_vectors , node_id_type node_id);
  bool              enkf_node_store_vector(enkf_node_type *enkf_node , enkf_fs_type * fs , int iens , state_enum state);
  bool              enkf_node_try_load(enkf_node_type *enkf_node , enkf_fs_type * fs , node_id_type node_id);
  bool              enkf_node_try_load_vector(enkf_node_type *enkf_node , enkf_fs_type * fs , int iens , state_enum state);
  bool              enkf_node_exists( enkf_node_type *enkf_node , enkf_fs_type * fs , int report_step , int iens , state_enum state);
  bool              enkf_node_vector_storage( const enkf_node_type * node );
  enkf_node_type  * enkf_node_container_alloc(const enkf_config_node_type * config, hash_type * node_hash);
/*****************************************************************/
/* Function callbacks */
ecl_write_ftype * enkf_node_get_func_pointer( const enkf_node_type * node );



void   enkf_node_set_inflation( enkf_node_type * inflation , const enkf_node_type * std , const enkf_node_type * min_std);
void   enkf_node_sqrt(enkf_node_type *enkf_node);
void   enkf_node_scale(enkf_node_type *   , double );
void   enkf_node_iadd(enkf_node_type *    , const enkf_node_type * );
void   enkf_node_iaddsqr(enkf_node_type * , const enkf_node_type * );
void   enkf_node_imul(enkf_node_type *    , const enkf_node_type * );
void   enkf_node_invalidate_cache( enkf_node_type * node );
const  enkf_config_node_type * enkf_node_get_config(const enkf_node_type * );
const char     *  enkf_config_node_get_infile(const enkf_config_node_type * );
const char     *  enkf_node_get_key(const enkf_node_type * );
const char     *  enkf_node_get_swapfile(const enkf_node_type *);
bool              enkf_node_has_func(const enkf_node_type * , node_function_type );
bool              enkf_node_internalize(const enkf_node_type * , int );

void              enkf_node_upgrade_file_103( const char * path , const char * file , ert_impl_type impl_type , int perc_complete , msg_type * msg);

#ifdef __cplusplus
}
#endif
#endif
