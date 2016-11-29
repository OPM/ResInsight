/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'enkf_analysis_config_analysis_load.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/enkf/ert_test_context.h>

#include <ert/enkf/site_config.h>
#include <ert/enkf/analysis_config.h>

int main(int argc , const char ** argv)  {
  util_install_signals();
  {
    const char * config_file = argv[1];

    ert_test_context_type * test_context = ert_test_context_alloc("AnalysisLoadFromSiteConfig" , config_file);
    enkf_main_type * enkf_main = ert_test_context_get_main(test_context);

    test_assert_true(analysis_config_has_module(enkf_main_get_analysis_config(enkf_main), "RML_ENKF_SITE_CONFIG1"));
    analysis_module_type * analysis_module = analysis_config_get_module(enkf_main_get_analysis_config(enkf_main), "RML_ENKF_SITE_CONFIG1");
    test_assert_string_equal(analysis_module_get_name(analysis_module), "RML_ENKF_SITE_CONFIG1");

    test_assert_true(analysis_config_has_module(enkf_main_get_analysis_config(enkf_main), "RML_ENKF_SITE_CONFIG2"));
    analysis_module_type * analysis_module2 = analysis_config_get_module(enkf_main_get_analysis_config(enkf_main), "RML_ENKF_SITE_CONFIG2");
    test_assert_string_equal(analysis_module_get_name(analysis_module2), "RML_ENKF_SITE_CONFIG2");

    ert_test_context_free(test_context);
  }
}


