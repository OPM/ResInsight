/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'workflow_joblist.c' is part of ERT - Ensemble based
   Reservoir Tool.
    
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

#include <stdbool.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <ert/util/hash.h>
#include <ert/util/int_vector.h>
#include <ert/util/util.h>
#include <ert/util/type_macros.h>

#include <ert/config/config.h>

#include <ert/job_queue/workflow_job.h>
#include <ert/job_queue/workflow_joblist.h>


struct workflow_joblist_struct {
  config_type * workflow_compiler;
  config_type * job_config;

  hash_type   * joblist;
};


workflow_joblist_type * workflow_joblist_alloc( ) {
  workflow_joblist_type * joblist = util_malloc( sizeof * joblist );

  joblist->job_config = workflow_job_alloc_config();
  joblist->workflow_compiler = config_alloc();
  joblist->joblist = hash_alloc();

  return joblist;
}


void workflow_joblist_free( workflow_joblist_type * joblist) {
  config_free( joblist->job_config );
  config_free( joblist->workflow_compiler );
  hash_free( joblist->joblist );
  free( joblist );
}


const workflow_job_type * workflow_joblist_get_job( const workflow_joblist_type * joblist , const char * job_name) {
  return hash_get( joblist->joblist , job_name );
}


void workflow_joblist_add_job( workflow_joblist_type * joblist , const workflow_job_type * job) {
  hash_insert_hash_owned_ref( joblist->joblist , workflow_job_get_name( job ) , job , workflow_job_free__ );
  workflow_job_update_config_compiler( job , joblist->workflow_compiler );
}


bool workflow_joblist_add_job_from_file( workflow_joblist_type * joblist , const char * job_name , const char * config_file ) {
  workflow_job_type * job = workflow_job_config_alloc( job_name , joblist->job_config , config_file );
  if (job != NULL) {
    workflow_joblist_add_job( joblist , job );
    return true;
  } else 
    return false;
}


config_type * workflow_joblist_get_compiler( const workflow_joblist_type * joblist ) {
  return joblist->workflow_compiler;
}
