/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'enkf_plot_tvector.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#ifndef ERT_ENKF_PLOT_TVECTOR_H
#define ERT_ENKF_PLOT_TVECTOR_H

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>
#include <stdbool.h>

#include <ert/util/util.h>

#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_config_node.h>
  
  typedef struct enkf_plot_tvector_struct enkf_plot_tvector_type;
  
  UTIL_SAFE_CAST_HEADER( enkf_plot_tvector );
  UTIL_IS_INSTANCE_HEADER( enkf_plot_tvector );

  
  
  void                     enkf_plot_tvector_reset( enkf_plot_tvector_type * plot_tvector );
  enkf_plot_tvector_type * enkf_plot_tvector_alloc( const enkf_config_node_type * config_node , int iens);
  void                     enkf_plot_tvector_load( enkf_plot_tvector_type * plot_tvector , enkf_fs_type * fs , const char * user_key );
  void *                   enkf_plot_tvector_load__( void * arg );
  void                     enkf_plot_tvector_free( enkf_plot_tvector_type * plot_tvector );
  void                     enkf_plot_tvector_iset( enkf_plot_tvector_type * plot_tvector , int index , time_t time , double value);
  
  int                     enkf_plot_tvector_size( const enkf_plot_tvector_type * plot_tvector );
  double                  enkf_plot_tvector_iget_value( const enkf_plot_tvector_type * plot_tvector , int index);
  time_t                  enkf_plot_tvector_iget_time( const enkf_plot_tvector_type * plot_tvector , int index);
  bool                    enkf_plot_tvector_iget_active( const enkf_plot_tvector_type * plot_tvector , int index);
  bool                    enkf_plot_tvector_all_active( const enkf_plot_tvector_type * plot_tvector );
  

#ifdef __cplusplus
}
#endif
#endif
