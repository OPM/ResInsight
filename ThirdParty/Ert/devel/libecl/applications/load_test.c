/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'load_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>

#include <ert/util/timer.h>

#include <ert/ecl/ecl_grid.h>
#include <ert/ecl/ecl_file.h>


void test_case( const char * base , bool load_all) {
  timer_type * ti = timer_alloc(false);
  timer_type * tr = timer_alloc(false);
  timer_type * tg = timer_alloc(false);

  char * grid_file    = ecl_util_alloc_filename( NULL , base , ECL_EGRID_FILE , false , 0 );
  char * init_file    = ecl_util_alloc_filename( NULL , base , ECL_INIT_FILE , false , 0 );
  char * restart_file = ecl_util_alloc_filename( NULL , base , ECL_UNIFIED_RESTART_FILE , false , 0 );

  ecl_grid_type * grid;
  ecl_file_type * restart;
  ecl_file_type * init;


  timer_start( tg );
  grid = ecl_grid_alloc(grid_file );
  timer_stop( tg );

  timer_start( ti );
  init = ecl_file_open( init_file , 0);
  if (load_all)
    ecl_file_load_all( init );
  timer_stop( ti );

  timer_start( tr );
  restart = ecl_file_open( restart_file , 0);
  if (load_all)
    ecl_file_load_all( restart );
  timer_stop( tr );


  printf("%-64s  Restart:%8.4f    Grid:%8.4f     Init:%8.4f \n",
         base,
         timer_get_total_time( tr ),
         timer_get_total_time( tg ),
         timer_get_total_time( ti ));

  timer_free( tr );
  timer_free( ti );
  timer_free( tg );
  ecl_file_close( init );
  ecl_file_close( restart );
  ecl_grid_free( grid );
  free( grid_file );
  free( init_file );
  free( restart_file );
}


int main(int argc, char ** argv) {
  bool load_all = true;
  int i;
  for (i=1; i < argc; i++)
    test_case( argv[i] , load_all);
  
}
