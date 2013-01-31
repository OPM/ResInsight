/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'load_internal.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/job_queue/ext_cmd.h>



bool loadConfig(config_type * config , const char * config_file) {
  bool OK = false;
  ext_cmd_type * cmd = ext_cmd_config_alloc( config , config_file);
  
  if (cmd != NULL) {
    OK = true;
    ext_cmd_free( cmd );
  } 
  
  return OK;
}



int main( int argc , char ** argv) {
  int status = 0;
  {
    config_type * config = ext_cmd_alloc_config();
    int iarg;
    bool OK = true;

    for (iarg = 1; iarg < argc; iarg++) 
      OK = OK && loadConfig( config , argv[iarg]); 
    
    if (!OK)
      status = 1;
    
    config_free(config);
  }
  exit( status );
}
