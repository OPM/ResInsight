/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'enkf_ecl_config.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/util.h>
#include <ert/util/test_util.h>
#include <ert/enkf/hook_manager.h>
#include <ert/util/subst_list.h>

int main(int argc, char ** argv) {

  ert_workflow_list_type * list = NULL;
  hook_manager_type * hook_manager = hook_manager_alloc(list);

  char * expected_path = util_alloc_abs_path(".ert_runpath_list");
  test_assert_string_equal(expected_path, hook_manager_get_runpath_list_file(hook_manager));
  free(expected_path);

  hook_manager_set_runpath_list_file(hook_manager, "Folder", NULL);
  expected_path = util_alloc_abs_path("Folder/.ert_runpath_list");
  test_assert_string_equal(expected_path, hook_manager_get_runpath_list_file(hook_manager));
  free(expected_path);

  hook_manager_set_runpath_list_file(hook_manager, "Folder", "thefilename.txt");
  expected_path = util_alloc_abs_path("Folder/thefilename.txt");
  test_assert_string_equal(expected_path, hook_manager_get_runpath_list_file(hook_manager));
  free(expected_path);

  hook_manager_set_runpath_list_file(hook_manager, "/tmp/ouagadogo", "thefilename.txt");
  test_assert_string_equal("/tmp/ouagadogo/thefilename.txt", hook_manager_get_runpath_list_file(hook_manager));

  hook_manager_free(hook_manager);

  exit(0);
}

