/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'test_work_area.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_TEST_WORK_AREA_H
#define ERT_TEST_WORK_AREA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/type_macros.hpp>

  typedef struct test_work_area_struct test_work_area_type;

  char                * test_work_area_alloc_input_path( const test_work_area_type * work_area , const char * input_path );
  test_work_area_type * test_work_area_alloc(const char * test_name );
  test_work_area_type * test_work_area_alloc_relative(const char * prefix , const char * test_path);
  void                  test_work_area_set_store( test_work_area_type * work_area , bool store);

  void                  test_work_area_free(test_work_area_type * work_area);
  const char          * test_work_area_get_cwd( const test_work_area_type * work_area );
  const char          * test_work_area_get_original_cwd( const test_work_area_type * work_area );
  void                  test_work_area_install_file( test_work_area_type * work_area , const char * input_src_file );
  void                  test_work_area_copy_directory( test_work_area_type * work_area , const char * input_directory);
  void                  test_work_area_copy_directory_content( test_work_area_type * work_area , const char * input_directory);
  void                  test_work_area_copy_file( test_work_area_type * work_area , const char * input_file);
  bool                  test_work_area_copy_parent_directory( test_work_area_type * work_area , const char * input_path);
  bool                  test_work_area_copy_parent_content( test_work_area_type * work_area , const char * input_path);
  void                  test_work_area_sync( test_work_area_type * work_area);

  test_work_area_type * temp_area_alloc_relative(const char * prefix , const char * test_path);
  test_work_area_type * temp_area_alloc(const char * test_path);

  UTIL_IS_INSTANCE_HEADER( test_work_area );

#ifdef __cplusplus
}
#endif
#endif
