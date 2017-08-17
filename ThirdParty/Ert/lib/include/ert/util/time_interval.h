/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'time_interval.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_TIME_INTERVAL_H
#define ERT_TIME_INTERVAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <time.h>

  typedef struct time_interval_struct time_interval_type;

  time_interval_type * time_interval_alloc( time_t start_time , time_t end_time );
  time_interval_type * time_interval_alloc_open( );
  time_interval_type * time_interval_alloc_copy( const time_interval_type * src);
  void                 time_interval_reopen( time_interval_type * time_interval);
  void                 time_interval_free( time_interval_type * ti );
  bool                 time_interval_is_empty( time_interval_type * ti );
  bool                 time_interval_update( time_interval_type * ti , time_t start_time , time_t end_time);
  bool                 time_interval_contains( const time_interval_type * ti , time_t t);
  bool                 time_interval_has_overlap( const time_interval_type * t1 , const time_interval_type * t2);
  bool                 time_interval_is_adjacent( const time_interval_type * t1 , const time_interval_type * t2);
  bool                 time_interval_update_start( time_interval_type * ti , time_t start_time );
  bool                 time_interval_update_end( time_interval_type * ti , time_t end_time );
  time_t               time_interval_get_start( const time_interval_type * ti);
  time_t               time_interval_get_end( const time_interval_type * ti);
  bool                 time_interval_extend( time_interval_type * t1 , const time_interval_type * t2);
  bool                 time_interval_intersect( time_interval_type * t1 , const time_interval_type * t2);
  bool                 time_interval_equal( const time_interval_type * t1 , const time_interval_type * t2);
  bool                 time_interval_arg_before( const time_interval_type * ti , time_t arg);
  bool                 time_interval_arg_after( const time_interval_type * ti , time_t arg);

#ifdef __cplusplus
}
#endif
#endif 
