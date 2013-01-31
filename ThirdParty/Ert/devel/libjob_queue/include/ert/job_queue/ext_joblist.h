/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ext_joblist.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __EXT_JOBLIST_H__
#define __EXT_JOBLIST_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/subst_list.h>

#include <ert/job_queue/ext_job.h>


typedef struct ext_joblist_struct ext_joblist_type;

ext_joblist_type * ext_joblist_alloc();
void               ext_joblist_free(ext_joblist_type * );
void               ext_joblist_add_job(ext_joblist_type * joblist , const char * name , ext_job_type * new_job);
ext_job_type     * ext_joblist_get_job(const ext_joblist_type * , const char * );
ext_job_type     * ext_joblist_get_job_copy(const ext_joblist_type *  , const char * );
//void               ext_joblist_python_fprintf(const ext_joblist_type * , const stringlist_type * , const char * , const subst_list_type *);
bool               ext_joblist_has_job(const ext_joblist_type *  , const char * );
stringlist_type  * ext_joblist_alloc_list( const ext_joblist_type * joblist);
bool               ext_joblist_del_job( ext_joblist_type * joblist , const char * job_name );

#ifdef __cplusplus
}
#endif
#endif

