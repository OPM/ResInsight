/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'type_vector_functions.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#ifndef __TYPE_VECTOR_FUNCTIONS_H__
#define __TYPE_VECTOR_FUNCTIONS_H__

#ifdef __cplusplus 
extern "C" {
#endif
#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>
  
  int_vector_type * bool_vector_alloc_active_list( const bool_vector_type * mask );
  bool_vector_type * int_vector_alloc_mask( const int_vector_type * active_list );
  int_vector_type * bool_vector_alloc_active_index_list(const bool_vector_type * mask , int default_value);

#ifdef __cplusplus
}
#endif
#endif
