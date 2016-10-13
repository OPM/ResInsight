/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'ert_init_context.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_INIT_CONTEXT_H
#define ERT_INIT_CONTEXT_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/type_macros.h>
#include <ert/util/bool_vector.h>
#include <ert/util/path_fmt.h>
#include <ert/util/subst_list.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/run_arg.h>
#include <ert/enkf/enkf_fs.h>

typedef struct ert_init_context_struct ert_init_context_type;

  stringlist_type      * ert_init_context_alloc_runpath_list(const bool_vector_type * iactive , path_fmt_type * runpath_fmt , subst_list_type * subst_list , int iter);
  char                 * ert_init_context_alloc_runpath( int iens , path_fmt_type * runpath_fmt , subst_list_type * subst_list , int iter);

  ert_init_context_type * ert_init_context_alloc(enkf_fs_type * init_fs , const bool_vector_type * iactive ,
                                                 path_fmt_type * runpath_fmt ,
                                                 subst_list_type * subst_list ,
                                                 init_mode_type init_mode ,
                                                 int iter);

  void                     ert_init_context_free( ert_init_context_type * );
  int                      ert_init_context_get_size( const ert_init_context_type * context );
  init_mode_type           ert_init_context_get_init_mode( const ert_init_context_type * context );
  bool_vector_type       * ert_init_context_get_iactive( const ert_init_context_type * context );
  int                      ert_init_context_get_iter( const ert_init_context_type * context );
  run_arg_type           * ert_init_context_iget_arg( const ert_init_context_type * context , int index);
  run_arg_type           * ert_init_context_iens_get_arg( const ert_init_context_type * context , int iens);


  UTIL_IS_INSTANCE_HEADER( ert_init_context );


#ifdef __cplusplus
}
#endif
#endif


