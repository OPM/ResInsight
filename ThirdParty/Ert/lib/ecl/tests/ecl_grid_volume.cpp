/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_grid_volume.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <signal.h>
#include <math.h>

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>

#include <ert/ecl/ecl_grid.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_kw.hpp>





int main(int argc , char ** argv) {
  const char * path_case = argv[1];
  char * grid_file = ecl_util_alloc_filename( NULL , path_case , ECL_EGRID_FILE , false , 0 );
  char * init_file = ecl_util_alloc_filename( NULL , path_case , ECL_INIT_FILE , false , 0 );

  ecl_file_type * init = ecl_file_open( init_file , 0 );
  ecl_grid_type * grid = ecl_grid_alloc( grid_file );
  const ecl_kw_type * poro_kw = ecl_file_iget_named_kw( init , "PORO" , 0 );
  const ecl_kw_type * porv_kw = ecl_file_iget_named_kw( init , "PORV" , 0 );
  ecl_kw_type * multpv = NULL;
  ecl_kw_type * NTG = NULL;
  bool error_found = false;

  double total_volume = 0;
  double total_diff = 0;
  int error_count = 0;
  int iactive;

  if (ecl_file_has_kw( init , "NTG"))
    NTG = ecl_file_iget_named_kw( init , "NTG" , 0);

  if (ecl_file_has_kw( init , "MULTPV"))
    multpv = ecl_file_iget_named_kw( init , "MULTPV" , 0);

  for (iactive = 0; iactive < ecl_grid_get_nactive( grid ); ++iactive) {
    int iglobal = ecl_grid_get_global_index1A( grid , iactive );
    double grid_volume = ecl_grid_get_cell_volume1( grid , iglobal );
    double eclipse_volume = ecl_kw_iget_float( porv_kw , iglobal ) / ecl_kw_iget_float( poro_kw , iactive );

    if (NTG)
      eclipse_volume /= ecl_kw_iget_float( NTG , iactive );

    if (multpv)
      eclipse_volume *= ecl_kw_iget_float( multpv , iactive);

    total_volume += grid_volume;
    total_diff += fabs( eclipse_volume - grid_volume );
    if (!util_double_approx_equal__( grid_volume , eclipse_volume , 2.5e-3, 0.00)) {
      double diff = 100 * (grid_volume - eclipse_volume) / eclipse_volume;
      printf("Error in cell: %d V1: %g    V2: %g   diff:%g %% \n", iglobal , grid_volume , eclipse_volume , diff);
      error_count++;
      error_found = true;
    }
  }
  printf("Total volume difference: %g %% \n", 100 * total_diff / total_volume );



  ecl_grid_free( grid );
  ecl_file_close( init );
  free( grid_file );
  free( init_file );
  if (error_found) {
    printf("Error_count: %d / %d \n",error_count , ecl_grid_get_nactive( grid ));
    exit(1);
  }  else
    exit(0);
}
