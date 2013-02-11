/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_wconhist.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_KW_WCONHIST_H__
#define __SCHED_KW_WCONHIST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>

#include <ert/util/type_macros.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/time_t_vector.h>

#include <ert/sched/sched_types.h>
#include <ert/sched/sched_macros.h> 

typedef  struct  sched_kw_wconhist_struct sched_kw_wconhist_type;
typedef  struct  wconhist_state_struct    wconhist_state_type;  

#define WCONHIST_DEFAULT_STATUS  OPEN

sched_kw_wconhist_type * sched_kw_wconhist_fscanf_alloc( FILE *, bool *, const char *);
void                     sched_kw_wconhist_free(sched_kw_wconhist_type * );
void                     sched_kw_wconhist_fprintf(const sched_kw_wconhist_type * , FILE *);
void                     sched_kw_wconhist_fwrite(const sched_kw_wconhist_type *, FILE *);
sched_kw_wconhist_type * sched_kw_wconhist_fread_alloc( FILE *);
hash_type              * sched_kw_wconhist_alloc_well_obs_hash(const sched_kw_wconhist_type *);
double                   sched_kw_wconhist_get_orat( sched_kw_wconhist_type * kw , const char * well_name);
void                     sched_kw_wconhist_scale_orat(  sched_kw_wconhist_type * kw , const char * well_name, double factor);
void                     sched_kw_wconhist_set_surface_flow(  sched_kw_wconhist_type * kw , const char * well_name , double orat);
bool                     sched_kw_wconhist_has_well( const sched_kw_wconhist_type * kw , const char * well_name);
bool                     sched_kw_wconhist_well_open( const sched_kw_wconhist_type * kw, const char * well_name);
void                     sched_kw_wconhist_shift_orat( sched_kw_wconhist_type * kw , const char * well_name, double shift_value);
void                     sched_kw_wconhist_shift_grat( sched_kw_wconhist_type * kw , const char * well_name, double shift_value);
void                     sched_kw_wconhist_shift_wrat( sched_kw_wconhist_type * kw , const char * well_name, double shift_value);
void                     sched_kw_wconhist_update_state(const sched_kw_wconhist_type * kw , wconhist_state_type * state , const char * well_name , int report_step );

void                     sched_kw_wconhist_init_well_list( const sched_kw_wconhist_type * kw , stringlist_type * well_list);



void                     wconhist_state_free__( void * arg );
wconhist_state_type    * wconhist_state_alloc( const time_t_vector_type * time);
void                     wconhist_state_free( wconhist_state_type * wconhist );

double         wconhist_state_iget_STAT( const void * state , int report_step );
  //well_status_enum         wconhist_state_iget_status( const void * state , int report_step );
well_cm_enum             wconhist_state_iget_WMCTLH( const void * state , int report_step );
double                   wconhist_state_iget_WBHPH( const void * state , int report_step );
double                   wconhist_state_iget_WOPRH( const void * state , int report_step );
double                   wconhist_state_iget_WGPRH( const void * state , int report_step );
double                   wconhist_state_iget_WWPRH( const void * state , int report_step );
double                   wconhist_state_iget_WWCTH(const void * state , int report_step );
double                   wconhist_state_iget_WGORH(const void * state , int report_step );
double                   wconhist_state_iget_WOPTH( const void * state , int report_step );
double                   wconhist_state_iget_WWPTH( const void * state , int report_step );
double                   wconhist_state_iget_WGPTH( const void * state , int report_step );

void                     sched_kw_wconhist_close_state(wconhist_state_type * state , int report_step );


UTIL_SAFE_CAST_HEADER( sched_kw_wconhist );
/*******************************************************************/



KW_HEADER(wconhist)

#ifdef __cplusplus
}
#endif
#endif
