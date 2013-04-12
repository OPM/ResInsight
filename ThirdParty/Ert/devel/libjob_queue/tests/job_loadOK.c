/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'job_loadOK.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/config/config.h>

#include <ert/job_queue/workflow_job.h>



bool loadConfig(config_type * config , const char * config_file , config_type * config_compiler) {
  bool OK = false;
  workflow_job_type * cmd = workflow_job_config_alloc( "NAME" , config , config_file);
  
  if (cmd != NULL) {
    OK = true;
    workflow_job_update_config_compiler( cmd , config_compiler );
    workflow_job_free( cmd );
  } 
  
  return OK;
}



int main( int argc , char ** argv) {
  int status = 0;
  {
    config_type * config = workflow_job_alloc_config();
    config_type * config_compiler = config_alloc();
    int iarg;
    bool OK = true;

    for (iarg = 1; iarg < argc; iarg++) 
      OK = OK && loadConfig(  config , argv[iarg] , config_compiler); 
    
    if (!OK)
      status = 1;
    
    config_free(config_compiler);
    config_free(config);
  }
  exit( status );
}
