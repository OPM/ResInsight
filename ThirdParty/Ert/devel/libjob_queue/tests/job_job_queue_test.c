/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'job_queue_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>
#include <ert/util/thread_pool.h>
#include <ert/util/arg_pack.h>

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>

#include <ert/job_queue/job_queue.h>

void submit_jobs_to_queue(job_queue_type * queue, test_work_area_type * work_area, char * executable_to_run, int number_of_jobs, int number_of_slowjobs, char* sleep_short, char* sleep_long, bool multithreaded) {
  int submitted_slowjobs = 0;
  for (int i = 0; i < number_of_jobs; i++) {
    char * runpath = util_alloc_sprintf("%s/%s_%d", test_work_area_get_cwd(work_area), "job", i);
    util_make_path(runpath);

    char * sleeptime = sleep_short;
    if (submitted_slowjobs < number_of_slowjobs) {
      sleeptime = sleep_long;
      submitted_slowjobs++;
    }

    if (multithreaded) {

      job_queue_add_job_mt(queue, executable_to_run, NULL, NULL, NULL, NULL, 1, runpath, "Testjob", 2, (const char *[2]) {
        runpath, sleeptime
      });
    } else {

      job_queue_add_job_st(queue, executable_to_run, NULL, NULL, NULL, NULL, 1, runpath, "Testjob", 2, (const char *[2]) {
        runpath, sleeptime
      });
    }
    free(runpath);
  }
}

void monitor_job_queue(job_queue_type * queue, int max_job_duration, time_t stop_time, int min_realizations) {
  bool cont = true;

  if (0 >= min_realizations)
    cont = false;

  while (cont) {
    //Check if minimum number of realizations have run, and if so, kill the rest after a certain time
    if ((job_queue_get_num_complete(queue) >= min_realizations)) {
      job_queue_set_max_job_duration(queue, max_job_duration);

      job_queue_set_job_stop_time(queue, stop_time);

      cont = false;
    }

    if (cont) {
      util_usleep(100);
    }
  }
}

void run_jobs_with_time_limit_test(char * executable_to_run, int number_of_jobs, int number_of_slowjobs, char * sleep_short, char * sleep_long, int max_sleep) {
  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "ERROR");

  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);
  job_queue_set_max_job_duration(queue, max_sleep);

  submit_jobs_to_queue(queue, work_area, executable_to_run, number_of_jobs, number_of_slowjobs, sleep_short, sleep_long, false);

  job_queue_run_jobs(queue, number_of_jobs, true);

  test_assert_int_equal(number_of_jobs - number_of_slowjobs, job_queue_get_num_complete(queue));
  test_assert_int_equal(number_of_slowjobs, job_queue_get_num_killed(queue));

  test_assert_bool_equal(false, job_queue_get_open(queue));
  job_queue_reset(queue);
  test_assert_bool_equal(true, job_queue_get_open(queue));

  test_assert_int_equal(0, job_queue_get_num_complete(queue));

  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}


void run_and_monitor_jobs(char * executable_to_run, int max_job_duration, time_t stop_time, int min_realizations, int num_completed, int interval_between_jobs) {
  int number_of_jobs = 10;
  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  arg_pack_type * arg_pack = arg_pack_alloc();
  arg_pack_append_ptr(arg_pack, queue);
  arg_pack_append_int(arg_pack, 0);
  arg_pack_append_bool(arg_pack, true);

  thread_pool_type * pool = thread_pool_alloc(1, true);
  thread_pool_add_job(pool, job_queue_run_jobs__, arg_pack);

  int job_run_time = 0;

  for (int i = 0; i < number_of_jobs; i++) {
    char * runpath = util_alloc_sprintf("%s/%s_%d", test_work_area_get_cwd(work_area), "job", i);
    util_make_path(runpath);

    char * sleeptime = util_alloc_sprintf("%d", job_run_time);

    job_queue_add_job_mt(queue, executable_to_run, NULL, NULL, NULL, NULL, 1, runpath, "Testjob", 2, (const char *[2]) {
      runpath, sleeptime
    });
    job_run_time += interval_between_jobs;

    free(sleeptime);
    free(runpath);
  }

  job_queue_submit_complete(queue);

  monitor_job_queue(queue, max_job_duration, stop_time, min_realizations);

  thread_pool_join(pool);
  thread_pool_free(pool);


  test_assert_int_equal(num_completed, job_queue_get_num_complete(queue));
  test_assert_int_equal(number_of_jobs - num_completed, job_queue_get_num_killed(queue));
  test_assert_bool_equal(false, job_queue_get_open(queue));
  job_queue_reset(queue);
  test_assert_bool_equal(true, job_queue_get_open(queue));
  test_assert_int_equal(0, job_queue_get_num_complete(queue));

  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}

void run_jobs_time_limit_multithreaded(char * executable_to_run, int number_of_jobs, int number_of_slowjobs, char * sleep_short, char * sleep_long, int max_sleep) {
  test_work_area_type * work_area = test_work_area_alloc("job_queue");


  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);
  job_queue_set_max_job_duration(queue, max_sleep);

  arg_pack_type * arg_pack = arg_pack_alloc();
  arg_pack_append_ptr(arg_pack, queue);
  arg_pack_append_int(arg_pack, 0);
  arg_pack_append_bool(arg_pack, true);

  thread_pool_type * pool = thread_pool_alloc(1, true);
  thread_pool_add_job(pool, job_queue_run_jobs__, arg_pack);

  submit_jobs_to_queue(queue, work_area, executable_to_run, number_of_jobs, number_of_slowjobs, sleep_short, sleep_long, true);

  job_queue_submit_complete(queue);
  thread_pool_join(pool);
  thread_pool_free(pool);

  test_assert_int_equal(number_of_jobs - number_of_slowjobs, job_queue_get_num_complete(queue));
  test_assert_int_equal(number_of_slowjobs, job_queue_get_num_killed(queue));
  test_assert_bool_equal(false, job_queue_get_open(queue));
  job_queue_reset(queue);
  test_assert_bool_equal(true, job_queue_get_open(queue));
  test_assert_int_equal(0, job_queue_get_num_complete(queue));

  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}

void JobQueueRunJobs_ReuseQueue_AllOk(char ** argv) {
  printf("Running JobQueueRunJobs_ReuseQueue_AllOk\n");

  int number_of_jobs = 20;
  int number_of_queue_reuse = 10;

  test_work_area_type * work_area = test_work_area_alloc("job_queue");

  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  for (int j = 0; j < number_of_queue_reuse; j++) {
    submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, 0, "0", "0", false);

    job_queue_run_jobs(queue, number_of_jobs, true);

    test_assert_int_equal(number_of_jobs, job_queue_get_num_complete(queue));
    test_assert_bool_equal(false, job_queue_get_open(queue));
    job_queue_reset(queue);
    test_assert_bool_equal(true, job_queue_get_open(queue));
    test_assert_int_equal(0, job_queue_get_num_complete(queue));
  }
  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);

}

void JobQueueSetStopTime_StopTimeEarly_MinRealisationsAreRun(char ** argv) {
  printf("Running JobQueueSetStopTime_StopTimeEarly_MinRealisationsAreRun\n");

  //Use stop_time to to stop jobs after min_realizations are finished
  int min_realizations = 5;
  int num_expected_completed = 5;
  int max_duration_time = 0;
  int interval_between_jobs = 2;
  time_t currenttime;
  time(&currenttime);
  time_t stoptime = currenttime;
  run_and_monitor_jobs(argv[1], max_duration_time, stoptime, min_realizations, num_expected_completed, interval_between_jobs);

}

void JobQueueSetStopTime_StopTimeLate_AllRealisationsAreRun(char ** argv) {
  printf("Running JobQueueSetStopTime_StopTimeLate_AllRealisationsAreRun\n");

  //Use stop_time to to stop jobs after min_realizations are finished
  int min_realizations = 5;
  int num_expected_completed = 10;
  int max_duration_time = 0;
  int interval_between_jobs = 0;
  time_t currenttime;
  time(&currenttime);
  time_t stoptime = currenttime + 15;
  run_and_monitor_jobs(argv[1], max_duration_time, stoptime, min_realizations, num_expected_completed, interval_between_jobs);
}

void JobQueueSetStopTime_StopTimeMedium_MoreThanMinRealisationsAreRun(char ** argv) {
  printf("Running JobQueueSetStopTime_StopTimeMedium_MoreThanMinRealisationsAreRun\n");

  //Use stop_time to to stop jobs after min_realizations are finished
  int min_realizations = 1;
  int num_expected_completed = 2;
  int max_duration_time = 0;
  int interval_between_jobs = 3;
  time_t currenttime;
  time(&currenttime);
  time_t stoptime = currenttime + 5;
  run_and_monitor_jobs(argv[1], max_duration_time, stoptime, min_realizations, num_expected_completed, interval_between_jobs);

}

void JobQueueSetStopTimeAndMaxDuration_MaxDurationShort_StopTimeLate_MinRealisationsAreRun(char ** argv) {
  printf("Running JobQueueSetStopTimeAndMaxDuration_MaxDurationShort_StopTimeLong_MinRealisationsAreRun\n");

  int min_realizations = 1;
  int num_expected_completed = 1;
  int max_duration_time = 1;
  int interval_between_jobs = 2;
  time_t currenttime;
  time(&currenttime);
  time_t stoptime = currenttime + 10;
  run_and_monitor_jobs(argv[1], max_duration_time, stoptime, min_realizations, num_expected_completed, interval_between_jobs);

}

void JobQueueSetStopTimeAndMaxDuration_MaxDurationLong_StopTimeEarly_MinRealisationsAreRun(char ** argv) {
  printf("Running JobQueueSetStopTimeAndMaxDuration_MaxDurationLong_StopTimeEarly_MinRealisationsAreRun\n");

  int min_realizations = 1;
  int num_expected_completed = 1;
  int max_duration_time = 10;
  int interval_between_jobs = 2;
  time_t currenttime;
  time(&currenttime);
  time_t stoptime = currenttime + 1;
  run_and_monitor_jobs(argv[1], max_duration_time, stoptime, min_realizations, num_expected_completed, interval_between_jobs);
}

void JobQueueSetMaxDurationAfterMinRealizations_MaxDurationShort_OnlyMinRealizationsAreRun(char ** argv) {
  printf("Running JobQueueSetMaxDurationAfterMinRealizations_MaxDurationShort_OnlyMinRealizationsAreRun\n");

  // Must have one job completed, the rest are then killed due to the max_duration_time gets exceeded.
  int min_realizations = 1;
  int num_expected_completed = 1;
  int max_duration_time = 1;
  int interval_between_jobs = 2;
  time_t currenttime = 0;

  run_and_monitor_jobs(argv[1], max_duration_time, currenttime, min_realizations, num_expected_completed, interval_between_jobs);
}

void JobQueueSetMaxDurationAfterMinRealizations_MaxDurationLooong_AllRealizationsAreRun(char ** argv) {
  printf("Running JobQueueSetMaxDurationAfterMinRealizations_MaxDurationLooong_AllRealizationsAreRun\n");

  // Min realizations is 1, but the max running time exceeds the time used by any of the jobs, so all run to completion
  int min_realizations = 1;
  int num_expected_completed = 10;
  int max_duration_time = 12;
  int interval_between_jobs = 1;
  time_t currenttime = 0;
  run_and_monitor_jobs(argv[1], max_duration_time, currenttime, min_realizations, num_expected_completed, interval_between_jobs);
}

void JobQueueSetMaxDurationAfterMinRealizations_MaxDurationSemiLong_MoreThanMinRealizationsAreRun(char ** argv) {
  printf("Running JobQueueSetMaxDurationAfterMinRealizations_MaxDurationSemiLong_MoreThanMinRealizationsAreRun\n");

  // Min is 3, but max_duration_time allows for one more to be completed
  int min_realizations = 3;
  int num_expected_completed = 4;
  int max_duration_time = 7;
  int interval_between_jobs = 2;
  time_t currenttime = 0;
  run_and_monitor_jobs(argv[1], max_duration_time, currenttime, min_realizations, num_expected_completed, interval_between_jobs);
}

void JobQueueSetMaxDurationAfterMinRealizations_MaxDurationShortButMinRealizationsIsAll_AllRealizationsAreRun(char ** argv) {
  printf("Running JobQueueSetMaxDurationAfterMinRealizations_MaxDurationShortButMinRealizationsIsAll_AllRealizationsAreRun\n");

  // Min is 10, so all run to completion
  int min_realizations = 10;
  int num_expected_completed = 10;
  int max_duration_time = 1;
  int interval_between_jobs = 0;
  time_t currenttime = 0;
  run_and_monitor_jobs(argv[1], max_duration_time, currenttime, min_realizations, num_expected_completed, interval_between_jobs);
}

void JobQueueSetMaxDuration_DurationZero_AllRealisationsAreRun(char ** argv) {
  printf("Running JobQueueSetMaxDuration_DurationZero_AllRealisationsAreRun\n");
  run_jobs_with_time_limit_test(argv[1], 10, 0, "1", "100", 0); // 0 as limit means no limit*/
}

void JobQueueSetMaxDuration_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds(char ** argv) {
  printf("Running JobQueueSetMaxDuration_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds\n");
  run_jobs_with_time_limit_test(argv[1], 100, 23, "1", "100", 5);
}

void JobQueueSetMaxDurationRunJobsLoopInThread_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds(char ** argv) {
  printf("Running JobQueueSetMaxDurationRunJobsLoopInThread_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds\n");
  run_jobs_time_limit_multithreaded(argv[1], 100, 23, "1", "100", 5);
}

void JobQueueSetAutoStopTime_ThreeQuickJobs_AutoStopTimeKillsTheRest(char ** argv) {
  printf("Running JobQueueSetMaxDurationRunJobsLoopInThread_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds\n");

  int number_of_jobs = 10;

  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  arg_pack_type * arg_pack = arg_pack_alloc();
  arg_pack_append_ptr(arg_pack, queue);
  arg_pack_append_int(arg_pack, 0);
  arg_pack_append_bool(arg_pack, true);

  thread_pool_type * pool = thread_pool_alloc(1, true);
  thread_pool_add_job(pool, job_queue_run_jobs__, arg_pack);

  int number_of_slowjobs = 3;
  char * sleep_short = "0";
  char * sleep_long = "100";

  submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, number_of_slowjobs, sleep_short, sleep_long, false);

  util_usleep(1000000);
  job_queue_submit_complete(queue);
  job_queue_set_auto_job_stop_time(queue);
  thread_pool_join(pool);

  test_assert_int_equal(number_of_jobs - number_of_slowjobs, job_queue_get_num_complete(queue));
  test_assert_int_equal(number_of_slowjobs, job_queue_get_num_killed(queue));

  test_assert_bool_equal(false, job_queue_get_open(queue));
  job_queue_reset(queue);
  test_assert_bool_equal(true, job_queue_get_open(queue));

  test_assert_int_equal(0, job_queue_get_num_complete(queue));

  thread_pool_free(pool); 
  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}

void JobQueueSetAutoStopTime_NoJobsAreFinished_AutoStopDoesNothing(char ** argv) {
  printf("Running JobQueueSetAutoStopTime_NoJobsAreFinished_AutoStopDoesNothing\n");

  int number_of_jobs = 10;

  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  char * sleep_long = "100";

  submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, number_of_jobs, "0", sleep_long, false);

  job_queue_set_auto_job_stop_time(queue);

  test_assert_int_equal(0, job_queue_get_num_complete(queue));
  test_assert_bool_equal(true, job_queue_get_open(queue));

  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}

void JobQueueSetAutoStopTime_AllJobsAreFinished_AutoStopDoesNothing(char ** argv) {
  printf("Running JobQueueSetAutoStopTime_AllJobsAreFinished_AutoStopDoesNothing\n");

  int number_of_jobs = 10;
  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "ERROR");

  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, 0, "0", "0", false);

  job_queue_run_jobs(queue, number_of_jobs, true);

  test_assert_int_equal(number_of_jobs, job_queue_get_num_complete(queue));
  test_assert_bool_equal(false, job_queue_get_open(queue));
  job_queue_reset(queue);
  test_assert_bool_equal(true, job_queue_get_open(queue));
  test_assert_int_equal(0, job_queue_get_num_complete(queue));
  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}

int main(int argc, char ** argv) {
  JobQueueRunJobs_ReuseQueue_AllOk(argv);

  JobQueueSetMaxDuration_DurationZero_AllRealisationsAreRun(argv);
  JobQueueSetMaxDuration_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds(argv);
  JobQueueSetMaxDurationRunJobsLoopInThread_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds(argv);

  JobQueueSetMaxDurationAfterMinRealizations_MaxDurationShort_OnlyMinRealizationsAreRun(argv);
  JobQueueSetMaxDurationAfterMinRealizations_MaxDurationLooong_AllRealizationsAreRun(argv);
  JobQueueSetMaxDurationAfterMinRealizations_MaxDurationSemiLong_MoreThanMinRealizationsAreRun(argv);
  JobQueueSetMaxDurationAfterMinRealizations_MaxDurationShortButMinRealizationsIsAll_AllRealizationsAreRun(argv);

  JobQueueSetStopTime_StopTimeEarly_MinRealisationsAreRun(argv);
  JobQueueSetStopTime_StopTimeLate_AllRealisationsAreRun(argv);
  JobQueueSetStopTime_StopTimeMedium_MoreThanMinRealisationsAreRun(argv);

  JobQueueSetStopTimeAndMaxDuration_MaxDurationShort_StopTimeLate_MinRealisationsAreRun(argv);
  JobQueueSetStopTimeAndMaxDuration_MaxDurationLong_StopTimeEarly_MinRealisationsAreRun(argv);

  JobQueueSetAutoStopTime_ThreeQuickJobs_AutoStopTimeKillsTheRest(argv);
  JobQueueSetAutoStopTime_NoJobsAreFinished_AutoStopDoesNothing(argv);
  JobQueueSetAutoStopTime_AllJobsAreFinished_AutoStopDoesNothing(argv);

  exit(0);
}
