/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'well_branch.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <util.h>
#include <stdlib.h>
#include <well_branch.h>
#include <well_const.h>
#include <well_conn.h>

/*

  See also documentation on the top of well_path.c for how a path and
  a branch are related.  
*/

struct well_branch_struct {
  int               branch_nr;
  well_conn_type ** connections;
  int               size;
  int               alloc_size;
};


static void well_branch_resize( well_branch_type * branch , int new_size) {
  if (new_size < branch->alloc_size)
    util_abort("%s: sorry - can only grow \n",__func__);
  
  branch->connections = util_realloc( branch->connections , new_size * sizeof * branch->connections );
  {
    int i;
    for (i=branch->alloc_size; i < new_size; i++)
      branch->connections[i] = NULL;
  }
  branch->alloc_size = new_size;
}


well_branch_type * well_branch_alloc(int branch_nr) {
  well_branch_type * branch = util_malloc( sizeof * branch );
  branch->branch_nr   = branch_nr;
  branch->connections = NULL;
  branch->alloc_size = 0;
  branch->size = 0;
  
  well_branch_resize( branch , 10 );
  return branch;
}


void well_branch_free( well_branch_type * branch ) {
  int i;
  for ( i=0; i < branch->alloc_size; i++) {
    well_conn_type * conn = branch->connections[i];
    if (conn != NULL)
      well_conn_free( conn );
  }
  free( branch->connections );
  free( branch );
}


int well_branch_get_length( const well_branch_type * branch) {
  return branch->size;
}


const well_conn_type ** well_branch_get_connections( const well_branch_type * branch ) {
  if (branch->size == 0)
    return NULL;
  else
    return (const well_conn_type **) branch->connections;
}


void well_branch_add_conn( well_branch_type * branch, well_conn_type * connection ) {
  if (branch->size == branch->alloc_size)
    well_branch_resize(branch , 2*( 1 + branch->alloc_size ));
  branch->connections[ branch->size ] = connection;
  branch->size++;
}


int well_branch_get_nr( const well_branch_type * branch ) {
  return branch->branch_nr;
}

