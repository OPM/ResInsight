/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'well_CF_dump.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <ert/ecl/ecl_grid.h>
#include <ert/ecl_well/well_state.h>
#include <ert/ecl_well/well_info.h>

/*
  This is a small test program which will load the well information in
  the same way as Resinsight does it.
*/

void usage() {
  printf("ri_well_test CASE.EGRID  CASE.UNRST / { CASE.X1 CASE.X2  ... CASE.Xn } \n");
  exit(1);
}


int main( int argc , char ** argv ) {
  if (argc < 3)
    usage();
  else {
    char * grid_file = argv[1];
    ecl_grid_type * grid = ecl_grid_alloc( grid_file );
    well_info_type * well_info = well_info_alloc( grid );
    int ifile;
    for (ifile = 2; ifile < argc; ifile++) {
      const char * rst_file = argv[ifile];
      printf("Loading restart file: %s \n",rst_file);
      well_info_load_rstfile( well_info , rst_file , true );
    }

    ecl_grid_free( grid );
    well_info_free( well_info );
  }
}
