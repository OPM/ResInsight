/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ert_util_vector_function.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>



int_vector_type * bool_vector_alloc_active_list( const bool_vector_type * mask ) {
  int_vector_type * active_list = int_vector_alloc(0,0);
  int i;

  for (i =0; i < bool_vector_size( mask ); i++)
    if (bool_vector_iget( mask , i ))
      int_vector_append( active_list , i );
  
  return active_list;
}


int_vector_type * bool_vector_alloc_active_index_list(const bool_vector_type * mask , int default_value) {
  int_vector_type * index_list = int_vector_alloc(bool_vector_size( mask) , default_value);
  int active_index = 0;
  int i;
  for (i=0; i < bool_vector_size(mask); i++) {
    if (bool_vector_iget( mask , i)) {
      int_vector_iset(index_list , i , active_index);
      active_index++;
    }
  }
  return index_list;
}


bool_vector_type * int_vector_alloc_mask( const int_vector_type * active_list ) {
  bool_vector_type * mask = bool_vector_alloc( 0 , false );
  int i;
  for (i=0; i < int_vector_size( active_list ); i++) 
    bool_vector_iset( mask , int_vector_iget( active_list , i) , true );

  return mask;
}



