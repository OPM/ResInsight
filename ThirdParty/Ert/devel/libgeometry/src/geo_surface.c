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


/*
static int geo_surface_cornerindex( const geo_surface_type * geo_surface , int cell_ix , int cell_iy) {
  return (geo_surface->nx + 1) * cell_iy + cell_ix;
}


static void geo_surface_init_cells( geo_surface_type * geo_surface ) {
  int ix,iy;
  geo_surface->cells = util_malloc( geo_surface->nx * geo_surface->ny * sizeof * geo_surface->cells , __func__);
  for (iy = 0; iy < geo_surface->ny; iy++) {
    for (ix = 0; ix < geo_surface->nx; ix++) {
      int cell_index = iy * geo_surface->nx + ix;
      
      geo_surface->cells[ cell_index ].index_list[0] = geo_surface_cornerindex( geo_surface , ix     , iy    );
      geo_surface->cells[ cell_index ].index_list[1] = geo_surface_cornerindex( geo_surface , ix + 1 , iy    );
      geo_surface->cells[ cell_index ].index_list[2] = geo_surface_cornerindex( geo_surface , ix + 1 , iy + 1);
      geo_surface->cells[ cell_index ].index_list[3] = geo_surface_cornerindex( geo_surface , ix     , iy + 1);
      
    }
  }
}
*/


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
        geo_pointset_add_xy( surface->pointset , x , y );
    }
  }
}


static void geo_surface_fscanf_zcoord( const geo_surface_type * surface , FILE * stream , double * zcoord) {
  int i;
  for (i=0; i < surface->nx * surface->ny; i++) 
    if (fscanf(stream , "%lg" , &zcoord[i]) != 1)
      util_abort("%s: hmm - fatal error when loading surface ..." , __func__);
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
  FILE * stream = util_fopen( filename , "w");
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
      surface->vec2[0] =  yinc * cos( surface->rot_angle );
      
      surface->cell_size[0] = xinc;
      surface->cell_size[1] = yinc;
    }  else
    util_abort("%s: reading irap header failed\n",__func__ );
}



static void geo_surface_fload_irap( geo_surface_type * surface , const char * filename , bool loadz) {
  FILE * stream = util_fopen( filename , "r");
  geo_surface_fload_irap_header( surface , stream );
  {
    double * zcoord = NULL;
        
    if (loadz) {
      zcoord = util_calloc( surface->nx * surface->ny , sizeof * zcoord  );
      geo_surface_fscanf_zcoord( surface , stream , zcoord );
    }

    geo_surface_init_regular( surface , zcoord );
    util_safe_free( zcoord );
  }
  fclose( stream );
}


static bool geo_surface_assert_equal_header( const geo_surface_type * surface1 , const geo_surface_type * surface2 ) {
  bool equal = true;

  

  equal = equal && (surface1->nx == surface2->nx);
  equal = equal && (surface1->ny == surface2->ny);
  
  if (!equal)
    util_abort("%s: failed - surface headers not identical \n",__func__);

  return equal;
}


/**
   The loading will fail hard if the header of surface does not agree
   with the header found in file.  */

void geo_surface_fload_irap_zcoord( const geo_surface_type * surface, const char * filename, double *zcoord) {
  FILE * stream = util_fopen( filename , "r");
  {
    {
      geo_surface_type * tmp_surface = geo_surface_alloc_empty( false );
      geo_surface_fload_irap_header( tmp_surface , stream );
      geo_surface_assert_equal_header( surface , tmp_surface );
      geo_surface_free( tmp_surface );
    }
    geo_surface_fscanf_zcoord( surface , stream , zcoord);
  }
  fclose( stream );
}


geo_surface_type * geo_surface_fload_alloc_irap( const char * filename , bool loadz) {
  geo_surface_type * surface = geo_surface_alloc_empty( loadz );
  geo_surface_fload_irap(  surface , filename , loadz );
  return surface;
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


int geo_surface_get_size( const geo_surface_type * surface ) {
  return geo_pointset_get_size( surface->pointset );
}






