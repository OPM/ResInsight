/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'regression.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_REGRESSION_H
#define ERT_REGRESSION_H

#include <ert/util/matrix.h>

#ifdef __cplusplus
extern "C" {
#endif
double  regression_scale( matrix_type * X ,  matrix_type * Y , matrix_type * X_mean , matrix_type * X_norm);
double  regression_unscale(const matrix_type * beta , const matrix_type * X_norm , const matrix_type * X_mean , double Y_mean , matrix_type * beta0);
void    regression_augmented_OLS(const matrix_type * X , const matrix_type * Y , const matrix_type *E, matrix_type * beta);

#ifdef __cplusplus
}
#endif
#endif
