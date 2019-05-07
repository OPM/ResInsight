/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'node_data.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_NODE_DATA_H
#define ERT_NODE_DATA_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void       * (  copyc_ftype ) (const void *);
typedef void         (  free_ftype )  (void *);



typedef struct node_data_struct node_data_type;


void                       node_data_free(node_data_type *);
void                       node_data_free_container(node_data_type * );
node_data_type     	 * node_data_alloc_deep_copy(const node_data_type * );
node_data_type     	 * node_data_alloc_shallow_copy(const node_data_type * );
node_data_type           * node_data_alloc_copy(const node_data_type * node , bool deep_copy);
void     		 * node_data_get_ptr(const node_data_type *);
const void     		 * node_data_get_const_ptr(const node_data_type *);
node_data_type		 * node_data_alloc_buffer(const void *, int );
node_data_type 		 * node_data_alloc_ptr(const void * , copyc_ftype * , free_ftype *);

node_data_type		 * node_data_alloc_int(int );
int                        node_data_get_int( const node_data_type * );
int                        node_data_fetch_and_inc_int( node_data_type * node_data );
node_data_type		 * node_data_alloc_double(double );
double                     node_data_get_double( const node_data_type * );
node_data_type		 * node_data_alloc_string(const char *);
char *                     node_data_get_string( const node_data_type * );



#ifdef __cplusplus
}
#endif
#endif
