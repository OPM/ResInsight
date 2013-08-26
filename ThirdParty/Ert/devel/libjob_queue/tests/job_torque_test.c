/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'job_lsf_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/test_util.h>
#include <ert/job_queue/torque_driver.h>

#include "ert/util/util.h"

void test_option(torque_driver_type * driver, const char * option, const char * value) {
  test_assert_true(torque_driver_set_option(driver, option, value));
  test_assert_string_equal(torque_driver_get_option(driver, option), value);
}

void setoption_setalloptions_optionsset() {
  torque_driver_type * driver = torque_driver_alloc();
  test_option(driver, TORQUE_QSUB_CMD, "XYZaaa");
  test_option(driver, TORQUE_QSTAT_CMD, "xyZfff");
  test_option(driver, TORQUE_QDEL_CMD, "ZZyfff");
  test_option(driver, TORQUE_QUEUE, "superhigh");
  test_option(driver, TORQUE_NUM_CPUS_PER_NODE, "42");
  test_option(driver, TORQUE_NUM_NODES, "36");
  test_option(driver, TORQUE_KEEP_QSUB_OUTPUT, "1");
  test_option(driver, TORQUE_KEEP_QSUB_OUTPUT, "0");

  printf("Options OK\n");
  torque_driver_free(driver);
}

void setoption_set_typed_options_wrong_format_returns_false() {
  torque_driver_type * driver = torque_driver_alloc();
  test_assert_false(torque_driver_set_option(driver, TORQUE_NUM_CPUS_PER_NODE, "42.2"));
  test_assert_false(torque_driver_set_option(driver, TORQUE_NUM_CPUS_PER_NODE, "fire"));
  test_assert_false(torque_driver_set_option(driver, TORQUE_NUM_NODES, "42.2"));
  test_assert_false(torque_driver_set_option(driver, TORQUE_NUM_NODES, "fire"));
  test_assert_true(torque_driver_set_option(driver, TORQUE_KEEP_QSUB_OUTPUT, "true"));
  test_assert_true(torque_driver_set_option(driver, TORQUE_KEEP_QSUB_OUTPUT, "1"));
  test_assert_false(torque_driver_set_option(driver, TORQUE_KEEP_QSUB_OUTPUT, "ja"));
  test_assert_false(torque_driver_set_option(driver, TORQUE_KEEP_QSUB_OUTPUT, "22"));
  test_assert_false(torque_driver_set_option(driver, TORQUE_KEEP_QSUB_OUTPUT, "1.1"));
}

void getoption_nooptionsset_defaultoptionsreturned() {
  torque_driver_type * driver = torque_driver_alloc();
  test_assert_string_equal(torque_driver_get_option(driver, TORQUE_QSUB_CMD), TORQUE_DEFAULT_QSUB_CMD);
  test_assert_string_equal(torque_driver_get_option(driver, TORQUE_QSTAT_CMD), TORQUE_DEFAULT_QSTAT_CMD);
  test_assert_string_equal(torque_driver_get_option(driver, TORQUE_QDEL_CMD), TORQUE_DEFAULT_QDEL_CMD);
  test_assert_string_equal(torque_driver_get_option(driver, TORQUE_KEEP_QSUB_OUTPUT), "0");
  test_assert_string_equal(torque_driver_get_option(driver, TORQUE_NUM_CPUS_PER_NODE), "1");
  test_assert_string_equal(torque_driver_get_option(driver, TORQUE_NUM_NODES), "1");

  printf("Default options OK\n");
  torque_driver_free(driver);
}

void create_submit_script_script_according_to_input() {
  char ** args = util_calloc(2, sizeof * args);
  args[0] = "/tmp/jaja/";
  args[1] = "number2arg";
  char * script_filename = util_alloc_filename("/tmp/", "qsub_script", "sh");
  torque_job_create_submit_script(script_filename, "job_program.py", 2, (const char **) args);
  printf("Create submit script OK\n");

  FILE* file_stream = util_fopen(script_filename, "r");
  bool at_eof = false;

  char * line = util_fscanf_alloc_line(file_stream, &at_eof);
  test_assert_string_equal("#!/bin/sh", line);
  free(line);

  line = util_fscanf_alloc_line(file_stream, &at_eof);
  test_assert_string_equal("job_program.py /tmp/jaja/ number2arg", line);
  free(line);

  line = util_fscanf_alloc_line(file_stream, &at_eof);
  free(line);
  test_assert_true(at_eof);

  fclose(file_stream);
}


int main(int argc, char ** argv) {
  getoption_nooptionsset_defaultoptionsreturned();
  setoption_setalloptions_optionsset();

  setoption_set_typed_options_wrong_format_returns_false();
  create_submit_script_script_according_to_input();
  exit(0);
}
