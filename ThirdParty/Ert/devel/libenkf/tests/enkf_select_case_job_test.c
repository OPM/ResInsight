/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_export_field_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/util.h>
#include <ert/util/string_util.h>
#include <ert/util/bool_vector.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_main.h>


int main(int argc , const char ** argv) {
  enkf_main_install_SIGNALS();
  
  const char * config_path  = argv[1];
  const char * config_file  = argv[2];
    
  test_work_area_type * work_area = test_work_area_alloc(config_file );
  test_work_area_copy_directory_content( work_area , config_path );

  enkf_main_type * enkf_main = enkf_main_bootstrap( NULL , config_file , true , true );  
  stringlist_type * args = stringlist_alloc_new();
  
  stringlist_append_copy( args , "NewCase");

  test_assert_string_not_equal( "NewCase" , enkf_main_get_current_fs( enkf_main ));
  enkf_main_select_case_JOB( enkf_main , args );
  test_assert_string_equal( "NewCase" , enkf_main_get_current_fs( enkf_main ));

  stringlist_free( args );
  enkf_main_free( enkf_main );
  test_work_area_free(work_area); 
  exit(0);
}
