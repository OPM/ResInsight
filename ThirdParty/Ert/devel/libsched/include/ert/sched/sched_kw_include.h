/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_include.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_KW_INCLUDE_H__
#define __SCHED_KW_INCLDUE_H__

#include <ert/util/stringlist.h>

#include <ert/sched/sched_macros.h>

typedef struct sched_kw_include_struct sched_kw_include_type;


sched_kw_include_type  * sched_kw_include_fscanf_alloc( FILE *, bool *, const char *);
void                     sched_kw_include_free(sched_kw_include_type * );
void                     sched_kw_include_fprintf(const sched_kw_include_type * , FILE *);
void                     sched_kw_include_fwrite(const sched_kw_include_type *, FILE *);
sched_kw_include_type  * sched_kw_include_fread_alloc( FILE *);

KW_HEADER(include)

#endif
