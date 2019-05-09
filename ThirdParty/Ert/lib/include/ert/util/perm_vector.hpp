/*
   Copyright (C) 2016  Equinor ASA, Norway.

   The file 'perm_vector.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef PERM_VECTOR_H
#define PERM_VECTOR_H

#include <ert/util/type_macros.hpp>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct perm_vector_struct perm_vector_type;

perm_vector_type * perm_vector_alloc( int * perm_input , int size );
void               perm_vector_free( perm_vector_type * perm_vector );
int                perm_vector_get_size( const perm_vector_type * perm);
int                perm_vector_iget( const perm_vector_type * perm, int index);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

