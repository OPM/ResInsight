/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_driver.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __LOCAL_DRIVER_H__
#define __LOCAL_DRIVER_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/job_queue/queue_driver.h>
  
  typedef struct local_driver_struct local_driver_type;
  typedef struct local_job_struct    local_job_type;


  void      * local_driver_alloc();


  void * local_driver_submit_job(void * __driver , 
                                 const char  * submit_cmd     , 
                                 int num_cpu ,   
                                 const char  * run_path       , 
                                 const char  * job_name ,
                                 int           argc,     
                                 const char ** argv );
  void            local_driver_kill_job(void * __driver , void * __job);
  void            local_driver_free__(void * __driver );
  job_status_type local_driver_get_job_status(void * __driver , void * __job);
  void            local_driver_free_job(void * __job);
  bool            local_driver_set_option( void * __driver , const char * option_key , const void * value);




#ifdef __cplusplus
}
#endif
#endif 
