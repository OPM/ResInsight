/*
   Copyright (C) 2013  Equinor ASA, Norway.

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
#ifndef ERT_TYPE_VECTOR_FUNCTIONS_H
#define ERT_TYPE_VECTOR_FUNCTIONS_H

#include <ert/util/int_vector.hpp>
#include <ert/util/bool_vector.hpp>
#include <ert/util/double_vector.hpp>

#ifdef __cplusplus
extern "C" {
#endif

  int_vector_type  * bool_vector_alloc_active_list( const bool_vector_type * mask );
  bool_vector_type * int_vector_alloc_mask( const int_vector_type * active_list );
  int_vector_type  * bool_vector_alloc_active_index_list(const bool_vector_type * mask , int default_value);
  bool               double_vector_approx_equal( const double_vector_type * v1 , const double_vector_type * v12 , double epsilon);

#ifdef __cplusplus
}
#endif
#endif
