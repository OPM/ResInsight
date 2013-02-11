/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_file.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_FILE_H__
#define __SCHED_FILE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <time.h>

#include <ert/util/time_t_vector.h>

#include <ert/sched/sched_kw.h>
#include <ert/sched/sched_types.h>

typedef struct sched_file_struct  sched_file_type;
typedef struct sched_block_struct sched_block_type;


typedef void (sched_file_callback_ftype)( void * sched_kw , int restart_nr , void * arg);

void                 sched_file_add_fixed_length_kw( sched_file_type * sched_file , const char * kw , int length );
sched_file_type *    sched_file_alloc(time_t);
sched_file_type *    sched_file_safe_cast(void * );
void                 sched_file_free(sched_file_type *);
void                 sched_file_parse(sched_file_type *, const char *);
void                 sched_file_parse_append(sched_file_type *  , const char * );
sched_file_type *    sched_file_parse_alloc(const char * , time_t);
void                 sched_file_fprintf_i(const sched_file_type *, int, const char *);
void                 sched_file_fprintf(const sched_file_type * sched_file, const char * file);

int                  sched_file_get_num_restart_files(const sched_file_type *);
int                  sched_file_get_restart_nr_from_time_t(const sched_file_type *, time_t);
int                  sched_file_get_restart_nr_from_days(const sched_file_type *  , double );
int                  sched_file_iget_block_size(const sched_file_type *, int);
int                  sched_file_time_t_to_restart_file(const sched_file_type *, time_t);

time_t               sched_file_iget_block_start_time(const sched_file_type *, int);
time_t               sched_file_iget_block_end_time(const sched_file_type *, int);
double               sched_file_iget_block_start_days(const sched_file_type * sched_file, int i);
double               sched_file_iget_block_end_days(const sched_file_type * sched_file, int i);
double               sched_file_get_sim_days(const sched_file_type * sched_file , int report_step);
time_t               sched_file_get_sim_time(const sched_file_type * sched_file , int report_step);

sched_kw_type *      sched_file_ijget_block_kw_ref(const sched_file_type *, int, int);
int                  sched_file_get_restart_file_from_time_t(const sched_file_type * , time_t );
void                 sched_file_summarize(const sched_file_type *  , FILE * );
sched_file_type    * sched_file_alloc_copy(const sched_file_type * , bool);
time_t_vector_type * sched_file_alloc_time_t_vector( const sched_file_type * sched_file );

void                 sched_file_update_blocks(sched_file_type * sched_file, int restart1 , int restart2 , sched_kw_type_enum kw_type, sched_file_callback_ftype * callback, void * arg);
void                 sched_file_update(sched_file_type * sched_file, sched_kw_type_enum kw_type, sched_file_callback_ftype * callback, void * arg);
bool                 sched_file_well_open( const sched_file_type * sched_file , int restart_nr , const char * well_name );
const char         * sched_file_iget_filename( const sched_file_type * sched_file , int file_nr );


int                sched_block_get_size(const sched_block_type * block);
sched_kw_type    * sched_block_iget_kw(sched_block_type * block, int i);
sched_block_type * sched_file_iget_block(const sched_file_type * sched_file, int i);



double sched_file_well_wconhist_rate( const sched_file_type * sched_file , 
                                      int restart_nr , 
                                      const char * well_name);

double sched_file_well_wconinje_rate( const sched_file_type * sched_file , 
                                      int restart_nr , 
                                      const char * well_name);


#ifdef __cplusplus
}
#endif
#endif
