/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'analysis_module.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ANALYSIS_MODULE_H
#define ERT_ANALYSIS_MODULE_H
#ifdef  __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/util/matrix.h>
#include <ert/util/bool_vector.h>

#include <ert/analysis/module_info.h>

/*
   These are option flag values which are used by the core ert code to
   query the module of it's needs and capabilities. For instance to to
   determine whether the data should be scaled prior to analysis the
   core code will issue the call:

      if (analysis_module_get_option( module, ANALYSIS_SCALE_DATA))
         obs_data_scale( obs_data , S , E , D , R , dObs );

   It is the responsability of the module to set the various flags.
*/


typedef enum {
    ANALYSIS_NEED_ED    = 1,
    ANALYSIS_USE_A      = 4,       // The module will read the content of A - but not modify it.
    ANALYSIS_UPDATE_A   = 8,       // The update will be based on modifying A directly, and not on an X matrix.
    ANALYSIS_SCALE_DATA = 16,
    ANALYSIS_ITERABLE   = 32       // The module can bu used as an iterative smoother.
} analysis_module_flag_enum;


#define ANALYSIS_MODULE_FLAG_ENUM_SIZE 5
#define ANALYSIS_MODULE_FLAG_ENUM_DEFS {.value = ANALYSIS_NEED_ED     , .name = "ANALYSIS_NEED_ED"},\
                                       {.value = ANALYSIS_USE_A       , .name = "ANALYSIS_USE_A"},\
                                       {.value = ANALYSIS_UPDATE_A    , .name = "ANALYSIS_UPDATE_A"},\
                                       {.value = ANALYSIS_SCALE_DATA  , .name = "ANALYSIS_SCALE_DATA"},\
                                       {.value = ANALYSIS_ITERABLE    , .name = "ANALYSIS_ITERABLE"}


#define EXTERNAL_MODULE_NAME "analysis_table"
#define EXTERNAL_MODULE_SYMBOL analysis_table

  typedef enum {
    LOAD_OK                     = 0,
    DLOPEN_FAILURE              = 1,
    LOAD_SYMBOL_TABLE_NOT_FOUND = 2
  } analysis_module_load_status_enum;


  typedef struct analysis_module_struct analysis_module_type;

  analysis_module_type * analysis_module_alloc_internal__( rng_type * rng , const char * symbol_table , bool verbose , analysis_module_load_status_enum * load_status);
  analysis_module_type * analysis_module_alloc_internal( rng_type * rng , const char * symbol_table );

  analysis_module_type * analysis_module_alloc_external__(rng_type * rng , const char * lib_name , bool verbose , analysis_module_load_status_enum * load_status);
  analysis_module_type * analysis_module_alloc_external( rng_type * rng , const char * libname );

  void                   analysis_module_free( analysis_module_type * module );
  void                   analysis_module_free__( void * arg);

  void analysis_module_initX(analysis_module_type * module ,
                             matrix_type * X ,
                             matrix_type * A ,
                             matrix_type * S ,
                             matrix_type * R ,
                             matrix_type * dObs ,
                             matrix_type * E ,
                             matrix_type * D);


  void analysis_module_updateA(analysis_module_type * module ,
                               matrix_type * A ,
                               matrix_type * S ,
                               matrix_type * R ,
                               matrix_type * dObs ,
                               matrix_type * E ,
                               matrix_type * D ,
                               const module_info_type* module_info);


  void                   analysis_module_init_update( analysis_module_type * module ,
                                                      const bool_vector_type * ens_mask ,
                                                      const matrix_type * S ,
                                                      const matrix_type * R ,
                                                      const matrix_type * dObs ,
                                                      const matrix_type * E ,
                                                      const matrix_type * D );


  const char           * analysis_module_get_lib_name( const analysis_module_type * module);
  bool                   analysis_module_internal( const analysis_module_type * module );
  bool                   analysis_module_set_var( analysis_module_type * module , const char * var_name , const char * string_value );
  const char           * analysis_module_get_table_name( const analysis_module_type * module);
  const char           * analysis_module_get_name( const analysis_module_type * module );
  void                   analysis_module_set_name( analysis_module_type * module , const char * name);
  bool                   analysis_module_check_option( const analysis_module_type * module , long flag);
  void                   analysis_module_complete_update( analysis_module_type * module );

  bool                   analysis_module_has_var( const analysis_module_type * module , const char * var );
  double                 analysis_module_get_double( const analysis_module_type * module , const char * var);
  int                    analysis_module_get_int( const analysis_module_type * module , const char * var);
  bool                   analysis_module_get_bool( const analysis_module_type * module , const char * var);
  void *                 analysis_module_get_ptr( const analysis_module_type * module , const char * var);
  const char           * analysis_module_flag_enum_iget( int index, int * value);



  UTIL_IS_INSTANCE_HEADER( analysis_module );

#ifdef  __cplusplus
}
#endif
#endif
