/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'workflow_job.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __WORKFLOW_JOB_H__
#define __WORKFLOW_JOB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/config/config.h>

  
  typedef void (workflow_job_ftype) (void * self , const stringlist_type * arg );
  typedef struct workflow_job_struct workflow_job_type;
  
  const char   * workflow_job_get_name( const workflow_job_type * workflow_job );
  bool           workflow_job_internal( const workflow_job_type * workflow_job );
  config_type  * workflow_job_alloc_config();
  workflow_job_type * workflow_job_alloc(const char * name , bool internal);
  void           workflow_job_free( workflow_job_type * workflow_job );
  void           workflow_job_free__( void * arg);
  void           workflow_job_set_executable( workflow_job_type * workflow_job , const char * executable );
  workflow_job_type * workflow_job_config_alloc( const char * name , config_type * config , const char * config_file);

  void           workflow_job_update_config_compiler( const workflow_job_type * workflow_job , config_type * config_compiler );
  void           workflow_job_set_executable( workflow_job_type * workflow_job , const char * executable);
  void           workflow_job_set_function( workflow_job_type * workflow_job , const char * function);
  void           workflow_job_set_module( workflow_job_type * workflow_job , const char * module);
  void           workflow_job_run( const workflow_job_type * job , void * self , bool verbose , const stringlist_type * arg);
  
#ifdef __cplusplus
}
#endif

#endif
