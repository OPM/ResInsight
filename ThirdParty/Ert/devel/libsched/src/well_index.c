/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'well_index.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/size_t_vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/util.h>  

#include <ert/sched/sched_types.h>
#include <ert/sched/well_index.h>


#define WELL_INDEX_TYPE_ID 99780634

struct well_index_struct {
  UTIL_TYPE_ID_DECLARATION;
  const void * state_ptr;
  char                    * well_name;
  char                    * variable;  /* Because many variables can be accessed both as eg WOPRH and WOPR the
                                          variable might not match the name used when looking up this index. The
                                          variable field should always contain the true historical (i.e. xxxxH)
                                          variable, as that is the most correct way to access the values of the
                                          schedule file. */
  int_vector_type         * kw_type;
  size_t_vector_type      * func;   
};





UTIL_IS_INSTANCE_FUNCTION( well_index , WELL_INDEX_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION_CONST( well_index , WELL_INDEX_TYPE_ID )



void well_index_add_type( well_index_type * index , sched_kw_type_enum kw_type , sched_history_callback_ftype * func) {
  int_vector_append( index->kw_type      , kw_type );
  size_t_vector_append( index->func      , ( size_t ) func );
}



well_index_type * well_index_alloc( const char * well_name , const char * variable , const void * state_ptr , sched_kw_type_enum kw_type , sched_history_callback_ftype * func ) {
  well_index_type * well_index = util_malloc( sizeof * well_index );
  
  UTIL_TYPE_ID_INIT( well_index , WELL_INDEX_TYPE_ID );
  
  well_index->well_name = util_alloc_string_copy( well_name );
  well_index->variable  = util_alloc_string_copy( variable ); 
  well_index->kw_type   = int_vector_alloc( 0 , 0 );
  well_index->func      = size_t_vector_alloc( 0 , 0 );
  well_index->state_ptr = state_ptr;
  
  well_index_add_type( well_index , kw_type , func );
  return well_index;
}


void well_index_free( well_index_type * index ) {
  size_t_vector_free( index->func );
  int_vector_free( index->kw_type );
  free( index->well_name );
  free( index->variable );
  free( index );
}


void well_index_free__( void * arg ) {
  well_index_free(  (well_index_type  *) arg );
}

const char * well_index_get_name( const well_index_type * well_index ) {
  return well_index->well_name;
}

const char * well_index_get_variable( const well_index_type * well_index ) {
  return well_index->variable;
}




sched_history_callback_ftype * well_index_get_callback( const well_index_type * well_index , sched_kw_type_enum kw_type) {
  sched_history_callback_ftype * func = NULL;
  int iindex = 0;
  while (true) {
    if (int_vector_iget( well_index->kw_type , iindex) == kw_type) {
      func = ( sched_history_callback_ftype *) size_t_vector_iget( well_index->func , iindex );
      break;
    }
    
    
    iindex++;
    if (iindex == int_vector_size( well_index->kw_type ))
      break;
  }
  
  return func;
}



const void * well_index_get_state( const well_index_type * well_index ) {
  return well_index->state_ptr;
}



const void * well_index_get_state__( const void * index ) {
  const well_index_type * well_index = well_index_safe_cast_const( index );
  return well_index_get_state( well_index );
}
