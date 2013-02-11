/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'misfit_member.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/misfit_member.h>


#define MISFIT_MEMBER_TYPE_ID         541066

struct misfit_member_struct {
  UTIL_TYPE_ID_DECLARATION;
  int          my_iens;
  hash_type   *obs;           /* hash table of misfit_ts_type instances - indexed by observation keys. The structure
                                 of this hash table is duplicated for each ensemble member.*/
};



static UTIL_SAFE_CAST_FUNCTION(misfit_member , MISFIT_MEMBER_TYPE_ID);


static void misfit_member_free( misfit_member_type * node ) {
  hash_free( node->obs );
  free( node );
}


void misfit_member_free__( void * node ) {
  misfit_member_free( misfit_member_safe_cast( node ));
}


misfit_member_type * misfit_member_alloc(int iens) {
  misfit_member_type * node = util_malloc( sizeof * node );
  UTIL_TYPE_ID_INIT( node , MISFIT_MEMBER_TYPE_ID);
  node->my_iens    = iens;
  node->obs        = hash_alloc();
  return node;
}


static void misfit_member_install_vector( misfit_member_type * node , const char * key , misfit_ts_type * vector ) {
  hash_insert_hash_owned_ref( node->obs, key , vector , misfit_ts_free__ );
}


static misfit_ts_type * misfit_member_safe_get_vector( misfit_member_type * node , const char * obs_key , int history_length) {
  if (!hash_has_key( node->obs , obs_key ))
    misfit_member_install_vector(node , obs_key , misfit_ts_alloc( history_length ) );
  return hash_get( node->obs , obs_key );
}


misfit_ts_type * misfit_member_get_ts( const misfit_member_type * node , const char * obs_key ) {
  return hash_get( node->obs , obs_key );
}

bool misfit_member_has_ts( const misfit_member_type * node , const char * obs_key ) {
  return hash_has_key( node->obs , obs_key );
}


void misfit_member_update( misfit_member_type * node , const char * obs_key , int history_length , int iens , const double ** work_chi2) {
  misfit_ts_type * vector = misfit_member_safe_get_vector( node , obs_key , history_length );
  for (int step = 0; step <= history_length; step++) 
    misfit_ts_iset( vector , step , work_chi2[step][iens]);
}


void misfit_member_fwrite( const misfit_member_type * node , FILE * stream) {
  util_fwrite_int( node->my_iens , stream);
  util_fwrite_int( hash_get_size( node->obs ) , stream);
  {
    hash_iter_type * obs_iter = hash_iter_alloc( node->obs );
    while ( !hash_iter_is_complete( obs_iter )) {
      const char * key           = hash_iter_get_next_key( obs_iter );
      misfit_ts_type * misfit_ts = hash_get( node->obs , key );
      util_fwrite_string( key , stream );
      misfit_ts_fwrite( misfit_ts , stream);
    }
    hash_iter_free( obs_iter );
  }
}


misfit_member_type * misfit_member_fread_alloc( FILE * stream ) {
  int my_iens                = util_fread_int( stream );
  misfit_member_type * node = misfit_member_alloc( my_iens );
  int hash_size = util_fread_int( stream );
  {
    int iobs;
    for (iobs = 0; iobs < hash_size; iobs++) {
      const char         * key   = util_fread_alloc_string( stream );
      misfit_ts_type * misfit_ts = misfit_ts_fread_alloc( stream );
      misfit_member_install_vector( node , key , misfit_ts );
      free( key );
    }
  }
  return node;
}

