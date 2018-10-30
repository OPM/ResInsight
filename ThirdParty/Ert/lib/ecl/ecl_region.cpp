/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_region.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdbool.h>
#include <string.h>

#include <ert/util/int_vector.hpp>
#include <ert/util/util.h>

#include <ert/geometry/geo_util.hpp>
#include <ert/geometry/geo_polygon.hpp>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_box.hpp>
#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_region.hpp>


/**
   This file implements a type called ecl_region which is a way to
   select and keep track of designated cells in an ECLIPSE reservoir
   grid. An instance is allocated with a grid (a shared reference
   which is NOT modified). Then we can select/deselect cells based on:

     1. Integer equality     - ecl_region_select_equal()
     2. Float interval       - ecl_region_select_in_interval()
     3. A rectangualar box   - ecl_region_select_from_box()
     4. Subsections of i/j/k - ecl_region_select_i1i2() / ecl_region_seleect_j1j2() / ...
     5. All cells            - ecl_region_select_all()

   When allocating an instance you determine whether all cells should
   be initially selected, or not-selected. The various select
   functions can (of course ...) be chained together. All functions
   exist in a xxx_select() and an opposite xxx_deselect() version.

   When you are finished with selecting you can query the ecl_region
   instance for the number of active cells, and get a (const int *) to
   the indices. You can also get the results in term of global
   indices. (Refer to ecl_grid for the difference between active and
   global indices).

   For the functions which take ecl_kw input, the ecl_kw instance must
   have either nx*ny*nz elements, or nactive(from the grid)
   elements. This is checked, and the program will fail hard if it is
   not satisfied.

   Example:
   --------

   ecl_grid_type   * ecl_grid;
   ecl_kw_type     * soil;
   ecl_kw_type     * regions;
   ecl_region_type * ecl_region;

   // Load grid, soil and regions somehow.

   ecl_region = ecl_region_alloc( ecl_grid , false );                       // Start with nothing selected
   ecl_region_select_in_interval( ecl_region , soil , 0.50, 1.00);          // Select all cells with soil > 0.50
   ecl_region_select_equal( ecl_region , regions , 3 );                     // Only consider ECLIPSE region 3.
   ecl_region_select_k1k2( ecl_region , 5 , 8);                             // Select layers 5,6,7,8
   {
      int num_cells         = ecl_region_get_global_size( ecl_region );     // How many cells are active
      const int * cell_list = ecl_region_get_global_list( ecl_region );     // Get a list of indices
      int i;
      printf("%d cells satisfy your selection. The cells are: \n");

      for (i=0; i < num_cells; i++)
         printf("Cell: %d \n",cell_list[i]);
   }

   ecl_region_free( ecl_region );

*/



#define ECL_REGION_TYPE_ID 1106377

struct ecl_region_struct {
  UTIL_TYPE_ID_DECLARATION;
  bool                * active_mask;          /* This marks active|inactive in the region, which is unrelated to active in the grid. */
  int_vector_type     * global_index_list;    /* This is a list of the cells in the region - irrespective of whether they are active in the grid or not. */
  int_vector_type     * active_index_list;    /* This means cells in the region which are also active in the grid */
  int_vector_type     * global_active_list;   /* This is a list of (maximum) nactive elements, where the values are in the [0,..nx*ny*nz) range. */
  bool                  global_index_list_valid;
  bool                  active_index_list_valid;

  char                * name;                 /* User name attached to region will typically be NULL. */
  bool                  preselect;
  /******************************************************************/
  /* Grid properties */
  int                   grid_nx,grid_ny,grid_nz,grid_vol,grid_active;
  const ecl_grid_type * parent_grid;
};



UTIL_IS_INSTANCE_FUNCTION( ecl_region , ECL_REGION_TYPE_ID)
UTIL_SAFE_CAST_FUNCTION( ecl_region , ECL_REGION_TYPE_ID)


static void ecl_region_invalidate_index_list( ecl_region_type * region ) {
  region->global_index_list_valid  = false;
  region->active_index_list_valid  = false;
}


void ecl_region_lock( ecl_region_type * region ){
  int_vector_set_read_only( region->global_index_list , true );
  int_vector_set_read_only( region->active_index_list , true );
  int_vector_set_read_only( region->global_active_list , true );
}


void ecl_region_unlock( ecl_region_type * region ){
  int_vector_set_read_only( region->global_index_list , false );
  int_vector_set_read_only( region->active_index_list , false );
  int_vector_set_read_only( region->global_active_list , false );
}


ecl_region_type * ecl_region_alloc( const ecl_grid_type * ecl_grid , bool preselect) {
  ecl_region_type * region = (ecl_region_type*)util_malloc( sizeof * region );
  UTIL_TYPE_ID_INIT( region , ECL_REGION_TYPE_ID);
  region->parent_grid = ecl_grid;
  ecl_grid_get_dims( ecl_grid , &region->grid_nx , &region->grid_ny , &region->grid_nz , &region->grid_active);
  region->grid_vol          = region->grid_nx * region->grid_ny * region->grid_nz;
  region->active_mask       = (bool*)util_calloc(region->grid_vol , sizeof * region->active_mask );
  region->active_index_list  = int_vector_alloc(0 , 0);
  region->global_index_list  = int_vector_alloc(0 , 0);
  region->global_active_list = int_vector_alloc(0 , 0);
  region->preselect          = preselect;
  region->name               = NULL;
  ecl_region_reset( region );  /* This MUST be called to ensure that xxx_valid is correctly initialized. */
  return region;
}






ecl_region_type * ecl_region_alloc_copy( const ecl_region_type * ecl_region ) {
  ecl_region_type * new_region = ecl_region_alloc( ecl_region->parent_grid , ecl_region->preselect );
  memcpy( new_region->active_mask , ecl_region->active_mask , ecl_region->grid_vol * sizeof * ecl_region->active_mask );
  ecl_region_invalidate_index_list( new_region );
  return new_region;
}


void ecl_region_free( ecl_region_type * region ) {
  free( region->active_mask );
  int_vector_free( region->active_index_list );
  int_vector_free( region->global_index_list );
  int_vector_free( region->global_active_list );
  free( region->name );
  free( region );
}


void ecl_region_free__( void * __region ) {
  ecl_region_type * region = ecl_region_safe_cast( __region );
  ecl_region_free( region );
}



/*****************************************************************/


static void ecl_region_assert_global_index_list( ecl_region_type * region ) {
  if (!region->global_index_list_valid) {
    int global_index;

    int_vector_reset( region->global_index_list  );
    for (global_index = 0; global_index < region->grid_vol; global_index++)
      if (region->active_mask[ global_index ])
        int_vector_append( region->global_index_list , global_index );

    region->global_index_list_valid = true;
  }
}


static void ecl_region_assert_active_index_list( ecl_region_type * region ) {
  if (!region->active_index_list_valid) {
    int global_index;

    int_vector_reset( region->active_index_list  );
    int_vector_reset( region->global_active_list );
    for (global_index = 0; global_index < region->grid_vol; global_index++) {
      if (region->active_mask[ global_index ]) {
        int active_index = ecl_grid_get_active_index1( region->parent_grid , global_index );
        if (active_index >= 0) {
          int_vector_append( region->active_index_list , active_index );
          int_vector_append( region->global_active_list , global_index );
        }
      }
    }
    region->active_index_list_valid = true;
  }
}


/*****************************************************************/


const int_vector_type * ecl_region_get_active_list( ecl_region_type * region ) {
  ecl_region_assert_active_index_list( region );
  return region->active_index_list;
}


const int_vector_type * ecl_region_get_global_active_list( ecl_region_type * region ) {
  ecl_region_assert_active_index_list( region );
  return region->global_active_list;
}


const int_vector_type * ecl_region_get_global_list( ecl_region_type * region ) {
  ecl_region_assert_global_index_list( region );
  return region->global_index_list;
}

/*****************************************************************/
/* Stupid cpp compat/legacy/cruft functions. */
int ecl_region_get_active_size_cpp(  ecl_region_type * region ) {
  return int_vector_size( ecl_region_get_active_list( region ));
}

int ecl_region_get_global_size_cpp( ecl_region_type * region ) {
  return int_vector_size( ecl_region_get_global_list( region ));
}

const int * ecl_region_get_active_list_cpp( ecl_region_type * region ) {
  return int_vector_get_const_ptr( ecl_region_get_active_list( region ));
}

const int * ecl_region_get_global_list_cpp(  ecl_region_type * region ) {
  return int_vector_get_const_ptr( ecl_region_get_global_list( region ));
}



/*****************************************************************/

static void ecl_region_assert_kw( const ecl_region_type * region , const ecl_kw_type * ecl_kw , bool * global_kw) {
  int kw_size = ecl_kw_get_size( ecl_kw );
  if (!(kw_size == region->grid_vol || kw_size == region->grid_active))
    util_abort("%s: size mismatch between ecl_kw instance and region->grid \n",__func__);
  if (kw_size == region->grid_vol)
    *global_kw = true;
  else
    *global_kw = false;

}


/*****************************************************************/

void ecl_region_reset( ecl_region_type * ecl_region ) {
  int i;
  for (i=0; i < ecl_region->grid_vol; i++)
    ecl_region->active_mask[i] = ecl_region->preselect;
  ecl_region_invalidate_index_list( ecl_region );
}


/*****************************************************************/

static void ecl_region_select_cell__( ecl_region_type * region , int i , int j , int k, bool select) {
  int global_index = ecl_grid_get_global_index3( region->parent_grid , i,j,k);
  region->active_mask[global_index] = select;
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_cell( ecl_region_type * region , int i , int j , int k) {
  ecl_region_select_cell__( region , i,j,k, true);
}

void ecl_region_deselect_cell( ecl_region_type * region , int i , int j , int k) {
  ecl_region_select_cell__( region , i,j,k, false);
}

/*****************************************************************/


static void ecl_region_select_equal__( ecl_region_type * region , const ecl_kw_type * ecl_kw, int value , bool select) {
  bool global_kw;
  ecl_region_assert_kw( region , ecl_kw , &global_kw);
  if (!ecl_type_is_int(ecl_kw_get_data_type( ecl_kw )))
    util_abort("%s: sorry - select by equality is only supported for integer keywords \n",__func__);
  {
    const int * kw_data = ecl_kw_get_int_ptr( ecl_kw );
    if (global_kw) {
      int global_index;
      for (global_index = 0; global_index < region->grid_vol; global_index++) {
        if (kw_data[ global_index ] == value)
          region->active_mask[ global_index ] = select;
      }
    } else {
      int active_index;
      for (active_index = 0; active_index < region->grid_active; active_index++) {
        if (kw_data[active_index] == value) {
          int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
          region->active_mask[ global_index ] = select;
        }
      }
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_equal( ecl_region_type * region , const ecl_kw_type * ecl_kw, int value ) {
  ecl_region_select_equal__( region , ecl_kw , value , true );
}


void ecl_region_deselect_equal( ecl_region_type * region , const ecl_kw_type * ecl_kw, int value ) {
  ecl_region_select_equal__( region , ecl_kw , value , false );
}

/*****************************************************************/

static void ecl_region_select_bool_equal__( ecl_region_type * region , const ecl_kw_type * ecl_kw, bool value , bool select) {
  bool global_kw;
  ecl_region_assert_kw( region , ecl_kw , &global_kw);
  if (!ecl_type_is_bool(ecl_kw_get_data_type( ecl_kw )))
    util_abort("%s: sorry - select by equality is only supported for boolean keywords \n",__func__);
  {
    if (global_kw) {
      int global_index;
      for (global_index = 0; global_index < region->grid_vol; global_index++) {
        if (ecl_kw_iget_bool(ecl_kw , global_index) == value)
          region->active_mask[ global_index ] = select;
      }
    } else {
      int active_index;
      for (active_index = 0; active_index < region->grid_active; active_index++) {
        if (ecl_kw_iget_bool(ecl_kw , active_index) == value) {
          int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
          region->active_mask[ global_index ] = select;
        }
      }
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_true( ecl_region_type * region , const ecl_kw_type * ecl_kw) {
  ecl_region_select_bool_equal__( region , ecl_kw , true , true );
}


void ecl_region_deselect_true( ecl_region_type * region , const ecl_kw_type * ecl_kw) {
  ecl_region_select_bool_equal__( region , ecl_kw , true , false );
}


void ecl_region_select_false( ecl_region_type * region , const ecl_kw_type * ecl_kw) {
  ecl_region_select_bool_equal__( region , ecl_kw , false , true );
}


void ecl_region_deselect_false( ecl_region_type * region , const ecl_kw_type * ecl_kw) {
  ecl_region_select_bool_equal__( region , ecl_kw , false , false );
}


/*****************************************************************/

static void ecl_region_select_in_interval__( ecl_region_type * region , const ecl_kw_type * ecl_kw, float min_value , float max_value , bool select) {
  bool global_kw;
  ecl_region_assert_kw( region , ecl_kw , &global_kw);
  if (!ecl_type_is_float(ecl_kw_get_data_type( ecl_kw )))
    util_abort("%s: sorry - select by in_interval is only supported for float keywords \n",__func__);
  {
    const float * kw_data = ecl_kw_get_float_ptr( ecl_kw );
    if (global_kw) {
      int global_index;
      for (global_index = 0; global_index < region->grid_vol; global_index++) {
        if (kw_data[ global_index ] >= min_value && kw_data[ global_index ] < max_value)
          region->active_mask[ global_index ] = select;
      }
    } else {
      int active_index;
      for (active_index = 0; active_index < region->grid_active; active_index++) {
        if (kw_data[ active_index ] >= min_value && kw_data[ active_index ] < max_value) {
          int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
          region->active_mask[ global_index ] = select;
        }
      }
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_in_interval( ecl_region_type * region , const ecl_kw_type * ecl_kw, float min_value , float max_value) {
  ecl_region_select_in_interval__( region , ecl_kw , min_value , max_value , true );
}


void ecl_region_deselect_in_interval( ecl_region_type * region , const ecl_kw_type * ecl_kw, float min_value , float max_value) {
  ecl_region_select_in_interval__( region , ecl_kw , min_value , max_value , false );
}

/*****************************************************************/

/**
   This is waaaay to fucking large:

   Float / Int / double *  Active / Global  *  More / Less ==> In total 12 code blocks written out ...
*/


/*
  NBNBNBNB: Select >= on float values and select > on integer!!!!!!
*/
static void ecl_region_select_with_limit__( ecl_region_type * region , const ecl_kw_type * ecl_kw, float limit , bool select_less , bool select) {
  bool global_kw;
  ecl_data_type data_type = ecl_kw_get_data_type( ecl_kw );
  ecl_region_assert_kw( region , ecl_kw , &global_kw);
  if (!ecl_type_is_numeric(data_type))
    util_abort("%s: sorry - select by in_interval is only supported for float and integer keywords \n",__func__);

  {
    if (ecl_type_is_float(data_type)) {
      const float * kw_data = ecl_kw_get_float_ptr( ecl_kw );
      float float_limit = limit;
      if (global_kw) {
        int global_index;
        for (global_index = 0; global_index < region->grid_vol; global_index++) {
          if (select_less) {
            if (kw_data[ global_index ] < float_limit)
              region->active_mask[ global_index ] = select;
          } else {
            if (kw_data[ global_index ] >= float_limit)
              region->active_mask[ global_index ] = select;
          }
        }
      } else {
        int active_index;
        for (active_index = 0; active_index < region->grid_active; active_index++) {
          if (select_less) {
            if (kw_data[ active_index ] < float_limit) {
              int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
              region->active_mask[ global_index ] = select;
            }
          } else {
            if (kw_data[ active_index ] >= float_limit) {
              int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
              region->active_mask[ global_index ] = select;
            }
          }
        }
      }
    } else if (ecl_type_is_int(data_type)) {
      const int * kw_data = ecl_kw_get_int_ptr( ecl_kw );
      int int_limit = (int) limit;
      if (global_kw) {
        int global_index;
        for (global_index = 0; global_index < region->grid_vol; global_index++) {
          if (select_less) {
            if (kw_data[ global_index ] < int_limit)
              region->active_mask[ global_index ] = select;
          } else {
            if (kw_data[ global_index ] > int_limit)
              region->active_mask[ global_index ] = select;
          }
        }
      } else {
        int active_index;
        for (active_index = 0; active_index < region->grid_active; active_index++) {
          if (select_less) {
            if (kw_data[ active_index ] < int_limit) {
              int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
              region->active_mask[ global_index ] = select;
            }
          } else {
            if (kw_data[ active_index ] > int_limit) {
              int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
              region->active_mask[ global_index ] = select;
            }
          }
        }
      }
    } else if (ecl_type_is_double(data_type)) {
      const double * kw_data = ecl_kw_get_double_ptr( ecl_kw );
      double double_limit = (double) limit;
      if (global_kw) {
        int global_index;
        for (global_index = 0; global_index < region->grid_vol; global_index++) {
          if (select_less) {
            if (kw_data[ global_index ] < double_limit)
              region->active_mask[ global_index ] = select;
          } else {
            if (kw_data[ global_index ] >= double_limit)
              region->active_mask[ global_index ] = select;
          }
        }
      } else {
        int active_index;
        for (active_index = 0; active_index < region->grid_active; active_index++) {
          if (select_less) {
            if (kw_data[ active_index ] < double_limit) {
              int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
              region->active_mask[ global_index ] = select;
            }
          } else {
            if (kw_data[ active_index ] >= double_limit) {
              int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
              region->active_mask[ global_index ] = select;
            }
          }
        }
      }
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_smaller( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , float limit) {
  ecl_region_select_with_limit__( ecl_region , ecl_kw , limit , true , true );
}

void ecl_region_deselect_smaller( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , float limit) {
  ecl_region_select_with_limit__( ecl_region , ecl_kw , limit , true , false );
}

void ecl_region_select_larger( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , float limit) {
  ecl_region_select_with_limit__( ecl_region , ecl_kw , limit , false , true );
}

void ecl_region_deselect_larger( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , float limit) {
  ecl_region_select_with_limit__( ecl_region , ecl_kw , limit , false , false );
}

/*****************************************************************/

/**
    Selection based on comparing two keywords.
*/

static void ecl_region_cmp_select__( ecl_region_type * region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2 , bool select_less , bool select) {
  bool global_kw;
  ecl_region_assert_kw( region , kw1 , &global_kw);
  if (!ecl_type_is_float(ecl_kw_get_data_type( kw1 )))
    util_abort("%s: sorry - select by cmp() is only supported for float keywords \n",__func__);
  {
    if (ecl_kw_size_and_type_equal(kw1, kw2)) {

      const float * kw1_data = ecl_kw_get_float_ptr( kw1 );
      const float * kw2_data = ecl_kw_get_float_ptr( kw2 );

      if (global_kw) {
        int global_index;
        for (global_index = 0; global_index < region->grid_vol; global_index++) {
          if (select_less) {
            if (kw1_data[ global_index ] < kw2_data[ global_index ])
              region->active_mask[ global_index ] = select;
          } else {
            if (kw1_data[ global_index ] >= kw2_data[ global_index ] )
              region->active_mask[ global_index ] = select;
          }
        }
      } else {
        int active_index;
        for (active_index = 0; active_index < region->grid_active; active_index++) {
          if (select_less) {
            if (kw1_data[ active_index ] < kw2_data[ active_index] ) {
              int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
              region->active_mask[ global_index ] = select;
            }
          } else {
            if (kw1_data[ active_index ] >= kw2_data[ active_index ]) {
              int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index );
              region->active_mask[ global_index ] = select;
            }
          }
        }
      }
    } else
      util_abort("%s: type/size mismatch between keywords. \n",__func__);
  }
  ecl_region_invalidate_index_list( region );
}

void ecl_region_cmp_select_less( ecl_region_type * ecl_region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2) {
  ecl_region_cmp_select__( ecl_region , kw1 , kw2 , true , true );
}

void ecl_region_cmp_deselect_less( ecl_region_type * ecl_region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2) {
  ecl_region_cmp_select__( ecl_region , kw1 , kw2 , true , false);
}

void ecl_region_cmp_select_more( ecl_region_type * ecl_region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2) {
  ecl_region_cmp_select__( ecl_region , kw1 , kw2 , false , true );
}

void ecl_region_cmp_deselect_more( ecl_region_type * ecl_region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2) {
  ecl_region_cmp_select__( ecl_region , kw1 , kw2 , false , false );
}


/*****************************************************************/

/**
   Will select all the cells in the box. Remember that the box is
   defined as an inclusive geometry. Alternatively the functions
   xxx_ijkbox() can be used, these functions take i1,i2,j1,j2,k1,k2 as
   input and create a temporary box object.
*/

static void ecl_region_select_from_box__( ecl_region_type * region , const ecl::ecl_box& ecl_box , bool select) {
  for (auto global_index : ecl_box.active_list())
    region->active_mask[ global_index ] = select;

  ecl_region_invalidate_index_list( region );
}


/*****************************************************************/

/**
   Observe that:

     1. All the indices are inclusive.
     2. All the indices have zero offset.

   Only a thin wrapper around the ecl_region_select_from_box() function.
*/

static void ecl_region_select_from_ijkbox__( ecl_region_type * region , int i1 , int i2 , int j1 , int j2 , int k1 , int k2 , bool select) {
  ecl::ecl_box tmp_box(region->parent_grid, i1, i2, j1, j2, k1, k2);
  ecl_region_select_from_box__( region , tmp_box , select );
}


void ecl_region_select_from_ijkbox( ecl_region_type * region , int i1 , int i2 , int j1 , int j2 , int k1 , int k2) {
  ecl_region_select_from_ijkbox__( region , i1 , i2 , j1 , j2 , k1 , k2 , true );
}


void ecl_region_deselect_from_ijkbox( ecl_region_type * region , int i1 , int i2 , int j1 , int j2 , int k1 , int k2) {
  ecl_region_select_from_ijkbox__( region , i1 , i2 , j1 , j2 , k1 , k2 , false );
}



/*****************************************************************/

/**
   Observe that i1 and i2 are:

     * ZERO offset.
     * An inclusive interval : [i1,i2]

   Input values below zero or above the upper limit are truncated.
*/

static void ecl_region_select_i1i2__( ecl_region_type * region , int i1 , int i2 , bool select) {
  if (i1 > i2)
    util_abort("%s: i1 > i2 - this is illogical ... \n",__func__);
  i1 = util_int_max(0 , i1);
  i2 = util_int_min(region->grid_nx - 1 , i2);
  {
    int i,j,k;
    for (k = 0; k < region->grid_nz; k++)
      for (j = 0; j < region->grid_ny; j++)
        for (i = i1; i <= i2; i++) {
          int global_index = ecl_grid_get_global_index3( region->parent_grid , i,j,k);
          region->active_mask[global_index] = select;
        }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_i1i2( ecl_region_type * region , int i1 , int i2) {
  ecl_region_select_i1i2__( region , i1 , i2 , true );
}


void ecl_region_deselect_i1i2( ecl_region_type * region , int i1 , int i2) {
  ecl_region_select_i1i2__( region , i1 , i2 , false );
}


/*****************************************************************/

/**
   Observe that j1 and j2 are:

     * ZERO offset.
     * An inclusive interval : [i1,i2]

   Input values below zero or above the upper limit are truncated.
*/

static void ecl_region_select_j1j2__( ecl_region_type * region , int j1 , int j2 , bool select) {
  if (j1 > j2)
    util_abort("%s: i1 > i2 - this is illogical ... \n",__func__);

  j1 = util_int_max(0 , j1);
  j2 = util_int_min(region->grid_ny - 1 , j2);
  {
    int i,j,k;
    for (k = 0; k < region->grid_nz; k++)
      for ( j = j1; j <= j2; j++)
        for ( i = 0; i < region->grid_nx; i++) {
          int global_index = ecl_grid_get_global_index3( region->parent_grid , i,j,k);
          region->active_mask[global_index] = select;
        }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_j1j2( ecl_region_type * region , int j1 , int j2) {
  ecl_region_select_j1j2__( region , j1 , j2 , true );
}


void ecl_region_deselect_j1j2( ecl_region_type * region , int j1 , int j2) {
  ecl_region_select_j1j2__( region , j1 , j2 , false );
}


/*****************************************************************/

/**
   Observe that k1 and k2 are:

     * ZERO offset.
     * An inclusive interval : [i1,i2]

   Input values below zero or above the upper limit are truncated.
*/

static void ecl_region_select_k1k2__( ecl_region_type * region , int k1 , int k2 , bool select) {
  if (k1 > k2)
    util_abort("%s: i1 > i2 - this is illogical ... \n",__func__);
  k1 = util_int_max(0 , k1);
  k2 = util_int_min(region->grid_nz - 1 , k2);
  {
    int i,j,k;
    for (k = k1; k <= k2; k++)
      for (j = 0; j < region->grid_ny; j++)
        for (i = 0; i < region->grid_nx; i++) {
          int global_index = ecl_grid_get_global_index3( region->parent_grid , i,j,k);
          region->active_mask[global_index] = select;
        }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_k1k2( ecl_region_type * region , int k1 , int k2) {
  ecl_region_select_k1k2__( region , k1 , k2 , true );
}


void ecl_region_deselect_k1k2( ecl_region_type * region , int k1 , int k2) {
  ecl_region_select_k1k2__( region , k1 , k2 , false );
}

/*****************************************************************/

/**
   This function will select all the cells with depth below the input
   parameter @depth (if @select_below == true). The depth of a cell is
   determined by the depth of the center of a cell.
*/


static void ecl_region_select_from_depth__( ecl_region_type * region , double depth_limit , bool select_deep  , bool select) {
  int global_index;
  for (global_index = 0; global_index < region->grid_vol; global_index++) {
    double cell_depth = ecl_grid_get_cdepth1( region->parent_grid , global_index );
    if (select_deep) {
      // The select/deselect mechanism should be applied to deep cells.
      if (cell_depth >= depth_limit)
        region->active_mask[ global_index ] = select;
    } else {
      // The select/deselect mechanism should be applied to shallow cells.
      if (cell_depth <= depth_limit)
        region->active_mask[ global_index ] = select;
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_shallow_cells( ecl_region_type * region , double depth_limit ) {
  ecl_region_select_from_depth__( region , depth_limit , false , true );
}


void ecl_region_deselect_shallow_cells( ecl_region_type * region , double depth_limit ) {
  ecl_region_select_from_depth__( region , depth_limit , false , false );
}

void ecl_region_select_deep_cells( ecl_region_type * region , double depth_limit ) {
  ecl_region_select_from_depth__( region , depth_limit , true , true );
}


void ecl_region_deselect_deep_cells( ecl_region_type * region , double depth_limit ) {
  ecl_region_select_from_depth__( region , depth_limit , true , false );
}

/*****************************************************************/

static void ecl_region_select_from_volume__( ecl_region_type * region , double volum_limit , bool select_small , bool select) {
  int global_index;
  for (global_index = 0; global_index < region->grid_vol; global_index++) {
    double cell_size = ecl_grid_get_cell_volume1( region->parent_grid , global_index );
    if (select_small) {
      // The select/deselect mechanism should be applied to small cells.
      if (cell_size <= volum_limit)
        region->active_mask[ global_index ] = select;
    } else {
      // The select/deselect mechanism should be applied to large cells.
      if (cell_size >= volum_limit)
        region->active_mask[ global_index ] = select;
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_small_cells( ecl_region_type * ecl_region , double volum_limit ) {
  ecl_region_select_from_volume__( ecl_region , volum_limit , true , true );
}

void ecl_region_deselect_small_cells( ecl_region_type * ecl_region , double volum_limit ) {
  ecl_region_select_from_volume__( ecl_region , volum_limit , true , false );
}

void ecl_region_select_large_cells( ecl_region_type * ecl_region , double volum_limit ) {
  ecl_region_select_from_volume__( ecl_region , volum_limit , false , true );
}

void ecl_region_deselect_large_cells( ecl_region_type * ecl_region , double volum_limit ) {
  ecl_region_select_from_volume__( ecl_region , volum_limit , false , false );
}

/*****************************************************************/

static void ecl_region_select_from_dz__( ecl_region_type * region , double dz_limit , bool select_thin , bool select) {
  int global_index;
  for (global_index = 0; global_index < region->grid_vol; global_index++) {
    double cell_dz = ecl_grid_get_cell_thickness1( region->parent_grid , global_index );
    if (select_thin) {
      // The select/deselect mechanism should be applied to thin cells.
      if (cell_dz <= dz_limit)
        region->active_mask[ global_index ] = select;
    } else {
      // The select/deselect mechanism should be applied to thick cells.
      if (cell_dz >= dz_limit)
        region->active_mask[ global_index ] = select;
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_thin_cells( ecl_region_type * ecl_region , double dz_limit ) {
  ecl_region_select_from_dz__( ecl_region , dz_limit , true , true );
}

void ecl_region_deselect_thin_cells( ecl_region_type * ecl_region , double dz_limit ) {
  ecl_region_select_from_dz__( ecl_region , dz_limit , true , false );
}

void ecl_region_select_thick_cells( ecl_region_type * ecl_region , double dz_limit ) {
  ecl_region_select_from_dz__( ecl_region , dz_limit , false , true );
}

void ecl_region_deselect_thick_cells( ecl_region_type * ecl_region , double dz_limit ) {
  ecl_region_select_from_dz__( ecl_region , dz_limit , false , false );
}
/*****************************************************************/
static void ecl_region_select_active_cells__( ecl_region_type * ecl_region , bool select_active , bool select) {
  int global_index;
  for (global_index = 0; global_index < ecl_region->grid_vol; global_index++) {
    if (select_active) {
      if (ecl_grid_get_active_index1( ecl_region->parent_grid , global_index) >= 0)
        ecl_region->active_mask[ global_index ] = select;
    } else {
      if (ecl_grid_get_active_index1( ecl_region->parent_grid , global_index) < 0)
        ecl_region->active_mask[ global_index ] = select;
    }
  }
  ecl_region_invalidate_index_list( ecl_region );
}


void ecl_region_select_active_cells( ecl_region_type * region ) {
  ecl_region_select_active_cells__( region , true , true );
}

void ecl_region_deselect_active_cells( ecl_region_type * region ) {
  ecl_region_select_active_cells__( region , true , false );
}

void ecl_region_select_inactive_cells( ecl_region_type * region ) {
  ecl_region_select_active_cells__( region , false , true );
}

void ecl_region_deselect_inactive_cells( ecl_region_type * region ) {
  ecl_region_select_active_cells__( region , false , false );
}

/*****************************************************************/
/**
   This function will select a cell based on global_index.
*/

static void ecl_region_select_global_index__( ecl_region_type * region , int global_index , bool select) {
  if ((global_index >= 0) && (global_index < region->grid_vol))
    region->active_mask[ global_index ] = select;
  else
    util_abort("%s: global_index:%d invalid - legal interval: [0,%d) \n",__func__ , global_index , region->grid_vol);
  ecl_region_invalidate_index_list( region );
}

void ecl_region_select_global_index( ecl_region_type * region , int global_index) {
  ecl_region_select_global_index__( region , global_index , true );
}

void ecl_region_deselect_global_index( ecl_region_type * region , int global_index) {
  ecl_region_select_global_index__( region , global_index , false );
}

/*****************************************************************/

/**
   Here comes functions for selecting all the cells which are in the
   vertical cylinder located at (x0,y0) with radius R. The functions
   with name 'zcylinder' operate on a finite cylinder in the z
   direction, specified with the additional arguments z1 and z2; the
   functions without z1 and z2 arguments operate on an infinite
   cylinder piercing the complete reservoir.

   Currently all user-exported functions call the
   ecl_region_clyinder_select__() with select_inside == true.
*/

static void ecl_region_cylinder_select__( ecl_region_type * region , double x0 , double y0, double R , double z1 , double z2 , bool select_inside , bool select) {
  double R2 = R*R;

  if (z1 < z2) {
    int global_index;
    for (global_index = 0; global_index < region->grid_vol; global_index++) {
      double x,y,z;
      ecl_grid_get_xyz1( region->parent_grid , global_index , &x , &y , &z);
      if ((z >= z1) && (z <= z2)) {
        double pointR2 = (x - x0) * (x - x0) + (y - y0) * (y - y0);
        if ((pointR2 < R2) && (select_inside))
          region->active_mask[ global_index ] = select;
        else if ((pointR2 > R2) && (!select_inside))
          region->active_mask[ global_index ] = select;
      }
    }
  } else {
    const int nx = region->grid_nx;
    const int ny = region->grid_ny;
    const int nz = region->grid_nz;
    int i,j;
    for (i=0; i < nx; i++) {
      for (j=0; j < ny; j++) {
        double x,y,z;
        ecl_grid_get_xyz3( region->parent_grid , i,j,0 , &x , &y , &z);
        {
          double pointR2 = (x - x0) * (x - x0) + (y - y0) * (y - y0);
          bool select_column = false;

          if ((pointR2 < R2) && (select_inside))
            select_column = true;
          else if ((pointR2 > R2) && (!select_inside))
            select_column = true;

          if (select_column) {
            int k;
            for (k=0; k < nz; k++) {
              int global_index = ecl_grid_get_global_index3( region->parent_grid , i,j,k);
              region->active_mask[ global_index ] = select;
            }
          }
        }
      }
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_in_cylinder( ecl_region_type * region , double x0 , double y0, double R) {
  ecl_region_cylinder_select__( region , x0 , y0 , R , 1 , 0 , true ,  true );
}


void ecl_region_deselect_in_cylinder( ecl_region_type * region , double x0 , double y0, double R) {
  ecl_region_cylinder_select__( region , x0 , y0 , R , 1 , 0 , true ,  false );
}


void ecl_region_select_in_zcylinder( ecl_region_type * region , double x0 , double y0, double R , double z1 , double z2) {
  ecl_region_cylinder_select__( region , x0 , y0 , R , util_double_min(z1 , z2) , util_double_max(z1,z2) , true , true);
}


void ecl_region_deselect_in_zcylinder( ecl_region_type * region , double x0 , double y0, double R, double z1 , double z2) {
  ecl_region_cylinder_select__( region , x0 , y0 , R , util_double_min(z1 , z2) , util_double_max(z1,z2) , true , false);
}

/*****************************************************************/


/**
   Select or deselect points based on their distance to the plane
   specified by normal vector @n and point @p.
*/

static void ecl_region_plane_select__( ecl_region_type * region, const double n[3] , const double p[3], bool select_above , bool select){
  const double a = n[0];
  const double b = n[1];
  const double c = -n[2];
  const double d = -a*p[0] - b*p[1] - c*p[2];
  /**
     Plane: ax + by + cz + d = 0
  */
  {
    int global_index;
    for (global_index = 0; global_index < region->grid_vol; global_index++) {
      double x,y,z;
      double D;
      ecl_grid_get_xyz1( region->parent_grid , global_index , &x , &y , &z);
      D = a*x + b*y + c*z + d;
      if ((D >= 0) && (select_above))
        region->active_mask[ global_index ] = select;
      else if ((D < 0) && (!select_above))
        region->active_mask[ global_index ] = select;
    }
  }
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_above_plane( ecl_region_type * region, const double n[3] , const double p[3]) {
  ecl_region_plane_select__( region , n , p , true , true );
}

void ecl_region_select_below_plane( ecl_region_type * region, const double n[3] , const double p[3]) {
  ecl_region_plane_select__( region , n , p , false , true );
}

void ecl_region_deselect_above_plane( ecl_region_type * region, const double n[3] , const double p[3]) {
  ecl_region_plane_select__( region , n , p , true , false );
}

void ecl_region_deselect_below_plane( ecl_region_type * region, const double n[3] , const double p[3]) {
  ecl_region_plane_select__( region , n , p , false , false );
}

/*****************************************************************/



/**
   When it comes to depth the polygon select function works as
   follows:

     1. The defining polygon is layed out at the top of the reservoir.

     2. The set {(i,j,0)} of cells in the top layer inside the polygon
        is selected by checking the polygon perimeter.

     3. The set {(i,j,0)} is extended to all k vertically. This implies that:

        * If the pillars of the grid are not vertical the selection
          polygon will effectively qbe translated as a function of
          depth.

        * If the pillars are not parallell the selection polygon will
          effectively change in size.
*/


static void ecl_region_polygon_select__( ecl_region_type * region ,
                                         const geo_polygon_type * polygon ,
                                         bool select_inside , bool select) {

  const int define_k = 0;                  // The k-level where the polygon is checked.
  const int k1       = 0;                  // Selection range in k
  const int k2       = region->grid_nz;

  {
    int i,j;
    for (i=0; i < region->grid_nx; i++) {
      for (j=0; j < region->grid_ny; j++) {
        double x,y,z;
        bool inside;
        int global_index = ecl_grid_get_global_index3( region->parent_grid , i , j , define_k);

        ecl_grid_get_xyz1( region->parent_grid , global_index , &x , &y , &z);
        inside = geo_polygon_contains_point( polygon , x , y );

        if (select_inside == inside) {
          int k;
          for (k=k1; k < k2; k++) {
            global_index = ecl_grid_get_global_index3( region->parent_grid , i , j , k);
            region->active_mask[ global_index ] = select;
          }
        }
      }
    }
  }
}

void ecl_region_select_inside_polygon( ecl_region_type * region , const geo_polygon_type * polygon) {
  ecl_region_polygon_select__( region , polygon , true , true );
}

void ecl_region_deselect_inside_polygon( ecl_region_type * region , const geo_polygon_type * polygon) {
  ecl_region_polygon_select__( region , polygon , true , false );
}

void ecl_region_select_outside_polygon( ecl_region_type * region , const geo_polygon_type * polygon) {
  ecl_region_polygon_select__( region , polygon , false , true );
}

void ecl_region_deselect_outside_polygon( ecl_region_type * region , const geo_polygon_type * polygon) {
  ecl_region_polygon_select__( region , polygon , false , false );
}

/*****************************************************************/
/**
   This function will select a cell based on active_index.
*/

static void ecl_region_select_active_index__( ecl_region_type * region , int active_index , bool select) {
  if ((active_index >= 0) && (active_index < region->grid_active)) {
    int global_index = ecl_grid_get_global_index1A( region->parent_grid , active_index);
    region->active_mask[ global_index ] = select;
  } else
    util_abort("%s: active_index:%d invalid - legal interval: [0,%d) \n",__func__ , active_index , region->grid_vol);
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_active_index( ecl_region_type * region , int active_index) {
  ecl_region_select_active_index__( region , active_index , true );
}

void ecl_region_deselect_active_index( ecl_region_type * region , int active_index) {
  ecl_region_select_active_index__( region , active_index , false );
}
/*****************************************************************/

static void ecl_region_select_from_layer__( ecl_region_type * region , const layer_type * layer , int k , int layer_value, bool select) {
  int_vector_type * i_list = int_vector_alloc(0,0);
  int_vector_type * j_list = int_vector_alloc(0,0);

  layer_cells_equal(layer, layer_value, i_list , j_list);
  {
    const int * i = int_vector_get_ptr( i_list );
    const int * j = int_vector_get_ptr( j_list );

    int index;
    for (index = 0; index < int_vector_size( i_list ); index++) {
      int global_index = ecl_grid_get_global_index3( region->parent_grid , i[index] , j[index] , k);
      region->active_mask[ global_index ] = select;
    }

  }
  if (int_vector_size( i_list ) > 0)
    ecl_region_invalidate_index_list( region );

  int_vector_free( i_list );
  int_vector_free( j_list );
}


void ecl_region_select_from_layer( ecl_region_type * region , const layer_type * layer , int k , int layer_value) {
  ecl_region_select_from_layer__( region , layer , k , layer_value , true );
}

void ecl_region_deselect_from_layer( ecl_region_type * region , const layer_type * layer , int k , int layer_value) {
  ecl_region_select_from_layer__( region , layer , k , layer_value , false );
}

/*****************************************************************/

static void ecl_region_select_all__( ecl_region_type * region , bool select) {
  int global_index;
  for (global_index = 0; global_index < region->grid_vol; global_index++)
    region->active_mask[ global_index ] = select;
  ecl_region_invalidate_index_list( region );
}


void ecl_region_select_all( ecl_region_type * region) {
  ecl_region_select_all__( region , true );
}

void ecl_region_deselect_all( ecl_region_type * region ) {
  ecl_region_select_all__( region , false );
}


/*****************************************************************/

void ecl_region_invert_selection( ecl_region_type * region ) {
  int global_index;
  for (global_index = 0; global_index < region->grid_vol; global_index++)
    region->active_mask[ global_index ] = !region->active_mask[ global_index ];
  ecl_region_invalidate_index_list( region );
}


/**

   Returns true if the region has selected grid cell ijk.

   ijk have zero offset.
*/

bool ecl_region_contains_ijk( const ecl_region_type * ecl_region , int i , int j , int k) {
  int global_index = ecl_grid_get_global_index3( ecl_region->parent_grid , i , j , k );
  return ecl_region->active_mask[ global_index ];
}


bool ecl_region_contains_global( const ecl_region_type * ecl_region , int global_index) {
  return ecl_region->active_mask[ global_index ];
}


bool ecl_region_contains_active( const ecl_region_type * ecl_region , int active_index) {
  int global_index = ecl_grid_get_global_index1A( ecl_region->parent_grid , active_index );
  return ecl_region->active_mask[ global_index ];
}



/*****************************************************************/


/**
   Will update the selection in @region to ONLY contain the elements
   which are also present in @new_region. Will FAIL hard if the two
   regions do not share the same grid instance (checked by pointer
   equality).

   A &= B
*/

void ecl_region_intersection( ecl_region_type * region , const ecl_region_type * new_region ) {
  if (region->parent_grid == new_region->parent_grid) {
    int global_index;
    for (global_index = 0; global_index < region->grid_vol; global_index++)
      region->active_mask[global_index] = (region->active_mask[global_index] && new_region->active_mask[global_index]);

    ecl_region_invalidate_index_list( region );
  } else
    util_abort("%s: The two regions do not share grid - aborting \n",__func__);
}


/**
   Will update the selection in @region to contain all the elements
   which are selected in either @region or @new_region.

   A |= B
*/
void ecl_region_union( ecl_region_type * region , const ecl_region_type * new_region ) {
  if (region->parent_grid == new_region->parent_grid) {
    int global_index;
    for (global_index = 0; global_index < region->grid_vol; global_index++)
      region->active_mask[global_index] = (region->active_mask[global_index] || new_region->active_mask[global_index]);

    ecl_region_invalidate_index_list( region );
  } else
    util_abort("%s: The two regions do not share grid - aborting \n",__func__);
}


/**
   Will update the selection in @region to deselect the elements which
   are also in new_region:

   A &= !B
*/
void ecl_region_subtract( ecl_region_type * region , const ecl_region_type * new_region) {
  if (region->parent_grid == new_region->parent_grid) {
    int global_index;
    for (global_index = 0; global_index < region->grid_vol; global_index++)
      region->active_mask[global_index] &= !new_region->active_mask[global_index];

    ecl_region_invalidate_index_list( region );
  } else
    util_abort("%s: The two regions do not share grid - aborting \n",__func__);
}


/**
   Will update the selection in @region to seselect the elements which
   are either in region or new_region:

   A ^= B
*/
void ecl_region_xor( ecl_region_type * region , const ecl_region_type * new_region) {
  if (region->parent_grid == new_region->parent_grid) {
    int global_index;
    for (global_index = 0; global_index < region->grid_vol; global_index++)
      region->active_mask[global_index] ^= !new_region->active_mask[global_index];

    ecl_region_invalidate_index_list( region );
  } else
    util_abort("%s: The two regions do not share grid - aborting \n",__func__);
}



/*****************************************************************/

const int_vector_type * ecl_region_get_kw_index_list( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , bool force_active) {
  const int_vector_type * index_set = NULL;
  int kw_size     = ecl_kw_get_size( ecl_kw );
  int grid_active = ecl_grid_get_active_size( ecl_region->parent_grid );
  int grid_global = ecl_grid_get_global_size( ecl_region->parent_grid );

  if (kw_size == grid_active)
    index_set = ecl_region_get_active_list( ecl_region );
  else if (kw_size == grid_global) {
    if (force_active)
      index_set = ecl_region_get_global_active_list( ecl_region );
    else
      index_set = ecl_region_get_global_list( ecl_region );
  } else
    util_abort("%s: size mismatch: grid_active:%d   grid_global:%d  kw_size:%d \n",__func__ , grid_active , grid_global , kw_size);

  return index_set;
}



void ecl_region_set_kw_int( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , int value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_set_indexed_int( ecl_kw , index_set , value );
}


void ecl_region_set_kw_float( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , float value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_set_indexed_float( ecl_kw , index_set , value );
}


void ecl_region_set_kw_double( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , double value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_set_indexed_double( ecl_kw , index_set , value );
}


/*****************************************************************/

void ecl_region_shift_kw_int( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , int value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_shift_indexed_int( ecl_kw , index_set , value );
}


void ecl_region_shift_kw_float( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , float value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_shift_indexed_float( ecl_kw , index_set , value );
}


void ecl_region_shift_kw_double( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , double value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_shift_indexed_double( ecl_kw , index_set , value );
}


/*****************************************************************/

void ecl_region_scale_kw_int( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , int value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_scale_indexed_int( ecl_kw , index_set , value );
}


void ecl_region_scale_kw_float( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , float value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_scale_indexed_float( ecl_kw , index_set , value );
}


void ecl_region_scale_kw_double( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , double value , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_scale_indexed_double( ecl_kw , index_set , value );
}


/*****************************************************************/

void ecl_region_kw_iadd( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * delta_kw , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_inplace_add_indexed( ecl_kw , index_set , delta_kw );
}

void ecl_region_kw_idiv( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * div_kw , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_inplace_div_indexed( ecl_kw , index_set , div_kw );
}


void ecl_region_kw_imul( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * mul_kw , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_inplace_mul_indexed( ecl_kw , index_set , mul_kw );
}


void ecl_region_kw_isub( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * delta_kw , bool force_active) {
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_inplace_sub_indexed( ecl_kw , index_set , delta_kw );
}


void ecl_region_kw_copy( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * src_kw , bool force_active) {
  const int_vector_type * target_index = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_copy_indexed( ecl_kw , target_index , src_kw );
}


/*****************************************************************/

void ecl_region_set_name( ecl_region_type * region , const char * name ) {
  region->name = util_realloc_string_copy( region->name , name );
}


const char * ecl_region_get_name( const ecl_region_type * region ) {
  return region->name;
}


bool ecl_region_equal( const ecl_region_type * region1 , const ecl_region_type * region2) {
  if (region1->parent_grid == region2->parent_grid) {  // Must be exactly the same grid instance to compare as equal.
    if (memcmp(region1->active_mask , region2->active_mask , region1->grid_vol * sizeof * region1->active_mask ) == 0)
      return true;
    else
      return false;
  } else
    return false;
}

/*****************************************************************/

int ecl_region_sum_kw_int( ecl_region_type * ecl_region, const ecl_kw_type * ecl_kw , bool force_active) {
  int sum;
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_element_sum_indexed( ecl_kw , index_set , &sum );
  return sum;
}


float ecl_region_sum_kw_float( ecl_region_type * ecl_region, const ecl_kw_type * ecl_kw , bool force_active) {
  float sum;
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_element_sum_indexed( ecl_kw , index_set , &sum);
  return sum;
}


double ecl_region_sum_kw_double( ecl_region_type * ecl_region, const ecl_kw_type * ecl_kw , bool force_active) {
  double sum;
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_element_sum_indexed( ecl_kw , index_set , &sum);
  return sum;
}


int ecl_region_sum_kw_bool( ecl_region_type * ecl_region, const ecl_kw_type * ecl_kw , bool force_active) {
  int sum;
  const int_vector_type * index_set = ecl_region_get_kw_index_list( ecl_region , ecl_kw , force_active);
  ecl_kw_element_sum_indexed( ecl_kw , index_set , &sum);
  return sum;
}
