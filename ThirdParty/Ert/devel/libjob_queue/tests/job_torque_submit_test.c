/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'job_torque_submit_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <unistd.h>

#include <ert/util/util.h>
#include <ert/util/test_util.h>

#include <ert/job_queue/torque_driver.h>

void test_submit(torque_driver_type * driver, const char * cmd) {
  char * run_path = util_alloc_cwd();
  torque_job_type * job = torque_driver_submit_job(driver, cmd, 1, run_path, "TEST-TORQUE-SUBMIT", 0, NULL);

  if (job != NULL) {
    int torque_status = torque_driver_get_job_status(driver, job);
    if (!((torque_status == JOB_QUEUE_RUNNING) || (torque_status == JOB_QUEUE_PENDING))) {
      test_exit("After start of job, the status is %d, it should have been JOB_QUEUE_RUNNING(%d) or JOB_QUEUE_PENDING (%d). Other statuses are JOB_QUEUE_EXIT(%d) and JOB_QUEUE_FAILED(%d)\n", torque_status, JOB_QUEUE_RUNNING, JOB_QUEUE_PENDING, JOB_QUEUE_EXIT, JOB_QUEUE_FAILED);
    }

    torque_driver_kill_job(driver, job);
    printf("Waiting 2 seconds");
    for (int i = 0; i < 2; i++) {
      printf(".");
      fflush(stdout);
      sleep(1);
    }
    printf("\n");

    torque_status = torque_driver_get_job_status(driver, job);
    if (torque_status != JOB_QUEUE_EXIT && torque_status != JOB_QUEUE_DONE) {
      exit(1);
      test_exit("After kill of job, the status is %d, it should have been JOB_QUEUE_EXIT, which is %d\n", torque_status, JOB_QUEUE_EXIT);
    }
  } else {
    exit(1);
    test_exit("Function %s returned null-pointer to job, terminating test.", "torque_driver_submit_job");
  }

  free(run_path);
  torque_driver_free_job(job);
}

void test_submit_nocommand(torque_driver_type * driver) {
  test_submit(driver, NULL);
}

int main(int argc, char ** argv) {
  torque_driver_type * driver = torque_driver_alloc();
  if (argc == 1) {
    test_submit_nocommand(driver);
  } else if (argc == 2) {
    test_submit(driver, argv[1]);
  } else {
    printf("Only accepts zero or one arguments (the job script to run)\n");
    exit(1);
  }
  printf("Submit, status and kill OK\n");
  torque_driver_free(driver);
  
  exit(0);
}
