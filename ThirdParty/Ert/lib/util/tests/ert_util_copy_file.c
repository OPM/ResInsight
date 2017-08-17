/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'enkf_util_copy_file.c' is part of ERT - Ensemble based Reservoir Tool.
    
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
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/util.h>
#include <ert/util/string_util.h>



void test_copy_file( const char * executable ) {
  struct stat stat_buf;
  mode_t mode0,mode1;
  stat( executable , &stat_buf );
  
  mode0 = stat_buf.st_mode;
  {
    test_work_area_type * test_area = test_work_area_alloc( "executable-copy" );

    util_copy_file( executable , "test.x");
    test_assert_true( util_file_exists( "test.x" ));
    stat( "test.x" , &stat_buf );
    mode1 = stat_buf.st_mode;
    
    test_assert_true( mode0 == mode1 );
    test_work_area_free( test_area );
  }
}



int main(int argc , char ** argv) {
   const char * executable = argv[1];
   test_copy_file( executable );
   exit(0);
}
