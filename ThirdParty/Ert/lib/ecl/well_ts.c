/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'well_ts.c' is part of ERT - Ensemble based Reservoir Tool.

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

/**
   The wells can typically change configuration during a simulation,
   new completions can be added, the well can be shut for a period, it
   can change purpose from injector to producer and so on.

   The well_ts datastructure is used to hold the complete history of
   one well; for each new report step a new well_state object is added
   to to the well_ts structure. Afterwards you can use the well_ts
   object to query for the well_state at different times.

   An example timeline for one well can look like this:


                  well_state0    well_state1   well_state2   well_state3
       [-------------x---------------x-------------x--------------]
                   0030            0060          0070           0090

   The well in this example is added at report step 30; after that we
   have well_state information from each of the reported report steps
   60,70 and 90. If we query the well_ts object for well state
   information at a particular report step the well_ts structure will
   return the well_state info at the time at or immediately before the
   query time:

     o If we ask for the well state at step 30 we will get the
       well_state0 object; if we ask for the well state at step 75 we
       will get the well_state2 object.

     o If we ask for the well_state before the well has appeared the
       first time we will get NULL.

     o The restart files have no meta information of when the
       simulation ended, so there is no way to detect it if you ask
       for the well state way beyond the end of the simulation. If you
       ask for the well state at report step 100 (i.e. beyond the end
       of the simulation) you will just get the well_state3 object.

   The time direction can be specified by both report step and
   simulation time - your choice.
*/



#include <stdlib.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/vector.h>

#include <ert/ecl_well/well_ts.h>
#include <ert/ecl_well/well_const.h>
#include <ert/ecl_well/well_state.h>



#define WELL_TS_TYPE_ID    6613005
#define WELL_NODE_TYPE_ID  1114652

typedef struct {
  UTIL_TYPE_ID_DECLARATION;
  int                  report_nr;
  time_t               sim_time;
  well_state_type    * well_state;  // The well_node instance owns the well_state instance.
} well_node_type;


struct well_ts_struct {
  UTIL_TYPE_ID_DECLARATION;
  char               * well_name;
  vector_type        * ts;
};

/******************************************************************/

static well_node_type * well_node_alloc( well_state_type * well_state) {
  well_node_type * node = util_malloc( sizeof * node );
  UTIL_TYPE_ID_INIT( node , WELL_NODE_TYPE_ID );
  node->report_nr  = well_state_get_report_nr( well_state );
  node->sim_time   = well_state_get_sim_time( well_state );
  node->well_state = well_state;
  return node;
}


static UTIL_SAFE_CAST_FUNCTION( well_node , WELL_NODE_TYPE_ID )
static UTIL_SAFE_CAST_FUNCTION_CONST( well_node , WELL_NODE_TYPE_ID )


static void well_node_free( well_node_type * well_node ) {
  well_state_free( well_node->well_state );
  free( well_node );
}

static void well_node_free__( void * arg ) {
  well_node_type * node = well_node_safe_cast( arg );
  well_node_free( node );
}

static int well_node_time_cmp( const void * arg1 , const void * arg2) {
  const well_node_type * node1 = well_node_safe_cast_const( arg1 );
  const well_node_type * node2 = well_node_safe_cast_const( arg2 );

  if (node1->sim_time < node2->sim_time)
    return -1;
  else if (node1->sim_time == node2->sim_time)
    return 0;
  else
    return 1;

}


/*****************************************************************/

static well_ts_type * well_ts_alloc_empty( ) {
  well_ts_type * well_ts = util_malloc( sizeof * well_ts );
  UTIL_TYPE_ID_INIT( well_ts , WELL_TS_TYPE_ID );

  well_ts->ts         = vector_alloc_new();
  return well_ts;
}

static UTIL_SAFE_CAST_FUNCTION( well_ts , WELL_TS_TYPE_ID )


well_ts_type * well_ts_alloc( const char * well_name ) {
  well_ts_type * well_ts = well_ts_alloc_empty();
  well_ts->well_name = util_alloc_string_copy( well_name );
  return well_ts;
}

char * well_ts_get_name( const well_ts_type * well_ts) {
  return well_ts->well_name;
}

static int well_ts_get_index__( const well_ts_type * well_ts , int report_step , time_t sim_time , bool use_report) {
  const int size = vector_get_size( well_ts->ts );
  if (size == 0)
    return 0;

  else {
    const well_node_type * first_node = vector_iget_const( well_ts->ts , 0 );
    const well_node_type * last_node  = vector_get_last_const( well_ts->ts );

    if (use_report) {
      if (report_step < first_node->report_nr)
        return -1;         // Before the start

      if (report_step >= last_node->report_nr)
        return size - 1;   // After end
    } else {
      if (sim_time < first_node->sim_time)
        return -1;         // Before the start

      if (sim_time >= last_node->sim_time)
        return size - 1;   // After end
    }

    // Binary search
    {
      int lower_index  = 0;
      int upper_index  = size - 1;

      while (true) {
        int center_index = (lower_index + upper_index) / 2;
        const well_node_type * center_node = vector_iget_const( well_ts->ts , center_index );
        double cmp;
        if (use_report)
          cmp = center_node->report_nr - report_step;
        else
          cmp = difftime( center_node->sim_time , sim_time );

        if (cmp > 0) {
          if ((center_index - lower_index) == 1)    // We found an interval of length 1
            return lower_index;
          else
            upper_index = center_index;

        } else {

          if ((upper_index - center_index) == 1)    // We found an interval of length 1
            return center_index;
          else
            lower_index = center_index;
        }
      }
    }
  }
}

/*

Index:   0                1                 2
         |----------------|-----------------|
Value:   0               50                76


*/

static int well_ts_get_index( const well_ts_type * well_ts , int report_step , time_t sim_time , bool use_report) {
  int index = well_ts_get_index__( well_ts , report_step , sim_time , use_report );

  // Inline check that the index is correct
  {
    bool OK = true;
    const well_node_type * node = vector_iget_const( well_ts->ts , index );
    well_node_type * next_node = NULL;

    if (index < (vector_get_size( well_ts->ts ) - 1))
      next_node = vector_iget( well_ts->ts , index + 1);

    if (use_report) {
      if (index < 0) {
        if (report_step >= node->report_nr)
          OK = false;
      } else {
        if (report_step < node->report_nr)
          OK = false;
        else {
          if (next_node != NULL)
            if (next_node->report_nr <= report_step)
              OK = false;
        }
      }
    } else {
      if (index < 0) {
        if (sim_time >= node->sim_time)
          OK = false;
      } else {
        if (sim_time < node->sim_time)
          OK = false;
        else {
          if (next_node != NULL)
            if (next_node->sim_time <= sim_time)
              OK = false;
        }
    }

    if (!OK)
      util_abort("%s: holy rider - internal error \n",__func__);
    }
  }

  return index;
}


void well_ts_add_well( well_ts_type * well_ts , well_state_type * well_state ) {
  well_node_type * new_node = well_node_alloc( well_state );
  vector_append_owned_ref( well_ts->ts , new_node , well_node_free__ );

  if (vector_get_size( well_ts->ts ) > 1) {
    const well_node_type * last_node = vector_get_last_const(well_ts->ts );
    if (new_node->sim_time < last_node->sim_time)
      // The new node is chronologically before the previous node;
      // i.e. we must sort the nodes in time. This should probably happen
      // quite seldom:
      vector_sort( well_ts->ts , well_node_time_cmp );
  }
}



void well_ts_free( well_ts_type * well_ts ){
  free( well_ts->well_name );
  vector_free( well_ts->ts );
  free( well_ts );
}



void well_ts_free__( void * arg ) {
  well_ts_type * well_ts = well_ts_safe_cast( arg );
  well_ts_free( well_ts );
}


int well_ts_get_size( const well_ts_type * well_ts) {
  return vector_get_size( well_ts->ts );
}


well_state_type * well_ts_get_first_state( const well_ts_type * well_ts) {
  return well_ts_iget_state( well_ts , 0 );
}


well_state_type * well_ts_get_last_state( const well_ts_type * well_ts) {
  return well_ts_iget_state( well_ts , vector_get_size( well_ts->ts ) - 1);
}


well_state_type * well_ts_iget_state( const well_ts_type * well_ts , int index) {
  well_node_type * node = vector_iget( well_ts->ts , index );

  return node->well_state;
}


well_state_type * well_ts_get_state_from_report( const well_ts_type * well_ts , int report_step) {
  int index = well_ts_get_index( well_ts , report_step , -1 , true );
  if (index < 0)
    return NULL;
  else
    return well_ts_iget_state( well_ts , index );
}


well_state_type * well_ts_get_state_from_sim_time( const well_ts_type * well_ts , time_t sim_time) {
  int index = well_ts_get_index( well_ts , -1 , sim_time , false );
  if (index < 0)
    return NULL;
  else
    return well_ts_iget_state( well_ts , index );
}

