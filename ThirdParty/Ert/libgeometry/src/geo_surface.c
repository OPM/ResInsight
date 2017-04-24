/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'geo_surface.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/geometry/geo_pointset.h>
#include <ert/geometry/geo_surface.h>

#define __PI                3.14159265
#define GEO_SURFACE_TYPE_ID 111743


struct geo_surface_struct {
  UTIL_TYPE_ID_DECLARATION;
  // Irap data:
  int    nx,ny;
  double rot_angle;  // Radians
  double origo[2];
  double cell_size[2];
  double vec1[2];
  double vec2[2];


  geo_pointset_type * pointset;
};


static void geo_surface_copy_header( const geo_surface_type * src , geo_surface_type * target) {
  target->nx = src->nx;
  target->ny = src->ny;
  target->rot_angle = src->rot_angle;
  {
    int i;
    for (i = 0; i < 2; i++) {
      target->origo[i] = src->origo[i];
      target->cell_size[i] = src->cell_size[i];
      target->vec1[i] = src->vec1[i];
      target->vec2[i] = src->vec2[i];
    }
  }
}

static geo_surface_type * geo_surface_alloc_empty( bool internal_z ) {
  geo_surface_type * surface = util_malloc( sizeof * surface );
  UTIL_TYPE_ID_INIT( surface , GEO_SURFACE_TYPE_ID )
  surface->pointset = geo_pointset_alloc( internal_z );
  return surface;
}


static UTIL_SAFE_CAST_FUNCTION( geo_surface , GEO_SURFACE_TYPE_ID )


static void geo_surface_init_regular( geo_surface_type * surface , const double * zcoord) {
  int zstride_nx = 1;
  int zstride_ny = surface->nx;

  int ix,iy;
  for (iy=0; iy < surface->ny; iy++) {
    for (ix=0; ix < surface->nx; ix++) {
      double x = surface->origo[0] + ix*surface->vec1[0] + iy*surface->vec2[0];
      double y =surface-> origo[1] + ix*surface->vec1[1] + iy*surface->vec2[1];
      if (zcoord != NULL) {
        int z_index = ix*zstride_nx + iy*zstride_ny;
        geo_pointset_add_xyz( surface->pointset , x,y, zcoord[ z_index ]);
      } else
        geo_pointset_add_xyz( surface->pointset , x , y, 0 );
    }
  }
}


static bool geo_surface_fscanf_zcoord( const geo_surface_type * surface , FILE * stream , double * zcoord) {
  int index = 0;

  while (true) {
    if (fscanf(stream , "%lg" , &zcoord[index]) == 1)
      index++;
    else
      return false; // File is too short

    if (index == surface->nx * surface->ny) {
      double extra_value;
      int fscanf_return = fscanf( stream , "%lg" , &extra_value);
      return (fscanf_return == EOF); // no more data dangling at the end of the file.
    }
  }
}



static void geo_surface_fprintf_irap_header( const geo_surface_type * surface , FILE * stream ) {
  const char * float_fmt = "%12.4f\n";
  const char * int_fmt   = "%d\n";

  fprintf(stream , int_fmt   , -996);
  fprintf(stream , int_fmt   , surface->ny);
  fprintf(stream , float_fmt , surface->cell_size[0]);
  fprintf(stream , float_fmt , surface->cell_size[1]);
  fprintf(stream , float_fmt , surface->origo[0]);
  fprintf(stream , float_fmt , surface->origo[0] + surface->cell_size[0] * (surface->nx - 1));
  fprintf(stream , float_fmt , surface->origo[1]);
  fprintf(stream , float_fmt , surface->origo[1] + surface->cell_size[1] * (surface->ny - 1));
  fprintf(stream , int_fmt   , surface->nx);
  fprintf(stream , float_fmt , surface->rot_angle * 180 / __PI );
  fprintf(stream , float_fmt , surface->origo[0]);
  fprintf(stream , float_fmt , surface->origo[1]);
  fprintf(stream , "0  0  0  0  0  0  0  \n");

}



static void geo_surface_fprintf_zcoord( const geo_surface_type * surface , FILE * stream , const double * zcoord ) {
  int num_columns = 6;
  const char * fmt = "%12.4f  ";
  int i;
  for (i=0; i < geo_surface_get_size( surface ); i++) {
    fprintf(stream , fmt , zcoord[i]);

    if (((i + 1) % num_columns) == 0)
      fprintf(stream , "\n");
  }
}


static void geo_surface_fprintf_irap__( const geo_surface_type * surface, const char * filename , const double * zcoord) {
  FILE * stream = util_mkdir_fopen( filename , "w");
  {
    geo_surface_fprintf_irap_header( surface , stream );
    geo_surface_fprintf_zcoord( surface , stream,  zcoord );
  }
  fclose( stream );
}


void geo_surface_fprintf_irap( const geo_surface_type * surface, const char * filename ) {
  const double * zcoord = geo_pointset_get_zcoord( surface->pointset );
  geo_surface_fprintf_irap__( surface , filename , zcoord );
}

void geo_surface_fprintf_irap_external_zcoord( const geo_surface_type * surface, const char * filename , const double * zcoord) {
  geo_surface_fprintf_irap__( surface , filename , zcoord );
}

geo_surface_type  * geo_surface_alloc_new( int nx,        int ny,
                                           double xinc,   double yinc,
                                           double xstart, double ystart,
                                           double angle ) {
    geo_surface_type * surface = geo_surface_alloc_empty( true );

    surface->origo[0]  = xstart;
    surface->origo[1]  = ystart;
    surface->rot_angle = angle * __PI / 180.0;
    surface->nx = nx;
    surface->ny = ny;

    surface->vec1[0] = xinc * cos( surface->rot_angle ) ;
    surface->vec1[1] = xinc * sin( surface->rot_angle ) ;

    surface->vec2[0] = -yinc * sin( surface->rot_angle ) ;
    surface->vec2[1] =  yinc * cos( surface->rot_angle );

    surface->cell_size[0] = xinc;
    surface->cell_size[1] = yinc;
    geo_surface_init_regular( surface, NULL );
    return surface;
}


static void geo_surface_fload_irap_header( geo_surface_type * surface, FILE * stream ) {
  int const996;
  int ny,nx;
  double xinc, yinc,xstart, xend,ystart,yend,angle;

  if (fscanf(stream , "%d  %d  %lg  %lg  %lg  %lg  %lg  %lg  %d  %lg",
             &const996 ,
             &ny ,
             &xinc ,
             &yinc ,
             &xstart ,
             &xend ,
             &ystart ,
             &yend ,
             &nx ,
             &angle) == 10)
    {
      {
        // Some information is rewritten/not used.
        double d;
        int i;
        if (fscanf(stream , "%lg %lg %d %d %d %d %d %d %d " , &d , &d , &i, &i, &i, &i, &i, &i, &i) != 9)
          util_abort("%s: reading irap header failed \n",__func__ );
      }

      surface->origo[0]  = xstart;
      surface->origo[1]  = ystart;
      surface->rot_angle = angle * __PI / 180.0;
      surface->nx = nx;
      surface->ny = ny;

      surface->vec1[0] = xinc * cos( surface->rot_angle ) ;
      surface->vec1[1] = xinc * sin( surface->rot_angle ) ;

      surface->vec2[0] = -yinc * sin( surface->rot_angle ) ;
      surface->vec2[1] =  yinc * cos( surface->rot_angle );

      surface->cell_size[0] = xinc;
      surface->cell_size[1] = yinc;
    }  else
    util_abort("%s: reading irap header failed\n",__func__ );
}



static bool geo_surface_fload_irap( geo_surface_type * surface , const char * filename , bool loadz) {
  bool read_ok  = true;
  {
    FILE * stream = util_fopen( filename , "r");
    geo_surface_fload_irap_header( surface , stream );
    {
      double * zcoord = NULL;

      if (loadz) {
        zcoord = util_calloc( surface->nx * surface->ny , sizeof * zcoord  );
        read_ok = geo_surface_fscanf_zcoord( surface , stream , zcoord );
      }

      if (read_ok)
        geo_surface_init_regular( surface , zcoord );
      util_safe_free( zcoord );
    }
    fclose( stream );
  }
  return read_ok;
}


bool geo_surface_equal_header( const geo_surface_type * surface1 , const geo_surface_type * surface2 ) {
  bool equal = true;

  equal = equal && (surface1->nx == surface2->nx);
  equal = equal && (surface1->ny == surface2->ny);
  equal = equal && (util_double_approx_equal(surface1->rot_angle, surface2->rot_angle));

  {
    int i;
    for (i = 0; i < 2; i++) {
      equal = equal && (util_double_approx_equal(surface1->origo[i], surface2->origo[i]));
      equal = equal && (util_double_approx_equal(surface1->cell_size[i], surface2->cell_size[i]));
      equal = equal && (util_double_approx_equal(surface1->vec1[i], surface2->vec1[i]));
      equal = equal && (util_double_approx_equal(surface1->vec2[i], surface2->vec2[i]));
    }
  }

  return equal;
}


/**
   The loading will fail hard if the header of surface does not agree
   with the header found in file.
*/

bool geo_surface_fload_irap_zcoord( const geo_surface_type * surface, const char * filename, double *zcoord) {
  FILE * stream = util_fopen__( filename , "r");
  if (stream) {
    bool loadOK = true;
    {
      geo_surface_type * tmp_surface = geo_surface_alloc_empty( false );

      geo_surface_fload_irap_header( tmp_surface , stream );
      loadOK = geo_surface_equal_header( surface , tmp_surface );
      geo_surface_free( tmp_surface );
    }
    if (loadOK)
      loadOK = geo_surface_fscanf_zcoord( surface , stream , zcoord);

    fclose( stream );
    return loadOK;
  } else
    return false;
}


geo_surface_type * geo_surface_fload_alloc_irap( const char * filename , bool loadz) {
  geo_surface_type * surface = geo_surface_alloc_empty( loadz );
  bool load_ok = geo_surface_fload_irap(  surface , filename , loadz );
  if (!load_ok) {
    geo_surface_free( surface );
    surface = NULL;
  }
  return surface;
}


geo_surface_type * geo_surface_alloc_copy( const geo_surface_type * src , bool copy_zdata) {
  geo_surface_type * target = geo_surface_alloc_empty( true );

  geo_surface_copy_header( src , target );
  if (!geo_surface_equal_header( src , target ))
    util_abort("%s: headers not equal after copy \n",__func__);

  geo_pointset_memcpy( src->pointset , target->pointset , copy_zdata );

  return target;
}

void geo_surface_free( geo_surface_type * surface ) {
  geo_pointset_free( surface->pointset );
  free( surface );
}

void geo_surface_free__( void * arg) {
  geo_surface_type * surface = geo_surface_safe_cast( arg );
  geo_surface_free( surface );
}



geo_pointset_type * geo_surface_get_pointset( const geo_surface_type * surface ) {
  return surface->pointset;
}

void geo_surface_iget_xy( const geo_surface_type* surface, int index, double* x, double* y) {
  const geo_pointset_type* pointset = geo_surface_get_pointset(surface);
  geo_pointset_iget_xy(pointset, index, x, y);
}



int geo_surface_get_size( const geo_surface_type * surface ) {
  return geo_pointset_get_size( surface->pointset );
}


int geo_surface_get_nx( const geo_surface_type * surface ) {
  return surface->nx;
}

int geo_surface_get_ny( const geo_surface_type * surface ) {
  return surface->ny;
}



bool geo_surface_equal( const geo_surface_type * surface1 , const geo_surface_type * surface2) {
  if (geo_surface_equal_header(surface1 , surface2))
    return geo_pointset_equal( surface1->pointset , surface2->pointset);
  else
    return false;
}

double geo_surface_iget_zvalue(const geo_surface_type * surface, int index) {
  return geo_pointset_iget_z( surface->pointset , index );
}


void geo_surface_iset_zvalue(geo_surface_type * surface, int index , double value) {
  return geo_pointset_iset_z( surface->pointset , index , value );
}


void geo_surface_assign_value( const geo_surface_type * src , double value) {
  geo_pointset_assign_z( src->pointset , value );
}

void geo_surface_shift( const geo_surface_type * src , double value) {
  geo_pointset_shift_z( src->pointset , value );
}

void geo_surface_scale( const geo_surface_type * src , double value) {
  geo_pointset_scale_z( src->pointset , value );
}


void geo_surface_iadd( geo_surface_type * self , const geo_surface_type * other) {
  if (geo_surface_equal_header(self, other))
    geo_pointset_iadd( self->pointset , other->pointset);
  else
    util_abort("%s: tried to combine incompatible surfaces\n",__func__);
}


void geo_surface_isub( geo_surface_type * self , const geo_surface_type * other) {
  if (geo_surface_equal_header(self, other))
    geo_pointset_isub( self->pointset , other->pointset);
  else
    util_abort("%s: tried to combine incompatible surfaces\n",__func__);
}


void geo_surface_imul( geo_surface_type * self , const geo_surface_type * other) {
  if (geo_surface_equal_header(self, other))
    geo_pointset_imul( self->pointset , other->pointset);
  else
    util_abort("%s: tried to combine incompatible surfaces\n",__func__);
}


void geo_surface_isqrt( geo_surface_type * surface ) {
  geo_pointset_isqrt( surface->pointset );
}
