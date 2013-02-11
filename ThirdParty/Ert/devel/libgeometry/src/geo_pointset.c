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


#include <stdlib.h>
#include <stdbool.h>

#include <ert/util/util.h>

#include <ert/geometry/geo_pointset.h>


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
  pointset->xcoord = util_realloc( pointset->xcoord , new_alloc_size * sizeof * pointset->xcoord  );
  pointset->ycoord = util_realloc( pointset->ycoord , new_alloc_size * sizeof * pointset->ycoord  );
  if (pointset->internal_z)  
    pointset->zcoord = util_realloc( pointset->zcoord , new_alloc_size * sizeof * pointset->zcoord);
  
  pointset->alloc_size = new_alloc_size;
}


geo_pointset_type *  geo_pointset_alloc( bool internal_z) {
  geo_pointset_type * pointset = util_malloc( sizeof * pointset );
  pointset->xcoord = NULL;
  pointset->ycoord = NULL;
  pointset->zcoord = NULL;
  pointset->internal_z = internal_z;
  pointset->size = 0;
  geo_pointset_resize( pointset , INIT_SIZE );
  return pointset;
}


void geo_pointset_add_xy( geo_pointset_type * pointset , double x , double y) {
  if (!pointset->internal_z) {
    if (pointset->size == pointset->alloc_size) 
      geo_pointset_resize( pointset , 1 + pointset->alloc_size * 2);
    
    pointset->xcoord[ pointset->size ] = x;
    pointset->ycoord[ pointset->size ] = y;

    pointset->size++;
  } else
    util_abort("%s: can not use function %s for pointsets with internal z.\n",__func__ , __func__);
}


void geo_pointset_add_xyz( geo_pointset_type * pointset , double x , double y, double z) {
  if (pointset->internal_z) {
    if (pointset->size == pointset->alloc_size) 
      geo_pointset_resize( pointset , 1 + pointset->alloc_size * 2);
    
    pointset->xcoord[ pointset->size ] = x;
    pointset->ycoord[ pointset->size ] = y;
    pointset->zcoord[ pointset->size ] = z;
    
    pointset->size++;
  } else
    util_abort("%s: can not use function %s for pointsets with internal z.\n",__func__ , __func__);
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


void geo_pointset_iget_xy( const geo_pointset_type * pointset , int index , double * x , double * y) {
  geo_pointset_assert_index( pointset , index );
  *x = pointset->xcoord[ index ];
  *y = pointset->ycoord[ index ];
}








