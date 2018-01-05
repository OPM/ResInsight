/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'matrix_stat.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_MATRIX_STAT_H
#define ERT_MATRIX_STAT_H

#include <ert/util/matrix.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef enum {
  LLSQ_SUCCESS = 0,
  LLSQ_INVALID_DIM = 1,
  LLSQ_UNDETERMINED = 2
} llsq_result_enum;


llsq_result_enum matrix_stat_llsq_estimate( matrix_type * beta , const matrix_type * X , const matrix_type * Y , const matrix_type * S);
llsq_result_enum matrix_stat_polyfit( matrix_type * beta , const matrix_type * X0 , const matrix_type * Y0 , const matrix_type * S);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
