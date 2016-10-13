/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'timer.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/timer.h>

#ifdef __cplusplus
extern "C" {
#endif

struct timer_struct {
  size_t   count;

  clock_t  clock_start;
  time_t   epoch_start;
  double   sum1 , sum2;
  double   min_time , max_time;
  bool     running , epoch_time;
};



timer_type * timer_alloc(bool epoch_time) {
  timer_type *timer;
  timer       = util_malloc(sizeof * timer );
  
  timer->epoch_time = epoch_time;
  timer_reset(timer);
  return timer;
}


void timer_free(timer_type *timer) {
  free(timer);
}


void timer_start(timer_type *timer) {
  if (timer->running) 
    util_abort("%s: Timer already running. Use timer_stop() or timer_restart(). Aborting \n",__func__ );
  timer->running    = true;

  if (timer->epoch_time)
    time( &timer->epoch_start );
  else
    timer->clock_start = clock();
  
}


double timer_stop(timer_type *timer) {
  time_t  epoch_time;
  clock_t clock_time = clock();
  
  time(&epoch_time);
  if (timer->running) {
    double cpu_sec;
    if (timer->epoch_time)
      cpu_sec = 1.0 * (epoch_time - timer->epoch_start);
    else
      cpu_sec = 1.0 * (clock_time - timer->clock_start) / CLOCKS_PER_SEC;
    
    timer->count++;
    timer->sum1    += cpu_sec;
    timer->sum2    += cpu_sec * cpu_sec;
    timer->min_time = util_double_min( timer->min_time , cpu_sec);
    timer->max_time = util_double_max( timer->max_time , cpu_sec);
    timer->running  = false;

    return cpu_sec;
  } else 
    util_abort("%s: Timer is not running. Aborting \n",__func__ );
  
  return -1;
}



void timer_reset(timer_type *timer) {
  timer->count    = 0;
  timer->sum1     = 0.0;
  timer->sum2     = 0.0;
  timer->min_time =  99999999;
  timer->max_time = -99999999;
  timer->running  = false;
}






void timer_stats(const timer_type *timer , double *mean, double *std_dev) {
  *mean    = timer->sum1 / timer->count;
  *std_dev = sqrt(timer->sum2 / timer->count - (*mean) * (*mean));
}


double timer_get_total_time(const timer_type *timer) {
  return timer->sum1;
}


double timer_get_max_time(const timer_type *timer) {
  return timer->max_time;
}


double timer_get_min_time(const timer_type *timer) {
  return timer->min_time;
}

double timer_get_avg_time(const timer_type *timer) {
  return timer->sum1 / timer->count;
}

#ifdef __cplusplus
}
#endif



