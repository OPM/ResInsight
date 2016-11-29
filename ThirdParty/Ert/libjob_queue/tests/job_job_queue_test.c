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
#include <ert/job_queue/job_queue_manager.h>

void submit_jobs_to_queue(job_queue_type * queue, test_work_area_type * work_area, char * executable_to_run, int number_of_jobs, int number_of_slowjobs, char* sleep_short, char* sleep_long) {
  int submitted_slowjobs = 0;
  for (int i = 0; i < number_of_jobs; i++) {
    char * runpath = util_alloc_sprintf("%s/%s_%d", test_work_area_get_cwd(work_area), "job", i);
    util_make_path(runpath);

    char * sleeptime = sleep_short;
    if (submitted_slowjobs < number_of_slowjobs) {
      sleeptime = sleep_long;
      submitted_slowjobs++;
    }

    job_queue_add_job(queue, executable_to_run, NULL, NULL, NULL, NULL, 1, runpath, "Testjob", 2, (const char *[2]) {runpath, sleeptime});
    free(runpath);
  }
  test_assert_int_equal( number_of_jobs , job_queue_get_active_size(queue) );
}

void monitor_job_queue(job_queue_type * queue, int max_job_duration, time_t stop_time, int min_realizations) {
  if (min_realizations > 0) {
    while (true) {
      util_usleep(100);

      //Check if minimum number of realizations have run, and if so, kill the rest after a certain time
      if ((job_queue_get_num_complete(queue) >= min_realizations)) {
        job_queue_set_max_job_duration(queue, max_job_duration);
        job_queue_set_job_stop_time(queue, stop_time);
        break;
      }
    }
  }
}



void run_jobs_with_time_limit_test(char * executable_to_run, int number_of_jobs, int number_of_slowjobs, char * sleep_short, char * sleep_long, int max_sleep) {
  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "STATUS", "ERROR");

  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);
  job_queue_set_max_job_duration(queue, max_sleep);

  submit_jobs_to_queue(queue, work_area, executable_to_run, number_of_jobs, number_of_slowjobs, sleep_short, sleep_long);

  job_queue_run_jobs(queue, number_of_jobs, false);

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


void run_and_monitor_jobs(char * executable_to_run,
                          int number_of_jobs ,
                          int max_job_duration,
                          time_t stop_time,
                          int min_realizations,
                          int min_completed,
                          int max_completed ,
                          int interval_between_jobs) {

  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "STATUS", "ERROR");
  job_queue_manager_type * queue_manager = job_queue_manager_alloc( queue );
  queue_driver_type * driver = queue_driver_alloc_local();

  job_queue_set_driver(queue, driver);


  int job_run_time = 0;

  for (int i = 0; i < number_of_jobs; i++) {
    char * runpath = util_alloc_sprintf("%s/%s_%d", test_work_area_get_cwd(work_area), "job", i);
    char * sleeptime = util_alloc_sprintf("%d", job_run_time);

    util_make_path(runpath);
    job_queue_add_job(queue, executable_to_run, NULL, NULL, NULL, NULL, 1, runpath, "Testjob", 2, (const char *[2]) {runpath, sleeptime});
    job_run_time += interval_between_jobs;

    free(sleeptime);
    free(runpath);
  }
  job_queue_submit_complete(queue);
  job_queue_manager_start_queue(queue_manager,0,false,false);
  monitor_job_queue( queue , max_job_duration , stop_time , min_realizations );
  job_queue_manager_wait(queue_manager);

  printf("Completed: %d <= %d <= %d ?\n",min_completed , job_queue_get_num_complete(queue) , max_completed);
  test_assert_true(job_queue_get_num_complete(queue) >= min_completed);
  test_assert_true(job_queue_get_num_complete(queue) <= max_completed);

  test_assert_int_equal(number_of_jobs - job_queue_get_num_complete( queue ) , job_queue_get_num_killed(queue));
  test_assert_bool_equal(false, job_queue_get_open(queue));
  job_queue_reset(queue);
  test_assert_bool_equal(true, job_queue_get_open(queue));
  test_assert_int_equal(0, job_queue_get_num_complete(queue));

  job_queue_free(queue);
  queue_driver_free(driver);
  job_queue_manager_free( queue_manager );
  test_work_area_free(work_area);
}

void run_jobs_time_limit_multithreaded(char * executable_to_run, int number_of_jobs, int number_of_slowjobs, char * sleep_short, char * sleep_long, int max_sleep) {
  test_work_area_type * work_area = test_work_area_alloc("job_queue");


  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "STATUS", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);
  job_queue_set_max_job_duration(queue, max_sleep);

  arg_pack_type * arg_pack = arg_pack_alloc();
  arg_pack_append_ptr(arg_pack, queue);
  arg_pack_append_int(arg_pack, 0);
  arg_pack_append_bool(arg_pack, false);

  thread_pool_type * pool = thread_pool_alloc(1, true);
  thread_pool_add_job(pool, job_queue_run_jobs__, arg_pack);

  submit_jobs_to_queue(queue, work_area, executable_to_run, number_of_jobs, number_of_slowjobs, sleep_short, sleep_long);

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

void test1(char ** argv) {
  printf("001: Running JobQueueRunJobs_ReuseQueue_AllOk\n");

  int number_of_jobs = 20;
  int number_of_queue_reuse = 10;

  test_work_area_type * work_area = test_work_area_alloc("job_queue");

  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "STATUS", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  for (int j = 0; j < number_of_queue_reuse; j++) {
    submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, 0, "0", "0");

    job_queue_run_jobs(queue, number_of_jobs, false);

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

void test2(char ** argv) {
  printf("002: Running JobQueueRunJobs_ReuseQueueWithStopTime_AllOk\n");

  int number_of_jobs = 3;
  int number_of_slow_jobs = 2;
  int number_of_queue_reuse = 3;

  test_work_area_type * work_area = test_work_area_alloc("job_queue");

  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "STATUS", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  for (int j = 0; j < number_of_queue_reuse; j++) {
    submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, number_of_slow_jobs, "1", "5");

    job_queue_run_jobs(queue, number_of_jobs, false);
    time_t current_time = time(NULL);
    job_queue_set_job_stop_time(queue, current_time);

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


void test3(char ** argv) {
  printf("003: Running JobQueueSetStopTime_StopTimeEarly_MinRealisationsAreRun\n");

  //Use stop_time to to stop jobs after min_realizations are finished
  int number_of_jobs = 10;
  int min_realizations = 5;
  int num_expected_completed = 5;
  int max_duration_time = 0;
  int interval_between_jobs = 2;
  time_t stoptime = time( NULL );

  run_and_monitor_jobs(argv[1], number_of_jobs , max_duration_time, stoptime, min_realizations, num_expected_completed, num_expected_completed , interval_between_jobs);
}

void test4(char ** argv) {
  printf("004: Running JobQueueSetMaxDuration_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds\n");
  run_jobs_with_time_limit_test(argv[1], 100, 23, "1", "100", 5);
}


void test5(char ** argv) {
  printf("005: Running JobQueueSetStopTime_StopTimeLate_AllRealisationsAreRun\n");

  //Use stop_time to to stop jobs after min_realizations are finished
  int number_of_jobs = 10;
  int min_realizations = 5;
  int num_expected_completed = 10;
  int max_duration_time = 0;
  int interval_between_jobs = 0;
  time_t currenttime;
  time(&currenttime);
  time_t stoptime = currenttime + 15;
  run_and_monitor_jobs(argv[1], number_of_jobs , max_duration_time, stoptime, min_realizations, num_expected_completed, num_expected_completed , interval_between_jobs);
}

void test6(char ** argv) {
  printf("006: Running JobQueueSetStopTimeAndMaxDuration_MaxDurationShort_StopTimeLong_MinRealisationsAreRun\n");

  int number_of_jobs = 10;
  int min_realizations = 1;
  int num_expected_completed = 1;
  int max_duration_time = 1;
  int interval_between_jobs = 2;
  time_t currenttime;
  time(&currenttime);
  time_t stoptime = currenttime + 10;
  run_and_monitor_jobs(argv[1], number_of_jobs , max_duration_time, stoptime, min_realizations, num_expected_completed, number_of_jobs , interval_between_jobs);

}

void test7(char ** argv) {
  printf("007: Running JobQueueSetStopTimeAndMaxDuration_MaxDurationLong_StopTimeEarly_MinRealisationsAreRun\n");

  int number_of_jobs = 10;
  int min_realizations = 1;
  int num_expected_completed = 1;
  int max_duration_time = 10;
  int interval_between_jobs = 2;
  time_t currenttime;
  time(&currenttime);
  time_t stoptime = currenttime + 1;
  run_and_monitor_jobs(argv[1], number_of_jobs , max_duration_time, stoptime, min_realizations, num_expected_completed, num_expected_completed , interval_between_jobs);
}

void test8(char ** argv) {
  printf("008: Running JobQueueSetMaxDurationAfterMinRealizations_MaxDurationShort_OnlyMinRealizationsAreRun\n");

  // Must have one job completed, the rest are then killed due to the max_duration_time gets exceeded.
  int number_of_jobs = 10;
  int min_realizations = 1;
  int num_expected_completed = 1;
  int max_duration_time = 1;
  int interval_between_jobs = 2;
  time_t currenttime = 0;

  run_and_monitor_jobs(argv[1], number_of_jobs , max_duration_time, currenttime, min_realizations, num_expected_completed, 3 , interval_between_jobs);
}

void test9(char ** argv) {
  printf("009: Running JobQueueSetMaxDurationAfterMinRealizations_MaxDurationLooong_AllRealizationsAreRun\n");

  // Min realizations is 1, but the max running time exceeds the time used by any of the jobs, so all run to completion
  int number_of_jobs = 10;
  int min_realizations = 1;
  int num_expected_completed = 10;
  int max_duration_time = 12;
  int interval_between_jobs = 1;
  time_t currenttime = 0;
  run_and_monitor_jobs(argv[1], number_of_jobs , max_duration_time, currenttime, min_realizations, num_expected_completed, num_expected_completed , interval_between_jobs);
}


void test10(char ** argv) {
  printf("010: Running JobQueueSetMaxDurationAfterMinRealizations_MaxDurationSemiLong_MoreThanMinRealizationsAreRun\n");

  int number_of_jobs = 10;
  int min_realizations = 3;
  int max_duration_time = 7;
  int interval_between_jobs = 2;
  time_t currenttime = 0;
  run_and_monitor_jobs(argv[1], number_of_jobs , max_duration_time, currenttime, min_realizations, min_realizations , number_of_jobs , interval_between_jobs);
}

void test11(char ** argv) {
  printf("011: Running JobQueueSetMaxDurationAfterMinRealizations_MaxDurationShortButMinRealizationsIsAll_AllRealizationsAreRun\n");

  // Min is 10, so all run to completion
  int number_of_jobs = 10;
  int min_realizations = 10;
  int num_expected_completed = 10;
  int max_duration_time = 1;
  int interval_between_jobs = 0;
  time_t currenttime = 0;
  run_and_monitor_jobs(argv[1], number_of_jobs , max_duration_time, currenttime, min_realizations, num_expected_completed, num_expected_completed , interval_between_jobs);
}

void test12(char ** argv) {
  printf("012: Running JobQueueSetMaxDuration_DurationZero_AllRealisationsAreRun\n");
  run_jobs_with_time_limit_test(argv[1], 10, 0, "1", "100", 0); // 0 as limit means no limit*/
}

void test13(char ** argv) {
  printf("013: Running JobQueueSetMaxDurationRunJobsLoopInThread_Duration5Seconds_KillsAllJobsWithDurationMoreThan5Seconds\n");
  run_jobs_time_limit_multithreaded(argv[1], 100, 23, "1", "100", 5);
}

void test14(char ** argv) {
  printf("014: Running JobQueueSetAutoStopTime_ThreeQuickJobs_AutoStopTimeKillsTheRest\n");

  int number_of_jobs = 10;

  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "STATUS", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_manager_type * queue_manager = job_queue_manager_alloc( queue );
  job_queue_set_driver(queue, driver);


  int number_of_slowjobs = 7;
  int number_of_fastjobs = number_of_jobs - number_of_slowjobs;
  char * sleep_short = "0";
  char * sleep_long = "100";
  submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, number_of_slowjobs, sleep_short, sleep_long);
  job_queue_submit_complete(queue);
  job_queue_manager_start_queue( queue_manager , 10 , false , false);

  /*
    The jobs are distributed with some very fast, and some quite
    long. Here we busy wait until all the fast ones have completed and
    then we calculate a stop for the remaining jobs with the
    job_queue_set_auto_job_stop_time() function.
  */

  while (true) {
    int num_complete = job_queue_get_num_complete(queue);
    if (num_complete == number_of_fastjobs)
      break;
    util_usleep( 100000 );
  }

  job_queue_set_auto_job_stop_time(queue);
  job_queue_manager_wait(queue_manager);


  test_assert_int_equal(number_of_jobs - number_of_slowjobs, job_queue_get_num_complete(queue));
  test_assert_int_equal(number_of_slowjobs, job_queue_get_num_killed(queue));

  test_assert_bool_equal(false, job_queue_get_open(queue));
  job_queue_reset(queue);
  test_assert_bool_equal(true, job_queue_get_open(queue));

  test_assert_int_equal(0, job_queue_get_num_complete(queue));

  job_queue_manager_free( queue_manager );
  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}

void test15(char ** argv) {
  printf("015: Running JobQueueSetAutoStopTime_NoJobsAreFinished_AutoStopDoesNothing\n");

  int number_of_jobs = 10;

  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "STATUS", "ERROR");
  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  char * sleep_long = "100";

  submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, number_of_jobs, "0", sleep_long);

  job_queue_set_auto_job_stop_time(queue);

  test_assert_int_equal(0, job_queue_get_num_complete(queue));
  test_assert_bool_equal(true, job_queue_get_open(queue));

  job_queue_free(queue);
  queue_driver_free(driver);
  test_work_area_free(work_area);
}

void test16(char ** argv) {
  printf("016: Running JobQueueSetAutoStopTime_AllJobsAreFinished_AutoStopDoesNothing\n");

  int number_of_jobs = 10;
  test_work_area_type * work_area = test_work_area_alloc("job_queue");
  job_queue_type * queue = job_queue_alloc(number_of_jobs, "OK.status", "STATUS", "ERROR");

  queue_driver_type * driver = queue_driver_alloc_local();
  job_queue_set_driver(queue, driver);

  submit_jobs_to_queue(queue, work_area, argv[1], number_of_jobs, 0, "0", "0");

  job_queue_run_jobs(queue, number_of_jobs, false);

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
  util_install_signals();

  test1(argv);
  test2(argv);
  test3(argv);
  test4(argv);
  test5(argv);
  test6(argv);
  test7(argv);
  test8(argv);
  test9(argv);
  test10(argv);
  test11(argv);
  test12(argv);
  test13(argv);
  test14(argv);
  test15(argv);
  test16(argv);

  exit(0);
}
