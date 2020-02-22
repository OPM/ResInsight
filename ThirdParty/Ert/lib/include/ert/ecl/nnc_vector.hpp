/*
   Copyright (C) 2013  Equinor ASA, Norway.

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


#ifndef ERT_NNC_VECTOR_H
#define ERT_NNC_VECTOR_H

#include <ert/util/int_vector.hpp>
#include <ert/util/type_macros.hpp>

typedef struct nnc_vector_struct nnc_vector_type;

#ifdef __cplusplus
#include <vector>
  const std::vector<int>& nnc_vector_get_grid_index_list(const nnc_vector_type * nnc_vector);
  const std::vector<int>& nnc_vector_get_nnc_index_list(const nnc_vector_type * nnc_vector);
#endif

#ifdef __cplusplus
extern "C" {
#endif

  UTIL_IS_INSTANCE_HEADER(nnc_vector);

  int                       nnc_vector_iget_nnc_index( const nnc_vector_type * nnc_vector , int index );
  int                       nnc_vector_iget_grid_index( const nnc_vector_type * nnc_vector , int index );
  nnc_vector_type         * nnc_vector_alloc(int lgr_nr);
  nnc_vector_type         * nnc_vector_alloc_copy(const nnc_vector_type * src_vector);
  void                      nnc_vector_free( nnc_vector_type * nnc_vector );
  void                      nnc_vector_add_nnc(nnc_vector_type * nnc_vector, int global_cell_number, int nnc_index);
  int                       nnc_vector_get_lgr_nr(const nnc_vector_type * nnc_vector );
  void                      nnc_vector_free__(void * arg);
  int                       nnc_vector_get_size( const nnc_vector_type * nnc_vector );
  bool                      nnc_vector_equal( const nnc_vector_type * nnc_vector1 , const nnc_vector_type * nnc_vector2);

#ifdef __cplusplus
}
#endif
#endif

