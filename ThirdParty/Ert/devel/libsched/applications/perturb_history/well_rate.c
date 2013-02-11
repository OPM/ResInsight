/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'well_rate.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <util.h>
#include <time_t_vector.h>
#include <double_vector.h>
#include <bool_vector.h>
#include <stringlist.h>
#include <time.h>
#include <math.h>
#include <pert_util.h>
#include <well_rate.h>
#include <sched_types.h>
#include <sched_kw_wconinje.h>

#define WELL_RATE_ID  6681055


struct well_rate_struct {
  UTIL_TYPE_ID_DECLARATION;
  char                     * name;
  double                     corr_length;
  bool                       producer;
  double_vector_type       * shift;
  double_vector_type       * mean_shift;
  double_vector_type       * std_shift;
  stringlist_type          * mean_shift_string;
  stringlist_type          * std_shift_string;
  double_vector_type       * rate; 
  double_vector_type       * base_value;
  bool_vector_type         * percent_std;
  sched_phase_enum           phase;
  const time_t_vector_type * time_vector;
  const sched_history_type * sched_history;
};
  





void well_rate_update_wconhist( well_rate_type * well_rate , sched_kw_wconhist_type * kw, int restart_nr ) {
  double shift = double_vector_iget( well_rate->shift , restart_nr );
  switch (well_rate->phase) {
  case(OIL):
    sched_kw_wconhist_shift_orat( kw , well_rate->name , shift);
    break;                                               
  case(GAS):                                             
    sched_kw_wconhist_shift_grat( kw , well_rate->name , shift);
    break;                                               
  case(WATER):                                           
    sched_kw_wconhist_shift_wrat( kw , well_rate->name , shift);
    break;
  }
}


void well_rate_update_wconinje( well_rate_type * well_rate , sched_kw_wconinje_type * kw, int restart_nr ) {
  sched_kw_wconinje_shift_surface_flow( kw , well_rate->name , double_vector_iget( well_rate->shift , restart_nr ));
  return;
}




/*
        a = exp(-(t_i - t_(i-1)) / corr_length)
     y(i) = a*y(i - 1) + (1 - a) * N(mean(i) , std(i))
     
*/

void well_rate_sample_shift( well_rate_type * well_rate ) {
  int size   = time_t_vector_size( well_rate->time_vector );
  double * R = util_malloc( size * sizeof * R , __func__);
  int i;
  rand_stdnormal_vector( size , R );
  for (i=0; i < size; i++) 
    R[i] = R[i] * double_vector_iget( well_rate->std_shift , i ) + double_vector_iget( well_rate->mean_shift , i );
  
  double_vector_iset( well_rate->shift , 0 , R[0]);
  
  for (i=1; i < size; i++) {
    double dt        = 1.0 * (time_t_vector_iget( well_rate->time_vector , i ) - time_t_vector_iget( well_rate->time_vector , i - 1)) / (24 * 3600);  /* Days */
    double a         = exp(-dt / well_rate->corr_length );
    double shift     = a * double_vector_iget( well_rate->shift , i - 1 ) + (1 - a) * R[i];
    double base_rate = double_vector_safe_iget( well_rate->base_value , i);
    
    /* The time series is sampled - irrespective of whether the well is open or not. */
    
    if ((shift + base_rate) < 0)
      shift = -base_rate;
    
    if (sched_history_well_open( well_rate->sched_history , well_rate->name , i)) 
      double_vector_iset( well_rate->shift , i , shift );
    else 
      double_vector_iset( well_rate->shift , i , 0);
    
  }
  free( R );
}

/*
  Ensures that the final rate is >= 0; this might lead to a minor
  violation of the parent-groups constraints.
*/

void well_rate_ishift( well_rate_type * well_rate ,  int index, double new_shift) {
  if (sched_history_well_open( well_rate->sched_history , well_rate->name , index)) {
    double base_rate = double_vector_safe_iget( well_rate->base_value , index);
    double shift     = double_vector_safe_iget( well_rate->shift , index) + new_shift;

    if ((base_rate + shift) < 0)
      shift = -base_rate;
    double_vector_iset( well_rate->shift , index , shift );
  }
}


double well_rate_iget_rate( const well_rate_type * well_rate , int report_step ) {
  return double_vector_safe_iget( well_rate->base_value , report_step );
}

int well_rate_get_length( const well_rate_type * well_rate ) {
  return double_vector_size( well_rate->base_value );
}



well_rate_type * well_rate_alloc(const sched_history_type * sched_history , const time_t_vector_type * time_vector , const char * name , double corr_length , const char * filename, sched_phase_enum phase, bool producer) {
  well_rate_type * well_rate = util_malloc( sizeof * well_rate , __func__);
  UTIL_TYPE_ID_INIT( well_rate , WELL_RATE_ID );
  well_rate->name         = util_alloc_string_copy( name );
  well_rate->time_vector  = time_vector;
  well_rate->corr_length  = corr_length;
  well_rate->shift        = double_vector_alloc(0,0);
  well_rate->mean_shift   = double_vector_alloc(0 , 0);
  well_rate->std_shift    = double_vector_alloc(0 , 0);
  well_rate->mean_shift_string   = stringlist_alloc_new();
  well_rate->std_shift_string    = stringlist_alloc_new();
  well_rate->base_value   = double_vector_alloc(0 , 0);
  well_rate->rate         = double_vector_alloc(0 , 0);
  well_rate->phase        = phase;
  well_rate->sched_history= sched_history; 
  well_rate->percent_std  = bool_vector_alloc( 0 , false );
  well_rate->producer     = producer;
  fscanf_2ts( time_vector , filename , well_rate->mean_shift_string , well_rate->std_shift_string);
  
  {
    char * key;
    if (well_rate->producer) {
      switch(well_rate->phase) {
      case (WATER):
        key = util_alloc_sprintf("WWPRH%s%s" , sched_history_get_join_string( sched_history ) , well_rate->name );
        break;
      case( GAS ):
        key = util_alloc_sprintf("WGPRH%s%s" , sched_history_get_join_string( sched_history ) , well_rate->name );
        break;
      case( OIL ):
        key = util_alloc_sprintf("WOPRH%s%s" , sched_history_get_join_string( sched_history ) , well_rate->name );
        break;
      default:
        key = NULL;
        util_abort("%s: unknown phase identitifier: %d \n",__func__ , well_rate->phase);
      }
    } else {
      switch(well_rate->phase) {
      case (WATER):
        key = util_alloc_sprintf("WWIRH%s%s" , sched_history_get_join_string( sched_history ) , well_rate->name );
        break;
      case( GAS ):
        key = util_alloc_sprintf("WGIRH%s%s" , sched_history_get_join_string( sched_history ) , well_rate->name );
        break;
      case( OIL ):
        key = util_alloc_sprintf("WOIRH%s%s" , sched_history_get_join_string( sched_history ) , well_rate->name );
        break;
      default:
        util_abort("%s: unknown phase identitifier: %d \n",__func__ , well_rate->phase);
        key = NULL;
      }
    }
    
    if (sched_history_has_key( sched_history , key)) {
      sched_history_init_vector( sched_history , key , well_rate->base_value );
      well_rate_eval_stat( well_rate );
    } else 
      fprintf(stderr,"** Warning - schedule history does not have key:%s - suspicious?\n", key );
    
    free( key );
  }
  return well_rate;
}


bool well_rate_well_open( const well_rate_type * well_rate , int index ) {
  return sched_history_well_open( well_rate->sched_history , well_rate->name , index );
}


void well_rate_eval_stat( well_rate_type * well_rate ) {
  for (int i = 0; i < stringlist_get_size( well_rate->mean_shift_string ); i++) {
    double mean_shift = sscanfp( double_vector_safe_iget( well_rate->base_value , i ) , stringlist_iget( well_rate->mean_shift_string , i));
    double std_shift  = sscanfp( double_vector_safe_iget( well_rate->base_value , i ) , stringlist_iget( well_rate->std_shift_string , i));
    
    double_vector_iset( well_rate->mean_shift , i , mean_shift );
    double_vector_iset( well_rate->std_shift  , i , std_shift);
  }
}


static UTIL_SAFE_CAST_FUNCTION( well_rate , WELL_RATE_ID );


void well_rate_free( well_rate_type * well_rate ) {
  free( well_rate->name );
  double_vector_free( well_rate->shift );
  double_vector_free( well_rate->mean_shift );
  double_vector_free( well_rate->std_shift );
  double_vector_free( well_rate->base_value );
  bool_vector_free( well_rate->percent_std );
  free( well_rate );
}

void well_rate_free__( void * arg ) {
  well_rate_type * well_rate = well_rate_safe_cast( arg );
  well_rate_free( well_rate );
}



sched_phase_enum well_rate_get_phase( const well_rate_type * well_rate ) {
  return well_rate->phase;
}


const char * well_rate_get_name( const well_rate_type * well_rate ) {
  return well_rate->name;
}


double_vector_type * well_rate_get_shift( well_rate_type * well_rate ) {
  return well_rate->shift;
}
