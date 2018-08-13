/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_box.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_box.hpp>
#include <ert/ecl/ecl_grid.hpp>


#define ECL_BOX_TYPE_ID 6610643

struct ecl_box_struct {
  UTIL_TYPE_ID_DECLARATION;
  int     grid_nx  , grid_ny  , grid_nz;
  int     grid_sx  , grid_sy  , grid_sz;   /* xxx_sx : x stride */

  int     i1,i2,j1,j2,k1,k2;
  int     box_nx  , box_ny  , box_nz;
  int     box_sx  , box_sy  , box_sz;
  int     box_offset;
  int     active_size;
  int    *active_list;  /* This is a list with active_size elements containing the index active index of the elemtents in the box. Will be NULL if there are no active elements. */
  int    *global_list;  /* This is a list of global indices which are present in the box. */
  const ecl_grid_type * parent_grid;
};



UTIL_IS_INSTANCE_FUNCTION( ecl_box , ECL_BOX_TYPE_ID)
UTIL_SAFE_CAST_FUNCTION( ecl_box , ECL_BOX_TYPE_ID)


/**
   Observe that:

    1. The coordinates i1,i2...k2 are assumed to be ZERO offset.
    2. The corrdinates are  _INCLUSIVE_, i.e. the box is [i1..i2] x [j1..j2] x [k1..k2]
    3. Coordinates are truncated to [0,ni).
    4. Coordinates are interchanged, so __i1 can be greater than __i2.
*/

ecl_box_type * ecl_box_alloc(const ecl_grid_type * ecl_grid , int __i1,int __i2 , int __j1 , int __j2 , int __k1, int __k2) {
  ecl_box_type * ecl_box = (ecl_box_type *)util_malloc(sizeof * ecl_box );
  UTIL_TYPE_ID_INIT( ecl_box , ECL_BOX_TYPE_ID);
  ecl_box->parent_grid = ecl_grid;
  /* Properties of the parent grid. */
  ecl_grid_get_dims( ecl_grid , &ecl_box->grid_nx , &ecl_box->grid_ny , &ecl_box->grid_nz , NULL);
  ecl_box->grid_sx   = 1;
  ecl_box->grid_sy   = ecl_box->grid_nx;
  ecl_box->grid_sz   = ecl_box->grid_nx * ecl_box->grid_ny;

  {
    int i1 = util_int_max( util_int_min(__i1 , __i2 ) , 0);
    int i2 = util_int_min( util_int_max(__i1 , __i2 ) , ecl_box->grid_nx - 1);

    int j1 = util_int_max( util_int_min(__j1 , __j2 ) , 0 );
    int j2 = util_int_min( util_int_max(__j1 , __j2 ) , ecl_box->grid_ny - 1);

    int k1 = util_int_max( util_int_min(__k1 , __k2 ) , 0 );
    int k2 = util_int_min( util_int_max(__k1 , __k2 ) , ecl_box->grid_nz - 1);

    ecl_box->i1 = i1;
    ecl_box->i2 = i2;
    ecl_box->j1 = j1;
    ecl_box->j2 = j2;
    ecl_box->k1 = k1;
    ecl_box->k2 = k2;

    /*Properties of the box: */
    ecl_box->box_nx = i2 - i1 + 1;
    ecl_box->box_ny = j2 - j1 + 1;
    ecl_box->box_nz = k2 - k1 + 1;

    ecl_box->box_sx = 1;
    ecl_box->box_sy = ecl_box->box_nx;
    ecl_box->box_sz = ecl_box->box_nx * ecl_box->box_ny;
    ecl_box->box_offset = i1 * ecl_box->box_sx + j1 * ecl_box->box_sy + k1 * ecl_box->box_sz;
    /* Counting the number of active elements in the box */


    {
      int global_counter = 0;
      int i,j,k;
      ecl_box->active_size = 0;
      ecl_box->active_list = (int*)util_calloc( ecl_box->box_nx * ecl_box->box_ny * ecl_box->box_nz , sizeof * ecl_box->active_list );
      ecl_box->global_list = (int*)util_calloc( ecl_box->box_nx * ecl_box->box_ny * ecl_box->box_nz , sizeof * ecl_box->global_list );
      for (k=k1; k <= k2; k++)
        for (j=j1; j <= j2; j++)
          for (i=i1; i <= i2; i++) {
            {
              int global_index = ecl_grid_get_global_index3( ecl_box->parent_grid , i , j , k);
              ecl_box->global_list[global_counter] = global_index;
              global_counter++;
            }
            {
              int active_index = ecl_grid_get_active_index3( ecl_box->parent_grid , i,j,k);
              if (active_index >= 0) {
                ecl_box->active_list[ecl_box->active_size] = active_index;
                ecl_box->active_size++;
              }
            }
          }

      ecl_box->active_list = (int*)util_realloc( ecl_box->active_list , ecl_box->active_size * sizeof * ecl_box->active_list );
    }
  }
  return ecl_box;
}


/**
   Returns true if the box contains the point (i,j,k). Observe the
   following:

    ijk: These are zero offset.
    ijk: Which are ON one of the box surfaces will return true.

*/


bool ecl_box_contains(const ecl_box_type * box , int i , int j , int k) {

  return (( box->i1 >= i ) && (i <= box->i2) &&
          ( box->j1 >= j ) && (j <= box->j2) &&
          ( box->k1 >= k ) && (k <= box->k2));

}




void ecl_box_free(ecl_box_type * ecl_box) {
  free(ecl_box->active_list );
  free(ecl_box->global_list );
  free(ecl_box);
}



/*
void ecl_kw_merge(ecl_kw_type * main_kw , const ecl_kw_type * sub_kw , const ecl_box_type * ecl_box) {
  if (main_kw->sizeof_ctype != sub_kw->sizeof_ctype)
    util_abort("%s: trying to combine two different underlying datatypes - aborting \n",__func__);

  if (ecl_kw_get_size(main_kw) != ecl_box_get_total_size(ecl_box))
    util_abort("%s box size and total_kw mismatch - aborting \n",__func__);

  if (ecl_kw_get_size(sub_kw)   != ecl_box_get_box_size(ecl_box))
    util_abort("%s box size and total_kw mismatch - aborting \n",__func__);

  ecl_box_set_values(ecl_box , ecl_kw_get_data_ref(main_kw) , ecl_kw_get_data_ref(sub_kw) , main_kw->sizeof_ctype);
}
*/

void ecl_box_set_values(const ecl_box_type * ecl_box , char * main_field , const char * sub_field , int element_size) {
  int i,j,k;

  for (k=0; k < ecl_box->box_nz; k++)
    for(j=0; j < ecl_box->box_ny; j++)
      for (i=0; i < ecl_box->box_nx; i++) {
        int grid_index = k*ecl_box->grid_sz   + j*ecl_box->grid_sy   + i*ecl_box->grid_sx + ecl_box->box_offset;
        int box_index  = k*ecl_box->box_sz    + j*ecl_box->box_sy    + i*ecl_box->box_sx;
        memcpy(&main_field[grid_index * element_size] , &sub_field[box_index * element_size] , element_size);
      }
}


/*
  Return the number of active element in the box.
*/
int ecl_box_get_active_size( const ecl_box_type * ecl_box ) {
  return ecl_box->active_size;
}


const int * ecl_box_get_active_list( const ecl_box_type * ecl_box ) {
  return ecl_box->active_list;
}


/*
  Return the number of global element in the box.
*/
int ecl_box_get_global_size( const ecl_box_type * ecl_box ) {
  return ecl_box->box_nx * ecl_box->box_ny * ecl_box->box_nz;
}


const int * ecl_box_get_global_list( const ecl_box_type * ecl_box ) {
  return ecl_box->global_list;
}




