/*
 Copyright (C) 2016  Statoil ASA, Norway.

 This file  is part of ERT - Ensemble based Reservoir Tool.

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
#include <time.h>
#include <unistd.h>

#include <ert/util/arg_pack.h>
#include <ert/util/rng.h>
#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/thread_pool.h>
#include <ert/util/type_macros.h>
#include <ert/util/util.h>

#include <ert/job_queue/job_queue.h>
#include <ert/job_queue/job_queue_manager.h>
#include <ert/job_queue/queue_driver.h>


#define JOB_TYPE_ID 77539
typedef struct
{
  UTIL_TYPE_ID_DECLARATION;
  char * run_path;
  bool callback_run;
  int queue_index;
  int submit_usleep;
  int callback_usleep;
  int run_usleep;
  int argc;
  char ** argv;
  const char * cmd;
} job_type;

UTIL_SAFE_CAST_FUNCTION(job, JOB_TYPE_ID)

job_type * alloc_job(int ind, const char * cmd) {
  job_type * job = util_malloc(sizeof *job);
  UTIL_TYPE_ID_INIT(job, JOB_TYPE_ID)
  job->callback_run    = false;
  job->queue_index     = -1;
  job->submit_usleep   = 0;
  job->callback_usleep = 0;
  job->run_usleep      = 2 * 1000*1000; // 4 sec
  job->run_path        = util_alloc_sprintf("timeout_test_%d", ind);
  job->cmd             = cmd;
  job->argc            = 4;

  job->argv    = util_malloc(4 * sizeof *job->argv);
  job->argv[0] = job->run_path;
  job->argv[1] = "RUNNING";
  job->argv[2] = "OK";
  job->argv[3] = util_alloc_sprintf("%d", job->run_usleep);

  util_make_path(job->run_path);
  return job;
}

job_type ** alloc_jobs(int num_jobs, const char * cmd) {
  job_type ** jobs = util_malloc(num_jobs * sizeof *jobs);
  for (int i = 0; i < num_jobs; i++) {
    job_type * job = alloc_job(i, cmd);
    job_safe_cast(job);
    jobs[i] = job;
  }
  return jobs;
}


void submit_jobs(job_queue_type * queue, int num_jobs, job_type ** jobs) {
  for (int i = 0; i < num_jobs; i++) {
    job_type * job = jobs[i];

    job->queue_index = job_queue_add_job(queue, job->cmd, NULL, NULL, NULL, job, 1, job->run_path, job->run_path,
            job->argc, (const char **) job->argv);
  }
}

void check_jobs(int num_jobs, job_type ** jobs) {
  for (int i = 0; i < num_jobs; i++) {
    job_type * job = jobs[i];
    if (!job->callback_run)
      fprintf(stderr, "The callback has not been registered on job:%d/%d \n", i, job->queue_index);
    test_assert_true(job->callback_run);
  }
}

int main(int argc, char ** argv) {
  setbuf(stdout, NULL);

  const int number_of_jobs = 1;
  util_alloc_abs_path(argv[1]);

  const int running_timeout = 0;
  const int sec = 1000*1000;

  test_work_area_type * work_area = test_work_area_alloc("job_timeout");
  test_work_area_set_store(work_area, true);

  job_type **jobs = alloc_jobs(number_of_jobs, argv[1]);

  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK", "DOES_NOT_EXIST", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_manager_type * queue_manager = job_queue_manager_alloc(queue);

  util_install_signals();
  job_queue_set_driver(queue, driver);
  job_queue_manager_start_queue(queue_manager, number_of_jobs, false, true);

  {
    submit_jobs(queue, number_of_jobs, jobs);

    if (job_queue_get_active_size(queue) > 0) {
      job_queue_iset_max_confirm_wait_time(queue, 0, running_timeout); // job 0
    } else {
      util_exit("Job failed to be queued!\n");
    }

    usleep(1 * sec); // 1.0 sec
    int job_status = job_queue_iget_job_status(queue, 0);

    if (job_status != JOB_QUEUE_IS_KILLED) {
      util_exit("Job should have been killed, had status %d != %d\n", job_status, JOB_QUEUE_IS_KILLED);
    }
  }
  if (!job_queue_manager_try_wait(queue_manager, 5 * sec))
    util_exit("job_queue never completed \n");
  job_queue_manager_free(queue_manager);
  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}
