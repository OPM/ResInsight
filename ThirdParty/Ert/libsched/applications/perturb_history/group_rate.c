/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'group_rate.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <util.h>
#include <time_t_vector.h>
#include <sched_history.h>
#include <string.h>
#include <double_vector.h>
#include <time.h>
#include <math.h>
#include <pert_util.h>
#include <group_rate.h>
#include <well_rate.h>
#include <hash.h>
#include <vector.h>
#include <sched_types.h>

#define GROUP_RATE_ID  6681055


struct group_rate_struct {
  UTIL_TYPE_ID_DECLARATION;
  char * name;
  bool                       producer;
  double_vector_type       * base_rate; 
  double_vector_type       * shift;
  double_vector_type       * min_shift;
  double_vector_type       * max_shift;
  stringlist_type          * min_shift_string;
  stringlist_type          * max_shift_string;
  sched_phase_enum           phase;
  const time_t_vector_type * time_vector;
  vector_type              * well_rates;    
  const sched_history_type * sched_history;
};
  


void group_rate_update_wconhist( group_rate_type * group_rate , sched_kw_wconhist_type * kw, int restart_nr ) {
  int well_nr;
  for (well_nr = 0; well_nr < vector_get_size( group_rate->well_rates ); well_nr++) {
    well_rate_type * well_rate = vector_iget( group_rate->well_rates , well_nr );
    well_rate_update_wconhist( well_rate ,kw , restart_nr );
  }
}


void group_rate_update_wconinje( group_rate_type * group_rate , sched_kw_wconinje_type * kw, int restart_nr ) {
  int well_nr;
  for (well_nr = 0; well_nr < vector_get_size( group_rate->well_rates ); well_nr++) {
    well_rate_type * well_rate = vector_iget( group_rate->well_rates , well_nr );
    well_rate_update_wconinje( well_rate ,kw , restart_nr );
  }
}





group_rate_type * group_rate_alloc(const sched_history_type * sched_history , const time_t_vector_type * time_vector , const char * name , const char * phase , const char * type_string , const char * filename) {
  group_rate_type * group_rate = util_malloc( sizeof * group_rate , __func__);
  UTIL_TYPE_ID_INIT( group_rate , GROUP_RATE_ID );
  group_rate->name             = util_alloc_string_copy( name );
  group_rate->time_vector      = time_vector;
  group_rate->shift            = double_vector_alloc(0,0);
  group_rate->base_rate        = double_vector_alloc(0,0);
  group_rate->min_shift        = double_vector_alloc(0 , 0);
  group_rate->max_shift        = double_vector_alloc(0 , 0);
  group_rate->min_shift_string = stringlist_alloc_new();
  group_rate->max_shift_string = stringlist_alloc_new();
  group_rate->phase            = sched_phase_type_from_string( phase );  
  group_rate->sched_history    = sched_history;
  {
    if (strcmp( type_string , "INJECTOR") == 0)
      group_rate->producer = false;
    else if ( strcmp( type_string , "PRODUCER") == 0)
      group_rate->producer = true;
  }
  
  fscanf_2ts( time_vector , filename , group_rate->min_shift_string , group_rate->max_shift_string );
  group_rate->well_rates   = vector_alloc_new();
  return group_rate;
}


void group_rate_init( group_rate_type * group_rate ) {
  
  for (int iw = 0; iw < vector_get_size( group_rate->well_rates ); iw++) {
    const well_rate_type * well_rate = vector_iget( group_rate->well_rates , iw );
    for (int tstep = 0; tstep < well_rate_get_length( well_rate ); tstep++) 
      double_vector_iadd( group_rate->base_rate , tstep , well_rate_iget_rate( well_rate , tstep ));
  }
  
  for (int i = 0; i < stringlist_get_size( group_rate->min_shift_string ); i++) {
    double_vector_iset( group_rate->min_shift , i , sscanfp( double_vector_safe_iget( group_rate->base_rate , i ) , stringlist_iget( group_rate->min_shift_string , i)));
    double_vector_iset( group_rate->max_shift  , i , sscanfp( double_vector_safe_iget( group_rate->base_rate , i ) , stringlist_iget( group_rate->max_shift_string , i)));
  }

}






static UTIL_SAFE_CAST_FUNCTION( group_rate , GROUP_RATE_ID );

void group_rate_free( group_rate_type * group_rate ) {
  free( group_rate->name );
  double_vector_free( group_rate->shift );
  double_vector_free( group_rate->base_rate );
  double_vector_free( group_rate->min_shift );
  double_vector_free( group_rate->max_shift );
  vector_free( group_rate->well_rates );
  free( group_rate );
}


void group_rate_free__( void * arg ) {
  group_rate_type * group_rate = group_rate_safe_cast( arg );
  group_rate_free( group_rate );
}



bool group_rate_is_producer( const group_rate_type * group_rate ) {
  return group_rate->producer;
}

sched_phase_enum group_rate_get_phase( const group_rate_type * group_rate ) {
  return group_rate->phase;
}


const char * group_rate_get_name( const group_rate_type * group_rate ) {
  return group_rate->name;
}


double_vector_type * group_rate_get_shift( group_rate_type * group_rate ) {
  return group_rate->shift;
}


void group_rate_add_well_rate( group_rate_type * group_rate , well_rate_type * well_rate) {
  if (well_rate_get_phase( well_rate ) == group_rate->phase) {
    char * key = util_alloc_sprintf("%s:%s" , well_rate_get_name( well_rate ) , sched_phase_type_string( group_rate->phase ));
    vector_append_owned_ref( group_rate->well_rates , well_rate , well_rate_free__ );
    free( key );
  }
}




void group_rate_sample( group_rate_type * group_rate ) {
  int length                       = time_t_vector_size( group_rate->time_vector );
  double * group_shift             = util_malloc( length * sizeof * group_shift , __func__); 
  int    * well_count              = util_malloc( length * sizeof * well_count , __func__); 
  int      num_wells               = vector_get_size( group_rate->well_rates );
  int i,well_nr;

  for (i = 0; i < length; i++) {
    group_shift[i] = 0;
    well_count[i] = 0;
  }
  
  for (well_nr=0; well_nr < num_wells; well_nr++) {
    well_rate_type * well_rate = vector_iget( group_rate->well_rates , well_nr );
    well_rate_sample_shift( well_rate );
    {
      const double * well_shift = double_vector_get_ptr( well_rate_get_shift( well_rate ));
      for (i = 0; i < length; i++) {
        group_shift[i] += well_shift[i];
        if (well_rate_well_open( well_rate , i ))
          well_count[i] += 1;
      }
    }
  }

  {
    for (i = 0; i < length; i++) {
      if (group_shift[i] > double_vector_iget( group_rate->max_shift , i)) {
        double adjustment = -(group_shift[i]  - double_vector_iget(group_rate->max_shift , i)) / well_count[i];
        for (well_nr = 0; well_nr < num_wells; well_nr++) {
          well_rate_type * well_rate = vector_iget( group_rate->well_rates , well_nr );
          well_rate_ishift( well_rate , i , adjustment );
        }
      } else if (group_shift[i] < double_vector_iget( group_rate->min_shift , i )) {
        double adjustment = -(group_shift[i]  - double_vector_iget(group_rate->min_shift , i)) / well_count[i];
        for (well_nr = 0; well_nr < num_wells; well_nr++) {
          well_rate_type * well_rate = vector_iget( group_rate->well_rates , well_nr );
          well_rate_ishift( well_rate , i , adjustment );
        }
      }
    }
  }

  
  
  free( well_count );
  free( group_shift );

}
