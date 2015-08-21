/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
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

#include <ert/util/util.h>
#include <ert/util/test_util.h>

#include <ert/job_queue/lsf_driver.h>
#include <ert/job_queue/lsf_job_stat.h>


void test_submit(lsf_driver_type * driver , const char * server , const char * bsub_cmd , const char * bjobs_cmd , const char * bkill_cmd , const char * cmd) {
  
  test_assert_true( lsf_driver_set_option(driver , LSF_DEBUG_OUTPUT , "TRUE" ) );
  test_assert_true( lsf_driver_set_option(driver , LSF_SERVER , server ) );
  
  if (bsub_cmd != NULL)
    test_assert_true( lsf_driver_set_option(driver , LSF_BSUB_CMD , server ));

  if (bjobs_cmd != NULL)
    test_assert_true( lsf_driver_set_option(driver , LSF_BJOBS_CMD , server ) );

  if (bkill_cmd != NULL)
    test_assert_true( lsf_driver_set_option(driver , LSF_BKILL_CMD , server ));
  
  {
    char * run_path = util_alloc_cwd();
    lsf_job_type * job = lsf_driver_submit_job( driver , cmd , 1 , run_path , "NAME" , 0 , NULL );
    if (job) {
      {
        int lsf_status = lsf_driver_get_job_status_lsf( driver , job );
        if (!((lsf_status == JOB_STAT_RUN) || (lsf_status == JOB_STAT_PEND))) 
          test_error_exit("Got lsf_status:%d expected: %d or %d \n",lsf_status , JOB_STAT_RUN , JOB_STAT_PEND);
      }
      
      lsf_driver_kill_job( driver , job );
      lsf_driver_set_bjobs_refresh_interval( driver , 0 );
      sleep(2);

      {
        int lsf_status = 0;
        for(int i=0; i < 10; i++){
          lsf_status = lsf_driver_get_job_status_lsf( driver , job );
          if (lsf_status != JOB_STAT_EXIT){
            sleep(2);
          }else{
            break;
          }
        }
        if (lsf_status != JOB_STAT_EXIT)
          test_error_exit("Got lsf_status:%d expected: %d \n",lsf_status , JOB_STAT_EXIT );
      }
    } else 
      test_error_exit("lsf_driver_submit_job() returned NULL \n");
    
    
    free( run_path );
  }
}


int main( int argc , char ** argv) {
  util_install_signals();
  {
    int iarg;
    lsf_driver_type * driver = lsf_driver_alloc();
    
    for (iarg = 2; iarg < argc; iarg++) {
      const char * server = argv[iarg];
      printf("Testing lsf server:%s \n",server);
      test_submit(driver , server , NULL , NULL , NULL , argv[1]);
    }
    
    lsf_driver_free( driver );
  }
  exit(0);
}
