/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'torque_driver.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#ifndef TORQUE_DRIVER_H
#define	TORQUE_DRIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/job_queue/queue_driver.h>

  /*
    The options supported by the Torque driver.
   */
#define TORQUE_QSUB_CMD          "QSUB_CMD"
#define TORQUE_QSTAT_CMD         "QSTAT_CMD"
#define TORQUE_QDEL_CMD          "QDEL_CMD"
#define TORQUE_QUEUE             "QUEUE"
#define TORQUE_NUM_CPUS_PER_NODE "NUM_CPUS_PER_NODE"
#define TORQUE_NUM_NODES         "NUM_NODES"
#define TORQUE_KEEP_QSUB_OUTPUT  "KEEP_QSUB_OUTPUT"

#define TORQUE_DEFAULT_QSUB_CMD   "qsub"
#define TORQUE_DEFAULT_QSTAT_CMD  "qstat"
#define TORQUE_DEFAULT_QDEL_CMD  "qdel"


  typedef struct torque_driver_struct torque_driver_type;
  typedef struct torque_job_struct torque_job_type;


  void * torque_driver_alloc();


  void * torque_driver_submit_job(void * __driver,
          const char * submit_cmd,
          int num_cpu,
          const char * run_path,
          const char * job_name,
          int argc,
          const char ** argv);

  void torque_driver_kill_job(void * __driver, void * __job);
  void torque_driver_free__(void * __driver);
  void torque_driver_free(torque_driver_type * driver);
  job_status_type torque_driver_get_job_status(void * __driver, void * __job);
  void torque_driver_free_job(void * __job);
  void torque_driver_set_qstat_refresh_interval(torque_driver_type * driver, int refresh_interval);

  const void * torque_driver_get_option(const void * __driver, const char * option_key);
  bool torque_driver_set_option(void * __driver, const char * option_key, const void * value);
  void torque_driver_init_option_list(stringlist_type * option_list); 
  
  void torque_job_create_submit_script(const char * run_path, const char * submit_cmd, int argc, const char ** job_argv);
    
  UTIL_SAFE_CAST_HEADER(torque_driver);

#ifdef	__cplusplus
}
#endif

#endif	/* TORQUE_DRIVER_H */

