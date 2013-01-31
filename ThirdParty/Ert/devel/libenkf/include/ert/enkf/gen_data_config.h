/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_data_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __GEN_DATA_CONFIG_H__
#define __GEN_DATA_CONFIG_H__
#ifdef __cplusplus 
extern "C" {
#endif
#include <stdbool.h>

#include <ert/util/stringlist.h>
#include <ert/util/util.h>
#include <ert/util/bool_vector.h>

#include <ert/enkf/enkf_fs_type.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/gen_data_common.h>

typedef enum { GEN_DATA_UNDEFINED = 0,  
               ASCII              = 1,   /*   The file is ASCII file with a vector of numbers formatted with "%g".       */
               ASCII_TEMPLATE     = 2,   /*   The data is inserted into a user defined template file.                    */
               BINARY_DOUBLE      = 3,   /*   The data is in a binary file with doubles.                                 */
               BINARY_FLOAT       = 4}   /*   The data is in a binary file with floats.                                  */          
  gen_data_file_format_type;

  bool                         gen_data_config_is_dynamic( const gen_data_config_type * config );
  void                         gen_data_config_set_dynamic( gen_data_config_type * config , enkf_fs_type * fs);
  void                         gen_data_config_load_active( gen_data_config_type * config , int report_step , bool force_load);
    
  /* 
     Observe that the format ASCII_template can *NOT* be used for
     loading files.
  */
  gen_data_config_type       * gen_data_config_alloc_empty( const char * key );
  
  void                         gen_data_config_set_ens_size( gen_data_config_type * config , int ens_size );
  gen_data_file_format_type    gen_data_config_get_input_format ( const gen_data_config_type * );
  gen_data_file_format_type    gen_data_config_get_output_format ( const gen_data_config_type * );
  ecl_type_enum                gen_data_config_get_internal_type(const gen_data_config_type * );
  gen_data_config_type       * gen_data_config_alloc_with_options(const char * key , bool , const stringlist_type *);
  void                         gen_data_config_free(gen_data_config_type * );
  int                          gen_data_config_get_initial_size( const gen_data_config_type * config );
  void                         gen_data_config_assert_size(gen_data_config_type *  , int , int);
  const bool_vector_type     * gen_data_config_get_active_mask( const gen_data_config_type * config );
  void                         gen_data_config_update_active(gen_data_config_type * config , int report_step , const bool_vector_type * data_mask);
  const bool     *             gen_data_config_get_iactive(const gen_data_config_type * );
  void                         gen_data_config_ecl_write(const gen_data_config_type *  , const char * , char * );
  void                         gen_data_config_get_template_data( const gen_data_config_type * , char ** , int * , int * , int *);
  gen_data_config_type       * gen_data_config_fscanf_alloc(const char * );
  const char  *                gen_data_config_get_key( const gen_data_config_type * config);
  int                          gen_data_config_get_byte_size( const gen_data_config_type * config , int report_step);
  int                          gen_data_config_get_data_size( const gen_data_config_type * config , int report_step);
  gen_data_file_format_type    gen_data_config_check_format( const void * format_string );
  bool gen_data_config_is_valid( const gen_data_config_type * gen_data_config );
  void gen_data_config_update(gen_data_config_type * config           , 
                              enkf_var_type var_type                  ,
                              gen_data_file_format_type input_format  ,
                              gen_data_file_format_type output_format ,
                              const char * template_ecl_file          , 
                              const char * template_data_key          );
  
  const char * gen_data_config_get_template_file( const gen_data_config_type * config );
  const char * gen_data_config_get_template_key( const gen_data_config_type * config );
  void gen_data_config_fprintf_config( const gen_data_config_type * config , enkf_var_type var_type , const char * outfile , const char * infile , 
                                       const char * min_std_file , FILE * stream);
  
  UTIL_SAFE_CAST_HEADER(gen_data_config);
  UTIL_SAFE_CAST_HEADER_CONST(gen_data_config);
  VOID_FREE_HEADER(gen_data_config)
  
#ifdef __cplusplus
}
#endif
#endif
