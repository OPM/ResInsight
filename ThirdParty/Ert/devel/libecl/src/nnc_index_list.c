
/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'nnc_index_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/int_vector.h>  
#include <ert/util/type_macros.h>

#include <ert/ecl/nnc_index_list.h>


#define NNC_INDEX_LIST_TYPE_ID 87225078


struct nnc_index_list_struct {
  UTIL_TYPE_ID_DECLARATION;
  int_vector_type  *index_list;            
  int               lgr_nr;
  bool              sorted;
}; 


UTIL_IS_INSTANCE_FUNCTION( nnc_index_list , NNC_INDEX_LIST_TYPE_ID )



nnc_index_list_type * nnc_index_list_alloc() {
  nnc_index_list_type * nnc_index_list = util_malloc( sizeof * nnc_index_list );
  UTIL_TYPE_ID_INIT(nnc_index_list , NNC_INDEX_LIST_TYPE_ID);
  nnc_index_list->index_list = int_vector_alloc(0 , 0 );
  nnc_index_list->sorted = true;
  return nnc_index_list; 
}


void nnc_index_list_free( nnc_index_list_type * nnc_index_list ) {
  int_vector_free( nnc_index_list->index_list ); 
  free( nnc_index_list ); 
}


const int_vector_type * nnc_index_list_get_list( nnc_index_list_type * nnc_index_list) {
  if (!nnc_index_list->sorted) 
    int_vector_select_unique( nnc_index_list->index_list );

  nnc_index_list->sorted = true;
  return nnc_index_list->index_list;
}



void nnc_index_list_add_index( nnc_index_list_type * nnc_index_list , int index) {
  nnc_index_list->sorted = false;
  int_vector_append( nnc_index_list->index_list , index );
}
