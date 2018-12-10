/*
   Copyright (C) 2018  Statoil ASA, Norway.

   The file 'ert_util_split_path.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>

#include "detail/util/path.hpp"

using namespace ecl::util;

int main(int argc , char ** argv) {

  test_assert_std_string_equal( std::string(""), path::dirname("entry"));
  test_assert_std_string_equal( std::string("entry") , path::basename("entry"));
  test_assert_std_string_equal( std::string(""), path::extension("entry"));

  test_assert_std_string_equal( std::string("path"), path::dirname("path/base.ext"));
  test_assert_std_string_equal( std::string("base") , path::basename("path/base.ext"));
  test_assert_std_string_equal( std::string("ext"), path::extension("path/base.ext"));

  test_assert_std_string_equal( std::string("/tmp"), path::dirname("/tmp/file"));
  test_assert_std_string_equal( std::string("file") , path::basename("/tmp/file"));
  test_assert_std_string_equal( std::string(""), path::extension("/tmp/file"));

  test_assert_std_string_equal( std::string("/"), path::dirname("/tmp"));
  test_assert_std_string_equal( std::string("tmp") , path::basename("/tmp"));
  test_assert_std_string_equal( std::string(""), path::extension("/tmp"));

  test_assert_std_string_equal( std::string("/tmp/user.ext"), path::dirname("/tmp/user.ext/file.ext"));
  test_assert_std_string_equal( std::string("file") , path::basename("/tmp/user.ext/file.ext"));
  test_assert_std_string_equal( std::string("ext"), path::extension("/tmp/user.ext/file.ext"));

  test_assert_std_string_equal( std::string("/tmp/user.ext"), path::dirname("/tmp/user.ext/"));
  test_assert_std_string_equal( std::string("") , path::basename("/tmp/user.ext/"));
  test_assert_std_string_equal( std::string(""), path::extension("/tmp/user.ext/"));
}
