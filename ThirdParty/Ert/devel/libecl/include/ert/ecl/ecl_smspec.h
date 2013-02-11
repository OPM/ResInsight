/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_smspec.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ECL_SMSPEC__
#define __ECL_SMSPEC__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdbool.h>

#include <ert/util/float_vector.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/smspec_node.h>

typedef struct ecl_smspec_struct ecl_smspec_type; 


/**
   These are the different variable types, see table 3.4 in the
   ECLIPFE file format docuemntation for naming conventions.

   Only the variable types marked with "X" below are supported in the
   remaining implementation. To add support for a new variable type
   the functions smspec_node_alloc(), ecl_smsepec_fread_header() and
   ecl_smspec_install_gen_key() must be updated.
*/
  const int_vector_type * ecl_smspec_get_index_map( const ecl_smspec_type * smspec );
  void                ecl_smspec_index_node( ecl_smspec_type * ecl_smspec , smspec_node_type * smspec_node);
  void                ecl_smspec_insert_node(ecl_smspec_type * ecl_smspec, smspec_node_type * smspec_node);  
  void ecl_smspec_add_node( ecl_smspec_type * ecl_smspec , smspec_node_type * smspec_node );
  ecl_smspec_var_type ecl_smspec_iget_var_type( const ecl_smspec_type * smspec , int index );
  bool                ecl_smspec_needs_num( ecl_smspec_var_type var_type );
  bool                ecl_smspec_needs_wgname( ecl_smspec_var_type var_type );
  const char        * ecl_smspec_get_var_type_name( ecl_smspec_var_type var_type );
  ecl_smspec_var_type ecl_smspec_identify_var_type(const char * var);
  ecl_smspec_type   * ecl_smspec_alloc_writer( const char * key_join_string , time_t sim_start , int nx , int ny , int nz);
  void                ecl_smspec_fwrite( const ecl_smspec_type * smspec , const char * ecl_case , bool fmt_file );
  
  ecl_smspec_type *        ecl_smspec_fread_alloc(const char *header_file, const char * key_join_string , bool include_restart);
  void                     ecl_smspec_free( ecl_smspec_type *);
  
  int                      ecl_smspec_get_sim_days_index( const ecl_smspec_type * smspec );
  int                      ecl_smspec_get_date_day_index( const ecl_smspec_type * smspec );
  int                      ecl_smspec_get_date_month_index( const ecl_smspec_type * smspec );
  int                      ecl_smspec_get_date_year_index( const ecl_smspec_type * smspec );


  const smspec_node_type * ecl_smspec_get_well_var_node( const ecl_smspec_type * smspec , const char * well , const char * var);
  int                      ecl_smspec_get_well_var_params_index(const ecl_smspec_type * ecl_smspec , const char * well , const char *var);
  bool                     ecl_smspec_has_well_var(const ecl_smspec_type * ecl_smspec , const char * well , const char *var);

  const smspec_node_type * ecl_smspec_get_group_var_node( const ecl_smspec_type * smspec , const char * group , const char * var);
  int                      ecl_smspec_get_group_var_params_index(const ecl_smspec_type * ecl_smspec , const char * group , const char *var);
  bool                     ecl_smspec_has_group_var(const ecl_smspec_type * ecl_smspec , const char * group , const char *var);

  const smspec_node_type * ecl_smspec_get_field_var_node( const ecl_smspec_type * smspec , const char * var);
  int                      ecl_smspec_get_field_var_params_index(const ecl_smspec_type * ecl_smspec , const char *var);
  bool                     ecl_smspec_has_field_var(const ecl_smspec_type * ecl_smspec , const char *var);

  const smspec_node_type * ecl_smspec_get_region_var_node(const ecl_smspec_type * ecl_smspec , const char *region_var , int region_nr);
  int                      ecl_smspec_get_region_var_params_index(const ecl_smspec_type * ecl_smspec , const char * region_var , int region_nr);
  bool                     ecl_smspec_has_region_var(const ecl_smspec_type * ecl_smspec , const char * region_var , int region_nr);
  
  const smspec_node_type * ecl_smspec_get_misc_var_node(const ecl_smspec_type * ecl_smspec , const char *var);
  int                      ecl_smspec_get_misc_var_params_index(const ecl_smspec_type * ecl_smspec , const char *var);
  bool                     ecl_smspec_has_misc_var(const ecl_smspec_type * ecl_smspec , const char *var);

  const smspec_node_type * ecl_smspec_get_block_var_node(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr);
  int                      ecl_smspec_get_block_var_params_index(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr);
  bool                     ecl_smspec_has_block_var(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr);
  
  const smspec_node_type * ecl_smspec_get_block_var_node_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k);
  int                      ecl_smspec_get_block_var_params_index_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k);
  bool                     ecl_smspec_has_block_var_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k);
  
  const smspec_node_type * ecl_smspec_get_well_completion_var_node(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr);
  int                      ecl_smspec_get_well_completion_var_params_index(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr);
  bool                     ecl_smspec_has_well_completion_var(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr);
  
  const smspec_node_type * ecl_smspec_get_general_var_node( const ecl_smspec_type * smspec , const char * lookup_kw );
  int                      ecl_smspec_get_general_var_params_index(const ecl_smspec_type * ecl_smspec , const char * lookup_kw);
  bool                     ecl_smspec_has_general_var(const ecl_smspec_type * ecl_smspec , const char * lookup_kw);
  const char             * ecl_smspec_get_general_var_unit( const ecl_smspec_type * ecl_smspec , const char * lookup_kw);

  
  //bool                ecl_smspec_general_is_total(const ecl_smspec_type * ecl_smspec , const char * gen_key);
  //bool                ecl_smspec_is_rate(const ecl_smspec_type * smspec , int kw_index);
  //ecl_smspec_var_type ecl_smspec_iget_var_type( const ecl_smspec_type * smspec , int index );
  //const char *        ecl_smspec_iget_unit( const ecl_smspec_type * smspec , int index );
  //int                 ecl_smspec_iget_num( const ecl_smspec_type * smspec , int index );
  //const char *        ecl_smspec_iget_wgname( const ecl_smspec_type * smspec , int index );
  //const char *        ecl_smspec_iget_keyword( const ecl_smspec_type * smspec , int index );
  



  void              ecl_smspec_init_var( ecl_smspec_type * ecl_smspec , smspec_node_type * smspec_node , const char * keyword , const char * wgname , int num, const char * unit );  
  void              ecl_smspec_select_matching_general_var_list( const ecl_smspec_type * smspec , const char * pattern , stringlist_type * keys);
  stringlist_type * ecl_smspec_alloc_matching_general_var_list(const ecl_smspec_type * smspec , const char * pattern);

  int               ecl_smspec_get_time_index( const ecl_smspec_type * ecl_smspec );
  time_t            ecl_smspec_get_start_time(const ecl_smspec_type * );
  /*****************************************************************/
  bool                       ecl_smspec_get_formatted( const ecl_smspec_type * ecl_smspec);
  const char               * ecl_smspec_get_header_file( const ecl_smspec_type * ecl_smspec );
  stringlist_type          * ecl_smspec_alloc_well_list( const ecl_smspec_type * smspec , const char * pattern);
  stringlist_type          * ecl_smspec_alloc_group_list( const ecl_smspec_type * smspec , const char * pattern);
  stringlist_type          * ecl_smspec_alloc_well_var_list( const ecl_smspec_type * smspec );
  const char               * ecl_smspec_get_simulation_path(const ecl_smspec_type * ecl_smspec);
  const stringlist_type    * ecl_smspec_get_restart_list( const ecl_smspec_type * ecl_smspec);
  const char               * ecl_smspec_get_join_string( const ecl_smspec_type * smspec);
  const float_vector_type  * ecl_smspec_get_params_default( const ecl_smspec_type * ecl_smspec );
  void                       ecl_smspec_update_wgname( ecl_smspec_type * smspec , smspec_node_type * node , const char * wgname );

  const int                * ecl_smspec_get_grid_dims( const ecl_smspec_type * smspec );
  int                        ecl_smspec_get_params_size( const ecl_smspec_type * smspec );
  const   smspec_node_type * ecl_smspec_iget_node( const ecl_smspec_type * smspec , int index );
  void                       ecl_smspec_lock( ecl_smspec_type * smspec );


  char                     * ecl_smspec_alloc_well_key( const ecl_smspec_type * smspec , const char * keyword , const char * wgname);

#ifdef __cplusplus
}
#endif
#endif
