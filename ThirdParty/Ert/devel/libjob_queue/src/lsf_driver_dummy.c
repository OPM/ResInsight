/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lsf_driver_dummy.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/job_queue/lsf_driver.h>


static void lsf_driver_invalid_usage( const char * func) {
  fprintf(stderr,"\n\n");
  fprintf(stderr,"-----------------------------------------------------------------\n");
  fprintf(stderr,"You have called the function %s() from the \n", func);
  fprintf(stderr,"LSF driver. The current lsf_driver is only a dummy driver, \n");
  fprintf(stderr,"and not capable of actually doing proper things. You must \n");
  fprintf(stderr,"select another driver to run your simulations.\n");
  fprintf(stderr,"\n");
  fprintf(stderr,"If your computing site has access to LSF the current version \n");
  fprintf(stderr,"of the LSF driver must be recompiled:   \n");
  fprintf(stderr,"\n");
  fprintf(stderr,"  1. Rebuild the lsf_driver with the preprossor symbol \'INCLUDE_LSF\' defined.\n");
  fprintf(stderr,"  2. Relink the whole application - with the libraries \'libbat\', \'libnsl\' and \'liblsf\'.\n");
  fprintf(stderr,"\n");
  fprintf(stderr,"-----------------------------------------------------------------\n");
  exit(1);
}

job_status_type lsf_driver_get_job_status(void * driver , void * job) {
  return 0;
}



void lsf_driver_free_job(void * job) {
  lsf_driver_invalid_usage( __func__ );
}



void lsf_driver_kill_job(void * driver , void * job) {
  lsf_driver_invalid_usage( __func__ );
}





void * lsf_driver_submit_job(void * driver , 
                             const char  * submit_cmd  , 
                             int           num_cpu     , 
                             const char  * run_path    , 
                             const char  * job_name    ,
                             int           argc        ,     
                             const char ** argv ) {
  lsf_driver_invalid_usage( __func__ );
  return NULL;
}



void lsf_driver_free(lsf_driver_type * driver ) {
  /* No op */
}

void lsf_driver_free__(void * driver ) {
  /* No op */
}


bool lsf_driver_set_option( void * driver , const char * option_key , const void * value) {
  return false;
}


const void * lsf_driver_get_option( const void * driver , const char * option_key) {
  return NULL;
}



bool lsf_driver_has_option( const void * driver , const char * option_key) {
  return false;
}

/*****************************************************************/



void * lsf_driver_alloc( ) {
  fprintf(stderr,"** Warning - this is an unfunctional LSF driver ** \n");
  return NULL;
}


/*****************************************************************/
