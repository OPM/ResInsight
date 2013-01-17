/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'well_info_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <time.h>
#include <stdbool.h>
#include <signal.h>

#include <util.h>
#include <int_vector.h>
#include <ecl_intehead.h>
#include <ecl_file.h>
#include <ecl_kw.h>
#include <ecl_kw_magic.h>
#include <ecl_util.h>

#include <well_state.h>
#include <well_info.h>
#include <well_conn.h>
#include <well_ts.h>

int main( int argc , char ** argv) {
  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGTERM , util_abort_signal);    /* If killing the enkf program with SIGTERM (the default kill signal) you will get a backtrace. 
                                             Killing with SIGKILL (-9) will not give a backtrace.*/
  signal(SIGABRT , util_abort_signal);    /* Signal abort. */ 
  {
    well_info_type * well_info = well_info_alloc( NULL );
    int i;
    for (i=1; i < argc; i++) {
      printf("Loading file: %s \n",argv[i]);
      well_info_load_rstfile( well_info , argv[i]);
    }
    
    // List all wells:
    {
      int iwell;
      for (iwell = 0; iwell < well_info_get_num_wells( well_info ); iwell++) {
        well_ts_type * well_ts = well_info_get_ts( well_info , well_info_iget_well_name( well_info , iwell));
        well_state_type * well_state = well_ts_get_last_state( well_ts );
        
        well_state_summarize( well_state , stdout );
        printf("\n");
      }
    }
    well_info_free( well_info );
    exit(1);

    // Look at the timeseries for one well:
    {
      well_ts_type * well_ts = well_info_get_ts( well_info , well_info_iget_well_name( well_info , 0));
      int i;
      for (i =0; i < well_ts_get_size( well_ts ); i++) {
        well_state_type * well_state = well_ts_iget_state( well_ts , i );
        
        printf("Well:%s  report:%04d  state:",well_state_get_name( well_state ), well_state_get_report_nr( well_state ));
        if (well_state_is_open( well_state ))
          printf("OPEN\n");
        else
          printf("CLOSED\n");
      }
    }
    
    // Look at one well_state:
    {
      well_state_type * well_state = well_info_iiget_state( well_info , 0 , 0 );
      printf("Well:%s  report:%04d \n",well_state_get_name( well_state ), well_state_get_report_nr( well_state ));
      {
        const well_conn_type ** connections = well_state_get_connections( well_state , 0 );
        printf("Branches: %d \n",well_state_get_num_branches( well_state ));
        printf("num_connections: %d \n",well_state_get_num_connections( well_state , 0 ));
	{ 
	  int iconn;
	  for (iconn = 0; iconn < well_state_get_num_connections( well_state , 0 ); iconn++) {
	    well_conn_type * conn = connections[ iconn ];
	    printf("Connection:%02d   i=%3d  j=%3d  k=%3d  State:",iconn , well_conn_get_i( conn ) , well_conn_get_j( conn ) , well_conn_get_k( conn ));
	    if (well_conn_open( conn ) )
            printf("Open\n");
	    else
	      printf("Closed\n");
	  }
        }
      }
    }
    well_info_free( well_info );
  }
}
