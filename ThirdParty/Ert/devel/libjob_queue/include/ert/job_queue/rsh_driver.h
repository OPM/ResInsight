/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rsh_driver.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __RSH_DRIVER_H__
#define __RSH_DRIVER_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <ert/util/hash.h>

#define RSH_HOST           "RSH_HOST"
#define RSH_HOSTLIST       "RSH_HOSTLIST"
#define RSH_CMD            "RSH_CMD" 
#define RSH_CLEAR_HOSTLIST "RSH_CLEAR_HOSTLIST"

  typedef struct rsh_driver_struct rsh_driver_type;
  typedef struct rsh_job_struct    rsh_job_type;
  
  void        rsh_driver_add_host(rsh_driver_type * , const char * , int );
  void      * rsh_driver_alloc( );
  
  void * rsh_driver_submit_job(void * __driver , 
                               const char  * submit_cmd     , 
                               int           num_cpu        , 
                               const char  * run_path       , 
                               const char  * job_name       ,
                               int           argc,     
                               const char ** argv );
  void            rsh_driver_kill_job(void * __driver , void * __job);
  void            rsh_driver_free__(void * __driver );
  job_status_type rsh_driver_get_job_status(void * __driver , void * __job);
  void            rsh_driver_free_job(void * __job);
  
  
  bool         rsh_driver_set_option( void * __driver, const char * option_key , const void * value );
  const void * rsh_driver_get_option( const void * __driver , const char * option_key);
  void         rsh_driver_init_option_list(stringlist_type * option_list);
  
#ifdef __cplusplus
}
#endif
#endif 
