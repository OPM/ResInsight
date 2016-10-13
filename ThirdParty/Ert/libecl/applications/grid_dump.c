/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'grid_info.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/ecl/ecl_grid.h>




int main(int argc, char ** argv) {
  if (argc < 2) {
    fprintf(stderr,"%s: filename \n",argv[0]);
    exit(1);
  }

  {
    const char * grid_file = argv[1];
    char * output_file = NULL;
    ecl_grid_type * ecl_grid;
    FILE * stream;

    if (argc == 3) {
      output_file = argv[2];
      stream = util_mkdir_fopen( output_file , "wb");
    } else
      stream = stdout;
    
    ecl_grid = ecl_grid_alloc(grid_file);
    ecl_grid_dump( ecl_grid , stream );
    
    if (output_file != NULL)
      fclose( stream );
    ecl_grid_free( ecl_grid );
  }
}
