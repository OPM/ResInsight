/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'hash_node.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <ert/util/util.h>
#include <ert/util/node_data.h>
#include <ert/util/hash_node.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hash_node_struct {
  char             *key;
  uint32_t          global_index;
  uint32_t          table_index;
  node_data_type   *data;
  hash_node_type   *next_node;
};
  



bool hash_node_key_eq(const hash_node_type * node , uint32_t global_index , const char *key) {
  bool eq;
  eq = true;
  if (global_index != node->global_index)
    eq = false;
  else if (strcmp(node->key , key) != 0)
    eq = false;
  return eq;
}


uint32_t hash_node_get_table_index(const hash_node_type * node)  { 
  return node->table_index; 
}

uint32_t hash_node_get_global_index(const hash_node_type * node) { 
  return node->global_index; 
}

const char * hash_node_get_key(const hash_node_type * node) { 
  return node->key; 
}

hash_node_type * hash_node_get_next(const hash_node_type * node) { 
  return node->next_node; 
}


void hash_node_set_next(hash_node_type *node , const hash_node_type *next_node) {
  node->next_node = (hash_node_type *) next_node;
}


uint32_t hash_node_set_table_index(hash_node_type *node , uint32_t table_size) {
  node->table_index = (node->global_index % table_size);
  return node->table_index;
}


/*****************************************************************/
/* The three functions below here are the only functions accessing the
   data field of the hash_node.  
*/

node_data_type * hash_node_get_data(const hash_node_type * node) {
  return node->data;
}


hash_node_type * hash_node_alloc_new(const char *key, node_data_type * data, hashf_type *hashf , uint32_t table_size) {
  hash_node_type *node;
  node              = util_malloc(sizeof *node );
  node->key         = util_alloc_string_copy( key );
  node->data        = data;
  node->next_node   = NULL;
        
  node->global_index = hashf(node->key , strlen(node->key));
  hash_node_set_table_index(node , table_size);
  return node;
}

void hash_node_free(hash_node_type * node) {
  free(node->key);
  node_data_free( node->data );
  free(node);
}


#ifdef __cplusplus
}
#endif



