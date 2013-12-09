/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_fs.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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







void test_mount() {
  test_work_area_type * work_area = test_work_area_alloc("enkf_fs/mount");
  
  enkf_fs_create_fs("mnt" , BLOCK_FS_DRIVER_ID , NULL );
  {
    enkf_fs_type * fs = enkf_fs_mount( "mnt" , false );
    test_assert_true( enkf_fs_is_instance( fs ));
    enkf_fs_umount( fs );
  }
  test_work_area_free( work_area );
}


void test_refcount() {
  test_work_area_type * work_area = test_work_area_alloc("enkf_fs/refcount");
  
  enkf_fs_create_fs("mnt" , BLOCK_FS_DRIVER_ID , NULL );
  {
    enkf_fs_type * fs = enkf_fs_mount( "mnt" , false );
    test_assert_int_equal( 1 , enkf_fs_get_refcount( fs ));
    {
      enkf_fs_type * fs2 = enkf_fs_get_ref( fs );
      enkf_fs_type * fs3 = enkf_fs_get_weakref( fs );
      test_assert_int_equal( 2 , enkf_fs_get_refcount( fs ));    
      test_assert_int_equal( 2 , enkf_fs_get_refcount( fs2 ));    
      test_assert_int_equal( 2 , enkf_fs_get_refcount( fs3 ));    
      enkf_fs_umount( fs2 );
    }
    test_assert_int_equal( 1 , enkf_fs_get_refcount( fs ));
    enkf_fs_umount( fs );
  }
  test_work_area_free( work_area );
}

void test_read_only() {
  test_work_area_type * work_area = test_work_area_alloc("enkf_fs/read_only");

  enkf_fs_create_fs("mnt" , BLOCK_FS_DRIVER_ID , NULL );
  {
    enkf_fs_type * fs_false = enkf_fs_mount( "mnt" , false );
    test_assert_false(enkf_fs_is_read_only(fs_false));

    enkf_fs_umount( fs_false );

    enkf_fs_type * fs_true = enkf_fs_mount( "mnt" , true );
    test_assert_true(enkf_fs_is_read_only(fs_true));

    enkf_fs_umount( fs_true );
  }
  test_work_area_free( work_area );
}




int main(int argc, char ** argv) {
  test_mount();
  test_refcount();
  test_read_only();
  exit(0);
}
