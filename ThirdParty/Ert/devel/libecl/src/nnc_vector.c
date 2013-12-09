
/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'nnc_vector.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>
#include <ert/util/vector.h>  
#include <ert/util/type_macros.h>
#include <ert/util/int_vector.h>

#include <ert/ecl/nnc_vector.h>



#define NNC_VECTOR_TYPE_ID 875615078


struct nnc_vector_struct {
  UTIL_TYPE_ID_DECLARATION;
  int                lgr_nr;
  int_vector_type * grid_index_list;
  int_vector_type * nnc_index_list;
}; 


UTIL_IS_INSTANCE_FUNCTION( nnc_vector , NNC_VECTOR_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION(nnc_vector , NNC_VECTOR_TYPE_ID)



nnc_vector_type * nnc_vector_alloc(int lgr_nr) {
  nnc_vector_type * nnc_vector = util_malloc( sizeof * nnc_vector );
  UTIL_TYPE_ID_INIT(nnc_vector , NNC_VECTOR_TYPE_ID);
  nnc_vector->grid_index_list = int_vector_alloc(0,0);
  nnc_vector->nnc_index_list  = int_vector_alloc(0,0);
  nnc_vector->lgr_nr = lgr_nr;
  return nnc_vector; 
}

void nnc_vector_free( nnc_vector_type * nnc_vector ) {
  int_vector_free( nnc_vector->grid_index_list );
  int_vector_free( nnc_vector->nnc_index_list );
  free( nnc_vector ); 
}


void nnc_vector_free__(void * arg) {
  nnc_vector_type * nnc_vector = nnc_vector_safe_cast( arg );
  nnc_vector_free( nnc_vector );
}


void nnc_vector_add_nnc(nnc_vector_type * nnc_vector, int global_cell_number , int nnc_index) {
  int_vector_append( nnc_vector->grid_index_list , global_cell_number );
  int_vector_append( nnc_vector->nnc_index_list , nnc_index);
}
   

const int_vector_type * nnc_vector_get_grid_index_list(const nnc_vector_type * nnc_vector) {
  return nnc_vector->grid_index_list;
}

const int_vector_type * nnc_vector_get_nnc_index_list(const nnc_vector_type * nnc_vector) {
  return nnc_vector->nnc_index_list;
}

int nnc_vector_get_size( const nnc_vector_type * nnc_vector ) {
  return int_vector_size( nnc_vector->grid_index_list );
}

int nnc_vector_get_lgr_nr( const nnc_vector_type * nnc_vector ) {
  return nnc_vector->lgr_nr;
}
