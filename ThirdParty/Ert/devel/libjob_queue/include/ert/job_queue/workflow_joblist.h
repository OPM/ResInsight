/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'workflow_joblist.h' is part of ERT - Ensemble based
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



#ifndef __WORKFLOW_JOBLIST_H__
#define __WORKFLOW_JOBLIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/job_queue/workflow_job.h>

typedef struct workflow_joblist_struct workflow_joblist_type;

  workflow_joblist_type   * workflow_joblist_alloc();
  void                      workflow_joblist_free( workflow_joblist_type * joblist);
  const workflow_job_type * workflow_joblist_get_job( const workflow_joblist_type * joblist , const char * job_name);
  void                      workflow_joblist_add_job( workflow_joblist_type * joblist , const workflow_job_type * job);
  bool                      workflow_joblist_add_job_from_file( workflow_joblist_type * joblist , const char * job_name , const char * config_file );
  config_type             * workflow_joblist_get_compiler( const workflow_joblist_type * joblist );
  config_type             * workflow_joblist_get_job_config( const workflow_joblist_type * joblist );
  
#ifdef __cplusplus
}
#endif
#endif
