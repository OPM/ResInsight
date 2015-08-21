/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'enkf_main_fs.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdbool.h>

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/enkf_main.h>
#include <ert/enkf/state_map.h>







int main(int argc, char ** argv) {
  const char * config_file = argv[1];
  test_work_area_type * work_area = test_work_area_alloc( "enkf_main_fs" );
  char * model_config;
  util_alloc_file_components( config_file , NULL , &model_config , NULL);
  test_work_area_copy_parent_content( work_area , config_file );
  {
    enkf_main_type * enkf_main = enkf_main_bootstrap( model_config , false , false );

    enkf_main_select_fs( enkf_main , "enkf");
    test_assert_true( enkf_main_case_is_current( enkf_main , "enkf"));
    test_assert_false( enkf_main_case_is_current( enkf_main , "default_fs"));
    test_assert_false( enkf_main_case_is_current( enkf_main , "does_not_exist"));

    test_assert_int_equal( 1 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
    {
      enkf_fs_type * fs_ref = enkf_main_get_fs_ref( enkf_main );
      test_assert_int_equal( 2 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
      enkf_fs_decref( fs_ref );
      test_assert_int_equal( 1 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
    }

    {
      state_map_type * map1 = enkf_fs_get_state_map( enkf_main_get_fs( enkf_main ));
      state_map_type * map2 = enkf_main_alloc_readonly_state_map(enkf_main , "enkf");
      test_assert_true(state_map_equal( map1 , map2 ));
      state_map_free( map2 );
    }
    {
      enkf_fs_type * fs1 = enkf_main_mount_alt_fs( enkf_main , "default" , false );
      enkf_fs_type * fs2 = enkf_main_mount_alt_fs( enkf_main , "enkf" , false );

      test_assert_int_equal( 2 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
      test_assert_int_equal( 2 , enkf_fs_get_refcount( fs2 ));
      test_assert_int_equal( 1 , enkf_fs_get_refcount( fs1 ));

      enkf_fs_decref( fs1 );
      enkf_fs_decref( fs2 );
    }

    {
      enkf_fs_type * enkf_fs = enkf_main_mount_alt_fs( enkf_main , "enkf" , false  );

      enkf_main_select_fs( enkf_main , "default");
      test_assert_int_equal( 1 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
      enkf_fs_decref( enkf_fs );
    }

    {
      enkf_fs_type * default_fs = enkf_main_mount_alt_fs( enkf_main , "default" , false );

      test_assert_int_equal( 2 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
      enkf_main_select_fs( enkf_main , "default");
      test_assert_int_equal( 2 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
      enkf_fs_decref( default_fs );
      test_assert_int_equal( 1 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
    }
    /*****************************************************************/
    {
      enkf_fs_type * fs = enkf_main_mount_alt_fs( enkf_main , "default" , false  );
      test_assert_int_equal( 2 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));

      enkf_main_set_fs( enkf_main , fs , NULL );
      enkf_fs_decref( fs );
      test_assert_int_equal( 1 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
    }
    {
      enkf_fs_type * fs = enkf_main_mount_alt_fs( enkf_main , "enkf" , false );
      enkf_fs_type * current = enkf_main_mount_alt_fs( enkf_main , "default" , false );

      test_assert_int_equal( 2 , enkf_fs_get_refcount( current ));
      test_assert_int_equal( 1 , enkf_fs_get_refcount( fs));
      enkf_main_set_fs( enkf_main , fs , NULL);
      test_assert_int_equal( 2 , enkf_fs_get_refcount( fs));
      test_assert_int_equal( 1 , enkf_fs_get_refcount( current ));

      enkf_fs_decref( current );
      enkf_fs_decref( fs);
    }




    test_assert_int_equal( 1 , enkf_fs_get_refcount( enkf_main_get_fs( enkf_main )));
    enkf_main_free( enkf_main );
  }
  test_work_area_free( work_area );
  exit(0);
}
