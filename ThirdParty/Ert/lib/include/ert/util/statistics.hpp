/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'statistics.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_STATISTICS_H
#define ERT_STATISTICS_H

#ifdef __cplusplus
extern "C" {
#endif
#include <ert/util/double_vector.hpp>

double      statistics_std( const double_vector_type * data_vector );
double      statistics_mean( const double_vector_type * data_vector );
double      statistics_empirical_quantile( double_vector_type * data , double quantile );
double      statistics_empirical_quantile__( const double_vector_type * data , double quantile );

#ifdef __cplusplus
}
#endif
#endif
