/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lookup_table.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <stdlib.h>

#include <ert/util/util.h>
#include <ert/util/perm_vector.h>
#include <ert/util/double_vector.h>
#include <ert/util/lookup_table.h>

struct lookup_table_struct {
  bool                 data_owner; 
  double_vector_type * x_vector;
  double_vector_type * y_vector;
  const double       * x_data;
  const double       * y_data; 
  int                  size; 
  double               xmin,ymin;
  double               xmax,ymax;
  int                  prev_index;

  bool                 sorted;                 
  bool                 has_low_limit;
  bool                 has_high_limit;
  double               low_limit;
  double               high_limit;
};




static void lookup_table_sort_data( lookup_table_type * lt) {
  if (double_vector_size( lt->x_vector ) > 0) {
    if (double_vector_get_read_only( lt->x_vector ))
      if (!double_vector_is_sorted( lt->x_vector , false))
        util_abort("%s: x vector is not sorted and read-only - this will not fly\n",__func__);
    
    {
      perm_vector_type * sort_perm = double_vector_alloc_sort_perm( lt->x_vector );
      double_vector_permute( lt->x_vector , sort_perm );
      double_vector_permute( lt->y_vector , sort_perm );
      perm_vector_free( sort_perm );
    }
    lt->ymax = double_vector_get_max( lt->y_vector );
    lt->ymin = double_vector_get_min( lt->y_vector );
    lt->xmin = double_vector_get_min( lt->x_vector );
    lt->xmax = double_vector_get_max( lt->x_vector );
    lt->size = double_vector_size( lt->x_vector);
    lt->prev_index = -1;
    lt->x_data = double_vector_get_const_ptr( lt->x_vector );
    lt->y_data = double_vector_get_const_ptr( lt->y_vector );
  }
  lt->sorted = true;
}


/**
   IFF the @read_only flag is set to true; the x vector MUST be
   sorted.  
*/

void lookup_table_set_data( lookup_table_type * lt , double_vector_type * x , double_vector_type * y , bool data_owner ) {
  
  if (lt->data_owner) {
    double_vector_free( lt->x_vector );
    double_vector_free( lt->y_vector );
  }

  lt->x_vector = x;
  lt->y_vector = y;
  lt->data_owner = data_owner;
  lt->sorted = false;
  lookup_table_sort_data( lt );
}


void lookup_table_set_low_limit( lookup_table_type * lt , double limit ) {
  lt->low_limit = limit;
  lt->has_low_limit = true;
}

bool lookup_table_has_low_limit(const lookup_table_type * lt  ) {
  return lt->has_low_limit;
}

void lookup_table_set_high_limit( lookup_table_type * lt , double limit ) {
  lt->high_limit = limit;
  lt->has_high_limit = true;
}

bool lookup_table_has_high_limit(const lookup_table_type * lt  ) {
  return lt->has_high_limit;
}



lookup_table_type * lookup_table_alloc( double_vector_type * x , double_vector_type * y , bool data_owner) {
  lookup_table_type * lt = util_malloc( sizeof * lt);
  lt->data_owner = false;
  if ((x == NULL) && (y == NULL)) {
    x = double_vector_alloc(0 , 0);
    y = double_vector_alloc(0 , 0);
    data_owner = true;
  } 
  lookup_table_set_data( lt , x , y , false );
  lt->data_owner = data_owner;
  lt->has_low_limit = false;
  lt->has_high_limit = false;
  
  return lt;
}

lookup_table_type * lookup_table_alloc_empty( ) {
  return lookup_table_alloc( NULL , NULL , true );
}


int lookup_table_get_size( const lookup_table_type * lt ) {
  return double_vector_size( lt->x_vector );
}


void lookup_table_append( lookup_table_type * lt , double x , double y) {
  double_vector_append( lt->x_vector , x );
  double_vector_append( lt->y_vector , y );
  lt->sorted = false;
}


void lookup_table_free( lookup_table_type * lt ) {
  if (lt->data_owner) {
    double_vector_free( lt->x_vector );
    double_vector_free( lt->y_vector );
  } 
  free( lt );
}


static void lookup_table_assert_sorted( lookup_table_type * lt) {
  if (!lt->sorted)
    lookup_table_sort_data( lt );
}


double lookup_table_interp( lookup_table_type * lt , double x) {
  lookup_table_assert_sorted( lt );
  {
    if ((x >= lt->xmin) && (x < lt->xmax)) {
      int index = double_vector_lookup_bin__( lt->x_vector , x , lt->prev_index );
      {
        double x1 = lt->x_data[ index ];
        double x2 = lt->x_data[ index + 1];
        double y1 = lt->y_data[ index ];
        double y2 = lt->y_data[ index + 1];
        
        lt->prev_index = index;
        return (( x - x1 ) * y2 + (x2 - x) * y1) / (x2 - x1 );
      }
    } else {
      if (x == lt->xmax)
        return double_vector_get_last( lt->y_vector );
      else {
        if (lt->has_low_limit && x < lt->xmin)
          return lt->low_limit;
        else if (lt->has_high_limit && x > lt->xmax)
          return lt->high_limit;
        else {
          util_abort("%s: out of bounds \n",__func__);
          return -1;
        }
      }
    }
  }
}
  


double lookup_table_get_max_value(  lookup_table_type * lookup_table ) {
  lookup_table_assert_sorted( lookup_table );
  return lookup_table->ymax;
}


double lookup_table_get_min_value(  lookup_table_type * lookup_table ) {
  lookup_table_assert_sorted( lookup_table );
  return lookup_table->ymin;
}


double lookup_table_get_max_arg(  lookup_table_type * lookup_table ) {
  lookup_table_assert_sorted( lookup_table );
  return lookup_table->xmax;
}


double lookup_table_get_min_arg( lookup_table_type * lookup_table ) {
  lookup_table_assert_sorted( lookup_table );
  return lookup_table->xmin;
}
