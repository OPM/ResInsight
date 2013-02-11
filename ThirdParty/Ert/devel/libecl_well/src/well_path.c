/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'well_path.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/ecl_well/well_path.h>
#include <ert/ecl_well/well_branch.h>
#include <ert/ecl_well/well_const.h>
#include <ert/ecl_well/well_conn.h>

/*
  This file implements the well_path structure which is container for
  the grid connections for one grid realisation; i.e. if the well is
  perforated in LGRs in addition to the global grid the parent
  well_state structure will have several well_path instances - one for
  the global grid and one for each LGR.

  The well_path structure consist of one or several well_branch_type
  instances; the well_branch is a collection of cells which are
  connected on a 1D line.

                     --------------D
                    /
                   /
  A---------------B----------C


  All of this figure is one well; how this structure is connected to
  one particular grid is described by the well_path structure in this
  file. The current well consist of two branches: A-B-C and B-D; these
  are two well_branch instance in the well_path structure.
*/


#define WELL_PATH_TYPE_ID 91916433

struct well_path_struct {
  UTIL_TYPE_ID_DECLARATION;
  char              * grid_name;     // Will be NULL for 'null_path' construction in the well_state object.
  well_branch_type ** branches;
  int                 max_branches;  // This might be misleading due to NULL pointers 'inside' the branches array.
  int                 alloc_size;
};



static void well_path_resize( well_path_type * well_path , int new_size) {
  well_path->branches = util_realloc( well_path->branches , new_size * sizeof * well_path->branches );
  {
    int i;
    for (i=well_path->alloc_size; i < new_size; i++)
      well_path->branches[i] = NULL;
  }
  
  well_path->alloc_size = new_size;
}

static UTIL_SAFE_CAST_FUNCTION( well_path , WELL_PATH_TYPE_ID )

well_path_type * well_path_alloc(const char * grid_name) {
  well_path_type * well_path = util_malloc( sizeof * well_path );
  UTIL_TYPE_ID_INIT( well_path , WELL_PATH_TYPE_ID );
  well_path->grid_name    = util_alloc_string_copy( grid_name );
  well_path->branches     = NULL;
  well_path->max_branches = 0;
  well_path->alloc_size   = 0;
  well_path_resize( well_path , 4 );
  return well_path;
}



well_branch_type * well_path_add_branch( well_path_type * well_path , int branch_nr) {

  well_branch_type * new_branch = well_branch_alloc( branch_nr );
  if (branch_nr >= well_path->alloc_size)
    well_path_resize( well_path , 2 * branch_nr );

  well_path->branches[ branch_nr ] = new_branch;
  if (branch_nr >= well_path->max_branches)
    well_path->max_branches = branch_nr + 1;
  
  return new_branch;
}


bool well_path_has_branch( const well_path_type * well_path , int branch_nr ) {
  if (branch_nr >= 0 && branch_nr < well_path->max_branches) {
    well_branch_type * branch = well_path->branches[ branch_nr ];
    if (branch != NULL)
      return true;
    else
      return false;
  } else
    return false;
}

/**
   This will return the maximum branch number; there can be holes in
   the branches array:

      branches = [ branch0, NULL , branch1 , NULL , branch3]

   In this case the the well_path_get_max_branches() will return five;
   however there are only three active branches in this case. The
   alternative function well_path_get_num_active_branches() will count
   the number of active (i.e. != NULL) branches.
*/
int well_path_get_max_branches( const well_path_type * well_path ) {
  return well_path->max_branches;
}


int well_path_get_num_active_branches( const well_path_type * well_path) {
  int branch_nr;
  int num_active = 0;
  for (branch_nr = 0; branch_nr < well_path->max_branches; branch_nr++)
    if (well_path->branches[ branch_nr ] != NULL)
      num_active++;
  return num_active;
}



/**
   Will return NULL if the branch does not exist.
*/

well_branch_type * well_path_iget_branch( const well_path_type * well_path , int branch_nr) {
  if (well_path_has_branch( well_path , branch_nr ))
    return well_path->branches[ branch_nr ];
  else
    return NULL;
}


void well_path_add_conn( well_path_type * well_path , well_conn_type * conn) {
  int branch_nr = well_conn_get_branch( conn );
  if (!well_path_has_branch( well_path , branch_nr)) 
    well_path_add_branch( well_path , branch_nr );
  {
    well_branch_type * branch = well_path_iget_branch( well_path , branch_nr );
    well_branch_add_conn( branch , conn );
  }
}


void well_path_free( well_path_type * well_path ) {
  int i;
  for (i=0; i < well_path->alloc_size; i++) {
    if (well_path->branches[i] != NULL) 
      well_branch_free( well_path->branches[i] );
  }
  util_safe_free( well_path->grid_name );
  free( well_path->branches );
  free( well_path );
}

void well_path_free__(void * arg) {
  well_path_type * well_path = well_path_safe_cast( arg );
  well_path_free( well_path );
}



const char * well_path_get_grid_name( const well_path_type * well_path ) {
  return well_path->grid_name;
}
