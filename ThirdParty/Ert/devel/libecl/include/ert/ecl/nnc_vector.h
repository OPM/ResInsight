/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'nnc_vector.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __NNC_VECTOR_H__
#define __NNC_VECTOR_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include <ert/util/int_vector.h>  
#include <ert/util/type_macros.h>

  typedef struct nnc_vector_struct nnc_vector_type; 
  
  UTIL_IS_INSTANCE_HEADER(nnc_vector);
    
  nnc_vector_type         * nnc_vector_alloc(int lgr_nr);   
  void                      nnc_vector_free( nnc_vector_type * nnc_vector );
  void                      nnc_vector_add_nnc(nnc_vector_type * nnc_vector, int global_cell_number); 
  const int_vector_type   * nnc_vector_get_index_list(nnc_vector_type * nnc_vector);
  int                       nnc_vector_get_lgr_nr(const nnc_vector_type * nnc_vector );
  void                      nnc_vector_free__(void * arg);
  
#ifdef __cplusplus
}
#endif
#endif

