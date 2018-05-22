/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'hash_sll.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.hpp>
#include <ert/util/hash_node.hpp>
#include <ert/util/hash_sll.hpp>

#ifdef __cplusplus
extern "C" {
#endif

struct hash_sll_struct {
  int              length;
  hash_node_type * head;
};


static hash_sll_type * hash_sll_alloc( void ) {
  hash_sll_type * hash_sll = (hash_sll_type*) util_malloc(sizeof * hash_sll );
  hash_sll->length = 0;
  hash_sll->head   = NULL;
  return hash_sll;
}


hash_sll_type ** hash_sll_alloc_table(int size) {
  hash_sll_type ** table = (hash_sll_type**) util_malloc(size * sizeof * table );
  int i;
  for (i=0; i<size; i++)
    table[i] = hash_sll_alloc();
  return table;
}


void hash_sll_del_node(hash_sll_type *hash_sll , hash_node_type *del_node) {
  if (del_node == NULL) 
    util_abort("%s: tried to delete NULL node - aborting \n",__func__);

  {
    hash_node_type *node, *p_node;
    p_node = NULL;
    node   = hash_sll->head;
    while (node != NULL && node != del_node) {
      p_node = node;
      node   = hash_node_get_next(node);
    }

    if (node == del_node) {
      if (p_node == NULL) 
        /* 
           We are attempting to delete the first element in the list.
        */
        hash_sll->head = hash_node_get_next(del_node);
      else
        hash_node_set_next(p_node , hash_node_get_next(del_node));
      hash_node_free(del_node);
      hash_sll->length--;
    } else 
      util_abort("%s: tried to delete node not in list - aborting \n",__func__);

  }
}


hash_node_type * hash_sll_get_head(const hash_sll_type *hash_sll) { return hash_sll->head; }

void hash_sll_add_node(hash_sll_type *hash_sll , hash_node_type *new_node) {
  hash_node_set_next(new_node, hash_sll->head);
  hash_sll->head = new_node;
  hash_sll->length++;
}



void hash_sll_free(hash_sll_type *hash_sll) {
  if (hash_sll->head != NULL) {
    hash_node_type *node , *next_node;
    node = hash_sll->head;
    while (node != NULL) {
      next_node = hash_node_get_next(node);
      hash_node_free(node);
      node = next_node;
    }
  }
  free( hash_sll );
}


bool hash_sll_empty(const hash_sll_type * hash_sll) {
  if (hash_sll->length == 0)
    return true;
  else
    return false;
}


hash_node_type * hash_sll_get(const hash_sll_type *hash_sll, uint32_t global_index , const char *key) {
  hash_node_type * node = hash_sll->head;
  
  while ((node != NULL) && (!hash_node_key_eq(node , global_index , key))) 
    node = hash_node_get_next(node);
  
  return node;
}


bool hash_sll_has_key(const hash_sll_type *hash_sll , uint32_t global_index , const char *key) {
  if (hash_sll_get(hash_sll , global_index , key))
    return true;
  else
    return false;
}


#ifdef __cplusplus
}
#endif
