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
#include <ert/util/type_macros.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/ert_log.h>
#include <ert/enkf/time_map.h>

#define DEFAULT_TIME  -1

static time_t time_map_iget__( const time_map_type * map , int step );
static void time_map_update_abort( time_map_type * map , int step , time_t time);
static void time_map_summary_update_abort( time_map_type * map , const ecl_sum_type * ecl_sum);

#define TIME_MAP_TYPE_ID 7751432
struct time_map_struct {
  UTIL_TYPE_ID_DECLARATION;
  time_t_vector_type * map;
  pthread_rwlock_t     rw_lock;
  bool                 modified;
  bool                 read_only;
  bool                 strict;
  const ecl_sum_type * refcase;
};


UTIL_SAFE_CAST_FUNCTION( time_map , TIME_MAP_TYPE_ID )
UTIL_IS_INSTANCE_FUNCTION( time_map , TIME_MAP_TYPE_ID )


time_map_type * time_map_alloc( ) {
  time_map_type * map = util_malloc( sizeof * map );
  UTIL_TYPE_ID_INIT( map , TIME_MAP_TYPE_ID );

  map->map = time_t_vector_alloc(0 , DEFAULT_TIME );
  map->modified = false;
  map->read_only = false;
  map->strict = true;
  map->refcase = NULL;
  pthread_rwlock_init( &map->rw_lock , NULL);
  return map;
}

bool time_map_is_strict( const time_map_type * time_map ){
  return time_map->strict;
}

/**
   The refcase will only be attached if it is consistent with the
   current time map; we will accept attaching a refcase which is
   shorter than the current case.
*/
bool time_map_attach_refcase( time_map_type * time_map , const ecl_sum_type * refcase) {
  bool attach_ok = true;
  pthread_rwlock_rdlock( &time_map->rw_lock );

  {
    int step;
    int max_step = util_int_min( time_map_get_size(time_map) ,  ecl_sum_get_last_report_step( refcase ) + 1);

    for (step = 0; step < max_step; step++) {
      time_t current_time = time_map_iget__( time_map , step );
      time_t sim_time = ecl_sum_get_report_time( refcase , step );

      if (current_time != sim_time) {
        attach_ok = false;
        break;
      }
    }

    if (attach_ok)
      time_map->refcase = refcase;
  }
  pthread_rwlock_unlock( &time_map->rw_lock );

  return attach_ok;
}

bool time_map_has_refcase( const time_map_type * time_map ) {
  if (time_map->refcase)
    return true;
  else
    return false;
}


void time_map_set_strict( time_map_type * time_map , bool strict) {
  time_map->strict = strict;
}


time_map_type * time_map_fread_alloc_readonly( const char * filename) {
  time_map_type * tm = time_map_alloc();

  if (util_file_exists(filename))
    time_map_fread( tm , filename );
  tm->read_only = true;

  return tm;
}


bool time_map_fscanf(time_map_type * map , const char * filename) {
  bool fscanf_ok = true;
  if (util_is_file( filename )) {
    time_t_vector_type * time_vector = time_t_vector_alloc(0,0);

    {
      FILE * stream = util_fopen(filename , "r");
      time_t last_date = 0;
      while (true) {
        char date_string[128];
        if (fscanf(stream , "%s" , date_string) == 1) {
          time_t date;
          if (util_sscanf_date_utc(date_string , &date)) {
            if (date > last_date)
              time_t_vector_append( time_vector , date );
            else {
              fprintf(stderr,"** ERROR: The dates in %s must be in stricly increasing order\n",filename);
              fscanf_ok = false;
              break;
            }
          } else {
            fprintf(stderr,"** ERROR: The string \'%s\' was not correctly parsed as a date (format: DD/MM/YYYY) ",date_string);
            fscanf_ok = false;
            break;
          }
          last_date = date;
        } else
          break;
      }
      fclose( stream );

      if (fscanf_ok) {
        int i;
        time_map_clear( map );
        for (i=0; i < time_t_vector_size( time_vector ); i++)
          time_map_update( map , i , time_t_vector_iget( time_vector , i ));
      }

    }
    time_t_vector_free( time_vector );
  } else
    fscanf_ok = false;

  return fscanf_ok;
}


bool time_map_equal( const time_map_type * map1 , const time_map_type * map2) {
  return time_t_vector_equal( map1->map , map2->map );
}


void time_map_free( time_map_type * map ) {
  time_t_vector_free( map->map );
  free( map );
}


bool time_map_is_readonly( const time_map_type * tm) {
  return tm->read_only;
}



/**
   Must hold the write lock. When a refcase is supplied we gurantee
   that all values written into the map agree with the refcase
   values. However the time map is not preinitialized with the refcase
   values.
*/

static bool time_map_update__( time_map_type * map , int step , time_t update_time) {
  bool   updateOK     = true;
  time_t current_time = time_t_vector_safe_iget( map->map , step);

  if (current_time == DEFAULT_TIME) {
    if (map->refcase) {
      if (step <= ecl_sum_get_last_report_step( map->refcase )) {
        time_t ref_time = ecl_sum_get_report_time( map->refcase , step );

        if (ref_time != update_time) {
          updateOK = false;
          ert_log_add_message( 1 ,  NULL , "Tried to load data where report step/data is incompatible with refcase - ignored" , false);
        }
      }
    }
  } else if (current_time != update_time)
    updateOK = false;


  if (updateOK) {
    map->modified = true;
    time_t_vector_iset( map->map , step , update_time );
  }

  return updateOK;
}


static bool time_map_summary_update__( time_map_type * map , const ecl_sum_type * ecl_sum) {
  bool updateOK = true;
  int first_step = ecl_sum_get_first_report_step( ecl_sum );
  int last_step  = ecl_sum_get_last_report_step( ecl_sum );
  int step;

  for (step = first_step; step <= last_step; step++) {
    if (ecl_sum_has_report_step(ecl_sum , step)) {
      time_t sim_time = ecl_sum_get_report_time( ecl_sum , step );

      updateOK = (updateOK && time_map_update__( map , step , sim_time ));
    }
  }

  updateOK = (updateOK && time_map_update__(map , 0 , ecl_sum_get_start_time( ecl_sum )));
  return updateOK;
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
      days = 1.0 * (sim_time - start_time) / (3600 * 24);
    else
      days = -1;
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

static void time_map_assert_writable( const time_map_type * map) {
  if (map->read_only)
    util_abort("%s: attempt to modify read-only time-map. \n",__func__);
}


/**
   Observe that the locking is opposite of the function name; i.e.
   the time_map_fwrite() function reads the time_map and takes the
   read lock, whereas the time_map_fread() function takes the write
   lock.
*/

void time_map_fwrite( time_map_type * map , const char * filename ) {
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    if (map->modified) {
      FILE * stream = util_mkdir_fopen(filename , "w");
      time_t_vector_fwrite( map->map , stream );
      fclose( stream );
    }
    map->modified = false;
  }
  pthread_rwlock_unlock( &map->rw_lock );
}


void time_map_fread( time_map_type * map , const char * filename) {
  time_map_assert_writable( map );
  pthread_rwlock_wrlock( &map->rw_lock );
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
  time_map_get_last_step( map );
  map->modified = false;
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

int time_map_get_size( time_map_type * map) {
  return time_map_get_last_step( map ) + 1;
}

time_t time_map_get_start_time( time_map_type * map) {
  return time_map_iget( map , 0 );
}


time_t time_map_get_end_time( time_map_type * map) {
  int last_step = time_map_get_last_step( map );
  return time_map_iget( map , last_step );
}

double time_map_get_end_days( time_map_type * map) {
  int last_step = time_map_get_last_step( map );
  return time_map_iget_sim_days( map , last_step );
}

/*****************************************************************/


bool time_map_update( time_map_type * map , int step , time_t time) {
  bool updateOK = time_map_try_update( map , step , time );
  if (!updateOK) {
    if (map->strict)
      time_map_update_abort(map , step , time);
    else
      ert_log_add_message(1 , NULL , "Report step/true time inconsistency - data will be ignored" , false);
  }
  return updateOK;
}


bool time_map_try_update( time_map_type * map , int step , time_t time) {
  bool updateOK;
  time_map_assert_writable( map );
  pthread_rwlock_wrlock( &map->rw_lock );
  {
    updateOK = time_map_update__( map , step , time );
  }
  pthread_rwlock_unlock( &map->rw_lock );
  return updateOK;
}



bool time_map_summary_update( time_map_type * map , const ecl_sum_type * ecl_sum) {
  bool updateOK = time_map_try_summary_update( map , ecl_sum );

  if (!updateOK) {
    if (map->strict)
      time_map_summary_update_abort( map , ecl_sum );
    else
      ert_log_add_message(1 , NULL , "Report step/true time inconsistency - data will be ignored" , false);
  }

  return updateOK;
}


bool time_map_try_summary_update( time_map_type * map , const ecl_sum_type * ecl_sum) {
  bool updateOK;

  time_map_assert_writable( map );
  pthread_rwlock_wrlock( &map->rw_lock );
  {
    updateOK = time_map_summary_update__( map , ecl_sum );
  }
  pthread_rwlock_unlock( &map->rw_lock );

  return updateOK;
}


int time_map_lookup_time( time_map_type * map , time_t time) {
  int index = -1;
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    int current_index = 0;
    while (true) {
      if (current_index >= time_t_vector_size( map->map ))
        break;

      if (time_map_iget__( map , current_index ) == time) {
        index = current_index;
        break;
      }

      current_index++;
    }
  }
  pthread_rwlock_unlock( &map->rw_lock );
  return index;
}

static bool time_map_valid_time__(const time_map_type * map , time_t time) {
  if (time_t_vector_size( map->map ) > 0) {
    if ((time >= time_map_iget__(map , 0)) &&
        (time <= time_map_iget__(map , time_t_vector_size( map->map ) - 1)))
      return true;
    else
      return false;
  } else
    return false;
}



int time_map_lookup_time_with_tolerance( time_map_type * map , time_t time , int seconds_before_tolerance, int seconds_after_tolerance) {
  int nearest_index = -1;
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    if (time_map_valid_time__( map , time )) {
      time_t nearest_diff = 999999999999;
      int current_index = 0;
      while (true) {
        time_t diff = time - time_map_iget__( map , current_index );
        if (diff == 0) {
          nearest_index = current_index;
          break;
        }

        if (abs(diff) < nearest_diff) {
          bool inside_tolerance = true;
          if (seconds_after_tolerance >= 0) {
            if (diff >= seconds_after_tolerance)
              inside_tolerance = false;
          }

          if (seconds_before_tolerance >= 0) {
            if (diff <= -seconds_before_tolerance)
              inside_tolerance = false;
          }

          if (inside_tolerance) {
            nearest_diff = diff;
            nearest_index = current_index;
          }
        }

        current_index++;

        if (current_index >= time_t_vector_size( map->map ))
          break;
      }
    }
  }
  pthread_rwlock_unlock( &map->rw_lock );
  return nearest_index;
}



int time_map_lookup_days( time_map_type * map , double sim_days) {
  int index = -1;
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    if (time_t_vector_size( map->map ) > 0) {
      time_t time = time_map_iget__(map , 0 );
      util_inplace_forward_days_utc( &time , sim_days );
      index = time_map_lookup_time( map , time );
    }
  }
  pthread_rwlock_unlock( &map->rw_lock );
  return index;
}


void time_map_clear( time_map_type * map ) {
  time_map_assert_writable( map );
  pthread_rwlock_wrlock( &map->rw_lock );
  {
    time_t_vector_reset( map->map );
    map->modified = true;
  }
  pthread_rwlock_unlock( &map->rw_lock );
}


/*
  This is a function specifically written to upgrade an on-disk
  time_map which is using localtime (fs_version <= 106) to a utc based
  time_map (fs_version >= 107).
*/

void time_map_summary_upgrade107( time_map_type * map , const ecl_sum_type * ecl_sum) {
  int first_step = ecl_sum_get_first_report_step( ecl_sum );
  int last_step  = ecl_sum_get_last_report_step( ecl_sum );

  time_t_vector_resize( map->map , last_step + 1);
  time_t_vector_iset_block( map->map , 0 , first_step , DEFAULT_TIME);
  for (int step=first_step; step <= last_step; step++) {
    if (ecl_sum_has_report_step(ecl_sum , step)) {
      time_t sim_time = ecl_sum_get_report_time( ecl_sum , step );
      time_t_vector_iset( map->map , step , sim_time);
    }
  }
  map->modified = true;
}


/*****************************************************************/

static void time_map_update_abort( time_map_type * map , int step , time_t time) {
  time_t current_time = time_map_iget__( map , step );
  int current[3];
  int new[3];

  util_set_date_values_utc( current_time , &current[0] , &current[1] , &current[2]);
  util_set_date_values_utc( time , &new[0] , &new[1] , &new[2]);

  util_abort("%s: time mismatch for step:%d   New: %02d/%02d/%04d   existing: %02d/%02d/%04d \n",__func__ , step ,
             new[0]     , new[1]     , new[2] ,
             current[0] , current[1] , current[2]);
}


static void time_map_summary_update_abort( time_map_type * map , const ecl_sum_type * ecl_sum) {
  /*
     If the normal summary update fails we just play through all
     time steps to pinpoint exactly the step where the update fails.
  */

  int first_step = ecl_sum_get_first_report_step( ecl_sum );
  int last_step  = ecl_sum_get_last_report_step( ecl_sum );
  int step;

  for (step = first_step; step <= last_step; step++) {
    if (ecl_sum_has_report_step(ecl_sum , step)) {
      time_t time = ecl_sum_get_report_time( ecl_sum , step );

      if (map->refcase) {
        if (ecl_sum_get_last_report_step( ecl_sum ) >= step) {
          time_t ref_time = ecl_sum_get_report_time( map->refcase , step );
          if (ref_time != time) {
            int ref[3];
            int new[3];

            util_set_date_values_utc( time , &new[0] , &new[1] , &new[2]);
            util_set_date_values_utc( ref_time , &ref[0] , &ref[1] , &ref[2]);

            fprintf(stderr," Time mismatch for step:%d  New: %02d/%02d/%04d   refcase: %02d/%02d/%04d \n", step ,
                    new[0] , new[1] , new[2] ,
                    ref[0] , ref[1] , ref[2]);
          }
        }
      }

      {
          time_t current_time = time_map_iget__( map , step );
          int current[3];
          int new[3];

          util_set_date_values_utc( current_time , &current[0] , &current[1] , &current[2]);
          util_set_date_values_utc( time , &new[0] , &new[1] , &new[2]);

          fprintf(stderr,"Time mismatch for step:%d   New: %02d/%02d/%04d   existing: %02d/%02d/%04d \n",step ,
                  new[0] , new[1] , new[2] ,
                  current[0] , current[1] , current[2]);
      }
    }
  }

  util_abort("%s: inconsistency when updating time map \n",__func__);
}



/*****************************************************************/


/*
  This function creates an integer index mapping from the time map
  into the summary case. In general the time <-> report step mapping
  of the summary data should coincide exactly with the one maintained
  in the time_map, however we allow extra timesteps in the summary
  instance. The extra timesteps will be ignored, holes in the summary
  timestep is not allowed - that will lead to a hard crash.

     time map                      Summary
     -------------------------------------------------
     0: 01/01/2000   <-------      0: 01/01/2000

     1: 01/02/2000   <-------      1: 01/02/2000

     2: 01/03/2000   <-\           2: 02/02/2000 (Ignored)
                        \
                         \--       3: 01/03/2000

     3: 01/04/2000   <-------      4: 01/04/2000


     index_map = { 0 , 1 , 3 , 4 }

  Observe that the time_map_update_summary() must be called prior to
  calling this function, to ensure that the time_map is sufficiently
  long. If timesteps are missing from the summary case we crash hard:


     time map                      Summary
     -------------------------------------------------
     0: 01/01/2000   <-------      0: 01/01/2000

     1: 01/02/2000   <-------      1: 01/02/2000

     2: 01/03/2000                 ## ERROR -> util_abort()

     3: 01/04/2000   <-------      2: 01/04/2000

*/



int_vector_type * time_map_alloc_index_map( time_map_type * map , const ecl_sum_type * ecl_sum ) {
  int_vector_type * index_map = int_vector_alloc(0 , -1 );
  pthread_rwlock_rdlock( &map->rw_lock );
  {
    int time_map_index = 0;
    int sum_index = 0;

    while (true) {
      time_t map_time = time_map_iget__( map , time_map_index);
      if (map_time == DEFAULT_TIME)
        break;

      {
        time_t sum_time;

        while (true) {
          sum_time = ecl_sum_get_report_time( ecl_sum , sum_index );

          if (sum_time > map_time) {
            int day,month,year;
            util_set_date_values_utc( map_time , &day , &month , &year);
            util_abort("%s: The eclipse summary cases is missing data for date:%02d/%02d/%4d - aborting\n", __func__ , day , month , year);
          } else if (sum_time < map_time) {
            sum_index++;
            if (sum_index > ecl_sum_get_last_report_step( ecl_sum ))
              break;
          } else
            break;

        }

        if (sum_time == map_time)
          int_vector_iset( index_map , time_map_index , sum_index);
        else {
          ert_log_add_message(1 , NULL , "Inconsistency in time_map - data will be ignored" , false);
          break;
        }


        time_map_index++;
        if (time_map_index == time_map_get_size( map ))
          break;

      }
    }
  }
  pthread_rwlock_unlock( &map->rw_lock );

  return index_map;
}



