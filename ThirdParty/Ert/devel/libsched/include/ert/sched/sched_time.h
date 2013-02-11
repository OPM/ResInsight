/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_time.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_TIME_H__
#define __SCHED_TIME_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <ert/sched/sched_types.h>

typedef struct sched_time_struct sched_time_type;

sched_time_type * sched_time_alloc( time_t date , double tstep_length , sched_time_enum  time_type );
void              sched_time_free( sched_time_type * time_node );
void              sched_time_free__( void * arg );
time_t            sched_time_get_date( const sched_time_type * time_node );
time_t            sched_time_get_type( const sched_time_type * time_node );
time_t            sched_time_get_target( const sched_time_type * time_node , time_t current_time);

#ifdef __cplusplus
}
#endif
#endif
