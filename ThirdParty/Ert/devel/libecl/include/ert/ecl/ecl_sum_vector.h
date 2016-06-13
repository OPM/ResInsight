/*
  Copyright (C) 2014  Statoil ASA, Norway.

  The file 'ecl_sum_vector.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_SUM_VECTOR_H
#define ERT_ECL_SUM_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>

#include <ert/ecl/ecl_sum.h>

typedef struct ecl_sum_vector_struct ecl_sum_vector_type;

  void ecl_sum_vector_free( ecl_sum_vector_type * keylist );
  ecl_sum_vector_type * ecl_sum_vector_alloc(const ecl_sum_type * ecl_sum);

  bool ecl_sum_vector_add_key( ecl_sum_vector_type * keylist, const char * key);
  void ecl_sum_vector_add_keys( ecl_sum_vector_type * keylist, const char * pattern);

  bool ecl_sum_vector_iget_is_rate(const ecl_sum_vector_type * ecl_sum_vector, int index);
  int ecl_sum_vector_iget_param_index(const ecl_sum_vector_type * ecl_sum_vector, int index);
  int ecl_sum_vector_get_size(const ecl_sum_vector_type * ecl_sum_vector);

  UTIL_IS_INSTANCE_HEADER( ecl_sum_vector);


#ifdef __cplusplus
}
#endif
#endif
