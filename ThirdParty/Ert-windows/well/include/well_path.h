/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'well_path.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __WELL_PATH_H__
#define __WELL_PATH_H__


#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <well_conn.h>
#include <well_branch.h>

  
  typedef struct          well_path_struct well_path_type;
  
  well_path_type       *  well_path_alloc(const char * grid_name );
  void                    well_path_free( well_path_type * path );
  void                    well_path_add_conn( well_path_type * well_path , well_conn_type * conn);
  well_branch_type      * well_path_iget_branch( const well_path_type * well_path , int branch_nr);
  void                    well_path_free__(void * arg);
  int                     well_path_get_max_branches( const well_path_type * well_path );
  int                     well_path_get_num_active_branches( const well_path_type * well_path );
  const char *            well_path_get_grid_name( const well_path_type * well_path );

#ifdef __cplusplus
}
#endif
#endif

