/*
  Copyright (C) 2014  Statoil ASA, Norway.

  The file 'ecl_sum_vector.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/ecl/ecl_sum_vector.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_smspec.h>

#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/type_macros.h>
#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>


#define ECL_SUM_VECTOR_TYPE_ID 8768778

struct ecl_sum_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  int_vector_type * node_index_list;
  bool_vector_type * is_rate_list;
  const ecl_sum_type * ecl_sum;
};


void ecl_sum_vector_free( ecl_sum_vector_type * ecl_sum_vector ){
    int_vector_free(ecl_sum_vector->node_index_list);
    bool_vector_free(ecl_sum_vector->is_rate_list);
}


UTIL_IS_INSTANCE_FUNCTION( ecl_sum_vector , ECL_SUM_VECTOR_TYPE_ID )


ecl_sum_vector_type * ecl_sum_vector_alloc(const ecl_sum_type * ecl_sum){
    ecl_sum_vector_type * ecl_sum_vector = util_malloc( sizeof * ecl_sum_vector );
    UTIL_TYPE_ID_INIT( ecl_sum_vector , ECL_SUM_VECTOR_TYPE_ID);
    ecl_sum_vector->ecl_sum  = ecl_sum;
    ecl_sum_vector->node_index_list = int_vector_alloc(0,0);
    ecl_sum_vector->is_rate_list  = bool_vector_alloc(0,false);
    return ecl_sum_vector;
}


bool ecl_sum_vector_add_key( ecl_sum_vector_type * ecl_sum_vector, const char * key){
  if (ecl_sum_has_general_var( ecl_sum_vector->ecl_sum , key)) {
    const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum_vector->ecl_sum , key );
    int params_index = smspec_node_get_params_index( node );
    bool is_rate_key = smspec_node_is_rate( node);

    int_vector_append(ecl_sum_vector->node_index_list, params_index);
    bool_vector_append(ecl_sum_vector->is_rate_list, is_rate_key);
    return true;
  } else
    return false;
}



void ecl_sum_vector_add_keys( ecl_sum_vector_type * ecl_sum_vector, const char * pattern){
    stringlist_type * keylist = ecl_sum_alloc_matching_general_var_list(ecl_sum_vector->ecl_sum , pattern);

    int num_keywords = stringlist_get_size(keylist);
    int i;
    for(i = 0; i < num_keywords ;i++){
        const char * key = stringlist_iget(keylist, i);
        const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum_vector->ecl_sum , key );
        int params_index = smspec_node_get_params_index( node );
        bool is_rate_key = smspec_node_is_rate( node);

        int_vector_append(ecl_sum_vector->node_index_list, params_index);
        bool_vector_append(ecl_sum_vector->is_rate_list, is_rate_key);
    }
    stringlist_free(keylist);
}

int ecl_sum_vector_get_size(const ecl_sum_vector_type * ecl_sum_vector){
    return int_vector_size(ecl_sum_vector->node_index_list);
}

bool ecl_sum_vector_iget_is_rate(const ecl_sum_vector_type * ecl_sum_vector, int index){
    return bool_vector_iget(ecl_sum_vector->is_rate_list, index);
}

int ecl_sum_vector_iget_param_index(const ecl_sum_vector_type * ecl_sum_vector, int index){
    return int_vector_iget(ecl_sum_vector->node_index_list, index);
}
