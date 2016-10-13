/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ert_init_context.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/type_macros.h>
#include <ert/util/vector.h>
#include <ert/util/path_fmt.h>
#include <ert/util/subst_list.h>
#include <ert/util/int_vector.h>
#include <ert/util/stringlist.h>
#include <ert/util/type_vector_functions.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/run_arg.h>
#include <ert/enkf/ert_init_context.h>


#define ERT_INIT_CONTEXT_TYPE_ID 555341328


struct ert_init_context_struct {
  UTIL_TYPE_ID_DECLARATION;
  vector_type      * run_args;
  bool_vector_type * iactive;   // This can be updated ....
  init_mode_type     init_mode;
  int                iter;
  int_vector_type  * iens_map;

};




char * ert_init_context_alloc_runpath( int iens , path_fmt_type * runpath_fmt , subst_list_type * subst_list , int iter) {
  char * runpath;
  {
    char * first_pass = path_fmt_alloc_path(runpath_fmt , false , iens, iter);    /* 1: Replace first %d with iens, if a second %d replace with iter */

    if (subst_list)
      runpath = subst_list_alloc_filtered_string( subst_list , first_pass );         /* 2: Filter out various magic strings like <CASE> and <CWD>. */
    else
      runpath = util_alloc_string_copy( first_pass );

    free( first_pass );
  }
  return runpath;
}


stringlist_type * ert_init_context_alloc_runpath_list(const bool_vector_type * iactive , path_fmt_type * runpath_fmt , subst_list_type * subst_list , int iter) {
  stringlist_type * runpath_list = stringlist_alloc_new();
  for (int iens = 0; iens < bool_vector_size( iactive ); iens++) {

    if (bool_vector_iget( iactive , iens ))
      stringlist_append_owned_ref( runpath_list , ert_init_context_alloc_runpath(iens , runpath_fmt , subst_list , iter));
    else
      stringlist_append_ref( runpath_list , NULL );

  }
  return runpath_list;
}


static ert_init_context_type * ert_init_context_alloc1(const bool_vector_type * iactive , init_mode_type init_mode, int iter) {
  ert_init_context_type * context = util_malloc( sizeof * context );
  UTIL_TYPE_ID_INIT( context , ERT_INIT_CONTEXT_TYPE_ID );

  context->iactive = bool_vector_alloc_copy( iactive );
  context->iens_map = bool_vector_alloc_active_index_list( iactive , -1 );
  context->run_args = vector_alloc_new();
  context->init_mode = init_mode;
  context->iter = iter;

  return context;
}

ert_init_context_type * ert_init_context_alloc(enkf_fs_type * init_fs , const bool_vector_type * iactive ,
                                               path_fmt_type * runpath_fmt ,
                                               subst_list_type * subst_list ,
                                               init_mode_type init_mode ,
                                               int iter) {

  ert_init_context_type * context = ert_init_context_alloc1( iactive , init_mode , iter );
  {
    stringlist_type * runpath_list = ert_init_context_alloc_runpath_list( iactive , runpath_fmt , subst_list , iter );
    for (int iens = 0; iens < bool_vector_size( iactive ); iens++) {
      if (bool_vector_iget( iactive , iens )) {
        run_arg_type * arg = run_arg_alloc_INIT_ONLY( init_fs , iens , iter , stringlist_iget( runpath_list , iens));
        vector_append_owned_ref( context->run_args , arg , run_arg_free__);
      }
    }
    stringlist_free( runpath_list );
  }
  return context;
}




UTIL_IS_INSTANCE_FUNCTION( ert_init_context , ERT_INIT_CONTEXT_TYPE_ID );



void ert_init_context_free( ert_init_context_type * context ) {

  vector_free( context->run_args );
  bool_vector_free( context->iactive );
  int_vector_free( context->iens_map );
  free( context );
}


int ert_init_context_get_size( const ert_init_context_type * context ) {
  return vector_get_size( context->run_args );
}



init_mode_type ert_init_context_get_init_mode( const ert_init_context_type * context ) {
  return context->init_mode;
}


bool_vector_type * ert_init_context_get_iactive( const ert_init_context_type * context ) {
  return context->iactive;
}


run_arg_type * ert_init_context_iget_arg( const ert_init_context_type * context , int index) {
  return vector_iget( context->run_args , index );
}


run_arg_type * ert_init_context_iens_get_arg( const ert_init_context_type * context , int iens) {
  int index = int_vector_iget( context->iens_map , iens );
  if (index >= 0)
    return vector_iget( context->run_args , index );
  else
    return NULL;
}
