/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'timer.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_TIMER_H
#define ERT_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdlib.h>


typedef struct timer_struct timer_type;

  timer_type * timer_alloc( bool );
  void         timer_free(timer_type *);
  void         timer_start(timer_type *);
  double       timer_stop(timer_type *);
  void         timer_reset(timer_type *);

  double       timer_get_total_time(const timer_type *timer);
  double       timer_get_max_time(const timer_type *timer);
  double       timer_get_min_time(const timer_type *timer);
  double       timer_get_avg_time(const timer_type *timer);


#ifdef __cplusplus
}
#endif
#endif

