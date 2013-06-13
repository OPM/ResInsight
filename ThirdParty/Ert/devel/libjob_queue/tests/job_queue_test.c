/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
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

#include <lsf/lsbatch.h>

#include <ert/util/util.h>
#include <ert/util/test_util.h>

#include <ert/job_queue/lsf_driver.h>
#include <ert/job_queue/queue_driver.h>
#include <ert/job_queue/job_queue.h>

#include "ert/job_queue/torque_driver.h"

void job_queue_set_driver_(job_driver_type driver_type) {
  job_queue_type * queue = job_queue_alloc(10, "OK", "ERROR");
  queue_driver_type * driver = queue_driver_alloc(driver_type);
  test_assert_false(job_queue_has_driver(queue));

  job_queue_set_driver(queue, driver);
  test_assert_true(job_queue_has_driver(queue));

  job_queue_free(queue);
  queue_driver_free(driver);

}

void set_option_max_running_max_running_value_set() {
  queue_driver_type * driver_torque = queue_driver_alloc(TORQUE_DRIVER);
  test_assert_true(queue_driver_set_option(driver_torque, MAX_RUNNING, "42"));
  test_assert_string_equal("42", queue_driver_get_option(driver_torque, MAX_RUNNING));
  queue_driver_free(driver_torque);


  queue_driver_type * driver_lsf = queue_driver_alloc(LSF_DRIVER);
  test_assert_true(queue_driver_set_option(driver_lsf, MAX_RUNNING, "72"));
  test_assert_string_equal("72", queue_driver_get_option(driver_lsf, MAX_RUNNING));
  queue_driver_free(driver_lsf);
}

void set_option_max_running_max_running_option_set() {
  queue_driver_type * driver_torque = queue_driver_alloc(TORQUE_DRIVER);
  test_assert_true(queue_driver_set_option(driver_torque, MAX_RUNNING, "42"));
  test_assert_string_equal("42", queue_driver_get_option(driver_torque, MAX_RUNNING));
  queue_driver_free(driver_torque);

}

void set_option_invalid_option_returns_false() {
  queue_driver_type * driver_torque = queue_driver_alloc(TORQUE_DRIVER);
  test_assert_false(queue_driver_set_option(driver_torque, "MAKS_RUNNING", "42"));
  queue_driver_free(driver_torque);
}

void set_option_invalid_value_returns_false() {
  queue_driver_type * driver_torque = queue_driver_alloc(TORQUE_DRIVER);
  test_assert_false(queue_driver_set_option(driver_torque, "MAX_RUNNING", "2a"));
  queue_driver_free(driver_torque);
}

void set_option_valid_on_specific_driver_returns_true() {
  queue_driver_type * driver_torque = queue_driver_alloc(TORQUE_DRIVER);
  test_assert_true(queue_driver_set_option(driver_torque, TORQUE_NUM_CPUS_PER_NODE, "33"));
  test_assert_string_equal("33", queue_driver_get_option(driver_torque, TORQUE_NUM_CPUS_PER_NODE));
  queue_driver_free(driver_torque);
}

int main(int argc, char ** argv) {
  job_queue_set_driver_(LSF_DRIVER);
  job_queue_set_driver_(TORQUE_DRIVER);

  set_option_max_running_max_running_value_set();
  set_option_max_running_max_running_option_set();
  set_option_invalid_option_returns_false();
  set_option_invalid_value_returns_false();

  set_option_valid_on_specific_driver_returns_true();

  exit(0);
}
