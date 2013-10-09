/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'job_program.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <unistd.h>
#include <ert/util/util.h>
int main( int argc , char ** argv) {
  int sleep_time;
  util_sscanf_int(argv[2], &sleep_time);
  sleep(sleep_time);
  
  char * filename = util_alloc_filename(argv[1], "OK", "status");
  
  if (util_file_exists(argv[1])) {
    FILE * file = util_fopen(filename, "w");
    fprintf(file, "All good");
    util_fclose(file);
    exit(0);
  } else 
    exit(1); 
}
