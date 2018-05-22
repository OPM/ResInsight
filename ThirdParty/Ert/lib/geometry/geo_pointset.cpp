/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'geo_pointset.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <ert/util/util.hpp>

#include <ert/geometry/geo_pointset.hpp>


#define INIT_SIZE 256


struct geo_pointset_struct {
  int         size;
  int         alloc_size;
  bool        internal_z;

  double    * xcoord;
  double    * ycoord;
  double    * zcoord;     // Can be NULL
};



static void geo_pointset_resize( geo_pointset_type * pointset, int new_alloc_size) {
  pointset->xcoord = (double*)util_realloc( pointset->xcoord , new_alloc_size * sizeof * pointset->xcoord  );
  pointset->ycoord = (double*)util_realloc( pointset->ycoord , new_alloc_size * sizeof * pointset->ycoord  );
  if (pointset->internal_z)
    pointset->zcoord = (double*)util_realloc( pointset->zcoord , new_alloc_size * sizeof * pointset->zcoord);

  pointset->alloc_size = new_alloc_size;
}


geo_pointset_type *  geo_pointset_alloc( bool internal_z) {
  geo_pointset_type * pointset = (geo_pointset_type*)util_malloc( sizeof * pointset );
  pointset->xcoord = NULL;
  pointset->ycoord = NULL;
  pointset->zcoord = NULL;
  pointset->internal_z = internal_z;
  pointset->size = 0;
  geo_pointset_resize( pointset , INIT_SIZE );
  return pointset;
}


void geo_pointset_memcpy( const geo_pointset_type * src, geo_pointset_type * target , bool copy_zdata) {
  if (src->internal_z != target->internal_z)
    util_abort("%s: when copying geo_poitset they must have equal value for internal_z\n", __func__);

  geo_pointset_resize( target , src->size );
  target->size = src->size;
  memcpy( target->xcoord , src->xcoord , src->size * sizeof * src->xcoord);
  memcpy( target->ycoord , src->ycoord , src->size * sizeof * src->ycoord);
  if (copy_zdata) {
    if (src->internal_z)
      memcpy( target->zcoord , src->zcoord , src->size * sizeof * src->zcoord);
    else
      util_abort("%s: can not pass copy_zdata = true for pointset with shared z\n",__func__);
  } else if (target->internal_z) {
    int i;
    for (i=0; i < target->size; i++)
      target->zcoord[i] = 0;
  }
}

void geo_pointset_add_xyz( geo_pointset_type * pointset , double x , double y, double z) {
  if (pointset->size == pointset->alloc_size)
    geo_pointset_resize( pointset , 1 + pointset->alloc_size * 2);

  pointset->xcoord[ pointset->size ] = x;
  pointset->ycoord[ pointset->size ] = y;
  if (pointset->internal_z)
      pointset->zcoord[ pointset->size ] = z;

  pointset->size++;
}

void geo_pointset_free( geo_pointset_type * pointset ) {
  free( pointset->xcoord );
  free( pointset->ycoord );
  util_safe_free( pointset->zcoord );
  free( pointset );
}


int geo_pointset_get_size( const geo_pointset_type * pointset ) {
  return pointset->size;
}


const double * geo_pointset_get_zcoord( const geo_pointset_type * pointset ) {
  return pointset->zcoord;
}


static void geo_pointset_assert_index( const geo_pointset_type * pointset , int index) {
  if ((index < 0) || (index >= pointset->size))
    util_abort("%s: invalid pointset index. Size:%d \n",__func__ , index );
}

static void geo_pointset_assert_zindex( const geo_pointset_type * pointset , int index) {
  if ((index < 0) || (index >= pointset->size))
    util_abort("%s: invalid pointset index. Size:%d \n",__func__ , index );

  if (pointset->zcoord == NULL)
    util_abort("%s: z coordinate not set\n",__func__);
}


void geo_pointset_iget_xy( const geo_pointset_type * pointset , int index , double * x , double * y) {
  geo_pointset_assert_index( pointset , index );
  *x = pointset->xcoord[ index ];
  *y = pointset->ycoord[ index ];
}


double geo_pointset_iget_z( const geo_pointset_type * pointset , int index ) {
  geo_pointset_assert_zindex( pointset , index );
  return pointset->zcoord[ index ];
}


void geo_pointset_iset_z( geo_pointset_type * pointset , int index , double value) {
  geo_pointset_assert_zindex( pointset , index );
  pointset->zcoord[ index ] = value;
}


bool geo_pointset_equal( const geo_pointset_type * pointset1 , const geo_pointset_type * pointset2) {
  bool equal = false;

  if (pointset1->size == pointset2->size) {
    size_t byte_size = pointset2->size * sizeof * pointset2->xcoord;
    if ((memcmp(pointset1->xcoord, pointset2->xcoord , byte_size) == 0) &&
        (memcmp(pointset1->ycoord, pointset2->ycoord , byte_size) == 0)) {

      if (pointset1->internal_z == pointset2->internal_z) {
        if (pointset1->zcoord) {
          if (memcmp(pointset1->zcoord, pointset2->zcoord , byte_size) == 0)
            equal = true;
        } else
          equal = true;
      }
    }
  }
  return equal;

}


void geo_pointset_assign_z( geo_pointset_type * pointset , double value ) {
  int index;
  for (index = 0; index < pointset->size; index++)
    pointset->zcoord[index] = value;
}


void geo_pointset_shift_z( geo_pointset_type * pointset , double value ) {
  int index;
  for (index = 0; index < pointset->size; index++)
    pointset->zcoord[index] += value;
}


void geo_pointset_scale_z( geo_pointset_type * pointset , double value ) {
  int index;
  for (index = 0; index < pointset->size; index++)
    pointset->zcoord[index] *= value;
}


void geo_pointset_iadd( geo_pointset_type * self , const geo_pointset_type * other ) {
  int index;
  if (self->size == other->size) {
    for (index = 0; index < self->size; index++)
      self->zcoord[index] += other->zcoord[index];
  } else
    util_abort("%s: size mismatch \n",__func__);
}


void geo_pointset_isub( geo_pointset_type * self , const geo_pointset_type * other ) {
  int index;
  if (self->size == other->size) {
    for (index = 0; index < self->size; index++)
      self->zcoord[index] -= other->zcoord[index];
  } else
    util_abort("%s: size mismatch \n",__func__);
}


void geo_pointset_imul( geo_pointset_type * self , const geo_pointset_type * other ) {
  int index;
  if (self->size == other->size) {
    for (index = 0; index < self->size; index++)
      self->zcoord[index] *= other->zcoord[index];
  } else
    util_abort("%s: size mismatch \n",__func__);
}


void geo_pointset_isqrt( geo_pointset_type * pointset ) {
  int index;
  for (index = 0; index < pointset->size; index++)
    pointset->zcoord[index] = sqrt(pointset->zcoord[index]);
}
