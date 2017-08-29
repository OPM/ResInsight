/*
   Copyright (C) 2017  Statoil ASA, Norway.

   The file 'test_ecl_file_index.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdio.h>
#include <utime.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/test_work_area.h>

#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_file.h>

void test_load_nonexisting_file() {
   ecl_file_type * ecl_file = ecl_file_fast_open("base_file", "a_file_that_does_not_exist_2384623", 0);
   test_assert_NULL( ecl_file );
}


void test_create_and_load_index_file() {

   test_work_area_type * work_area = test_work_area_alloc("ecl_file_index_testing");
   {
      char * file_name = "initial_data_file";
      char * index_file_name = "index_file";

      //creating the data file
      size_t data_size = 10;
      ecl_kw_type * kw1 = ecl_kw_alloc("TEST1_KW", data_size, ECL_INT);
      for(int i = 0; i < data_size; ++i)
         ecl_kw_iset_int(kw1, i, 537 + i);
      fortio_type * fortio = fortio_open_writer(file_name, false, ECL_ENDIAN_FLIP);
      ecl_kw_fwrite(kw1, fortio); 
      
      data_size = 5;
      ecl_kw_type * kw2 = ecl_kw_alloc("TEST2_KW", data_size, ECL_FLOAT);
      for(int i = 0; i < data_size; ++i)
         ecl_kw_iset_float(kw2, i, 0.15 * i);
      ecl_kw_fwrite(kw2, fortio);
      fortio_fclose(fortio); 
      //finished creating data file

      //creating ecl_file
      ecl_file_type * ecl_file = ecl_file_open( file_name , 0 );
      test_assert_true( ecl_file_has_kw( ecl_file , "TEST1_KW" )  );
      ecl_file_write_index( ecl_file , index_file_name);
      int ecl_file_size = ecl_file_get_size( ecl_file );
      ecl_file_close( ecl_file ); 
      //finished using ecl_file

      test_assert_false( ecl_file_index_valid(file_name, "nofile"));
      test_assert_false( ecl_file_index_valid("nofile", index_file_name));

      struct utimbuf tm1 = {1 , 1};
      struct utimbuf tm2 = {2 , 2};
      utime(file_name, &tm2);
      utime(index_file_name, &tm1);
      test_assert_false( ecl_file_index_valid(file_name, index_file_name) );
      utime(file_name, &tm1);
      utime(index_file_name, &tm2);
      test_assert_true( ecl_file_index_valid(file_name, index_file_name) );

      ecl_file_type * ecl_file_index = ecl_file_fast_open( file_name, index_file_name , 0);
      test_assert_true( ecl_file_is_instance(ecl_file_index)  );
      test_assert_true( ecl_file_get_global_view(ecl_file_index) );

      test_assert_int_equal(ecl_file_size, ecl_file_get_size(ecl_file_index) );    
  
      test_assert_true( ecl_file_has_kw( ecl_file_index, "TEST1_KW" )  );
      test_assert_true( ecl_file_has_kw( ecl_file_index, "TEST2_KW" )  );

      ecl_kw_type * kwi1 = ecl_file_iget_kw( ecl_file_index , 0 );  
      test_assert_true (ecl_kw_equal(kw1, kwi1));
      test_assert_double_equal( 537.0, ecl_kw_iget_as_double(kwi1, 0)  );
      test_assert_double_equal( 546.0, ecl_kw_iget_as_double(kwi1, 9)  );

      ecl_kw_type * kwi2 = ecl_file_iget_kw( ecl_file_index , 1 );
      test_assert_true (ecl_kw_equal(kw2, kwi2));
      test_assert_double_equal( 0.15, ecl_kw_iget_as_double(kwi2, 1)  );
      test_assert_double_equal( 0.60, ecl_kw_iget_as_double(kwi2, 4)  );     

      ecl_kw_free(kw1);
      ecl_kw_free(kw2);
      ecl_file_close( ecl_file_index );
   }
   test_work_area_free( work_area );
}


int main( int argc , char ** argv) {
   util_install_signals();
   test_load_nonexisting_file();
   test_create_and_load_index_file();
}
