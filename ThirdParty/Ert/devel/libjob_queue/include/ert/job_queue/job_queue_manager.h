/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'job_queue.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __JOB_QUEUE_MANAGER_H__
#define __JOB_QUEUE_MANAGER_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>

#include <ert/job_queue/job_queue.h>

typedef struct job_queue_manager_struct job_queue_manager_type;

  job_queue_manager_type * job_queue_manager_alloc( job_queue_type * job_queue );
  void job_queue_manager_free( job_queue_manager_type * manager );
  void job_queue_manager_start_queue( job_queue_manager_type * manager , int num_total_run , bool verbose);
  void job_queue_manager_wait( job_queue_manager_type * manager);

  UTIL_IS_INSTANCE_HEADER( job_queue_manager );

#ifdef __cplusplus
}
#endif
#endif
