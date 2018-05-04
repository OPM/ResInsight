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
#include <ert/ecl/ecl_sum_vector.hpp>
#include <ert/ecl/ecl_sum.hpp>
#include <ert/ecl/ecl_smspec.hpp>

#include <ert/util/util.hpp>
#include <ert/util/vector.hpp>
#include <ert/util/type_macros.hpp>
#include <ert/util/int_vector.hpp>
#include <ert/util/bool_vector.hpp>


#define ECL_SUM_VECTOR_TYPE_ID 8768778

struct ecl_sum_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  int_vector_type * node_index_list;
  bool_vector_type * is_rate_list;
  stringlist_type * key_list;
  const ecl_sum_type * ecl_sum;
};


void ecl_sum_vector_free( ecl_sum_vector_type * ecl_sum_vector ){
    int_vector_free(ecl_sum_vector->node_index_list);
    bool_vector_free(ecl_sum_vector->is_rate_list);
    stringlist_free(ecl_sum_vector->key_list);
    free(ecl_sum_vector);
}


UTIL_IS_INSTANCE_FUNCTION( ecl_sum_vector , ECL_SUM_VECTOR_TYPE_ID )

static void ecl_sum_vector_add_node(ecl_sum_vector_type * vector, const smspec_node_type * node, const char * key ) {
  int params_index = smspec_node_get_params_index( node );
  bool is_rate_key = smspec_node_is_rate( node);

  int_vector_append(vector->node_index_list, params_index);
  bool_vector_append(vector->is_rate_list, is_rate_key);
  stringlist_append_copy( vector->key_list, key );
}


ecl_sum_vector_type * ecl_sum_vector_alloc(const ecl_sum_type * ecl_sum, bool add_keywords) {
    ecl_sum_vector_type * ecl_sum_vector = (ecl_sum_vector_type*)util_malloc( sizeof * ecl_sum_vector );
    UTIL_TYPE_ID_INIT( ecl_sum_vector , ECL_SUM_VECTOR_TYPE_ID);
    ecl_sum_vector->ecl_sum  = ecl_sum;
    ecl_sum_vector->node_index_list = int_vector_alloc(0,0);
    ecl_sum_vector->is_rate_list  = bool_vector_alloc(0,false);
    ecl_sum_vector->key_list = stringlist_alloc_new( );
    if (add_keywords) {
      const ecl_smspec_type * smspec = ecl_sum_get_smspec(ecl_sum);
      for (int i=0; i < ecl_smspec_num_nodes(smspec); i++) {
        const smspec_node_type * node = ecl_smspec_iget_node( smspec , i );
        if (!smspec_node_is_valid(node))
          continue;

        const char * key = smspec_node_get_gen_key1(node);
        /*
          The TIME keyword is special case handled to not be included; that is
          to match the same special casing in the key matching function.
        */
        if (!util_string_equal(key, "TIME"))
          ecl_sum_vector_add_node( ecl_sum_vector, node, key);
      }
    }
    return ecl_sum_vector;
}

static void ecl_sum_vector_add_invalid_key(ecl_sum_vector_type * vector, const char * key) {
  int_vector_append(vector->node_index_list, -1);
  bool_vector_append(vector->is_rate_list, false);
  stringlist_append_copy(vector->key_list, key);
}


/*
  This function will allocate a keyword vector for the keys in the @ecl_sum
  argument passed in, it will contain all the same keys as in the input argument
  @src_vector. If the @src_vector contains keys which are not present in
  @ecl_sum an entry marked as *invalid* will be added. The whole point about
  this function is to ensure that calls to:

       ecl_sum_fwrite_interp_csv_line( )

  will result in nicely aligned output even if the different summary cases do
  not have the exact same keys.
*/

ecl_sum_vector_type * ecl_sum_vector_alloc_layout_copy(const ecl_sum_vector_type * src_vector, const ecl_sum_type * ecl_sum) {
  ecl_sum_vector_type * new_vector = ecl_sum_vector_alloc(ecl_sum, false);
  for (int i=0; i < stringlist_get_size(src_vector->key_list); i++) {
    const char * key = stringlist_iget(src_vector->key_list, i);
    if (ecl_sum_has_general_var(ecl_sum, key))
      ecl_sum_vector_add_key(new_vector, key);
    else
      ecl_sum_vector_add_invalid_key(new_vector, key);
  }
  return new_vector;
}



bool ecl_sum_vector_add_key( ecl_sum_vector_type * ecl_sum_vector, const char * key){
  if (ecl_sum_has_general_var( ecl_sum_vector->ecl_sum , key)) {
    const smspec_node_type * node = ecl_sum_get_general_var_node( ecl_sum_vector->ecl_sum , key );
    ecl_sum_vector_add_node(ecl_sum_vector, node, key);
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
        ecl_sum_vector_add_node(ecl_sum_vector, node, key);
    }
    stringlist_free(keylist);
}

int ecl_sum_vector_get_size(const ecl_sum_vector_type * ecl_sum_vector){
    return int_vector_size(ecl_sum_vector->node_index_list);
}

bool ecl_sum_vector_iget_is_rate(const ecl_sum_vector_type * ecl_sum_vector, int index){
    return bool_vector_iget(ecl_sum_vector->is_rate_list, index);
}

bool ecl_sum_vector_iget_valid(const ecl_sum_vector_type * ecl_sum_vector, int index) {
  return (int_vector_iget(ecl_sum_vector->node_index_list, index) >= 0);
}

int ecl_sum_vector_iget_param_index(const ecl_sum_vector_type * ecl_sum_vector, int index){
    return int_vector_iget(ecl_sum_vector->node_index_list, index);
}


const char* ecl_sum_vector_iget_key(const ecl_sum_vector_type * ecl_sum_vector, int index){
  return stringlist_iget( ecl_sum_vector->key_list , index);
}
