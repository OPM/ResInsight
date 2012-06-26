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


#ifndef __SMSPEC_NODE_H__
#define __SMSPEC_NODE_H__

#include <stdbool.h>

#include <ecl_smspec.h>


#ifdef __cplusplus
extern "C" {
#endif


#define SMSPEC_NUMS_INVALID   -991199

  typedef struct smspec_node_struct smspec_node_type;

  char * smspec_alloc_block_ijk_key( const char * join_string , const char * keyword , int i , int j , int k);
  char * smspec_alloc_completion_ijk_key( const char * join_string , const char * keyword, const char * wgname , int i , int j , int k);
  char * smspec_alloc_completion_num_key( const char * join_string , const char * keyword, const char * wgname , int num);
  char * smspec_alloc_group_key( const char * join_string , const char * keyword , const char * wgname);
  char * smspec_alloc_well_key( const char * join_string , const char * keyword , const char * wgname);
  char * smspec_alloc_region_key( const char * join_string , const char * keyword , int num);
  char * smspec_alloc_segment_key( const char * join_string , const char * keyword , const char * wgname , int num);
  char * smspec_alloc_block_num_key( const char * join_string , const char * keyword , int num);
  char * smspec_alloc_local_well_key( const char * join_string , const char * keyword , const char * lgr_name , const char * wgname);
  char * smspec_alloc_local_block_key( const char * join_string , const char * keyword , const char * lgr_name , int i , int j , int k);
  char * smspec_alloc_local_completion_key( const char * join_string, const char * keyword , const char * lgr_name , const char * wgname , int i , int j , int k);



  
  smspec_node_type *  smspec_node_alloc_empty(ecl_smspec_var_type var_type, const char * keyword , const char * unit , int param_index);
  void                smspec_node_set_wgname( smspec_node_type * index , const char * wgname );
  void                smspec_node_set_lgr_name( smspec_node_type * index , const char * lgr_name );
  void                smspec_node_set_num( smspec_node_type * index , int num);
  void                smspec_node_set_gen_key( smspec_node_type * smspec_node , const char * key_join_string);
  void                smspec_node_set_flags( smspec_node_type * smspec_node);

  smspec_node_type * smspec_node_alloc( ecl_smspec_var_type var_type , 
                                          const char * wgname  , 
                                          const char * keyword , 
                                          const char * unit    , 
                                          const char * key_join_string , 
                                          int num , int index);

  smspec_node_type * smspec_node_alloc_lgr( ecl_smspec_var_type var_type , 
                                              const char * wgname  , 
                                              const char * keyword , 
                                              const char * unit    , 
                                              const char * lgr , 
                                              const char * key_join_string , 
                                              int   lgr_i, int lgr_j , int lgr_k,
                                              int index);

  void                smspec_node_free( smspec_node_type * index );
  int                 smspec_node_get_index( const smspec_node_type * smspec_node );  
  const char        * smspec_node_get_gen_key( const smspec_node_type * smspec_node);
  ecl_smspec_var_type smspec_node_get_var_type( const smspec_node_type * smspec_node);
  int                 smspec_node_get_num( const smspec_node_type * smspec_node);
  const char        * smspec_node_get_wgname( const smspec_node_type * smspec_node);
  const char        * smspec_node_get_keyword( const smspec_node_type * smspec_node);
  const char        * smspec_node_get_unit( const smspec_node_type * smspec_node);
  bool                smspec_node_is_rate( const smspec_node_type * smspec_node );
  bool                smspec_node_is_total( const smspec_node_type * smspec_node );
  
  
#ifdef __cplusplus
}
#endif
#endif
