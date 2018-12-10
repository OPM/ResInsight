/*
  Copyright (C) 2012  Statoil ASA, Norway.

  The file 'smspec_node.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_SMSPEC_NODE_H
#define ERT_SMSPEC_NODE_H

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DUMMY_WELL ":+:+:+:+"
#define IS_DUMMY_WELL(well) (strcmp((well) , DUMMY_WELL) == 0)
#define SMSPEC_PARAMS_INDEX_INVALID -77


#define SMSPEC_TIME_KEYWORD "TIME"
#define SMSPEC_TIME_NUMS_VALUE     -32676

#define SMSPEC_YEARS_KEYWORD "YEARS"
#define SMSPEC_YEARS_NUMS_VALUE     -32676


typedef enum {ECL_SMSPEC_INVALID_VAR            =  0 ,
              ECL_SMSPEC_FIELD_VAR              =  1 ,   /* X */
              ECL_SMSPEC_REGION_VAR             =  2 ,   /* X */
              ECL_SMSPEC_GROUP_VAR              =  3 ,   /* X */
              ECL_SMSPEC_WELL_VAR               =  4 ,   /* X */
              ECL_SMSPEC_SEGMENT_VAR            =  5 ,   /* X */
              ECL_SMSPEC_BLOCK_VAR              =  6 ,   /* X */
              ECL_SMSPEC_AQUIFER_VAR            =  7 ,
              ECL_SMSPEC_COMPLETION_VAR         =  8 ,   /* X */
              ECL_SMSPEC_NETWORK_VAR            =  9 ,
              ECL_SMSPEC_REGION_2_REGION_VAR    = 10 ,
              ECL_SMSPEC_LOCAL_BLOCK_VAR        = 11 ,   /* X */
              ECL_SMSPEC_LOCAL_COMPLETION_VAR   = 12 ,   /* X */
              ECL_SMSPEC_LOCAL_WELL_VAR         = 13 ,   /* X */
              ECL_SMSPEC_MISC_VAR               = 14     /* X */}  ecl_smspec_var_type;


#define SMSPEC_NUMS_INVALID   -991199
#define SMSPEC_NUMS_WELL       1
#define SMSPEC_NUMS_GROUP      2
#define SMSPEC_NUMS_FIELD      0

  typedef struct smspec_node_struct smspec_node_type;

  char * smspec_alloc_block_ijk_key( const char * join_string , const char * keyword , int i , int j , int k);
  char * smspec_alloc_completion_ijk_key( const char * join_string , const char * keyword, const char * wgname , int i , int j , int k);
  char * smspec_alloc_completion_num_key( const char * join_string , const char * keyword, const char * wgname , int num);
  char * smspec_alloc_group_key( const char * join_string , const char * keyword , const char * wgname);
  char * smspec_alloc_well_key( const char * join_string , const char * keyword , const char * wgname);
  char * smspec_alloc_region_key( const char * join_string , const char * keyword , int num);
  char * smspec_alloc_region_2_region_r1r2_key( const char * join_string , const char * keyword , int r1, int r2);
  char * smspec_alloc_region_2_region_num_key( const char * join_string , const char * keyword , int num);
  char * smspec_alloc_segment_key( const char * join_string , const char * keyword , const char * wgname , int num);
  char * smspec_alloc_block_num_key( const char * join_string , const char * keyword , int num);
  char * smspec_alloc_local_well_key( const char * join_string , const char * keyword , const char * lgr_name , const char * wgname);
  char * smspec_alloc_local_block_key( const char * join_string , const char * keyword , const char * lgr_name , int i , int j , int k);
  char * smspec_alloc_local_completion_key( const char * join_string, const char * keyword , const char * lgr_name , const char * wgname , int i , int j , int k);

  bool smspec_node_equal( const smspec_node_type * node1,  const smspec_node_type * node2);

  void smspec_node_init( smspec_node_type * smspec_node,
                         ecl_smspec_var_type var_type ,
                         const char * wgname  ,
                         const char * keyword ,
                         const char * unit    ,
                         const char * key_join_string ,
                         const int grid_dims[3] ,
                         int num);

  smspec_node_type * smspec_node_alloc( ecl_smspec_var_type var_type ,
                                        const char * wgname  ,
                                        const char * keyword ,
                                        const char * unit    ,
                                        const char * key_join_string ,
                                        const int grid_dims[3] ,
                                        int num , int param_index, float default_value);

  smspec_node_type * smspec_node_alloc_lgr( ecl_smspec_var_type var_type ,
                                            const char * wgname  ,
                                            const char * keyword ,
                                            const char * unit    ,
                                            const char * lgr ,
                                            const char * key_join_string ,
                                            int   lgr_i, int lgr_j , int lgr_k,
                                            int param_index,
                                            float default_value);

  smspec_node_type *  smspec_node_alloc_new(int params_index, float default_value);
  smspec_node_type *  smspec_node_alloc_copy( const smspec_node_type* );

  void                smspec_node_free( smspec_node_type * index );
  void                smspec_node_free__(void * arg);
  void                smspec_node_set_params_index( smspec_node_type * smspec_node , int params_index);
  int                 smspec_node_get_params_index( const smspec_node_type * smspec_node );
  const char        * smspec_node_get_gen_key1( const smspec_node_type * smspec_node);
  const char        * smspec_node_get_gen_key2( const smspec_node_type * smspec_node);
  ecl_smspec_var_type smspec_node_get_var_type( const smspec_node_type * smspec_node);
  int                 smspec_node_get_num( const smspec_node_type * smspec_node);
  const char        * smspec_node_get_wgname( const smspec_node_type * smspec_node);
  const char        * smspec_node_get_keyword( const smspec_node_type * smspec_node);
  const char        * smspec_node_get_unit( const smspec_node_type * smspec_node);
  void                smspec_node_set_unit( smspec_node_type * smspec_node , const char * unit );
  bool                smspec_node_is_rate( const smspec_node_type * smspec_node );
  bool                smspec_node_is_total( const smspec_node_type * smspec_node );
  bool                smspec_node_is_historical( const smspec_node_type * smspec_node );
  bool                smspec_node_need_nums( const smspec_node_type * smspec_node );
  void                smspec_node_fprintf( const smspec_node_type * smspec_node , FILE * stream);

  void                smspec_node_set_default( smspec_node_type * smspec_node , float default_value);
  float               smspec_node_get_default( const smspec_node_type * smspec_node);

  const int*  smspec_node_get_ijk( const smspec_node_type * smpsec_node );
  const char* smspec_node_get_lgr_name( const smspec_node_type * smpsec_node );
  const int*  smspec_node_get_lgr_ijk( const smspec_node_type * smpsec_node );

  int smspec_node_get_R1( const smspec_node_type * smpsec_node );
  int smspec_node_get_R2( const smspec_node_type * smpsec_node );

  int smspec_node_cmp( const smspec_node_type * node1, const smspec_node_type * node2);
  int smspec_node_cmp__( const void * node1, const void * node2);
  bool smspec_node_identify_total(const char * keyword, ecl_smspec_var_type var_type);
  bool smspec_node_identify_rate(const char * keyword);

#ifdef __cplusplus
}
#endif
#endif
