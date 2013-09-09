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

#include <lsf/lsbatch.h>

#include <ert/util/util.h>
#include <ert/util/test_util.h>

#include <ert/job_queue/lsf_driver.h>



void test_submit(lsf_driver_type * driver , const char * server , const char * bsub_cmd , const char * bjobs_cmd , const char * bkill_cmd , const char * cmd) {
  lsf_driver_set_option(driver , LSF_SERVER , server );
  
  if (bsub_cmd != NULL)
    lsf_driver_set_option(driver , LSF_BSUB_CMD , server );

  if (bjobs_cmd != NULL)
    lsf_driver_set_option(driver , LSF_BJOBS_CMD , server );

  if (bkill_cmd != NULL)
    lsf_driver_set_option(driver , LSF_BKILL_CMD , server );

  {
    char * run_path = util_alloc_cwd();
    lsf_job_type * job = lsf_driver_submit_job( driver , cmd , 1 , run_path , "NAME" , 0 , NULL );
    
    if (job != NULL) {
      {
        int lsf_status = lsf_driver_get_job_status_lsf( driver , job );
        if (!((lsf_status == JOB_STAT_RUN) || (lsf_status == JOB_STAT_PEND)))
          exit(1);
      }
      
      lsf_driver_kill_job( driver , job );
      lsf_driver_set_bjobs_refresh_interval( driver , 0 );
      sleep(1);

      {
        int lsf_status = lsf_driver_get_job_status_lsf( driver , job );
        if (lsf_status != JOB_STAT_EXIT)
          exit(1);
      }
    } else
      exit(1);
    
    free( run_path );
  }
}


int main( int argc , char ** argv) {
  lsf_driver_type * driver = lsf_driver_alloc();
  stringlist_type * server_list = stringlist_alloc_from_split( argv[2] , " ");
  int iarg;
  for (iarg = 0; iarg < stringlist_get_size( server_list ); iarg++) {
    const char * server = stringlist_iget( server_list , iarg );
    test_submit(driver , server , NULL , NULL , NULL , argv[1]);
  }
  stringlist_free( server_list );

  exit(0);
}
