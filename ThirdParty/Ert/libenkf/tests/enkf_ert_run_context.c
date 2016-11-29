/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ert_run_context.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.h>
#include <ert/util/path_fmt.h>
#include <ert/util/subst_list.h>

#include <ert/enkf/ert_run_context.h>
#include <ert/enkf/ert_init_context.h>
#include <ert/enkf/run_arg.h>

void test_create() {
  bool_vector_type * iactive = bool_vector_alloc(10,true);
  bool_vector_iset( iactive , 6 , false );
  bool_vector_iset( iactive , 8 , false );
  {
    enkf_fs_type * init_fs = NULL;
    subst_list_type * subst_list = subst_list_alloc( NULL );
    path_fmt_type * runpath_fmt = path_fmt_alloc_directory_fmt("/tmp/path/%04d");
    ert_init_context_type * context = ert_init_context_alloc( init_fs ,  iactive , runpath_fmt , subst_list , INIT_CONDITIONAL , 13 );

    test_assert_true( ert_init_context_is_instance( context ));
    test_assert_int_equal( 8 , ert_init_context_get_size( context ));

    {
      run_arg_type * run_arg0 = ert_init_context_iget_arg( context , 0 );

      test_assert_int_equal( 13 , run_arg_get_iter( run_arg0 ));
      test_assert_string_equal( "/tmp/path/0000" , run_arg_get_runpath( run_arg0 ));

      test_assert_true( run_arg_is_instance( run_arg0 ));
    }
    ert_init_context_free( context );
    path_fmt_free( runpath_fmt );
  }
  bool_vector_free( iactive );
}


void test_create_ENSEMBLE_EXPERIMENT() {
  bool_vector_type * iactive = bool_vector_alloc(10,true);
  bool_vector_iset( iactive , 0 , false );
  bool_vector_iset( iactive , 8 , false );
  {
    subst_list_type * subst_list = subst_list_alloc( NULL );
    path_fmt_type * runpath_fmt = path_fmt_alloc_directory_fmt("/tmp/path/%04d/%d");
    enkf_fs_type * fs = NULL;
    ert_run_context_type * context = ert_run_context_alloc_ENSEMBLE_EXPERIMENT( fs, iactive , runpath_fmt , subst_list , 7 );

    test_assert_true( ert_run_context_is_instance( context ));
    test_assert_int_equal( 8 , ert_run_context_get_size( context ));

    {
      run_arg_type * run_arg0 = ert_run_context_iens_get_arg( context , 0 );
      run_arg_type * run_arg2 = ert_run_context_iens_get_arg( context , 2 );
      run_arg_type * run_argi = ert_run_context_iget_arg( context , 1 );

      test_assert_NULL( run_arg0 );
      test_assert_true( run_arg_is_instance( run_argi ));
      test_assert_ptr_equal( run_arg2 , run_argi);
    }

    {
      run_arg_type * run_arg1 = ert_run_context_iget_arg( context , 1 );

      test_assert_int_equal( 7 , run_arg_get_iter( run_arg1 ));
      test_assert_string_equal( "/tmp/path/0002/7" , run_arg_get_runpath( run_arg1 ));

      test_assert_true( run_arg_is_instance( run_arg1 ));
    }
    ert_run_context_free( context );
    path_fmt_free( runpath_fmt );
    subst_list_free( subst_list );
  }
  bool_vector_free( iactive );
}




void test_iactive_update() {
  bool_vector_type * iactive = bool_vector_alloc(10,true);
  {
    subst_list_type * subst_list = subst_list_alloc( NULL );
    path_fmt_type * runpath_fmt = path_fmt_alloc_directory_fmt("/tmp/path/%04d/%d");
    enkf_fs_type * fs = NULL;
    ert_run_context_type * context = ert_run_context_alloc_ENSEMBLE_EXPERIMENT( fs, iactive , runpath_fmt , subst_list , 7 );

    ert_run_context_deactivate_realization( context , 0 );
    ert_run_context_deactivate_realization( context , 5 );
    ert_run_context_deactivate_realization( context , 9 );

    ert_run_context_free( context );
    path_fmt_free( runpath_fmt );
    subst_list_free( subst_list );
  }
  test_assert_int_equal( bool_vector_count_equal( iactive , true ) , 7 );
  test_assert_false( bool_vector_iget( iactive , 0 ));
  test_assert_false( bool_vector_iget( iactive , 5 ));
  test_assert_false( bool_vector_iget( iactive , 9 ));
  bool_vector_free( iactive );
}


int main( int argc , char ** argv) {
  test_create();
  test_create_ENSEMBLE_EXPERIMENT();
  test_iactive_update();
  exit(0);
}
