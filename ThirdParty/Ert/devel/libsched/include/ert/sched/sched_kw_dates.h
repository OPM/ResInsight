/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_dates.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_KW_DATES__
#define __SCHED_KW_DATES__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <time.h>

#include <ert/util/stringlist.h>

#include <ert/sched/sched_macros.h>


typedef struct sched_kw_dates_struct sched_kw_dates_type;

sched_kw_dates_type   * sched_kw_dates_fscanf_alloc(FILE * , bool *, const char * );
void                    sched_kw_dates_fprintf(const sched_kw_dates_type * , FILE *);
void                    sched_kw_dates_free(sched_kw_dates_type * );
void                    sched_kw_dates_fwrite(const sched_kw_dates_type * , FILE * );
sched_kw_dates_type   * sched_kw_dates_fread_alloc(FILE * );

int                     sched_kw_dates_get_size(const sched_kw_dates_type *);
sched_kw_dates_type   * sched_kw_dates_alloc_from_time_t(time_t );
time_t                  sched_kw_dates_iget_date(const sched_kw_dates_type *, int);
/*******************************************************************/



KW_HEADER(dates)

#ifdef __cplusplus
}
#endif
#endif
