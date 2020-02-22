/*
   Copyright (C) 2012  Equinor ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_COARSE_CELL_H
#define ERT_ECL_COARSE_CELL_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecl_coarse_cell_struct ecl_coarse_cell_type;

bool        ecl_coarse_cell_equal(const ecl_coarse_cell_type * coarse_cell1,
                                  const ecl_coarse_cell_type * coarse_cell2);
ecl_coarse_cell_type * ecl_coarse_cell_alloc(void);
void        ecl_coarse_cell_update(ecl_coarse_cell_type * coarse_cell,
                                   int i,
                                   int j,
                                   int k,
                                   int global_index);
void        ecl_coarse_cell_free(ecl_coarse_cell_type * coarse_cell);
void        ecl_coarse_cell_free__(void * arg);

int         ecl_coarse_cell_get_i1(const ecl_coarse_cell_type * coarse_cell);
int         ecl_coarse_cell_get_j1(const ecl_coarse_cell_type * coarse_cell);
int         ecl_coarse_cell_get_k1(const ecl_coarse_cell_type * coarse_cell);
int         ecl_coarse_cell_get_i2(const ecl_coarse_cell_type * coarse_cell);
int         ecl_coarse_cell_get_j2(const ecl_coarse_cell_type * coarse_cell);
int         ecl_coarse_cell_get_k2(const ecl_coarse_cell_type * coarse_cell);
const int * ecl_coarse_cell_get_box_ptr(const ecl_coarse_cell_type * coarse_cell);

int         ecl_coarse_cell_get_size(const ecl_coarse_cell_type * coarse_cell);
int         ecl_coarse_cell_iget_cell_index(ecl_coarse_cell_type * coarse_cell,
                                            int group_index);
const int * ecl_coarse_cell_get_index_ptr(ecl_coarse_cell_type * coarse_cell);
const int_vector_type * ecl_coarse_cell_get_index_vector(ecl_coarse_cell_type * coarse_cell);

void ecl_coarse_cell_reset_active_index(ecl_coarse_cell_type * coarse_cell);
void ecl_coarse_cell_update_index(ecl_coarse_cell_type * coarse_cell,
                                  int global_index,
                                  int * active_index,
                                  int * active_fracture_index,
                                  int active_value);
int  ecl_coarse_cell_get_active_index(const ecl_coarse_cell_type * coarse_cell);
int  ecl_coarse_cell_get_active_fracture_index(const ecl_coarse_cell_type * coarse_cell);
int  ecl_coarse_cell_iget_active_cell_index(const ecl_coarse_cell_type * coarse_cell,
                                            int index);
int  ecl_coarse_cell_iget_active_value(const ecl_coarse_cell_type * coarse_cell,
                                       int index);
int  ecl_coarse_cell_get_num_active(const ecl_coarse_cell_type * coarse_cell);
void ecl_coarse_cell_fprintf(const ecl_coarse_cell_type * coarse_cell,
                             FILE * stream);

#ifdef __cplusplus
}
#endif
#endif
