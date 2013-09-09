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
#include <ert/enkf/qc_module.h>
#include <ert/util/subst_list.h>

int main(int argc, char ** argv) {

  ert_workflow_list_type * list = NULL;
  qc_module_type * qc_module = qc_module_alloc(list, "");
 
  char * expected_path = util_alloc_abs_path(".ert_runpath_list");
  test_assert_string_equal(expected_path, qc_module_get_runpath_list_file(qc_module));
  free(expected_path);
  
  qc_module_set_runpath_list_file(qc_module, "Folder", NULL);
  expected_path = util_alloc_abs_path("Folder/.ert_runpath_list");
  test_assert_string_equal(expected_path, qc_module_get_runpath_list_file(qc_module));
  free(expected_path);
  
  qc_module_set_runpath_list_file(qc_module, "Folder", "thefilename.txt");
  expected_path = util_alloc_abs_path("Folder/thefilename.txt");
  test_assert_string_equal(expected_path, qc_module_get_runpath_list_file(qc_module));
  free(expected_path);
  
  qc_module_set_runpath_list_file(qc_module, "/tmp/ouagadogo", "thefilename.txt");
  test_assert_string_equal("/tmp/ouagadogo/thefilename.txt", qc_module_get_runpath_list_file(qc_module));
  
  qc_module_free(qc_module);

  exit(0);
}

