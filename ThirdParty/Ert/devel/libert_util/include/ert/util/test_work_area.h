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


#ifndef __TEST_WORK_AREA_H__
#define __TEST_WORK_AREA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

  typedef struct test_work_area_struct test_work_area_type;

  test_work_area_type * test_work_area_alloc(const char * test_name , bool store);
  void                  test_work_area_free(test_work_area_type * work_area);
  const char          * test_work_area_get_cwd( const test_work_area_type * work_area ); 
  const char          * test_work_area_get_original_cwd( const test_work_area_type * work_area );
  void                  test_work_area_install_file( test_work_area_type * work_area , const char * input_src_file );
  void                  test_work_area_copy_directory( test_work_area_type * work_area , const char * input_directory);
  void                  test_work_area_copy_directory_content( test_work_area_type * work_area , const char * input_directory);
  void                  test_work_area_copy_file( test_work_area_type * work_area , const char * input_file);

#ifdef __cplusplus
}
#endif
#endif
