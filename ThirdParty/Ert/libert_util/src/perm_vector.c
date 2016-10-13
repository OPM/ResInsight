/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'perm_vector.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/type_macros.h>
#include <ert/util/util.h>
#include <ert/util/perm_vector.h>

#define PERM_VECTOR_TYPE_ID 661433

struct perm_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  int   size;
  int * perm;
};


/*
  This constructor will *take ownership* of the input int* array; and
  call free( ) on it when the perm_vector is destroyed.
*/

perm_vector_type * perm_vector_alloc( int * perm_input, int size ) {
  perm_vector_type * perm = util_malloc( sizeof * perm );
  UTIL_TYPE_ID_INIT( perm , PERM_VECTOR_TYPE_ID );
  perm->size = size;
  perm->perm = perm_input;
  return perm;
}


void perm_vector_free( perm_vector_type * perm) {
  free( perm->perm );
  free( perm );
}



int perm_vector_get_size( const perm_vector_type * perm) {
  return perm->size;
}


int perm_vector_iget( const perm_vector_type * perm, int index) {
  if (index < perm->size)
    return perm->perm[index];
  else {
    util_abort("%s: invalid index:%d \n",__func__ , index );
    return -1;
  }
}


