/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ert_test_context.h' is part of ERT - Ensemble based
   Reservoir Tool.

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

#ifndef ERT_TEST_CONTEXT_H
#define ERT_TEST_CONTEXT_H

#include <stdbool.h>

#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_main.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ert_test_context_struct ert_test_context_type;

void                    ert_test_context_set_store( ert_test_context_type * test_context , bool store);
ert_test_context_type * ert_test_context_alloc( const char * test_name , const char * model_config);
ert_test_context_type * ert_test_context_alloc_python( const char * test_name , const char * model_config);
void                    ert_test_context_free( ert_test_context_type * test_context );
enkf_main_type        * ert_test_context_get_main( ert_test_context_type * test_context );
bool                    ert_test_context_install_workflow_job( ert_test_context_type * test_context , const char * job_name , const char * job_file);
bool                    ert_test_context_run_worklow_job( ert_test_context_type * test_context , const char * job_name, const stringlist_type * args);
void                    ert_test_context_fwrite_workflow_job( FILE * stream , const char * job_name , const stringlist_type * args);
bool                    ert_test_context_install_workflow( ert_test_context_type * test_context , const char * workflow_name , const char * workflow_file);
bool                    ert_test_context_run_worklow( ert_test_context_type * test_context , const char * workflow_name);
const            char * ert_test_context_get_cwd( const ert_test_context_type * test_context );

UTIL_IS_INSTANCE_HEADER( ert_test_context );

#ifdef __cplusplus
}
#endif

#endif


