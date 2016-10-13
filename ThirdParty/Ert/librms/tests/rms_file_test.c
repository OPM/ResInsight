/*
   Copyright (C) 2014  Statoil ASA, Norway.
    
   The file 'rms_file_test.c' is part of ERT - Ensemble based Reservoir Tool.
    
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
#include <ert/util/util.h>

#include <ert/rms/rms_util.h>
#include <ert/rms/rms_file.h>



void test_rms_file_fread_alloc_data_tag(rms_file_type * rms_file) {
  rms_tag_type * parameter_tag = rms_file_fread_alloc_tag(rms_file , "parameter" , NULL , NULL);
  test_assert_not_NULL(parameter_tag);
  test_assert_string_equal("parameter", rms_tag_get_name(parameter_tag));
  rms_tag_free(parameter_tag);
}


void test_rms_file_fread_alloc_data_tagkey(rms_file_type *rms_file) {
  rms_tagkey_type * name_tagkey = rms_file_fread_alloc_data_tagkey(rms_file , "parameter" , NULL , NULL);
  test_assert_not_NULL(name_tagkey);
  test_assert_int_equal(rms_float_type, rms_tagkey_get_rms_type(name_tagkey));
  rms_tagkey_free(name_tagkey);
}


int main(int argc , char ** argv) {
  const char * filename    = argv[1];
  rms_file_type * rms_file = rms_file_alloc(filename , false);
  test_assert_not_NULL(rms_file);

  test_rms_file_fread_alloc_data_tag(rms_file);
  test_rms_file_fread_alloc_data_tagkey(rms_file);

  rms_file_free(rms_file);
  exit(0);
}
