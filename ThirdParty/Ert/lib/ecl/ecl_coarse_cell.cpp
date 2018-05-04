/*
   Copyright (c) 2012  statoil asa, norway.

   The file 'ecl_coarse_cell.c' is part of ert - ensemble based reservoir tool.

   ERT is free software: you can redistribute it and/or modify
   it under the terms of the gnu general public license as published by
   the free software foundation, either version 3 of the license, or
   (at your option) any later version.

   ERT is distributed in the hope that it will be useful, but without any
   warranty; without even the implied warranty of merchantability or
   fitness for a particular purpose.

   See the gnu general public license at <http://www.gnu.org/licenses/gpl.html>
   for more details.
*/

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <ert/util/util.hpp>
#include <ert/util/type_macros.hpp>
#include <ert/util/int_vector.hpp>

#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_coarse_cell.hpp>

/******************************************************************/
/*

  +---------+---------+---------+---------+---------+---------+
  | 24      | 25      | 26      | 27      | 28      | 29      |
  |   [X]   |   [X]   |   [X]   |   [X]   |         |         |
  |         |         |         |         |         |         |
  +---------+=========+=========+=========+=========+---------+
  | 18      | 19      . 20      . 21      . 22      | 23      |
  |   [X]   |         .         .   [C]   .         |         |          j
  |         |         .         .         .         |         |       /|\
  +---------+. . . . ... . . . ... . . . ... . . . .+---------+        |
  | 12      | 13      . 14      . 15      . 16      | 17      |        |
  |   [X]   |   [A]   .         .   [B]   .         |   [X]   |        |
  |         |         .         .         .         |         |        |
  +---------+. . . . ... . . . ... . . . ... . . . .+---------+        +---------> i
  |  6      |  7      .  8      .  9      . 10      | 11      |
  |         |         .         .         .         |         |
  |         |         .         .         .         |         |
  +---------+=========+=========+=========+=========+---------+
  |  0      |  1      |  2      |  3      |  4      |  5      |
  |         |   [X]   |   [X]   |   [X]   |         |         |
  |         |         |         |         |         |         |
  +---------+---------+---------+---------+---------+---------+


 - A global grid of nx = 6, ny = 5 cells. The running number inside
   the cells represents the cells global index.

 - The grid contains a coarse cell of dimensions 4 x 3, the coarse
   cell is in the global region: i \in [1,4] and j \in [1,3]. 

 - When solving the dynamical equations the properties of the coarse
   cell will enter as one entity, but when defining the grid initially
   one can in prinsiple have more than one active cell. The example
   above shows that the coarse region has three active cells, marked
   with [A], [B] and [C] respectively.

 - In the global grid the cells marked with [X] are active, i.e. when
   counting from zero the coarse cell will be the fourth active
   cell. This particular grid has a total of ten normal active cells
   (i.e. 10 x [X]) and one coarse cell which is active. 


 - The dynamic variables like PRESSURE and SWAT are only represented
   with one element for the entire coarse cell. Static properties like
   PERMX and PORO are typically defined for all the cells in the
   entire grid, the different active cells in the coarse cell are
   stored in the active_cells vector, and when we just want one
   representative property from the the coarse cell the first (often
   only ...) element in this vector is used.

 ----


 - When we introduce coarse cells the mapping between global cell
   indices and active cell indices is not unique any longer. In the
   current grid active_index == 3 will point to the coarse cell, and
   all the cells in the coarse region will:

     - ecl_grid_cell_active1( grid , global_index)      => True
     - ecl_grid_get_active_index1( grid , global_index) => 3



For the example above the ecl_coarse_cell datastructure would have the
following content:

  ijk           = {0 , 5 , 0 , 4 , k1 , k2 }
  active_index  = 3
  cell_list     = {7,8,9,10,13,14,15,16,19,20,21,22}
  active_cells  = {13,15,21}
  active_values = {1,1,1}   <= This can 2/3 for dual porosity.
   
*/




#define ECL_COARSE_CELL_TYPE_ID 91554106

struct ecl_coarse_cell_struct {
  UTIL_TYPE_ID_DECLARATION;
  int               ijk[6];  // {i1, i2 , j1 , j2 , k1 , k2}
  int               active_index;
  int               active_fracture_index;

  bool              __cell_list_sorted;  
  int_vector_type * cell_list;
  int_vector_type * active_cells;
  int_vector_type * active_values;
};




static UTIL_SAFE_CAST_FUNCTION( ecl_coarse_cell , ECL_COARSE_CELL_TYPE_ID )

void ecl_coarse_cell_assert( ecl_coarse_cell_type * coarse_cell ) {
  int box_size =
    (1 + coarse_cell->ijk[1] - coarse_cell->ijk[0]) *
    (1 + coarse_cell->ijk[2] - coarse_cell->ijk[3]) *
    (1 + coarse_cell->ijk[4] - coarse_cell->ijk[5]);

  if (box_size != int_vector_size( coarse_cell->cell_list ))
    util_abort("%s: using invalid coarse cell. Box size:%d cells. cell_list:%d cells \n",__func__ , box_size , int_vector_size( coarse_cell->cell_list ));
}




ecl_coarse_cell_type * ecl_coarse_cell_alloc() {
  const int LARGE = 1 << 30;
  ecl_coarse_cell_type * coarse_cell = (ecl_coarse_cell_type *) util_malloc( sizeof * coarse_cell );
  UTIL_TYPE_ID_INIT( coarse_cell , ECL_COARSE_CELL_TYPE_ID );

  coarse_cell->ijk[0] =  LARGE;
  coarse_cell->ijk[2] =  LARGE;
  coarse_cell->ijk[4] =  LARGE;

  coarse_cell->ijk[1] = -LARGE;
  coarse_cell->ijk[3] = -LARGE;
  coarse_cell->ijk[5] = -LARGE;

  coarse_cell->active_index          = -1;
  coarse_cell->active_fracture_index = -1;
  coarse_cell->__cell_list_sorted     = false;
  coarse_cell->cell_list     = int_vector_alloc(0,0);
  coarse_cell->active_cells  = int_vector_alloc(0,0);
  coarse_cell->active_values = int_vector_alloc(0,0);
  return coarse_cell;
}


bool ecl_coarse_cell_equal( const ecl_coarse_cell_type * coarse_cell1 , const ecl_coarse_cell_type * coarse_cell2) {
  bool equal = true;
  if (coarse_cell1->active_index != coarse_cell2->active_index)
    equal = false;

  if (coarse_cell1->active_fracture_index != coarse_cell2->active_fracture_index)
    equal = false;
  
  if (equal) {
    if (memcmp( coarse_cell1->ijk , coarse_cell2->ijk , 6 * sizeof * coarse_cell1->ijk) != 0)
      equal = false;
  }

  
  if (equal)
    equal = int_vector_equal(coarse_cell1->active_cells , coarse_cell2->active_cells);
  
  if (equal)
    equal = int_vector_equal(coarse_cell1->active_values , coarse_cell2->active_values);

  if (equal)
    equal = int_vector_equal(coarse_cell1->cell_list , coarse_cell2->cell_list);

  if (!equal) {
    ecl_coarse_cell_fprintf( coarse_cell1 , stdout );
    ecl_coarse_cell_fprintf( coarse_cell2 , stdout );
  }
  
  return equal;
}

/*
  Should not be called more than once with the same arguments; that is
  not checked.
*/

void ecl_coarse_cell_update( ecl_coarse_cell_type * coarse_cell , int i , int j , int k , int global_index ) {
  coarse_cell->ijk[0] = util_int_min( coarse_cell->ijk[0] , i );
  coarse_cell->ijk[2] = util_int_min( coarse_cell->ijk[2] , j );
  coarse_cell->ijk[4] = util_int_min( coarse_cell->ijk[4] , k );

  coarse_cell->ijk[1] = util_int_max( coarse_cell->ijk[1] , i );
  coarse_cell->ijk[3] = util_int_max( coarse_cell->ijk[3] , j );
  coarse_cell->ijk[5] = util_int_max( coarse_cell->ijk[5] , k );

  int_vector_append( coarse_cell->cell_list , global_index );
}


void ecl_coarse_cell_free( ecl_coarse_cell_type * coarse_cell ) {
  int_vector_free( coarse_cell->cell_list );
  int_vector_free( coarse_cell->active_cells );
  int_vector_free( coarse_cell->active_values );
  free( coarse_cell );
}


void ecl_coarse_cell_free__( void * arg ) {
  ecl_coarse_cell_type * coarse_cell = ecl_coarse_cell_safe_cast( arg );
  ecl_coarse_cell_free( coarse_cell );
}


/*****************************************************************/

static void ecl_coarse_cell_sort( ecl_coarse_cell_type * coarse_cell ) {
  if (!coarse_cell->__cell_list_sorted) {
    int_vector_sort( coarse_cell->cell_list );
    coarse_cell->__cell_list_sorted = true;
  }
}

int ecl_coarse_cell_get_size( const ecl_coarse_cell_type * coarse_cell) {
  return int_vector_size( coarse_cell->cell_list );
}


int ecl_coarse_cell_iget_cell_index(  ecl_coarse_cell_type * coarse_cell , int group_index) {
  ecl_coarse_cell_sort( coarse_cell );
  return int_vector_iget( coarse_cell->cell_list , group_index );
}

const int * ecl_coarse_cell_get_index_ptr(  ecl_coarse_cell_type * coarse_cell ) {
  ecl_coarse_cell_sort( coarse_cell );
  return int_vector_get_const_ptr( coarse_cell->cell_list );
}


const int_vector_type * ecl_coarse_cell_get_index_vector(  ecl_coarse_cell_type * coarse_cell ) {
  ecl_coarse_cell_sort( coarse_cell );
  return  coarse_cell->cell_list;
}

/*****************************************************************/


int ecl_coarse_cell_get_i1( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->ijk[0];
}

int ecl_coarse_cell_get_i2( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->ijk[1];
}

int ecl_coarse_cell_get_j1( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->ijk[2];
}

int ecl_coarse_cell_get_j2( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->ijk[3];
}

int ecl_coarse_cell_get_k1( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->ijk[4];
}

int ecl_coarse_cell_get_k2( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->ijk[5];
}

const int * ecl_coarse_cell_get_box_ptr( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->ijk;
}


/*****************************************************************/

void ecl_coarse_cell_update_index( ecl_coarse_cell_type * coarse_cell , int global_index , int * active_index , int * active_fracture_index , int active_value) {
  if (active_value & CELL_ACTIVE_MATRIX) {
    if (coarse_cell->active_index == -1) {
      coarse_cell->active_index = *active_index;
      (*active_index) += 1;
    }
  }
  
  if (active_value & CELL_ACTIVE_FRACTURE) {
    if (coarse_cell->active_fracture_index == -1) {
      coarse_cell->active_fracture_index = *active_fracture_index;
      (*active_fracture_index) += 1;
    }
  }
  
  int_vector_append( coarse_cell->active_cells , global_index );
  int_vector_append( coarse_cell->active_values , active_value );
  
  if (int_vector_size( coarse_cell->active_values ) > 1) {
    if (int_vector_reverse_iget( coarse_cell->active_values , -2 ) != active_value)
      util_abort("%s: Sorry - current coarse cell implementation requires that all active cells have the same active value\n",__func__);
  }
}


int ecl_coarse_cell_get_active_index( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->active_index;
}

int ecl_coarse_cell_get_active_fracture_index( const ecl_coarse_cell_type * coarse_cell ) {
  return coarse_cell->active_fracture_index;
}


/**
   Will return the global index of the 'ith active cell in the coarse
   cell.
*/
int ecl_coarse_cell_iget_active_cell_index( const ecl_coarse_cell_type * coarse_cell , int index) {
  return int_vector_iget( coarse_cell->active_cells , index );
}

int ecl_coarse_cell_iget_active_value( const ecl_coarse_cell_type * coarse_cell , int index) {
  return int_vector_iget( coarse_cell->active_values , index );
}

int ecl_coarse_cell_get_num_active( const ecl_coarse_cell_type * coarse_cell) {
  return int_vector_size( coarse_cell->active_cells );
}


/*****************************************************************/

void ecl_coarse_cell_fprintf( const ecl_coarse_cell_type * coarse_cell , FILE * stream ) {
  fprintf(stream,"Coarse box: \n");
  fprintf(stream,"   i             : %3d - %3d\n",coarse_cell->ijk[0] , coarse_cell->ijk[1]);
  fprintf(stream,"   j             : %3d - %3d\n",coarse_cell->ijk[2] , coarse_cell->ijk[3]);
  fprintf(stream,"   k             : %3d - %3d\n",coarse_cell->ijk[4] , coarse_cell->ijk[5]);
  fprintf(stream,"   active_cells  : " ); int_vector_fprintf( coarse_cell->active_cells  , stream , "" , "%5d "); 
  fprintf(stream,"   active_values : " ); int_vector_fprintf( coarse_cell->active_values , stream , "" , "%5d "); 
  //fprintf(stream,"   Cells         : " ); int_vector_fprintf( coarse_cell->cell_list , stream , "" , "%5d ");
}
