/*
   Copyright (C) 2011  Statoil ASA, Norway.
   The file 'time_map.c' is part of ERT - Ensemble based Reservoir Tool.

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
#ifndef ERT_TIME_MAP_H
#define ERT_TIME_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include <ert/util/type_macros.h>
#include <ert/util/int_vector.h>

#include <ert/ecl/ecl_sum.h>

typedef struct time_map_struct time_map_type;

  UTIL_SAFE_CAST_HEADER( time_map  );
  UTIL_IS_INSTANCE_HEADER( time_map );

  bool             time_map_try_summary_update( time_map_type * map , const ecl_sum_type * ecl_sum);
  bool             time_map_try_update( time_map_type * map , int step , time_t time);
  bool             time_map_attach_refcase( time_map_type * time_map , const ecl_sum_type * refcase);
  bool             time_map_has_refcase( const time_map_type * time_map );
  bool             time_map_is_strict( const time_map_type * time_map );
  void             time_map_set_strict( time_map_type * time_map , bool strict);
  void             time_map_clear( time_map_type * map );
  bool             time_map_equal( const time_map_type * map1 , const time_map_type * map2);
  time_map_type  * time_map_alloc( );
  void             time_map_free( time_map_type * map );
  bool             time_map_update( time_map_type * map , int step , time_t time);
  bool             time_map_summary_update( time_map_type * map , const ecl_sum_type * ecl_sum);
  time_t           time_map_iget( time_map_type * map , int step );
  void             time_map_fwrite( time_map_type * map , const char * filename);
  void             time_map_fread( time_map_type * map , const char * filename);
  bool             time_map_fscanf(time_map_type * map , const char * filename);
  double           time_map_iget_sim_days( time_map_type * map , int step );
  int              time_map_get_last_step( time_map_type * map);
  int              time_map_get_size( time_map_type * map);
  time_t           time_map_get_start_time( time_map_type * map);
  time_t           time_map_get_end_time( time_map_type * map);
  double           time_map_get_end_days( time_map_type * map);
  bool             time_map_is_readonly( const time_map_type * tm);
  time_map_type  * time_map_fread_alloc_readonly( const char * filename);
  int_vector_type * time_map_alloc_index_map( time_map_type * map , const ecl_sum_type * ecl_sum );
  int              time_map_lookup_time( time_map_type * map , time_t time);
  int              time_map_lookup_days( time_map_type * map , double sim_days);
  int              time_map_lookup_time_with_tolerance( time_map_type * map , time_t time , int seconds_before_tolerance, int seconds_after_tolerance);
  void             time_map_summary_upgrade107( time_map_type * map , const ecl_sum_type * ecl_sum);

#ifdef __cplusplus
}
#endif
#endif
