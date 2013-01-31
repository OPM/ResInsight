/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'local_updatestep.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/vector.h>

#include <ert/enkf/local_ministep.h>
#include <ert/enkf/local_updatestep.h>
#include <ert/enkf/local_config.h>
#include <ert/enkf/enkf_macros.h>

/**
   One enkf update is described/configured by the data structure in
   local_ministep.c. This file implements a local report_step, which
   is a collection of ministeps - in many cases a local_updatestep will
   only consist of one single local_ministep; but in principle it can
   contain several.
*/

#define LOCAL_UPDATESTEP_TYPE_ID 77159

struct local_updatestep_struct {
  UTIL_TYPE_ID_DECLARATION;
  char        * name;
  vector_type * ministep;
};



UTIL_SAFE_CAST_FUNCTION(local_updatestep , LOCAL_UPDATESTEP_TYPE_ID)


local_updatestep_type * local_updatestep_alloc( const char * name ) {
  local_updatestep_type * updatestep = util_malloc( sizeof * updatestep );
  
  UTIL_TYPE_ID_INIT( updatestep , LOCAL_UPDATESTEP_TYPE_ID );
  updatestep->name      = util_alloc_string_copy( name );
  updatestep->ministep  = vector_alloc_new();
  
  return updatestep;
}


/**
   Observe that use_count values are not copied. 
*/
local_updatestep_type * local_updatestep_alloc_copy( const local_updatestep_type * src , const char * name ) {
  local_updatestep_type * new = local_updatestep_alloc( name );
  for (int i = 0; i < vector_get_size(src->ministep ); i++)
    local_updatestep_add_ministep( new , vector_iget( src->ministep , i) );
  return new;
}


void local_updatestep_free( local_updatestep_type * updatestep) {
  free( updatestep->name );
  vector_free( updatestep->ministep );
  free( updatestep );
}


void local_updatestep_free__(void * arg) {
  local_updatestep_type * updatestep = local_updatestep_safe_cast( arg );
  local_updatestep_free( updatestep );
}


void local_updatestep_add_ministep( local_updatestep_type * updatestep , local_ministep_type * ministep) {
  vector_append_ref( updatestep->ministep , ministep );   /* Observe that the vector takes NO ownership */
}



local_ministep_type * local_updatestep_iget_ministep( const local_updatestep_type * updatestep , int index) {
  return vector_iget( updatestep->ministep , index );
}


local_obsset_type * local_updatestep_iget_obsset( const local_updatestep_type * updatestep , int index) {
  return local_ministep_get_obsset( vector_iget( updatestep->ministep , index ) );
}


int local_updatestep_get_num_ministep( const local_updatestep_type * updatestep) {
  return vector_get_size( updatestep->ministep );
}

const char * local_updatestep_get_name( const local_updatestep_type * updatestep ) {
  return updatestep->name; 
}


/*****************************************************************/


void local_updatestep_fprintf( const local_updatestep_type * updatestep , FILE * stream) {
  fprintf(stream , "%s %s\n" , local_config_get_cmd_string( CREATE_UPDATESTEP ) , updatestep->name );
  {
    int i;
    for (i=0; i < vector_get_size( updatestep->ministep ); i++) {
      const local_ministep_type * ministep = vector_iget_const( updatestep->ministep , i );
      fprintf(stream , "%s %s %s\n",local_config_get_cmd_string( ATTACH_MINISTEP ) , updatestep->name , local_ministep_get_name( ministep ));
    }
  }
}
