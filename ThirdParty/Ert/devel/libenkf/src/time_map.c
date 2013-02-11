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


#define  _GNU_SOURCE   /* Must define this to get access to pthread_rwlock_t */
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/time_t_vector.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/time_map.h>

#define DEFAULT_TIME  -1


struct time_map_struct {
  time_t_vector_type * map;
  time_t               start_time;
  pthread_rwlock_t     rw_lock;
};


time_map_type * time_map_alloc( ) {
  time_map_type * map = util_malloc( sizeof * map );
  map->map = time_t_vector_alloc(0 , DEFAULT_TIME );
  map->start_time = DEFAULT_TIME;
  pthread_rwlock_init( &map->rw_lock , NULL);
  return map;
}


void time_map_free( time_map_type * map ) {
  time_t_vector_free( map->map );
  free( map );
}


/**
   Must hold the write lock. 
*/

static void time_map_update__( time_map_type * map , int step , time_t time) {
  time_t current_time = time_t_vector_safe_iget( map->map , step);

  if (current_time == DEFAULT_TIME) 
    time_t_vector_iset( map->map , step , time );
  else {
    if (current_time != time) {
      int current[3];
      int new[3];
      
      util_set_date_values( current_time , &current[0] , &current[1] , &current[2]);
      util_set_date_values( time , &new[0] , &new[1] , &new[2]);
      
      util_abort("%s: time mismatch for step:%d   New: %02d/%02d/%04d   existing: %02d/%02d/%04d \n",__func__ , step , 
                 new[0]     , new[1]     , new[2] , 
                 current[0] , current[1] , current[2]);
      
    }
  }

  if (step == 0)
    map->start_time = time;
}


static void time_map_summary_update__( time_map_type * map , const ecl_sum_type * ecl_sum) {
  int first_step = ecl_sum_get_first_report_step( ecl_sum );
  int last_step  = ecl_sum_get_last_report_step( ecl_sum );
  int step;

  for (step = first_step; step <= last_step; step++) {
    if (ecl_sum_has_report_step(ecl_sum , step)) {
      time_t time = ecl_sum_get_report_time( ecl_sum , step ); 
      time_map_update__( map , step , time );
    }
  }
  time_map_update__(map , 0 , ecl_sum_get_start_time( ecl_sum ));
}


static time_t time_map_iget__( const time_map_type * map , int step ) {
  return time_t_vector_safe_iget( map->map , step );
}


/*****************************************************************/

double time_map_iget_sim_days( time_map_type * map , int step ) {
  double days;

  pthread_rwlock_rdlock( &map->rw_lock );
  {
    time_t start_time = time_map_iget__( map , 0 );
    time_t sim_time   = time_map_iget__( map , step );
    
    if (sim_time >= start_time)
      return 1.0 * (sim_time - start_time) / (3600 * 24);
    else
      return -1;
  }
  pthread_rwlock_unlock( &map->rw_lock );

  return days;
}


time_t time_map_iget( time_map_type * map , int step ) {
  time_t t;

  pthread_rwlock_rdlock( &map->rw_lock );
  t = time_map_iget__( map , step );
  pthread_rwlock_unlock( &map->rw_lock );

  return t;
}

void time_map_update( time_map_type * map , int step , time_t time) {
  pthread_rwlock_wrlock( &map->rw_lock );
  time_map_update__( map , step , time );
  pthread_rwlock_unlock( &map->rw_lock );
}


void time_map_summary_update( time_map_type * map , const ecl_sum_type * ecl_sum) {
  pthread_rwlock_wrlock( &map->rw_lock );
  time_map_summary_update__( map , ecl_sum );
  pthread_rwlock_unlock( &map->rw_lock );
}


void time_map_fwrite( time_map_type * map , const char * filename ) {
  pthread_rwlock_wrlock( &map->rw_lock );
  {
    FILE * stream = util_fopen(filename , "w");
    time_t_vector_fwrite( map->map , stream );
    fclose( stream );
}
  pthread_rwlock_unlock( &map->rw_lock );
}


void time_map_fread( time_map_type * map , const char * filename) {
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    if (util_file_exists( filename )) {
      FILE * stream = util_fopen( filename , "r");
      time_t_vector_type * file_map = time_t_vector_fread_alloc( stream );
      
      for (int step=0; step < time_t_vector_size( file_map ); step++) 
        time_map_update__( map , step , time_t_vector_iget( file_map , step ));
      
      time_t_vector_free( file_map );
      fclose( stream );
    }
  }
  pthread_rwlock_unlock( &map->rw_lock );
}



/*
  Observe that the return value from this function is an inclusive
  value; i.e. it should be permissible to ask for results at this report
  step. 
*/

int time_map_get_last_step( time_map_type * map) {
  int last_step;
  
  pthread_rwlock_rdlock( &map->rw_lock );
  last_step = time_t_vector_size( map->map ) - 1;
  pthread_rwlock_unlock( &map->rw_lock );
  
  return last_step;
}
