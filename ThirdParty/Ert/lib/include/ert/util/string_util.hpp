/*
   Copyright (C) 2013  Equinor ASA, Norway.

   The file 'string_util.h' is part of ERT - Ensemble based Reservoir Tool.

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
#ifndef ERT_STRING_UTIL_H
#define ERT_STRING_UTIL_H

#include <ert/util/int_vector.hpp>
#include <ert/util/bool_vector.hpp>

#ifdef __cplusplus
extern "C" {
#endif

  bool               string_util_init_active_list( const char * range_string , int_vector_type * active_list );
  bool               string_util_update_active_list( const char * range_string , int_vector_type * active_list );
  int_vector_type *  string_util_alloc_active_list( const char * range_string );

  bool               string_util_init_active_mask( const char * range_string , bool_vector_type * active_mask);
  bool               string_util_update_active_mask( const char * range_string , bool_vector_type * active_mask);
  bool_vector_type * string_util_alloc_active_mask( const char * range_string );

  bool               string_util_update_value_list( const char * range_string , int_vector_type * value_list);
  bool               string_util_init_value_list( const char * range_string , int_vector_type * value_list );
  int_vector_type  * string_util_alloc_value_list(const char * range_string);

#ifdef __cplusplus
}
#endif
#endif
