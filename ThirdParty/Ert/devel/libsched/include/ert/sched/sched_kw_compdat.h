/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_kw_compdat.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_KW_COMPDAT_H__
#define __SCHED_KW_COMPDAT_H__
#include <stdio.h>

#include <ert/util/set.h>
#include <ert/util/stringlist.h>

#include <ert/sched/sched_macros.h>

typedef struct sched_kw_compdat_struct sched_kw_compdat_type;

sched_kw_compdat_type * sched_kw_compdat_alloc();
void                    sched_kw_compdat_free(sched_kw_compdat_type * );
void                    sched_kw_compdat_fprintf(const sched_kw_compdat_type * , FILE *);
sched_kw_compdat_type * sched_kw_compdat_fread_alloc(FILE *stream);
void                    sched_kw_compdat_fwrite(const sched_kw_compdat_type * , FILE *stream);


KW_HEADER(compdat)


#endif
