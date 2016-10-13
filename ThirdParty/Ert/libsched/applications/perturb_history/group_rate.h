/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'group_rate.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_GROUP_RATE_H
#define ERT_GROUP_RATE_H

#include <time_t_vector.h>
#include <well_rate.h>
#include <sched_types.h>
#include <sched_history.h>

typedef struct group_rate_struct group_rate_type;
group_rate_type     * group_rate_alloc(const sched_history_type * sched_history , const time_t_vector_type * time_vector , const char * name , const char * phase, const char * type_string , const char * filename);
void                  group_rate_free__( void * arg );
void                  group_rate_add_well_rate( group_rate_type * group_rate , well_rate_type * well_rate);
sched_phase_enum      group_rate_get_phase( const group_rate_type * group_rate );
void                  group_rate_sample( group_rate_type * group_rate );
void                  group_rate_update_wconhist( group_rate_type * group_rate , sched_kw_wconhist_type * kw, int restart_nr );
void                  group_rate_update_wconinje( group_rate_type * group_rate , sched_kw_wconinje_type * kw, int restart_nr );
bool                  group_rate_is_producer( const group_rate_type * group_rate );
void                  group_rate_init( group_rate_type * group_rate );

#endif
