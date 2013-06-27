/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'data_ranking.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/vector.h>
#include <ert/util/double_vector.h>
#include <ert/util/msg.h>
#include <ert/util/buffer.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/ranking_common.h>
#include <ert/enkf/data_ranking.h>


#define DATA_RANKING_TYPE_ID 71420672

struct data_ranking_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                  ens_size;   
  double_vector_type * data_ensemble;
  int                * sort_permutation;
  bool_vector_type   * valid;
  char               * user_key;
  bool                 sort_increasing;
};


UTIL_SAFE_CAST_FUNCTION( data_ranking , DATA_RANKING_TYPE_ID )
UTIL_IS_INSTANCE_FUNCTION( data_ranking , DATA_RANKING_TYPE_ID );


void data_ranking_free( data_ranking_type * ranking ) {
  double_vector_free( ranking->data_ensemble );
  bool_vector_free( ranking->valid );
  util_safe_free( ranking->sort_permutation );
  util_safe_free( ranking->user_key );
  free( ranking );
}





static void data_ranking_init(data_ranking_type * ranking , 
                              enkf_fs_type * fs , 
                              const enkf_config_node_type * config_node, 
                              const char * key_index , 
                              int step , 
                              state_enum state ) {

  enkf_node_type * enkf_node = enkf_node_alloc( config_node );
  int iens;
  for (iens = 0; iens < ranking->ens_size; iens++) {

    double value;
    node_id_type node_id = {.report_step = step , 
                            .iens = iens , 
                            .state = state };

    if (enkf_node_user_get( enkf_node , fs , key_index , node_id , &value)) {
      double_vector_iset( ranking->data_ensemble , iens , value );
      bool_vector_iset( ranking->valid , iens , true );
    }
  }

  if (ranking->sort_increasing) 
    ranking->sort_permutation = double_vector_alloc_sort_perm( ranking->data_ensemble );
  else 
     ranking->sort_permutation = double_vector_alloc_rsort_perm( ranking->data_ensemble );
  
  enkf_node_free( enkf_node );
}



data_ranking_type * data_ranking_alloc( bool sort_increasing , int ens_size , const char * user_key , const char * key_index , enkf_fs_type * fs , const enkf_config_node_type * config_node , int step , state_enum state) {
  data_ranking_type * ranking = util_malloc( sizeof * ranking );
  UTIL_TYPE_ID_INIT( ranking , DATA_RANKING_TYPE_ID );
  ranking->ens_size = ens_size;
  ranking->sort_increasing = sort_increasing;

  if (ranking->sort_increasing)
    ranking->data_ensemble = double_vector_alloc( ens_size ,  INFINITY);  // To ensure it comes last when sorting
  else
    ranking->data_ensemble = double_vector_alloc( ens_size , -INFINITY);  // To ensure it comes last when sorting

  ranking->valid = bool_vector_alloc( ens_size , false );
  ranking->sort_permutation = NULL;
  ranking->user_key = util_alloc_string_copy( user_key );

  data_ranking_init( ranking , fs , config_node , key_index , step , state );
  return ranking;
}



void data_ranking_free__( void * arg) {
  data_ranking_type * ranking = data_ranking_safe_cast( arg );
  data_ranking_free( ranking );
}


const int * data_ranking_get_permutation( const data_ranking_type * data_ranking ) {
  return data_ranking->sort_permutation;
}


void data_ranking_display( const data_ranking_type * data_ranking , FILE * stream) {
  const int ens_size                  = data_ranking->ens_size;
  const int * permutations            = data_ranking->sort_permutation;
  
  {
    int i;
    fprintf(stream,"\n\n");
    fprintf(stream,"  #    Realization    %12s\n" , data_ranking->user_key);
    fprintf(stream,"----------------------------------\n");
    for (i = 0; i < ens_size; i++) {
      if (bool_vector_iget( data_ranking->valid , permutations[i])) {
        int    iens         = permutations[i];
        fprintf(stream,"%3d    %3d          %14.3f\n",i,iens,double_vector_iget(data_ranking->data_ensemble , iens));
      }
    }
    fprintf(stream,"----------------------------------\n");
  }
}
