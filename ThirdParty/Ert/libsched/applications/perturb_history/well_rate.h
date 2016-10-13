/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'well_rate.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_WELL_RATE_H
#define ERT_WELL_RATE_H

#include <time_t_vector.h>
#include <sched_file.h>
#include <sched_kw_wconhist.h>
#include <sched_kw_wconinje.h>
#include <sched_types.h>
#include <sched_history.h>

typedef struct well_rate_struct well_rate_type;
well_rate_type     * well_rate_alloc(const sched_history_type * sched_history , 
                                     const time_t_vector_type * time_vector , 
                                     const char * name , double corr_length , const char * filename, sched_phase_enum phase, bool producer);
void                 well_rate_free__( void * arg );
double_vector_type * well_rate_get_shift( well_rate_type * well_rate );
sched_phase_enum     well_rate_get_phase( const well_rate_type * well_rate );
const char         * well_rate_get_name( const well_rate_type * well_rate );
void                 well_rate_sample_shift( well_rate_type * well_rate );
bool                 well_rate_well_open( const well_rate_type * well_rate , int index );
void                 well_rate_ishift( well_rate_type * well_rate  ,int index, double new_shift);
void                 well_rate_update_wconhist( well_rate_type * well_rate , sched_kw_wconhist_type * kw, int restart_nr );
void                 well_rate_update_wconinje( well_rate_type * well_rate , sched_kw_wconinje_type * kw, int restart_nr );
double               well_rate_iget_rate( const well_rate_type * well_rate , int report_step );
int                  well_rate_get_length( const well_rate_type * well_rate );
void                 well_rate_eval_stat( well_rate_type * well_rate );

#endif
