/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'queue_driver.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdbool.h>

#include <ert/util/util.h>

#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/lsf_driver.h>
#include <ert/job_queue/local_driver.h>
#include <ert/job_queue/rsh_driver.h>
#include <ert/job_queue/torque_driver.h>


/**
   This file implements the datatype queue_driver_type which is an
   abstract datatype for communicating with a subsystem for
   communcating with other low-level systems for running external
   jobs. The job_queue instance, which will handle a queue of jobs,
   interacts with the jobs through a queue_driver instance.

   The queue_driver type is a quite small datastructure which "wraps"
   and underlying specific driver instance; examples of specific
   driver instances are the lsf_driver which communicates with the LSF
   system and the local_driver which runs jobs directly on the current
   workstation. The queue_driver type contains essentially three
   different types of fields:

    1. Functions pointers for manipulating the jobs, and the state of
       the low-level driver.

    2. An opaque (i.e. void *) pointer to the state of the low level
       driver. This will be passed as first argument to all the
       function pointers, e.g. like the "self" in Python methods.

    3. Some data fields which are common to all driver types.

 */

#define QUEUE_DRIVER_ID 86516032

struct queue_driver_struct {
  UTIL_TYPE_ID_DECLARATION;
  /* 
     Function pointers - pointing to low level functions in the implementations of
     e.g. lsf_driver.
   */
  submit_job_ftype * submit;
  free_job_ftype * free_job;
  kill_job_ftype * kill_job;
  get_status_ftype * get_status;
  free_queue_driver_ftype * free_driver;
  set_option_ftype * set_option;
  get_option_ftype * get_option;
  has_option_ftype * has_option;

  void * data; /* Driver specific data - passed as first argument to the driver functions above. */

  /*
    Generic data - common to all driver types.
   */
  char * name; /* String name of driver. */
  job_driver_type driver_type; /* Enum value for driver. */
  char * max_running_string;
  int max_running; /* Possible to maintain different max_running values for different
                                        drivers; the value 0 is interpreted as no limit - i.e. the queue layer
                                        will (try) to send an unlimited number of jobs to the driver. */

};

/**
   Observe that after the driver instance has been allocated it does
   NOT support modification of the common fields, only the data owned
   by the specific low level driver, i.e. the LSF data, can be
   modified runtime.

   The driver returned from the queue_driver_alloc_empty() function is
   NOT properly initialized and NOT ready for use.  
 */

static queue_driver_type * queue_driver_alloc_empty() {
  queue_driver_type * driver = util_malloc(sizeof * driver);
  UTIL_TYPE_ID_INIT(driver, QUEUE_DRIVER_ID);
  driver->max_running = 0;
  driver->driver_type = NULL_DRIVER;
  driver->submit = NULL;
  driver->get_status = NULL;
  driver->kill_job = NULL;
  driver->free_job = NULL;
  driver->free_driver = NULL;
  driver->get_option = NULL;
  driver->set_option = NULL;
  driver->has_option = NULL;
  driver->name = NULL;
  driver->data = NULL;
  driver->max_running_string = NULL;

  return driver;
}

static UTIL_SAFE_CAST_FUNCTION(queue_driver, QUEUE_DRIVER_ID)


/**
   The driver created in this function has all the function pointers
   correctly initialized; but no options have been set. I.e. unless
   the driver in question needs no options (e.g. the LOCAL driver) the
   returned driver will NOT be ready for use.
 */


queue_driver_type * queue_driver_alloc(job_driver_type type) {
  queue_driver_type * driver = queue_driver_alloc_empty();
  driver->driver_type = type;
  switch (type) {
    case LSF_DRIVER:
      driver->submit = lsf_driver_submit_job;
      driver->get_status = lsf_driver_get_job_status;
      driver->kill_job = lsf_driver_kill_job;
      driver->free_job = lsf_driver_free_job;
      driver->free_driver = lsf_driver_free__;
      driver->set_option = lsf_driver_set_option;
      driver->get_option = lsf_driver_get_option;
      driver->has_option = lsf_driver_has_option;
      driver->name = util_alloc_string_copy("LSF");
      driver->data = lsf_driver_alloc();
      break;
    case LOCAL_DRIVER:
      driver->submit = local_driver_submit_job;
      driver->get_status = local_driver_get_job_status;
      driver->kill_job = local_driver_kill_job;
      driver->free_job = local_driver_free_job;
      driver->free_driver = local_driver_free__;
      driver->name = util_alloc_string_copy("local");
      driver->data = local_driver_alloc();
      break;
    case RSH_DRIVER:
      driver->submit = rsh_driver_submit_job;
      driver->get_status = rsh_driver_get_job_status;
      driver->kill_job = rsh_driver_kill_job;
      driver->free_job = rsh_driver_free_job;
      driver->free_driver = rsh_driver_free__;
      driver->set_option = rsh_driver_set_option;
      driver->get_option = rsh_driver_get_option;
      driver->name = util_alloc_string_copy("RSH");
      driver->data = rsh_driver_alloc();
      break;
    case TORQUE_DRIVER:
      driver->submit = torque_driver_submit_job;
      driver->get_status = torque_driver_get_job_status;
      driver->kill_job = torque_driver_kill_job;
      driver->free_job = torque_driver_free_job;
      driver->free_driver = torque_driver_free__;
      driver->set_option = torque_driver_set_option;
      driver->get_option = torque_driver_get_option;
      driver->name = util_alloc_string_copy("TORQUE");
      driver->data = torque_driver_alloc();
      break;
    default:
      util_abort("%s: unrecognized driver type:%d \n", type);
  }
  return driver;
}

/*****************************************************************/


/*****************************************************************/

void queue_driver_set_max_running(queue_driver_type * driver, int max_running) {
  driver->max_running_string = util_realloc_sprintf(driver->max_running_string,"%d", max_running);
  driver->max_running = max_running;
}

int queue_driver_get_max_running(const queue_driver_type * driver) {
  return driver->max_running;
}

const char * queue_driver_get_name(const queue_driver_type * driver) {
  return driver->name;
}


/*****************************************************************/


static bool queue_driver_set_generic_option__(queue_driver_type * driver, const char * option_key, const void * value) {
  bool option_set = true;
  {
    if (strcmp(MAX_RUNNING, option_key) == 0) {
      int max_running_int = 0;
      if (util_sscanf_int(value, &max_running_int)) {
        queue_driver_set_max_running(driver, max_running_int);
        option_set = true;
      }
      else
        option_set = false;
    } else
      option_set = false;
  }
  return option_set;
}

static void * queue_driver_get_generic_option__(queue_driver_type * driver, const char * option_key) {
  if (strcmp(MAX_RUNNING, option_key) == 0) {
    return driver->max_running_string;
  } else {
    util_abort("%s: driver:%s does not support generic option %s\n", __func__, driver->name, option_key);
    return NULL;
  }
}

static bool queue_driver_has_generic_option__(queue_driver_type * driver, const char * option_key) {
  if (strcmp(MAX_RUNNING, option_key) == 0)
    return true;
  else
    return false;
}

/** 
   Set option - can also be used to perform actions - not only setting
   of parameters. There is no limit :-) 
 */
bool queue_driver_set_option(queue_driver_type * driver, const char * option_key, const void * value) {
  if (queue_driver_set_generic_option__(driver, option_key, value)) {
    return true;
  } else if (driver->set_option != NULL)
    /* The actual low level set functions can not fail! */
    return driver->set_option(driver->data, option_key, value);
  else {
    util_abort("%s: driver:%s does not support run time setting of options\n", __func__, driver->name);
    return false;
  }
  return false;
}

/*****************************************************************/

bool queue_driver_has_option(queue_driver_type * driver, const char * option_key) {
  if (driver->has_option != NULL)
    return driver->has_option(driver, option_key);
  else
    return false;
}

/*****************************************************************/

const void * queue_driver_get_option(queue_driver_type * driver, const char * option_key) {
  if (queue_driver_has_generic_option__(driver, option_key)) {
    return queue_driver_get_generic_option__(driver, option_key);
  } else if (driver->get_option != NULL)
    /* The actual low level set functions can not fail! */
    return driver->get_option(driver->data, option_key);
  else {
    util_abort("%s: driver:%s does not support run time reading of options\n", __func__, driver->name);
    return NULL;
  }
  return NULL;
}

/*****************************************************************/


queue_driver_type * queue_driver_alloc_LSF(const char * queue_name, const char * resource_request, const char * remote_lsf_server) {
  queue_driver_type * driver = queue_driver_alloc(LSF_DRIVER);

  queue_driver_set_option(driver, LSF_QUEUE, queue_name);
  queue_driver_set_option(driver, LSF_RESOURCE, resource_request);
  queue_driver_set_option(driver, LSF_SERVER, remote_lsf_server);

  return driver;
}

queue_driver_type * queue_driver_alloc_TORQUE() {
  queue_driver_type * driver = queue_driver_alloc(TORQUE_DRIVER);
  return driver;
}

queue_driver_type * queue_driver_alloc_RSH(const char * rsh_cmd, const hash_type * rsh_hostlist) {
  queue_driver_type * driver = queue_driver_alloc(RSH_DRIVER);

  queue_driver_set_option(driver, RSH_HOSTLIST, rsh_hostlist);
  queue_driver_set_option(driver, RSH_CMD, rsh_cmd);

  return driver;
}

queue_driver_type * queue_driver_alloc_local() {
  queue_driver_type * driver = queue_driver_alloc(LOCAL_DRIVER);

  /* No options set for the local driver. */

  return driver;
}

/* These are the functions used by the job_queue layer. */

void * queue_driver_submit_job(queue_driver_type * driver, const char * run_cmd, int num_cpu, const char * run_path, const char * job_name, int argc, const char ** argv) {
  return driver->submit(driver->data, run_cmd, num_cpu, run_path, job_name, argc, argv);
}

void queue_driver_free_job(queue_driver_type * driver, void * job_data) {
  driver->free_job(job_data);
}

void queue_driver_kill_job(queue_driver_type * driver, void * job_data) {
  driver->kill_job(driver->data, job_data);
}

job_status_type queue_driver_get_status(queue_driver_type * driver, void * job_data) {
  job_status_type status = driver->get_status(driver->data, job_data);
  return status;
}

void queue_driver_free_driver(queue_driver_type * driver) {
  driver->free_driver(driver->data);
}

/*****************************************************************/

void queue_driver_free(queue_driver_type * driver) {
  queue_driver_free_driver(driver);
  util_safe_free(driver->name);
  util_safe_free(driver->max_running_string);
  free(driver);
}

void queue_driver_free__(void * driver) {
  queue_driver_type * queue_driver = queue_driver_safe_cast(driver);
  queue_driver_free(queue_driver);
}


/*****************************************************************/

/* Small functions to support enum introspection. */

const char * queue_driver_type_enum_iget(int index, int * value) {

  return util_enum_iget(index, JOB_DRIVER_ENUM_SIZE, (const util_enum_element_type []) {
    JOB_DRIVER_ENUM_DEFS
  }, value);
}

const char * queue_driver_status_emun_iget(int index, int * value) {

  return util_enum_iget(index, JOB_STATUS_ENUM_SIZE, (const util_enum_element_type []) {
    JOB_STATUS_ENUM_DEFS
  }, value);
}
