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
#include <ert/util/test_work_area.h>
#include <ert/util/test_util.h>

#include <ert/job_queue/torque_driver.h>

void assert_status( torque_driver_type * driver, torque_job_type * job, int status_mask) {
  int torque_status = torque_driver_get_job_status(driver, job);
  if ((torque_status & status_mask) == 0)
    test_exit("Incorrect status:%d  - expected overlap with: %d\n" , torque_status , status_mask);
}



void test_submit(torque_driver_type * driver, const char * cmd) {
  char * run_path = util_alloc_cwd();
  torque_job_type * job = torque_driver_submit_job(driver, cmd, 1, run_path, "TEST-TORQUE-SUBMIT", 0, NULL);

  if (job != NULL) {
    assert_status( driver , job, JOB_QUEUE_RUNNING + JOB_QUEUE_PENDING);
    torque_driver_kill_job(driver, job);
    printf("Waiting 2 seconds");
    for (int i = 0; i < 2; i++) {
      printf(".");
      fflush(stdout);
      sleep(1);
    }
    printf("\n");

    int torque_status = torque_driver_get_job_status(driver, job);
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




void test_submit_failed_qstat(torque_driver_type * driver, const char * cmd) {
  char * run_path = util_alloc_cwd();
  torque_job_type * job = torque_driver_submit_job(driver, cmd, 1, run_path, "TEST-TORQUE-SUBMIT", 0, NULL);

  {
    test_work_area_type * work_area = test_work_area_alloc("torque-failed-qstat");
    test_work_area_copy_file( work_area , torque_driver_get_option( driver , TORQUE_QSTAT_CMD ));
    assert_status( driver , job , JOB_QUEUE_RUNNING + JOB_QUEUE_PENDING);

    {
      char * qstat_cmd = util_alloc_abs_path( "qstat.local" );
      FILE * stream = util_fopen( qstat_cmd , "w");
      fprintf(stream , "#!/bin/sh\n");
      fprintf(stream , "echo XYZ - Error\n");
      fclose( stream );
      util_addmode_if_owner(qstat_cmd, S_IXUSR );
      torque_driver_set_option(driver, TORQUE_QSTAT_CMD, qstat_cmd);
      free( qstat_cmd );
    }

    assert_status( driver , job , JOB_QUEUE_STATUS_FAILURE );
    test_work_area_free( work_area );
  }

  torque_driver_free_job(job);
  free(run_path);
}



int main(int argc, char ** argv) {
  torque_driver_type * driver = torque_driver_alloc();
  if (argc == 1) {
    test_submit_nocommand(driver);
  } else if (argc == 2) {
    test_submit(driver, argv[1]);
    test_submit_failed_qstat( driver , argv[1] );
  } else {
    printf("Only accepts zero or one arguments (the job script to run)\n");
    exit(1);
  }
  printf("Submit, status and kill OK\n");
  torque_driver_free(driver);

  exit(0);
}
