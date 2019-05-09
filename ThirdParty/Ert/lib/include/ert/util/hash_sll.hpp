/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'hash_sll.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_HASH_SLL_H
#define ERT_HASH_SLL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/hash_node.hpp>

typedef struct hash_sll_struct hash_sll_type;

hash_sll_type  **hash_sll_alloc_table(int );
/*hash_sll_type *  hash_sll_alloc(void);*/
void             hash_sll_del_node(hash_sll_type * , hash_node_type *);
void             hash_sll_add_node(hash_sll_type *, hash_node_type *);
void             hash_sll_free(hash_sll_type *);
bool             hash_sll_has_key(const hash_sll_type *, uint32_t , const char *);
bool             hash_sll_empty(const hash_sll_type * hash_sll);
hash_node_type * hash_sll_get(const hash_sll_type *, uint32_t , const char *);
hash_node_type * hash_sll_get_head(const hash_sll_type *);
#ifdef __cplusplus
}
#endif
#endif
