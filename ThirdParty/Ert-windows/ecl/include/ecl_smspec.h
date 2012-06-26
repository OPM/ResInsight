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
#include <stringlist.h>

typedef struct ecl_smspec_struct ecl_smspec_type; 


/**
   These are the different variable types, see table 3.4 in the
   ECLIPFE file format docuemntation for naming conventions.

   Only the variable types marked with "X" below are supported in the
   remaining implementation. To add support for a new variable type
   the functions smspec_node_alloc(), ecl_smsepec_fread_header() and
   ecl_smspec_install_gen_key() must be updated.
*/

typedef enum {ECL_SMSPEC_INVALID_VAR            =  0 ,
              ECL_SMSPEC_AQUIFER_VAR            =  1 ,   
              ECL_SMSPEC_WELL_VAR               =  2 ,   /* X */
              ECL_SMSPEC_REGION_VAR             =  3 ,   /* X */
              ECL_SMSPEC_FIELD_VAR              =  4 ,   /* X */
              ECL_SMSPEC_GROUP_VAR              =  5 ,   /* X */
              ECL_SMSPEC_BLOCK_VAR              =  6 ,   /* X */
              ECL_SMSPEC_COMPLETION_VAR         =  7 ,   /* X */ 
              ECL_SMSPEC_LOCAL_BLOCK_VAR        =  8 ,   /* X */
              ECL_SMSPEC_LOCAL_COMPLETION_VAR   =  9 ,   /* X */
              ECL_SMSPEC_LOCAL_WELL_VAR         = 10 ,   /* X */
              ECL_SMSPEC_NETWORK_VAR            = 11 ,
              ECL_SMSPEC_REGION_2_REGION_VAR    = 12 ,
              ECL_SMSPEC_SEGMENT_VAR            = 13 ,   /* X */ 
              ECL_SMSPEC_MISC_VAR               = 14     /* X */}  ecl_smspec_var_type;

ecl_smspec_var_type ecl_smspec_iget_var_type( const ecl_smspec_type * smspec , int index );
bool                ecl_smspec_needs_num( ecl_smspec_var_type var_type );
bool                ecl_smspec_needs_wgname( ecl_smspec_var_type var_type );
const char        * ecl_smspec_get_var_type_name( ecl_smspec_var_type var_type );
ecl_smspec_var_type ecl_smspec_identify_var_type(const char * var);

bool              ecl_smspec_general_is_total(const ecl_smspec_type * ecl_smspec , const char * gen_key);
bool              ecl_smspec_is_rate(const ecl_smspec_type * smspec , int kw_index);

ecl_smspec_type * ecl_smspec_fread_alloc(const char * , const char *, bool include_restart);
void              ecl_smspec_free( ecl_smspec_type *);
void              ecl_smspec_set_time_info( const ecl_smspec_type *  , const float *  , double *  , time_t * );

int               ecl_smspec_get_well_var_index(const ecl_smspec_type * ecl_smspec , const char * well , const char *var);
bool              ecl_smspec_has_well_var(const ecl_smspec_type * ecl_smspec , const char * well , const char *var);

int               ecl_smspec_get_group_var_index(const ecl_smspec_type * ecl_smspec , const char * group , const char *var);
bool              ecl_smspec_has_group_var(const ecl_smspec_type * ecl_smspec , const char * group , const char *var);

int               ecl_smspec_get_field_var_index(const ecl_smspec_type * ecl_smspec , const char *var);
bool              ecl_smspec_has_field_var(const ecl_smspec_type * ecl_smspec , const char *var);

int               ecl_smspec_get_block_var_index(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr);
bool              ecl_smspec_has_block_var(const ecl_smspec_type * ecl_smspec , const char * block_var , int block_nr);
int               ecl_smspec_get_block_var_index_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k);
bool              ecl_smspec_has_block_var_ijk(const ecl_smspec_type * ecl_smspec , const char * block_var , int i , int j , int k);

int               ecl_smspec_get_region_var_index(const ecl_smspec_type * ecl_smspec , int region_nr , const char *var);
bool              ecl_smspec_has_region_var(const ecl_smspec_type * ecl_smspec , int region_nr , const char *var);

int               ecl_smspec_get_misc_var_index(const ecl_smspec_type * ecl_smspec , const char *var);
bool              ecl_smspec_has_misc_var(const ecl_smspec_type * ecl_smspec , const char *var);

int               ecl_smspec_get_well_completion_var_index(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr);
bool              ecl_smspec_has_well_completion_var(const ecl_smspec_type * ecl_smspec , const char * well , const char *var, int cell_nr);

int               ecl_smspec_get_general_var_index(const ecl_smspec_type * ecl_smspec , const char * lookup_kw);
bool              ecl_smspec_has_general_var(const ecl_smspec_type * ecl_smspec , const char * lookup_kw);
const char      * ecl_smspec_get_general_var_unit( const ecl_smspec_type * ecl_smspec , const char * lookup_kw);


ecl_smspec_var_type ecl_smspec_iget_var_type( const ecl_smspec_type * smspec , int index );
const char *        ecl_smspec_iget_unit( const ecl_smspec_type * smspec , int index );
int                 ecl_smspec_iget_num( const ecl_smspec_type * smspec , int index );
const char *        ecl_smspec_iget_wgname( const ecl_smspec_type * smspec , int index );
const char *        ecl_smspec_iget_keyword( const ecl_smspec_type * smspec , int index );





void              ecl_smspec_select_matching_general_var_list( const ecl_smspec_type * smspec , const char * pattern , stringlist_type * keys);
stringlist_type * ecl_smspec_alloc_matching_general_var_list(const ecl_smspec_type * smspec , const char * pattern);

time_t            ecl_smspec_get_start_time(const ecl_smspec_type * );
/*****************************************************************/
bool                    ecl_smspec_get_formatted( const ecl_smspec_type * ecl_smspec);
const char            * ecl_smspec_get_header_file( const ecl_smspec_type * ecl_smspec );
const char            * ecl_smspec_get_simulation_case(const ecl_smspec_type * );
  stringlist_type       * ecl_smspec_alloc_well_list( const ecl_smspec_type * smspec , const char * pattern);
  stringlist_type       * ecl_smspec_alloc_group_list( const ecl_smspec_type * smspec , const char * pattern);
stringlist_type       * ecl_smspec_alloc_well_var_list( const ecl_smspec_type * smspec );
const char            * ecl_smspec_get_simulation_path(const ecl_smspec_type * ecl_smspec);
const char            * ecl_smspec_get_base_name( const ecl_smspec_type * ecl_smspec);
const stringlist_type * ecl_smspec_get_restart_list( const ecl_smspec_type * ecl_smspec);
const char            * ecl_smspec_get_join_string( const ecl_smspec_type * smspec);

  int                     ecl_smspec_get_param_size( const ecl_smspec_type * smspec );
  const char            * ecl_smspec_iget_general_key( const ecl_smspec_type * smspec , int index);


#ifdef __cplusplus
}
#endif
#endif
