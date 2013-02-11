/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_plot_arg.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/double_vector.h>
#include <ert/util/time_t_vector.h>

#include <ert/enkf/enkf_plot_arg.h>


#define ENKF_PLOT_ARG_ID 6771861

struct enkf_plot_arg_struct {
  UTIL_TYPE_ID_DECLARATION;
  bool                 time_mode;
  bool                 days_valid;
  time_t               start_time;
  time_t_vector_type * time_vector;
  double_vector_type  * arg_vector; 
};

static UTIL_SAFE_CAST_FUNCTION( enkf_plot_arg , ENKF_PLOT_ARG_ID )




enkf_plot_arg_type * enkf_plot_arg_alloc( bool time_mode , time_t start_time) {
  enkf_plot_arg_type * plot_arg = util_malloc( sizeof * plot_arg);

  UTIL_TYPE_ID_INIT( plot_arg , ENKF_PLOT_ARG_ID );
  plot_arg->arg_vector = double_vector_alloc(0,0);
  plot_arg->time_vector = time_t_vector_alloc(0,0);
  enkf_plot_arg_reset( plot_arg , time_mode , start_time );
  return plot_arg;
}


void enkf_plot_arg_reset( enkf_plot_arg_type * plot_arg , bool time_mode , time_t start_time) {
  plot_arg->start_time = start_time;
  plot_arg->time_mode  = time_mode;
  plot_arg->days_valid = false;
  double_vector_reset( plot_arg->arg_vector );
  time_t_vector_reset( plot_arg->time_vector );
}


void enkf_plot_arg_free( enkf_plot_arg_type * plot_arg ) {
  time_t_vector_free( plot_arg->time_vector );
  double_vector_free( plot_arg->arg_vector );
}


void enkf_plot_arg_free__( void * arg ) {
  enkf_plot_arg_type * plot_arg = enkf_plot_arg_safe_cast( arg );
  enkf_plot_arg_free( plot_arg );
}



static void enkf_plot_arg_assert_time_mode( const enkf_plot_arg_type * plot_arg ) {
  if (!plot_arg->time_mode)
    util_abort("%s: trying to update plot_arg instance created with time_mode==false with time_t argument\n",__func__);
}


static void enkf_plot_arg_assert_not_time_mode( const enkf_plot_arg_type * plot_arg ) {
  if (plot_arg->time_mode)
    util_abort("%s: trying to update plot_arg instance created with time_mode==true with non-time argument\n",__func__);
}


void enkf_plot_arg_append_time( enkf_plot_arg_type * plot_arg , time_t time) {
  enkf_plot_arg_assert_time_mode( plot_arg );
  time_t_vector_append( plot_arg->time_vector , time );
  plot_arg->days_valid = false;
}


void enkf_plot_arg_append( enkf_plot_arg_type * plot_arg , double value) {
  enkf_plot_arg_assert_time_mode( plot_arg );
  double_vector_append( plot_arg->arg_vector , value );
}


/*****************************************************************/

static void enkf_plot_arg_assert_days( enkf_plot_arg_type * plot_arg ) {
  enkf_plot_arg_assert_time_mode( plot_arg );
  if (!plot_arg->days_valid) {
    int i;
    double_vector_reset( plot_arg->arg_vector );
    for (i=0; i < time_t_vector_size( plot_arg->time_vector ); i++) {
      time_t itime = time_t_vector_iget( plot_arg->time_vector , i);
      double days  = util_difftime_days( plot_arg->start_time , itime );
      double_vector_iset( plot_arg->arg_vector , i , days);
    }
    plot_arg->days_valid = true;
  }
}

const double_vector_type * enkf_plot_arg_get_vector( const enkf_plot_arg_type * plot_arg ) {
 enkf_plot_arg_assert_not_time_mode( plot_arg );
  return plot_arg->arg_vector;
}


const time_t_vector_type * enkf_plot_arg_get_time_t_vector( const enkf_plot_arg_type * plot_arg ) {
  enkf_plot_arg_assert_time_mode( plot_arg );
  return plot_arg->time_vector;
}

const double_vector_type * enkf_plot_arg_get_days_vector( enkf_plot_arg_type * plot_arg ) {
  enkf_plot_arg_assert_days( plot_arg );
  return plot_arg->arg_vector;
}


