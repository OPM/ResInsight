/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_box.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_BOX_H
#define ERT_ECL_BOX_H

#include <ert/ecl/ecl_grid.hpp>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecl_box_struct ecl_box_type;


void           ecl_box_set_size       (ecl_box_type * , int , int , int , int , int , int );
ecl_box_type * ecl_box_alloc(const ecl_grid_type * ecl_grid , int i1,int i2 , int j1 , int j2 , int k1, int k2);
void           ecl_box_free            (ecl_box_type * );
void           ecl_box_set_values(const ecl_box_type * , char * , const char * , int );
int            ecl_box_get_total_size(const ecl_box_type * );
int            ecl_box_get_active_size( const ecl_box_type * ecl_box );
const int    * ecl_box_get_active_list( const ecl_box_type * ecl_box );
int            ecl_box_get_global_size( const ecl_box_type * ecl_box );
const int    * ecl_box_get_global_list( const ecl_box_type * ecl_box );
bool           ecl_box_contains(const ecl_box_type * box , int i , int j , int k);

UTIL_IS_INSTANCE_HEADER( ecl_box );
UTIL_SAFE_CAST_HEADER( ecl_box );

#ifdef __cplusplus
}
#endif
#endif
