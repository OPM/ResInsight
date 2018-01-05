/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'time_interval.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/time_interval.h>

#define TIME_INTERVAL_EMPTY   (time_t) -1
#define TIME_T_MAX            (time_t) ((1UL << (( sizeof(time_t) << 3) -1 )) -1 ) 
#define TIME_T_MIN           -TIME_T_MAX


struct time_interval_struct {
  bool   valid;
  time_t start_time;
  time_t end_time;
};


/* 
   If you set something invalid - the whole interval is destroyed. 
*/

bool time_interval_update( time_interval_type * ti , time_t start_time , time_t end_time) {
  ti->start_time = start_time;
  ti->end_time   = end_time;

  ti->valid = (start_time <= end_time) ? true : false;
  return ti->valid;
}


bool time_interval_update_start( time_interval_type * ti , time_t start_time ) {
  return time_interval_update( ti , start_time , ti->end_time );
}


bool time_interval_update_end( time_interval_type * ti , time_t end_time ) {
  return time_interval_update( ti , ti->start_time , end_time );
}


void time_interval_reopen( time_interval_type * time_interval) {
  time_interval_update( time_interval , TIME_T_MIN , TIME_T_MAX);
}


time_interval_type * time_interval_alloc( time_t start_time , time_t end_time ) {
  time_interval_type * ti = (time_interval_type*)util_malloc( sizeof * ti );
  time_interval_update( ti , start_time , end_time );
  return ti;
}


time_interval_type * time_interval_alloc_open( ) {
  return time_interval_alloc( TIME_T_MIN , TIME_T_MAX );
}

time_interval_type * time_interval_alloc_copy( const time_interval_type * src) {
  return time_interval_alloc( src->start_time , src->end_time );
}


void time_interval_free( time_interval_type * ti ) {
  free( ti );
}
  

bool time_interval_is_empty( time_interval_type * ti ) {
  return (ti->end_time <= ti->start_time);
}


bool time_interval_contains( const time_interval_type * ti , time_t t) {
  if (!ti->valid)
    return false;
  else {
    if (t < ti->start_time)
      return false;
    else if (t >= ti->end_time)
      return false;
    else
      return true;
  }
}



bool time_interval_has_overlap( const time_interval_type * t1 , const time_interval_type * t2) {
  if (t1->valid && t2->valid) {
    if (time_interval_contains(t1 , t2->start_time))
      return true;

    if (time_interval_contains(t1 , t2->end_time))
      return true;
    
    return false;
  } else
    return false;
}

bool time_interval_is_adjacent( const time_interval_type * t1 , const time_interval_type * t2) {
  if ((t1->end_time == t2->start_time) || (t1->start_time == t2->end_time))
    return true;
  else
    return false;
}


time_t time_interval_get_start( const time_interval_type * ti) {
  return ti->start_time;
}



time_t time_interval_get_end( const time_interval_type * ti) {
  return ti->end_time;
}


bool time_interval_extend( time_interval_type * t1 , const time_interval_type * t2) {
  if (time_interval_has_overlap(t1,t2) || time_interval_is_adjacent( t1 , t2)) {
    time_t start_time = util_time_t_min( t1->start_time , t2->start_time );
    time_t end_time = util_time_t_max( t1->end_time , t2->end_time );

    return time_interval_update(t1 , start_time , end_time);
  } else
    return false;
}


bool time_interval_intersect( time_interval_type * t1 , const time_interval_type * t2) {
  if (time_interval_has_overlap(t1,t2) || time_interval_is_adjacent( t1 , t2)) {
    time_t start_time = util_time_t_max( t1->start_time , t2->start_time );
    time_t end_time = util_time_t_min( t1->end_time , t2->end_time );

    return time_interval_update(t1 , start_time , end_time);
  } else
    return false;
}


bool time_interval_equal( const time_interval_type * t1 , const time_interval_type * t2) {
  if ((t1->start_time == t2->start_time) && (t1->end_time == t2->end_time))
    return true;
  else
    return false;
}


bool time_interval_arg_after( const time_interval_type * ti , time_t arg) {
  return util_after( arg , ti->end_time );
}


bool time_interval_arg_before( const time_interval_type * ti , time_t arg) {
  return util_before( arg , ti->start_time );
}
