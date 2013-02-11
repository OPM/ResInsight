/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'grid_layer.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ecl_grid.h>
#include <stdlib.h>

static void usage() {
  exit(1);
}

int main( int argc , char ** argv ) {
  if (argc < 3)
    usage();
  {
    ecl_grid_type * ecl_grid = ecl_grid_alloc( argv[1] );
    int nx,ny,nz;
    int iarg;
    {
      const ecl_grid_type * lgr = ecl_grid_get_lgr( ecl_grid , "TROLLA");
      int i,j,k;
      int global_cell;
      global_cell = ecl_grid_get_parent_cell3( lgr , 9,9,5);
      ecl_grid_get_ijk1( ecl_grid , global_cell , &i, &j, &k);
      printf("Global cell(TROLLA , 10,10,6) = %d,%d,%d\n", i+1 ,j+1 , k+1);

      global_cell = ecl_grid_get_parent_cell3( lgr , 9,9,2);
      ecl_grid_get_ijk1( ecl_grid , global_cell , &i, &j, &k);
      printf("Global cell(TROLLA , 10,10,3) = %d,%d,%d\n", i+1 ,j+1 , k+1);
      
      global_cell = ecl_grid_get_parent_cell3( lgr , 25,11,0);
      ecl_grid_get_ijk1( ecl_grid , global_cell , &i, &j, &k);
      printf("Global cell(TROLLA , 26,12,1) = %d,%d,%d\n", i+1 ,j+1 , k+1);


      lgr = ecl_grid_get_lgr(ecl_grid , "TROLLA2");
      global_cell = ecl_grid_get_parent_cell3( lgr , 9,9,2);
      ecl_grid_get_ijk1( ecl_grid , global_cell , &i, &j, &k);
      printf("Global cell(TROLLA2 , 10,10,3) = %d,%d,%d\n", i+1 ,j+1 , k+1);
      
    }
    exit(1);
    
    ecl_grid_get_dims( ecl_grid , &nx , &ny , &nz , NULL );
    for (iarg = 2; iarg < argc; iarg++) {
      int k;
      if (util_sscanf_int( argv[iarg] , &k )) {
        int i,j;
        k--;
        for (j=(ny - 1); j >= 0; j--) {
          for (i=0; i < nx; i++) {
            if (ecl_grid_get_active_index3(ecl_grid , i, j, k) >= 0) {
              if ( (i % 10) == 0)
                printf("*");
              else
                printf("X");
            }
            else
              printf(" ");
          }
          printf("\n");
        }
      }
    }
  }
}
