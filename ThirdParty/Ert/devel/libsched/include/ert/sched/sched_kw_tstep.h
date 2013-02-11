/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_tstep.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_KW_TSTEP__
#define __SCHED_KW_TSTEP__
#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>

#include <ert/util/hash.h>

#include <ert/sched/sched_macros.h>

typedef struct sched_kw_tstep_struct sched_kw_tstep_type;

sched_kw_tstep_type * sched_kw_tstep_fscanf_alloc(FILE *, bool *, const char *);
void                  sched_kw_tstep_free(sched_kw_tstep_type * );
void                  sched_kw_tstep_fprintf(const sched_kw_tstep_type *, FILE *);
void                  sched_kw_tstep_fwrite(const sched_kw_tstep_type * , FILE *);
sched_kw_tstep_type * sched_kw_tstep_fread_alloc(FILE *);

int                   sched_kw_tstep_get_size(const sched_kw_tstep_type *);
sched_kw_tstep_type * sched_kw_tstep_alloc_from_double(double);
double                sched_kw_tstep_iget_step(const sched_kw_tstep_type *, int);
time_t                sched_kw_tstep_get_new_time(const sched_kw_tstep_type *, time_t);
int                   sched_kw_tstep_get_length( const sched_kw_tstep_type * kw);


/*******************************************************************/



KW_HEADER(tstep)
#ifdef __cplusplus
}
#endif
#endif
