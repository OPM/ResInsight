/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'misfit_ensemble.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/vector.h>
#include <ert/util/double_vector.h>
#include <ert/util/msg.h>
#include <ert/util/buffer.h>

#include <ert/enkf/enkf_obs.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/misfit_ensemble.h>
#include <ert/enkf/misfit_member.h>
#include <ert/enkf/misfit_ts.h>


/**
   This file implements a type misfit_ensemble which is used to rank the
   different realization according to various criteria.

   The top level datastructure in this file is the misfit_ensemble, and
   that is the only exported datatype, but in addition there are the
   misfit_member which is the misfit for one ensemble member, and
   misfit_ts which is the misfit for one ensemble member / one
   observation key.
*/





#define MISFIT_ENSEMBLE_TYPE_ID   441066

struct misfit_ensemble_struct {
  UTIL_TYPE_ID_DECLARATION;
  bool                  initialized;
  int                   history_length;  
  vector_type         * ensemble;           /* Vector of misfit_member_type instances - one for each ensemble member. */
};


/*****************************************************************/

static double ** __2d_malloc(int rows , int columns) {
  double ** d = util_calloc( rows , sizeof * d );
  for (int i =0; i < rows; i++)
    d[i] = util_calloc( columns , sizeof * d[i]);
  return d;
}

static void  __2d_free(double ** d , int rows) {
  for (int i =0; i < rows; i++)
    free(d[i]);
  free(d);
}


void misfit_ensemble_update( misfit_ensemble_type * misfit_ensemble , const ensemble_config_type * ensemble_config , const enkf_obs_type * enkf_obs , enkf_fs_type * fs , int ens_size , int history_length) {
  misfit_ensemble_clear( misfit_ensemble );
  {
    state_enum cmp_state           = FORECAST;
    msg_type * msg                 = msg_alloc("Evaluating misfit for observation: " , false);
    double ** chi2_work            = __2d_malloc( history_length + 1 , ens_size );
    bool_vector_type * iens_valid  = bool_vector_alloc( ens_size , true );
    
    hash_iter_type * obs_iter = enkf_obs_alloc_iter( enkf_obs );
    const char * obs_key      = hash_iter_get_next_key( obs_iter );
    
    misfit_ensemble->history_length = history_length;
    misfit_ensemble_set_ens_size( misfit_ensemble , ens_size );
    
    msg_show( msg );
    while (obs_key != NULL) {
      obs_vector_type * obs_vector = enkf_obs_get_vector( enkf_obs , obs_key );
      msg_update( msg , obs_key );
      
      bool_vector_reset( iens_valid );
      bool_vector_iset( iens_valid , ens_size - 1 , true );
      obs_vector_ensemble_chi2( obs_vector , 
                                fs , 
                                iens_valid , 
                                0 , 
                                misfit_ensemble->history_length, 
                                0 , 
                                ens_size , 
                                cmp_state , 
                                chi2_work);
      
      /** 
          Internalizing the results from the chi2_work table into the misfit structure.
      */
      for (int iens = 0; iens < ens_size; iens++) {
        misfit_member_type * node = misfit_ensemble_iget_member( misfit_ensemble , iens );
        if (bool_vector_iget( iens_valid , iens))
          misfit_member_update( node , obs_key , misfit_ensemble->history_length , iens , (const double **) chi2_work);
      }
      obs_key = hash_iter_get_next_key( obs_iter );
    }
    
    bool_vector_free( iens_valid );
    msg_free(msg , true );
    hash_iter_free( obs_iter );
    
    __2d_free( chi2_work , misfit_ensemble->history_length + 1);
    misfit_ensemble->initialized = true;
  }
}


void misfit_ensemble_fwrite( const misfit_ensemble_type * misfit_ensemble , FILE * stream ) {
  int ens_size = vector_get_size( misfit_ensemble->ensemble);
  util_fwrite_int( misfit_ensemble->history_length , stream );
  util_fwrite_int( vector_get_size( misfit_ensemble->ensemble ) , stream);

  /* Writing the nodes - one for each ensemble member */
  {
    int iens;
    for (iens = 0; iens < ens_size; iens++) 
      misfit_member_fwrite( vector_iget( misfit_ensemble->ensemble , iens ) , stream ); 
  }
  
}




/**
   Observe that the object is NOT in a valid state when leaving this function, 
   must finalize in either misfit_ensemble_alloc() or misfit_ensemble_fread_alloc().
*/

static misfit_ensemble_type * misfit_ensemble_alloc_empty() {
  misfit_ensemble_type * table    = util_malloc( sizeof * table );

  table->initialized     = false;
  table->ensemble        = vector_alloc_new();
  
  return table;
}


/**
   This funcion is a feeble attempt at allowing the ensemble size to
   change runtime. If the new ensemble size is larger than the current
   ensemble size ALL the currently internalized misfit information is
   dropped on the floor; if the the ensemble is shrinked only the the
   last elements of the misfit table are discarded (NOT exactly battle-tested).

*/
void misfit_ensemble_set_ens_size( misfit_ensemble_type * misfit_ensemble , int ens_size) {
  int iens;
  if (ens_size > vector_get_size( misfit_ensemble->ensemble )) {
    /* The new ensemble is larger than what we have currently internalized, 
       we drop everything and add empty misfit_member instances. */
    vector_clear( misfit_ensemble->ensemble );
    for (iens = 0; iens < ens_size; iens++)
      vector_append_owned_ref( misfit_ensemble->ensemble , misfit_member_alloc( iens ) , misfit_member_free__);
    
  } else 
    /* We shrink the vector by removing the last elements. */
    vector_shrink( misfit_ensemble->ensemble , ens_size);
}


void misfit_ensemble_fread( misfit_ensemble_type * misfit_ensemble , FILE * stream ) {
  misfit_ensemble_clear( misfit_ensemble );
  {
    int ens_size;
    
    misfit_ensemble->history_length = util_fread_int( stream );
    ens_size                        = util_fread_int( stream );
    misfit_ensemble_set_ens_size( misfit_ensemble , ens_size );
    {
      for (int iens = 0; iens < ens_size; iens++) {
        misfit_member_type * node = misfit_member_fread_alloc( stream );
        vector_iset_owned_ref( misfit_ensemble->ensemble , iens , node , misfit_member_free__);
      }
    }
    
  }
  misfit_ensemble->initialized = true;
  return misfit_ensemble;
}



misfit_ensemble_type * misfit_ensemble_alloc( ) {
  misfit_ensemble_type * table = misfit_ensemble_alloc_empty( );
  return table;
}



misfit_member_type * misfit_ensemble_iget_member( const misfit_ensemble_type * table , int iens) {
  return vector_iget( table->ensemble , iens);
}




void misfit_ensemble_clear( misfit_ensemble_type * table) {
  vector_clear( table->ensemble );
  table->initialized = false;
}


void misfit_ensemble_free(misfit_ensemble_type * table ) {
  vector_free( table->ensemble );
  free( table );
}


bool misfit_ensemble_initialized( const misfit_ensemble_type * misfit_ensemble ) {
  return misfit_ensemble->initialized;
}


/*****************************************************************/


int misfit_ensemble_get_ens_size( const misfit_ensemble_type * misfit_ensemble ) {
  return vector_get_size( misfit_ensemble->ensemble );
}
