/*
  Copyright (C) 2019  Equinor ASA, Norway.

  The file 'test_area.cpp' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string>

#include <ert/util/test_util.hpp>
#include <ert/util/test_work_area.hpp>

void test_create() {
    ecl::util::TestArea ta("Name");

    test_assert_true( ta.test_cwd() != ta.original_cwd() );

}



int main(int argc, char **argv) {
    test_create();
}
