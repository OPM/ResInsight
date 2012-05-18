/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'well_state.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
   The well_state_type structure contains state information about one
   well for one particular point in time.
*/


#include <time.h>
#include <stdbool.h>

#include <util.h>
#include <vector.h>
#include <int_vector.h>
#include <ecl_intehead.h>
#include <ecl_file.h>
#include <ecl_kw.h>
#include <ecl_kw_magic.h>
#include <ecl_util.h>

#include <well_const.h>
#include <well_conn.h>
#include <well_state.h>

#define WELL_STATE_TYPE_ID 613307832

struct well_state_struct {
  UTIL_TYPE_ID_DECLARATION;
  char           * name;
  time_t           valid_from_time;
  int              valid_from_report;
  bool             open;
  well_conn_type * wellhead;
  vector_type    * connections;
  well_type_enum   type;
};


UTIL_IS_INSTANCE_FUNCTION( well_state , WELL_STATE_TYPE_ID)


static well_state_type * well_state_alloc_empty() {
  well_state_type * well_state = util_malloc( sizeof * well_state , __func__ );
  UTIL_TYPE_ID_INIT( well_state , WELL_STATE_TYPE_ID );
  well_state->connections = vector_alloc_new();
  return well_state;
}



void well_state_add_conn( well_state_type * well_state , int grid_nr , well_conn_type * conn) {
  vector_append_owned_ref( well_state->connections , conn , well_conn_free__);
}




well_state_type * well_state_alloc( const ecl_file_type * ecl_file , const ecl_intehead_type * header , int report_nr , int grid_nr , int well_nr) {
  const ecl_kw_type * iwel_kw   = ecl_file_iget_named_kw( ecl_file , IWEL_KW   , grid_nr);
  const ecl_kw_type * zwel_kw   = ecl_file_iget_named_kw( ecl_file , ZWEL_KW   , grid_nr);
  const ecl_kw_type * icon_kw   = ecl_file_iget_named_kw( ecl_file , ICON_KW   , grid_nr);
  {
    well_state_type * well_state = well_state_alloc_empty();
    const int iwel_offset = header->niwelz * well_nr;
    const int zwel_offset = header->nzwelz * well_nr;
    
    well_state->valid_from_time   = header->sim_time;
    well_state->valid_from_report = report_nr;
    well_state->name              = util_alloc_strip_copy(ecl_kw_iget_ptr( zwel_kw , zwel_offset ));  // Hardwired max 8 characters in Well Name
    {
      well_state->wellhead = well_conn_alloc_wellhead( iwel_kw , header , well_nr );
    }
    {
      int int_state = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_STATUS_ITEM );
      if (int_state > 0)
        well_state->open = true;
      else
        well_state->open = false;
    }
    {
      int num_connections = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_CONNECTIONS_ITEM );
      int lgr_index = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_LGR_ITEM );
      for (int conn_nr = 0; conn_nr < num_connections; conn_nr++) {
        well_conn_type * conn =  well_conn_alloc( icon_kw , header , well_nr , conn_nr );
        well_state_add_conn( well_state , grid_nr , conn );
      }
      printf("lgr_index:%d \n",lgr_index);
    }

    {
      int int_type = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_TYPE_ITEM);
      switch (int_type) {
      case(IWEL_PRODUCER):
        well_state->type = PRODUCER;
        break;
      case(IWEL_OIL_INJECTOR):
        well_state->type = OIL_INJECTOR;
        break;
      case(IWEL_GAS_INJECTOR):
        well_state->type = GAS_INJECTOR;
        break;
      case(IWEL_WATER_INJECTOR):
        well_state->type = WATER_INJECTOR;
        break;
      default:
        util_abort("%s: what the ...\n",__func__);
      }
    }

    return well_state;
  }
}


void well_state_free( well_state_type * well ) {
  vector_free( well->connections );
  well_conn_free( well->wellhead );
  free( well->name );
  free( well );
}

/*****************************************************************/

int well_state_get_report_nr( const well_state_type * well_state ) {
  return well_state->valid_from_report;
}

time_t well_state_get_sim_time( const well_state_type * well_state ) {
  return well_state->valid_from_time;
}

int well_state_get_num_connections( const well_state_type * well_state ) {
  return vector_get_size( well_state->connections );
}

well_conn_type * well_get_wellhead( const well_state_type * well_state ) {
  return well_state->wellhead;
}

well_conn_type * well_state_iget_connection( const well_state_type * well_state , int index) {
  return vector_iget( well_state->connections , index );
}

well_type_enum well_state_get_type( const well_state_type * well_state){ 
  return well_state->type;
}

bool well_state_is_open( const well_state_type * well_state ) {
  return well_state->open;
}

const char * well_state_get_name( const well_state_type * well_state ) {
  return well_state->name;
}


