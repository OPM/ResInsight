/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_region.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_REGION_H
#define ERT_ECL_REGION_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/util/int_vector.h>

#include <ert/geometry/geo_polygon.h>

#include <ert/ecl/ecl_box.h>
#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/layer.h>


typedef enum {
  SELECT_ALL           =  0,
  DESELECT_ALL         =  1,
  SELECT_FROM_IJK      =  2,
  DESELECT_FROM_IJK    =  3,
  SELECT_FROM_I        =  4,
  DSELECT_FROM_I       =  5,
  SELECT_FROM_J        =  6,
  DSELECT_FROM_J       =  7,
  SELECT_FROM_K        =  8,
  DSELECT_FROM_K       =  9,
  SELECT_EQUAL         = 10,
  DESELECT_EQUAL       = 11,
  SELECT_IN_INTERVAL   = 12,
  DESELECT_IN_INTERVAL = 13,
  INVERT_SELECTION     = 14
} ecl_region_select_cmd;



typedef struct ecl_region_struct ecl_region_type;

  void              ecl_region_unlock( ecl_region_type * region );
  void              ecl_region_lock( ecl_region_type * region );
  void              ecl_region_reset( ecl_region_type * ecl_region );
  ecl_region_type * ecl_region_alloc_copy( const ecl_region_type * ecl_region );
  ecl_region_type * ecl_region_alloc( const ecl_grid_type * ecl_grid , bool preselect);
  void              ecl_region_free( ecl_region_type * region );
  void              ecl_region_free__( void * __region );

  const int_vector_type * ecl_region_get_active_list( ecl_region_type * region );
  const int_vector_type * ecl_region_get_global_list( ecl_region_type * region );
  const int_vector_type * ecl_region_get_global_active_list( ecl_region_type * region );

  bool            ecl_region_contains_ijk( const ecl_region_type * ecl_region , int i , int j , int k);
  bool            ecl_region_contains_global( const ecl_region_type * ecl_region , int global_index);
  bool            ecl_region_contains_active( const ecl_region_type * ecl_region , int active_index);

  void              ecl_region_select_true( ecl_region_type * region , const ecl_kw_type * ecl_kw);

  void              ecl_region_invert_selection( ecl_region_type * region );
  void              ecl_region_select_all( ecl_region_type * region);
  void              ecl_region_deselect_all( ecl_region_type * region );
  void              ecl_region_deselect_true( ecl_region_type * region , const ecl_kw_type * ecl_kw);
  void              ecl_region_select_false( ecl_region_type * region , const ecl_kw_type * ecl_kw);

  void              ecl_region_select_in_interval( ecl_region_type * region , const ecl_kw_type * ecl_kw, float min_value , float max_value);
  void              ecl_region_deselect_in_interval( ecl_region_type * region , const ecl_kw_type * ecl_kw, float min_value , float max_value);

  void              ecl_region_select_equal( ecl_region_type * region , const ecl_kw_type * ecl_kw, int value);
  void              ecl_region_deselect_equal( ecl_region_type * region , const ecl_kw_type * ecl_kw, int value);

  void              ecl_region_select_inactive_cells( ecl_region_type * region );
  void              ecl_region_deselect_inactive_cells( ecl_region_type * region );
  void              ecl_region_select_active_cells( ecl_region_type * region );
  void              ecl_region_deselect_active_cells( ecl_region_type * region );

  void              ecl_region_select_from_box( ecl_region_type * region , const ecl_box_type * ecl_box );
  void              ecl_region_deselect_from_box( ecl_region_type * region , const ecl_box_type * ecl_box );

  void              ecl_region_select_from_ijkbox( ecl_region_type * region , int i1 , int i2 , int j1 , int j2 , int k1 , int k2);
  void              ecl_region_deselect_from_ijkbox( ecl_region_type * region , int i1 , int i2 , int j1 , int j2 , int k1 , int k2);

  void              ecl_region_select_i1i2( ecl_region_type * region , int i1 , int i2);
  void              ecl_region_deselect_i1i2( ecl_region_type * region , int i1 , int i2);
  void              ecl_region_select_j1j2( ecl_region_type * region , int j1 , int j2);
  void              ecl_region_deselect_j1j2( ecl_region_type * region , int j1 , int i2);
  void              ecl_region_select_k1k2( ecl_region_type * region , int k1 , int k2);
  void              ecl_region_deselect_k1k2( ecl_region_type * region , int k1 , int i2);

  void              ecl_region_select_shallow_cells( ecl_region_type * region , double depth_limit );
  void              ecl_region_deselect_shallow_cells( ecl_region_type * region , double depth_limit );
  void              ecl_region_select_deep_cells( ecl_region_type * region , double depth_limit );
  void              ecl_region_deselect_deep_cells( ecl_region_type * region , double depth_limit );

  void              ecl_region_select_thin_cells( ecl_region_type * ecl_region , double dz_limit );
  void              ecl_region_deselect_thin_cells( ecl_region_type * ecl_region , double dz_limit );
  void              ecl_region_select_thick_cells( ecl_region_type * ecl_region , double dz_limit );
  void              ecl_region_deselect_thick_cells( ecl_region_type * ecl_region , double dz_limit );

  void              ecl_region_select_small_cells( ecl_region_type * ecl_region , double volum_limit );
  void              ecl_region_deselect_small_cells( ecl_region_type * ecl_region , double volum_limit );
  void              ecl_region_select_large_cells( ecl_region_type * ecl_region , double volum_limit );
  void              ecl_region_deselect_large_cells( ecl_region_type * ecl_region , double volum_limit );

  void              ecl_region_select_global_index( ecl_region_type * ecl_region , int global_index);
  void              ecl_region_deselect_global_index( ecl_region_type * ecl_region , int global_index);

  void              ecl_region_select_active_index( ecl_region_type * ecl_region , int active_index);
  void              ecl_region_deselect_active_index( ecl_region_type * ecl_region , int active_index);

  void              ecl_region_intersection( ecl_region_type * region , const ecl_region_type * new_region );
  void              ecl_region_union( ecl_region_type * region , const ecl_region_type * new_region );
  void              ecl_region_subtract( ecl_region_type * region , const ecl_region_type * new_region);
  void              ecl_region_xor( ecl_region_type * region , const ecl_region_type * new_region);

  void              ecl_region_select_smaller( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , float limit);
  void              ecl_region_deselect_smaller( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , float limit);
  void              ecl_region_select_larger( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , float limit);
  void              ecl_region_deselect_larger( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , float limit);

  void              ecl_region_cmp_select_less( ecl_region_type * ecl_region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2);
  void              ecl_region_cmp_deselect_less( ecl_region_type * ecl_region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2);
  void              ecl_region_cmp_select_more( ecl_region_type * ecl_region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2);
  void              ecl_region_cmp_deselect_more( ecl_region_type * ecl_region , const ecl_kw_type * kw1 , const ecl_kw_type * kw2);

  void              ecl_region_select_in_cylinder( ecl_region_type * region , double x0 , double y0, double R);
  void              ecl_region_deselect_in_cylinder( ecl_region_type * region , double x0 , double y0, double R);
  void              ecl_region_select_in_zcylinder( ecl_region_type * region , double x0 , double y0, double R , double z1 , double z2);
  void              ecl_region_deselect_in_zcylinder( ecl_region_type * region , double x0 , double y0, double R, double z1 , double z2);

  void              ecl_region_select_above_plane( ecl_region_type * region, const double n[3] , const double p[3]);
  void              ecl_region_select_below_plane( ecl_region_type * region, const double n[3] , const double p[3]);
  void              ecl_region_deselect_above_plane( ecl_region_type * region, const double n[3] , const double p[3]);
  void              ecl_region_deselect_below_plane( ecl_region_type * region, const double n[3] , const double p[3]);

  void              ecl_region_select_inside_polygon( ecl_region_type * region    , const geo_polygon_type * polygon);
  void              ecl_region_deselect_inside_polygon( ecl_region_type * region  , const geo_polygon_type * polygon);
  void              ecl_region_select_outside_polygon( ecl_region_type * region   , const geo_polygon_type * polygon);
  void              ecl_region_deselect_outside_polygon( ecl_region_type * region , const geo_polygon_type * polygon);

  void              ecl_region_select_from_layer( ecl_region_type * region , const layer_type * layer , int k , int layer_value);
  void              ecl_region_deselect_from_layer( ecl_region_type * region , const layer_type * layer , int k , int layer_value);
  void              ecl_region_deselect_false( ecl_region_type * region , const ecl_kw_type * ecl_kw);

  
/*****************************************************************/
/* Functions to manipulate ecl_kw instances . */

  void        ecl_region_set_kw_int( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , int value, bool force_active);
  void        ecl_region_set_kw_float( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , float value , bool force_active);
  void        ecl_region_set_kw_double( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , double value , bool force_active);
  void        ecl_region_kw_copy( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * src_kw , bool force_active);
  int         ecl_region_get_kw_size( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , bool force_active);

  void      ecl_region_kw_iadd( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * delta_kw , bool force_active);
  void      ecl_region_kw_idiv( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * div_kw , bool force_active);
  void      ecl_region_kw_imul( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * mul_kw , bool force_active);
  void      ecl_region_kw_isub( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , const ecl_kw_type * delta_kw , bool force_active);

  bool      ecl_region_equal( const ecl_region_type * region1 , const ecl_region_type * region2);

  void      ecl_region_scale_kw_float( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , float value , bool force_active);
  void      ecl_region_scale_kw_double( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , double value , bool force_active);
  void      ecl_region_scale_kw_int( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , int value , bool force_active);
  void      ecl_region_shift_kw_int( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , int value , bool force_active);
  void      ecl_region_shift_kw_double( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , double value , bool force_active);
  void      ecl_region_shift_kw_float( ecl_region_type * ecl_region , ecl_kw_type * ecl_kw , float value , bool force_active);

  const int_vector_type * ecl_region_get_kw_index_list( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , bool force_active);
  

/*****************************************************************/
/* set/get the name */
  void         ecl_region_set_name( ecl_region_type * region , const char * name );
  const char * ecl_region_get_name( const ecl_region_type * region );

/*****************************************************************/
/* Stupid cpp compat/legacy/cruft functions. */
  int         ecl_region_get_active_size_cpp(  ecl_region_type * region );
  int         ecl_region_get_global_size_cpp( ecl_region_type * region );
  const int * ecl_region_get_active_list_cpp( ecl_region_type * region );
  const int * ecl_region_get_global_list_cpp(  ecl_region_type * region );


/*****************************************************************/

  double ecl_region_sum_kw_double( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , bool force_active);
  int    ecl_region_sum_kw_int( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , bool force_active);
  float  ecl_region_sum_kw_float( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , bool force_active);
  int    ecl_region_sum_kw_bool( ecl_region_type * ecl_region , const ecl_kw_type * ecl_kw , bool force_active);



UTIL_IS_INSTANCE_HEADER( ecl_region );
UTIL_SAFE_CAST_HEADER( ecl_region );

#ifdef __cplusplus
}
#endif
#endif
