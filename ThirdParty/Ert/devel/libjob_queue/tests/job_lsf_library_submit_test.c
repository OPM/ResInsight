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

#include <assert.h>
#include <ert/util/util.h>

#include <ert/job_queue/lsf_driver.h>
#include <ert/job_queue/lsf_job_stat.h>



void test_submit(lsf_driver_type * driver, const char * cmd) {
  assert( lsf_driver_set_option(driver , LSF_DEBUG_OUTPUT , "TRUE" ) );
  assert( LSF_SUBMIT_INTERNAL == lsf_driver_get_submit_method( driver ));
  {
    char * run_path = util_alloc_cwd();
    lsf_job_type * job = lsf_driver_submit_job( driver , cmd , 1 , run_path , "NAME" , 0 , NULL );
    assert( job );
    {
      {
        int lsf_status = lsf_driver_get_job_status_lsf( driver , job );
        assert( (lsf_status == JOB_STAT_RUN) || (lsf_status == JOB_STAT_PEND) );
      }

      lsf_driver_kill_job( driver , job );
      lsf_driver_set_bjobs_refresh_interval( driver , 0 );
      sleep(1);

      {
        int lsf_status = lsf_driver_get_job_status_lsf( driver , job );
        assert( lsf_status == JOB_STAT_EXIT);
      }
    }

    free( run_path );
  }
}



int main( int argc , char ** argv) {
  lsf_driver_type * driver = lsf_driver_alloc();
  test_submit(driver , argv[1]);
  lsf_driver_free( driver );
  exit(0);
}
