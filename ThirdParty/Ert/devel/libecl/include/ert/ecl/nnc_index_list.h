/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'nnc_index_list.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __NNC_INDEX_LIST_H__
#define __NNC_INDEX_LIST_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include <ert/util/int_vector.h>  
#include <ert/util/type_macros.h>

  typedef struct nnc_index_list_struct nnc_index_list_type; 
  
  UTIL_IS_INSTANCE_HEADER(nnc_index_list);
    
  nnc_index_list_type         * nnc_index_list_alloc();   
  void                      nnc_index_list_free( nnc_index_list_type * index_list );
  void                      nnc_index_list_add_index(nnc_index_list_type * index_list , int index); 
  const int_vector_type   * nnc_index_list_get_list(nnc_index_list_type * index_list); 

#ifdef __cplusplus
}
#endif
#endif

