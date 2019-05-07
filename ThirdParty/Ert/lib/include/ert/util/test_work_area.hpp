/*
   Copyright (C) 2013  Equinor ASA, Norway.

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

#include <string>

namespace ecl {
namespace util {

class TestArea {
public:
    TestArea(const std::string& test_name, bool store_area = false);
    ~TestArea();
    const std::string& test_cwd() const;
    const std::string& original_cwd() const;

    void copy_directory(const std::string input_directory) const;
    void copy_directory_content(const std::string input_directory) const;
    bool copy_parent(const std::string input_path) const;
    bool copy_parent_content(const std::string original_path) const;

    void copy_file(const std::string& input_src_file) const;
    std::string original_path(const std::string& input_path) const;

private:
    bool store;
    std::string cwd;
    std::string org_cwd;
};


}
}

typedef ecl::util::TestArea test_work_area_type;

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

  char                * test_work_area_alloc_input_path( const test_work_area_type * work_area , const char * input_path );
  test_work_area_type * test_work_area_alloc(const char * test_name );
  test_work_area_type * test_work_area_alloc__(const char * test_name, bool store_area );
  test_work_area_type * test_work_area_alloc_relative(const char * prefix , const char * test_path);

  void                  test_work_area_free(test_work_area_type * work_area);
  const char          * test_work_area_get_cwd( const test_work_area_type * work_area );
  const char          * test_work_area_get_original_cwd( const test_work_area_type * work_area );
  void                  test_work_area_install_file( const test_work_area_type * work_area , const char * input_src_file );
  void                  test_work_area_copy_directory( const test_work_area_type * work_area , const char * input_directory);
  void                  test_work_area_copy_directory_content( const test_work_area_type * work_area , const char * input_directory);
  void                  test_work_area_copy_file( const test_work_area_type * work_area , const char * input_file);
  bool                  test_work_area_copy_parent_directory( const test_work_area_type * work_area , const char * input_path);
  bool                  test_work_area_copy_parent_content( const test_work_area_type * work_area , const char * input_path);

#ifdef __cplusplus
}
#endif
#endif
