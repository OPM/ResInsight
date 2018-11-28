/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'make_grid.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
  if (argc != 5) {
    fprintf(stderr,"%s: basename nx ny nz \n",argv[0]);
    exit(1);
  }

  {
    const char    * base_input = argv[1];
    int             nx         = atoi(argv[2]);
    int             ny         = atoi(argv[3]);
    int             nz         = atoi(argv[4]);

    char * path , *basename;
    ecl_grid_type * ecl_grid;
    
    util_alloc_file_components( base_input , &path , &basename , NULL );

    ecl_grid = ecl_grid_alloc_rectangular(nx,ny,nz , 1 ,1 ,1 , NULL );
    {
      char * EGRID_file = util_alloc_filename( path , basename , "EGRID");

      printf("Writing file: %s ...",EGRID_file); fflush(stdout);
      ecl_grid_fwrite_EGRID2( ecl_grid , EGRID_file, ECL_METRIC_UNITS);
      free( EGRID_file );
    }

    {
      char * grdecl_file = util_alloc_filename( path , basename , "grdecl");
      FILE * stream = util_fopen( grdecl_file , "w");
      printf("\nWriting file: %s ...",grdecl_file); fflush(stdout);
      ecl_grid_fprintf_grdecl( ecl_grid , stream );
      fclose( stream );
      free( grdecl_file );
      printf("\n");
    }
    
    free( basename );
    free( path );
    ecl_grid_free( ecl_grid );
  }
}
