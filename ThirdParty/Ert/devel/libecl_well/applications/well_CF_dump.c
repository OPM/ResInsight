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



int main( int argc , char ** argv ) {
  char * grid_file = argv[1];
  char * rst_file = argv[2];
  ecl_grid_type * grid = ecl_grid_alloc( grid_file );
  well_info_type * well_info = well_info_alloc( grid );
  
  well_info_load_rstfile( well_info , rst_file , true );
  {
    int iw;
    int ic;

    for (iw=0; iw < well_info_get_num_wells( well_info ); iw++) {
      const char * well_name = well_info_iget_well_name( well_info , iw );
      well_state_type * well_state = well_info_get_state_from_report(well_info , well_name , 100 );
      const well_conn_collection_type * connections = well_state_get_global_connections( well_state );
      printf("Well: %s \n",well_state_get_name( well_state ));

      for (ic = 0; ic < well_conn_collection_get_size( connections ); ic++) {
        well_conn_type * conn = well_conn_collection_iget( connections , ic );
        printf("    %2d  %2d  %2d   CF: %g \n",well_conn_get_i( conn ),
               well_conn_get_j( conn ),
               well_conn_get_k( conn ),
               well_conn_get_connection_factor( conn ));
        
      }
    }
  }
  
  well_info_free( well_info );
  ecl_grid_free( grid );
}
