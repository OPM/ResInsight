/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'well_state.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef __WELL_STATE_H__
#define __WELL_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

#include <ert/ecl/ecl_file.h> 
#include <ert/ecl/ecl_intehead.h>

#include <ert/ecl_well/well_conn.h>
#include <ert/ecl_well/well_const.h>


#define GLOBAL_GRID_NAME   "GLOBAL" // The name assigned to the global grid for name based lookup.

  typedef struct well_state_struct well_state_type;
  
  well_state_type      * well_state_alloc( ecl_file_type * ecl_file , int report_step , int well_nr);
  void                   well_state_free( well_state_type * well );
  const char           * well_state_get_name( const well_state_type * well );
  int                    well_state_get_report_nr( const well_state_type * well_state );
  time_t                 well_state_get_sim_time( const well_state_type * well_state );
  well_conn_type       * well_state_iget_connection( const well_state_type * well_state , int index);
  well_type_enum         well_state_get_type( const well_state_type * well_state);
  bool                   well_state_is_open( const well_state_type * well_state );   
  
  const well_conn_type * well_state_iget_wellhead( const well_state_type * well_state , int grid_nr);
  const well_conn_type * well_state_get_wellhead( const well_state_type * well_state , const char * grid_name);
  
  
  const well_conn_type ** well_state_iget_lgr_connections(const well_state_type * well_state , int grid_nr , int branch_nr );
  const well_conn_type ** well_state_get_lgr_connections(const well_state_type * well_state , const char * lgr_name , int branch_nr);
  const well_conn_type ** well_state_get_connections(const well_state_type * well_state , int branch_nr );

  int                     well_state_iget_num_lgr_connections(const well_state_type * well_state , int grid_nr , int branch_nr );
  int                     well_state_get_num_lgr_connections(const well_state_type * well_state , const char * lgr_name , int branch_nr);
  int                     well_state_get_num_connections(const well_state_type * well_state , int branch_nr );

  int                     well_state_iget_lgr_num_branches( const well_state_type * well_state , int grid_nr);
  int                     well_state_get_lgr_num_branches( const well_state_type * well_state , const char * lgr_name);
  int                     well_state_get_num_branches(const well_state_type * well_state );

  void                    well_state_summarize( const well_state_type * well_state , FILE * stream );

  UTIL_IS_INSTANCE_HEADER( well_state );
  
#ifdef __cplusplus
}
#endif

#endif
