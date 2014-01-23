/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_main_jobs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/stringlist.h>
#include <ert/util/string_util.h>
#include <ert/util/int_vector.h>

#include <ert/enkf/enkf_main.h>
#include <ert/enkf/field_config.h>


void * enkf_main_exit_JOB(void * self , const stringlist_type * args ) {
  enkf_main_type  * enkf_main = enkf_main_safe_cast( self );
  enkf_main_exit( enkf_main );
  return NULL;
}


void * enkf_main_assimilation_JOB( void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  int ens_size                 = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive   = bool_vector_alloc( 0 , true );

  bool_vector_iset( iactive , ens_size - 1 , true );
  enkf_main_run_assimilation( enkf_main , iactive , 0 , 0 ,  ANALYZED );
  return NULL;
}


void * enkf_main_ensemble_run_JOB( void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  int ens_size                 = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive   = bool_vector_alloc( 0 , true );

  // Ignore args until string_utils is in place ..... 
  // if (stringlist_get_size( args ) 

  bool_vector_iset( iactive , ens_size - 1 , true );
  enkf_main_run_exp( enkf_main , iactive , true , 0 , 0 , ANALYZED);
  return NULL;
}


void * enkf_main_smoother_JOB( void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  int ens_size                 = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive   = bool_vector_alloc( ens_size , true );
  bool rerun                   = true;
  const char * target_case     = stringlist_iget( args , 0 );
  
  enkf_main_run_smoother( enkf_main , target_case , iactive , rerun);
  bool_vector_free( iactive );
  return NULL;
}


void * enkf_main_iterated_smoother_JOB( void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  int ens_size                 = enkf_main_get_ensemble_size( enkf_main );
  bool_vector_type * iactive   = bool_vector_alloc( 0 , true );

  bool_vector_iset( iactive , ens_size - 1 , true );
  enkf_main_run_iterated_ES( enkf_main);
  return NULL;
}


void * enkf_main_select_module_JOB( void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  analysis_config_type * analysis_config = enkf_main_get_analysis_config( enkf_main );
  
  analysis_config_select_module( analysis_config , stringlist_iget( args , 0 ));
  
  return NULL;
}



void * enkf_main_create_reports_JOB(void * self , const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  ert_report_list_type * report_list = enkf_main_get_report_list( enkf_main );
  
  ert_report_list_create( report_list , enkf_main_get_current_fs( enkf_main ) , true );
  return NULL;
}

void * enkf_main_scale_obs_std_JOB(void * self, const stringlist_type * args ) {
  enkf_main_type   * enkf_main = enkf_main_safe_cast( self );
  
  double scale_factor;
  util_sscanf_double(stringlist_iget(args, 0), &scale_factor);

  if (enkf_main_have_obs(enkf_main)) {
    enkf_obs_type * observations = enkf_main_get_obs(enkf_main);
    enkf_obs_scale_std(observations, scale_factor);
  }
  return NULL;
}


void * enkf_main_export_field_JOB(void * self, const stringlist_type * args) {
  const char *      field            = stringlist_iget(args, 0); 
  const char *      file_name        = stringlist_iget(args, 1); 
  int_vector_type * realization_list = string_util_alloc_active_list(""); //Realizations range: rest of optional input arguments
  int               report_step      = 0;
  util_sscanf_int(stringlist_iget(args,2), &report_step);
  state_enum        state            = enkf_types_get_state_enum(stringlist_iget(args, 3)); 
    
  char * range_str = stringlist_alloc_joined_substring( args , 4 , stringlist_get_size(args), "");  
  string_util_update_active_list(range_str, realization_list); 
  
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  if (0 == int_vector_size(realization_list)) {
      const char * range_str = util_alloc_sprintf("0-%d", enkf_main_get_ensemble_size( enkf_main )-1); 
      string_util_update_active_list(range_str, realization_list); 
  }  
  
  field_file_format_type file_type = field_config_default_export_format(file_name); 
  if ((RMS_ROFF_FILE == file_type) || (ECL_GRDECL_FILE == file_type)) 
    enkf_main_export_field(enkf_main, field, file_name, realization_list, file_type, report_step, state);
  else
    printf("EXPORT_FIELD filename argument: File extension must be either .roff or .grdecl\n"); 
    
  int_vector_free(realization_list);  
  return NULL; 
}

void * enkf_main_export_field_to_RMS_JOB(void * self, const stringlist_type * args) {
  const char *      field            = stringlist_iget(args, 0); 
  const char *      file_name        = stringlist_iget(args, 1); 
  int_vector_type * realization_list = string_util_alloc_active_list(""); //Realizations range: rest of optional input arguments
  int               report_step      = 0;
  util_sscanf_int(stringlist_iget(args,2), &report_step);
  state_enum        state            = enkf_types_get_state_enum(stringlist_iget(args, 3)); 
  
  char * range_str = stringlist_alloc_joined_substring( args , 4 , stringlist_get_size(args), "");  
  string_util_update_active_list(range_str, realization_list); 
    
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  if (0 == int_vector_size(realization_list)) {
      const char * range_str = util_alloc_sprintf("0-%d", enkf_main_get_ensemble_size( enkf_main )-1); 
      string_util_update_active_list(range_str, realization_list); 
  }  
  
  char * file_name_with_ext = util_alloc_string_copy(file_name); 
  char *ext = strrchr(file_name_with_ext , '.');
  if (ext == NULL) {
    file_name_with_ext = util_strcat_realloc(file_name_with_ext, ".roff");
  }

  enkf_main_export_field(enkf_main, field, file_name_with_ext, realization_list, RMS_ROFF_FILE, report_step, state);

  int_vector_free(realization_list);  
  return NULL; 
}

void * enkf_main_export_field_to_ECL_JOB(void * self, const stringlist_type * args) {
  const char *      field            = stringlist_iget(args, 0); 
  const char *      file_name        = stringlist_iget(args, 1); 
  int_vector_type * realization_list = string_util_alloc_active_list(""); //Realizations range: rest of optional input arguments
  int               report_step      = 0;
  util_sscanf_int(stringlist_iget(args,2), &report_step);
  state_enum        state            = enkf_types_get_state_enum(stringlist_iget(args, 3)); 
  
  char * range_str = stringlist_alloc_joined_substring( args , 4 , stringlist_get_size(args), "");  
  string_util_update_active_list(range_str, realization_list); 
  
  enkf_main_type * enkf_main = enkf_main_safe_cast( self );
  if (0 == int_vector_size(realization_list)) {
      const char * range_str = util_alloc_sprintf("0-%d", enkf_main_get_ensemble_size( enkf_main )-1); 
      string_util_update_active_list(range_str, realization_list); 
  }  
  
  char * file_name_with_ext = util_alloc_string_copy(file_name); 
  char *ext = strrchr(file_name_with_ext , '.');
  if (ext == NULL) 
    file_name_with_ext = util_strcat_realloc(file_name_with_ext, ".grdecl");
  
  enkf_main_export_field(enkf_main, field, file_name_with_ext, realization_list, ECL_GRDECL_FILE, report_step, state);

  int_vector_free(realization_list);  
  return NULL; 
}


