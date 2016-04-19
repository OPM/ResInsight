/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'job_lsf_submit_test.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include "ert/util/build_config.h"

#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>
#include <ert/util/test_work_area.h>

#include <ert/job_queue/lsf_driver.h>


void test_empty_file() {
  const char * stdout_file = "bsub_empty";
  {
    FILE * stream = util_fopen(stdout_file , "w");
    fclose( stream );
  }
  test_assert_int_equal( lsf_job_parse_bsub_stdout("bsub" , stdout_file ) , 0);
}


void test_OK() {
  const char * stdout_file = "bsub_OK";
  {
    FILE * stream = util_fopen(stdout_file , "w");
    fprintf(stream , "Job <12345> is submitted to default queue <normal>.\n");
    fclose( stream );
  }
  test_assert_int_equal( lsf_job_parse_bsub_stdout("bsub" , stdout_file ) , 12345);
}


void test_file_does_not_exist() {
  test_assert_int_equal( lsf_job_parse_bsub_stdout("bsub" , "does/not/exist") , 0);
}



void parse_invalid( void * arg ) {
  const char * filename = (const char*) arg;
  lsf_job_parse_bsub_stdout("bsub" , filename);
}


void test_parse_fail_abort() {
  const char * stdout_file = "bsub_abort";
  {
    FILE * stream = util_fopen(stdout_file , "w");
    fprintf(stream , "Job 12345 is submitted to default queue <normal>.\n");
    fclose( stream );
  }
  test_assert_util_abort( "lsf_job_parse_bsub_stdout" , parse_invalid , (void *) stdout_file );
}


int main(int argc, char ** argv) {
  test_work_area_type * work_area = test_work_area_alloc( "bsub_parse_stdout");
  {
    test_empty_file();
    test_file_does_not_exist( );
    test_OK();
    test_parse_fail_abort();
  }
  test_work_area_free( work_area );
}


