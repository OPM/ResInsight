/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'nnc_info.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __NNC_INFO_H__
#define __NNC_INFO_H__
#ifdef __cplusplus
extern "C" {
#endif
  
#include <ert/util/int_vector.h>  
#include <ert/util/type_macros.h>

#include <ert/ecl/nnc_vector.h>

  typedef struct nnc_info_struct nnc_info_type; 
  
  UTIL_IS_INSTANCE_HEADER(nnc_info);
    
  nnc_info_type         * nnc_info_alloc(int lgr_nr);   
  void                    nnc_info_free( nnc_info_type * nnc_info );
  void                    nnc_info_add_nnc(nnc_info_type * nnc_info, int lgr_nr, int global_cell_number); 

  const int_vector_type * nnc_info_iget_index_list(const nnc_info_type * nnc_info, int lgr_index); 
  nnc_vector_type       * nnc_info_iget_vector( const nnc_info_type * nnc_info , int lgr_index);

  const int_vector_type * nnc_info_get_index_list(const nnc_info_type * nnc_info, int lgr_nr); 
  nnc_vector_type       * nnc_info_get_vector( const nnc_info_type * nnc_info , int lgr_nr);

  const int_vector_type * nnc_info_get_self_index_list(const nnc_info_type * nnc_info);
  nnc_vector_type       * nnc_info_get_self_vector( const nnc_info_type * nnc_info );

  int                     nnc_info_get_lgr_nr(const nnc_info_type * nnc_info );
  int                     nnc_info_get_size( const nnc_info_type * nnc_info );

#ifdef __cplusplus
}
#endif
#endif

