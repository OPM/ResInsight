/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'subst_func.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <stdbool.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/stringlist.h>
#include <ert/util/subst_func.h>
#include <ert/util/hash.h>
#include <ert/util/rng.h>


#define SUBST_FUNC_TYPE_ID      646781
#define SUBST_FUNC_POOL_TYPE_ID 7641

struct subst_func_pool_struct {
  UTIL_TYPE_ID_DECLARATION;
  hash_type  * func_table;
};



struct subst_func_struct {
  UTIL_TYPE_ID_DECLARATION;
  subst_func_ftype  * func;
  char              * name;
  char              * doc_string;    /* doc_string for this function - can be NULL. */
  bool                vararg;
  int                 argc_min;
  int                 argc_max; 
  void              * arg;           /* 100% unmanaged void argument passed in from the construction. */ 
};



char * subst_func_eval( const subst_func_type * subst_func , const stringlist_type * args) {
  if (!subst_func->vararg) {
    /* Checking that we have the right number of arguments. */
    int argc = stringlist_get_size( args );
    if (argc < subst_func->argc_min || argc > subst_func->argc_max) {
      fprintf(stderr,"Fatal error when appying function:%s - got %d arguments: [",subst_func->name , argc);
      stringlist_fprintf(args , " " , stderr);
      fprintf(stderr,"] expected %d-%d arguments.\n", subst_func->argc_min , subst_func->argc_max);
      util_abort("%s: Fatal error - aborting \n",__func__);
    }
  }
  printf("Running:%s \n",subst_func->name);
  return subst_func->func( args , subst_func->arg );
}


subst_func_type * subst_func_alloc( const char * func_name , const char * doc_string , subst_func_ftype * func , bool vararg, int argc_min , int argc_max , void * arg) {
  subst_func_type * subst_func = util_malloc( sizeof * subst_func );
  UTIL_TYPE_ID_INIT( subst_func , SUBST_FUNC_TYPE_ID );
  subst_func->func       = func;
  subst_func->name       = util_alloc_string_copy( func_name );
  subst_func->vararg     = vararg;
  subst_func->argc_min   = argc_min;
  subst_func->argc_max   = argc_max;
  subst_func->doc_string = util_alloc_string_copy( doc_string );
  subst_func->arg        = arg; 
  return subst_func;
}


void subst_func_free( subst_func_type * subst_func ) {
  util_safe_free( subst_func->doc_string );
  free( subst_func->name );
  free( subst_func );
}


UTIL_SAFE_CAST_FUNCTION( subst_func , SUBST_FUNC_TYPE_ID);

static void subst_func_free__( void * arg ) {
  subst_func_free( subst_func_safe_cast( arg ));
}




/*****************************************************************/

UTIL_IS_INSTANCE_FUNCTION( subst_func_pool , SUBST_FUNC_POOL_TYPE_ID);

subst_func_pool_type * subst_func_pool_alloc( ) {
  subst_func_pool_type * pool = util_malloc( sizeof * pool );
  UTIL_TYPE_ID_INIT( pool , SUBST_FUNC_POOL_TYPE_ID );
  pool->func_table = hash_alloc_unlocked();
  return pool;
}



void subst_func_pool_free( subst_func_pool_type * pool ) {
  hash_free( pool->func_table );
  free( pool );
}


void subst_func_pool_add_func( subst_func_pool_type * pool , const char * func_name , const char * doc_string , subst_func_ftype * func , bool vararg, int argc_min , int argc_max , void * arg) {
  subst_func_type * subst_func = subst_func_alloc( func_name , doc_string , func , vararg , argc_min , argc_max , arg);
  hash_insert_hash_owned_ref( pool->func_table , func_name , subst_func , subst_func_free__);
}


subst_func_type * subst_func_pool_get_func( const subst_func_pool_type * pool , const char * func_name ) {
  return hash_get( pool->func_table , func_name );
}

bool subst_func_pool_has_func( const subst_func_pool_type * pool , const char * func_name ) {
  return hash_has_key( pool->func_table , func_name );
}


/*****************************************************************/

char * subst_func_randint( const stringlist_type * args , void * arg) {
  rng_type * rng = rng_safe_cast( arg );
  return util_alloc_sprintf("%u" , rng_forward( rng ));
}

char * subst_func_randfloat( const stringlist_type * args , void * arg) {
  rng_type * rng = rng_safe_cast( arg );
  return util_alloc_sprintf("%12.10f" , 1e9 * rng_get_double( rng ));
}


char * subst_func_exp( const stringlist_type * args , void * ext_arg) {
  double arg;
  if (util_sscanf_double( stringlist_iget(args , 0 ) , &arg)) 
    return util_alloc_sprintf("%g" , exp(arg));
  else
    return NULL;
}


char * subst_func_log( const stringlist_type * args , void * ext_arg) {
  double arg;
  if (util_sscanf_double( stringlist_iget(args , 0 ) , &arg)) 
    return util_alloc_sprintf("%g" , log(arg));
  else
    return NULL;
}


char * subst_func_pow10( const stringlist_type * args , void * ext_arg) {
  double arg;
  if (util_sscanf_double( stringlist_iget(args , 0 ) , &arg)) 
    return util_alloc_sprintf("%g" , pow(10 , arg));
  else
    return NULL;
}



char * subst_func_add( const stringlist_type * args , void * ext_arg) {
  double sum = 0;
  bool OK = true;
  int index;
  for (index = 0; index < stringlist_get_size( args ); index++) {
    double term;
    if (util_sscanf_double( stringlist_iget(args , index ) , &term))
      sum += term;
    else
      OK = false;
  }

  if (OK)
    return util_alloc_sprintf("%g" , sum);
  else
    return NULL;
}


char * subst_func_mul( const stringlist_type * args , void * ext_arg) {
  double product = 0;
  bool OK = true;
  int index;
  for (index = 0; index < stringlist_get_size( args ); index++) {
    double factor;
    if (util_sscanf_double( stringlist_iget(args , index ) , &factor))
      product *= factor;
    else
      OK = false;
  }
  
  
  if (OK)
    return util_alloc_sprintf("%g" , product);
  else
    return NULL;
}
