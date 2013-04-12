/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_sum.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ECL_SUM_H__
#define __ECL_SUM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <ert/util/stringlist.h>
#include <ert/util/time_t_vector.h>
#include <ert/util/double_vector.h>
#include <ert/util/time_interval.h>

#include <ert/ecl/ecl_smspec.h>
#include <ert/ecl/ecl_sum_tstep.h>
#include <ert/ecl/smspec_node.h>


  typedef struct {
    char * locale;
    char * sep;
    char * newline;
    char * value_fmt;
    char * date_fmt;
    char * days_fmt;
    char * header_fmt;
    bool   print_header;
    bool   print_dash;
    char * date_header;
    char * date_dash;
    char * value_dash;
  } ecl_sum_fmt_type;
    
  void ecl_sum_fmt_init_csv( ecl_sum_fmt_type * fmt );
  void ecl_sum_fmt_init_summary_x( ecl_sum_fmt_type * fmt );
  /*****************************************************************/

typedef struct ecl_sum_struct       ecl_sum_type;

  double         ecl_sum_get_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const smspec_node_type * node);
  double         ecl_sum_get_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const smspec_node_type * node);
  double         ecl_sum_time2days( const ecl_sum_type * ecl_sum , time_t sim_time);
  
  void           ecl_sum_set_unified( ecl_sum_type * ecl_sum , bool unified );
  void           ecl_sum_set_fmt_case( ecl_sum_type * ecl_sum , bool fmt_case );

  int              ecl_sum_get_report_step_from_time( const ecl_sum_type * sum , time_t sim_time);
  int              ecl_sum_get_report_step_from_days( const ecl_sum_type * sum , double sim_days);
  bool             ecl_sum_check_sim_time( const ecl_sum_type * sum , time_t sim_time);
  bool             ecl_sum_check_sim_days( const ecl_sum_type * sum , double sim_days);
  const char *     ecl_sum_get_keyword( const ecl_sum_type * sum , const char * gen_key );
  const char *     ecl_sum_get_wgname( const ecl_sum_type * sum , const char * gen_key );
  const char *     ecl_sum_get_unit( const ecl_sum_type * sum , const char * gen_key );
  int              ecl_sum_get_num( const ecl_sum_type * sum , const char * gen_key );

  double           ecl_sum_iget( const ecl_sum_type * ecl_sum , int time_index , int param_index);
  int              ecl_sum_iget_num( const ecl_sum_type * sum , int param_index );
  const char *     ecl_sum_iget_wgname( const ecl_sum_type * sum , int param_index );
  const char *     ecl_sum_iget_keyword( const ecl_sum_type * sum , int param_index );
  int              ecl_sum_get_data_length( const ecl_sum_type * ecl_sum );
  double           ecl_sum_iget_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , int param_index);
  double           ecl_sum_iget_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , int param_index );



  void             ecl_sum_summarize( const ecl_sum_type * ecl_sum , FILE * stream );
  bool             ecl_sum_general_is_total(const ecl_sum_type * ecl_sum , const char * gen_key);
  bool             ecl_sum_var_is_total(const ecl_sum_type * ecl_sum , const char * gen_key);
  void             ecl_sum_free_data(ecl_sum_type * );
  void             ecl_sum_free__(void * );
  void             ecl_sum_free(ecl_sum_type * );
  ecl_sum_type   * ecl_sum_fread_alloc(const char * , const stringlist_type * data_files, const char * key_join_string);
  ecl_sum_type   * ecl_sum_fread_alloc_case(const char *  , const char * key_join_string);
  ecl_sum_type   * ecl_sum_fread_alloc_case__(const char *  , const char * key_join_string , bool include_restart);
  
  /* Accessor functions : */
  double            ecl_sum_get_well_var(const ecl_sum_type * ecl_sum , int time_index , const char * well , const char *var);
  bool              ecl_sum_has_well_var(const ecl_sum_type * ecl_sum , const char * well , const char *var);
  double            ecl_sum_get_well_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * well , const char * var);
  double            ecl_sum_get_well_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * well , const char * var);
  
  double            ecl_sum_get_group_var(const ecl_sum_type * ecl_sum , int time_index , const char * group , const char *var);
  bool              ecl_sum_has_group_var(const ecl_sum_type * ecl_sum , const char * group , const char *var);
  
  double            ecl_sum_get_field_var(const ecl_sum_type * ecl_sum , int time_index , const char *var);
  bool              ecl_sum_has_field_var(const ecl_sum_type * ecl_sum , const char *var);
  double            ecl_sum_get_field_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var);
  double            ecl_sum_get_field_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var);
  
  double            ecl_sum_get_block_var(const ecl_sum_type * ecl_sum , int time_index , const char * block_var , int block_nr);
  int               ecl_sum_get_block_var_index(const ecl_sum_type * ecl_sum , const char * block_var , int block_nr);
  bool              ecl_sum_has_block_var(const ecl_sum_type * ecl_sum , const char * block_var , int block_nr);
  double            ecl_sum_get_block_var_ijk(const ecl_sum_type * ecl_sum , int time_index , const char * block_var , int i , int j , int k);
  int               ecl_sum_get_block_var_index_ijk(const ecl_sum_type * ecl_sum , const char * block_var , int i , int j , int k);
  bool              ecl_sum_has_block_var_ijk(const ecl_sum_type * ecl_sum , const char * block_var , int i , int j , int k);
  
  double            ecl_sum_get_region_var(const ecl_sum_type * ecl_sum , int time_index , const char *var , int region_nr);
  bool              ecl_sum_has_region_var(const ecl_sum_type * ecl_sum ,  const char *var , int region_nr);
  
  double            ecl_sum_get_misc_var(const ecl_sum_type * ecl_sum , int time_index , const char *var);
  int               ecl_sum_get_misc_var_index(const ecl_sum_type * ecl_sum , const char *var);
  bool              ecl_sum_has_misc_var(const ecl_sum_type * ecl_sum , const char *var);
  
  double            ecl_sum_get_well_completion_var(const ecl_sum_type * ecl_sum , int time_index  , const char * well , const char *var, int cell_nr);
  int               ecl_sum_get_well_completion_var_index(const ecl_sum_type * ecl_sum , const char * well , const char *var, int cell_nr);
  bool              ecl_sum_has_well_completion_var(const ecl_sum_type * ecl_sum , const char * well , const char *var, int cell_nr);
  
  double            ecl_sum_get_general_var(const ecl_sum_type * ecl_sum , int time_index , const char * lookup_kw);
  int               ecl_sum_get_general_var_params_index(const ecl_sum_type * ecl_sum , const char * lookup_kw);
  const smspec_node_type * ecl_sum_get_general_var_node(const ecl_sum_type * ecl_sum , const char * lookup_kw);
  bool              ecl_sum_has_general_var(const ecl_sum_type * ecl_sum , const char * lookup_kw);
  bool              ecl_sum_has_key(const ecl_sum_type * ecl_sum , const char * lookup_kw);
  double            ecl_sum_get_general_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var);
  double            ecl_sum_get_general_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var);
  const char *      ecl_sum_get_general_var_unit( const ecl_sum_type * ecl_sum , const char * var);
  /***************/
  void              ecl_sum_fprintf(const ecl_sum_type * , FILE * , const stringlist_type * , bool report_only , const ecl_sum_fmt_type * fmt);
  
  
  
  
  /* Time related functions */
  int    ecl_sum_get_first_gt( const ecl_sum_type * ecl_sum , int param_index , double limit); 
  int    ecl_sum_get_first_lt( const ecl_sum_type * ecl_sum , int param_index , double limit);
  int    ecl_sum_get_last_report_step( const ecl_sum_type * ecl_sum );
  int    ecl_sum_get_first_report_step( const ecl_sum_type * ecl_sum );
  bool   ecl_sum_has_report_step(const ecl_sum_type * ecl_sum , int report_step );
  time_t ecl_sum_get_report_time( const ecl_sum_type * ecl_sum , int report_step );
  time_t ecl_sum_iget_sim_time( const ecl_sum_type * ecl_sum , int index );
  double ecl_sum_iget_sim_days( const ecl_sum_type * ecl_sum , int time_index);
  int    ecl_sum_iget_report_step( const ecl_sum_type * ecl_sum , int internal_index );
  int    ecl_sum_iget_mini_step( const ecl_sum_type * ecl_sum , int internal_index );
  double ecl_sum_iget_general_var(const ecl_sum_type * ecl_sum , int internal_index , const char * lookup_kw);
  
  
  void                 ecl_sum_init_data_vector( const ecl_sum_type * ecl_sum , double_vector_type * data_vector , int data_index , bool report_only );
  double_vector_type * ecl_sum_alloc_data_vector( const ecl_sum_type * ecl_sum  , int data_index , bool report_only);
  time_t_vector_type * ecl_sum_alloc_time_vector( const ecl_sum_type * ecl_sum  , bool report_only);
  time_t       ecl_sum_get_data_start( const ecl_sum_type * ecl_sum );
  time_t       ecl_sum_get_end_time( const ecl_sum_type * ecl_sum);
  time_t       ecl_sum_get_start_time(const ecl_sum_type * );
  const time_interval_type * ecl_sum_get_sim_time( const ecl_sum_type * ecl_sum);

  const char * ecl_sum_get_base(const ecl_sum_type * ecl_sum );
  const char * ecl_sum_get_path(const ecl_sum_type * ecl_sum );
  const char * ecl_sum_get_abs_path(const ecl_sum_type * ecl_sum );
  const char * ecl_sum_get_case(const ecl_sum_type * );
  bool         ecl_sum_same_case( const ecl_sum_type * ecl_sum , const char * input_file );
  
  void ecl_sum_resample_from_sim_days( const ecl_sum_type * ecl_sum , const double_vector_type * sim_days , double_vector_type * value , const char * gen_key);
  void ecl_sum_resample_from_sim_time( const ecl_sum_type * ecl_sum , const time_t_vector_type * sim_time , double_vector_type * value , const char * gen_key);
  time_t ecl_sum_time_from_days( const ecl_sum_type * ecl_sum , double sim_days );
  double ecl_sum_days_from_time( const ecl_sum_type * ecl_sum , time_t sim_time );
  double                ecl_sum_get_sim_length( const ecl_sum_type * ecl_sum ) ;
  double                ecl_sum_get_first_day( const ecl_sum_type * ecl_sum );

  /*****************************************************************/
  stringlist_type     * ecl_sum_alloc_well_list( const ecl_sum_type * ecl_sum , const char * pattern);
  stringlist_type     * ecl_sum_alloc_group_list( const ecl_sum_type * ecl_sum , const char * pattern);
  stringlist_type     * ecl_sum_alloc_well_var_list( const ecl_sum_type * ecl_sum );
  stringlist_type     * ecl_sum_alloc_matching_general_var_list(const ecl_sum_type * ecl_sum , const char * pattern);  
  void                  ecl_sum_select_matching_general_var_list( const ecl_sum_type * ecl_sum , const char * pattern , stringlist_type * keys);
  const ecl_smspec_type * ecl_sum_get_smspec( const ecl_sum_type * ecl_sum );
  ecl_smspec_var_type   ecl_sum_identify_var_type(const char * var);
  ecl_smspec_var_type   ecl_sum_get_var_type( const ecl_sum_type * ecl_sum , const char * gen_key);
  bool                  ecl_sum_var_is_rate( const ecl_sum_type * ecl_sum , const char * gen_key);
  bool                  ecl_sum_var_is_total( const ecl_sum_type * ecl_sum , const char * gen_key);
  
  int                   ecl_sum_iget_report_end( const ecl_sum_type * ecl_sum , int report_step );
  int                   ecl_sum_iget_report_start( const ecl_sum_type * ecl_sum , int report_step );

  ecl_sum_type        * ecl_sum_alloc_writer( const char * ecl_case , bool fmt_output , bool unified , const char * key_join_string , time_t sim_start , int nx , int ny , int nz);
  void                  ecl_sum_set_case( ecl_sum_type * ecl_sum , const char * ecl_case);
  void                  ecl_sum_fwrite( const ecl_sum_type * ecl_sum );
  void                  ecl_sum_fwrite_smspec( const ecl_sum_type * ecl_sum );
  smspec_node_type    * ecl_sum_add_var( ecl_sum_type * ecl_sum , const char * keyword , const char * wgname , int num , const char * unit , float default_value);
  smspec_node_type    * ecl_sum_add_blank_var( ecl_sum_type * ecl_sum , float default_value);
  void                  ecl_sum_init_var( ecl_sum_type * ecl_sum , smspec_node_type * smspec_node , const char * keyword , const char * wgname , int num , const char * unit);
  ecl_sum_tstep_type  * ecl_sum_add_tstep( ecl_sum_type * ecl_sum , int report_step , double sim_days);
  void                  ecl_sum_update_wgname( ecl_sum_type * ecl_sum , smspec_node_type * node , const char * wgname );

  char                * ecl_sum_alloc_well_key( const ecl_sum_type * ecl_sum , const char * keyword , const char * wgname);
UTIL_IS_INSTANCE_HEADER( ecl_sum );

#ifdef __cplusplus
}
#endif
#endif
