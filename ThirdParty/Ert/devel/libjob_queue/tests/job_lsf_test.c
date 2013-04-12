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
#include <stdbool.h>

#include <lsf/lsbatch.h>

#include <ert/util/test_util.h>

#include <ert/job_queue/lsf_driver.h>
#include <ert/job_queue/lsb.h>


void test_option(lsf_driver_type * driver , const char * option , const char * value) {
  test_assert_true( lsf_driver_set_option( driver , option , value));
  test_assert_string_equal(lsf_driver_get_option( driver , option) , value);
}


void test_server(lsf_driver_type * driver , const char * server, lsf_submit_method_enum submit_method) {
  lsf_driver_set_option(driver , LSF_SERVER , server );
  test_assert_true( lsf_driver_get_submit_method( driver ) == submit_method );
}


void test_status(int lsf_status , job_status_type job_status) {
  test_assert_true( lsf_driver_convert_status( lsf_status ) == job_status);
}


/*
  This test should ideally be run twice in two different environments;
  with and without dlopen() access to the lsf libraries.
*/

int main( int argc , char ** argv) {
  lsf_driver_type * driver = lsf_driver_alloc();

  test_option( driver , LSF_BSUB_CMD    , "Xbsub");
  test_option( driver , LSF_BJOBS_CMD   , "Xbsub");
  test_option( driver , LSF_BKILL_CMD   , "Xbsub");
  test_option( driver , LSF_RSH_CMD     , "RSH");
  test_option( driver , LSF_LOGIN_SHELL , "shell");
  test_option( driver , LSF_BSUB_CMD    , "bsub");
  printf("Options OK\n");

  {
    
    lsf_submit_method_enum submit_NULL;
    lsb_type * lsb = lsb_alloc();
    if (lsb_ready(lsb))
      submit_NULL = LSF_SUBMIT_INTERNAL;
    else
      submit_NULL = LSF_SUBMIT_LOCAL_SHELL;
    

    test_server( driver , NULL        , submit_NULL ); 
    test_server( driver , "LoCaL"     , LSF_SUBMIT_LOCAL_SHELL ); 
    test_server( driver , "LOCAL"     , LSF_SUBMIT_LOCAL_SHELL ); 
    test_server( driver , "XLOCAL"    , LSF_SUBMIT_REMOTE_SHELL );
    test_server( driver , NULL        , submit_NULL ); 
    test_server( driver , "NULL"      , submit_NULL ); 
    test_server( driver , "be-grid01" , LSF_SUBMIT_REMOTE_SHELL );
    printf("Servers OK\n");

    lsb_free( lsb );
  }
  test_status( JOB_STAT_PEND   , JOB_QUEUE_PENDING );
  test_status( JOB_STAT_PSUSP  , JOB_QUEUE_RUNNING );
  test_status( JOB_STAT_USUSP  , JOB_QUEUE_RUNNING );
  test_status( JOB_STAT_SSUSP  , JOB_QUEUE_RUNNING );
  test_status( JOB_STAT_RUN    , JOB_QUEUE_RUNNING );
  test_status( JOB_STAT_NULL   , JOB_QUEUE_NOT_ACTIVE );
  test_status( JOB_STAT_DONE   , JOB_QUEUE_DONE );
  test_status( JOB_STAT_EXIT   , JOB_QUEUE_EXIT );
  test_status( JOB_STAT_UNKWN  , JOB_QUEUE_EXIT );
  test_status( 192             , JOB_QUEUE_DONE ); 
  printf("Status OK \n");

  exit(0);
}
