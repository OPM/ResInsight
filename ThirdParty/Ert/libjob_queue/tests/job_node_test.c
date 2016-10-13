/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'job_node_test.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include "ert/util/build_config.h"

#include <ert/util/test_util.h>
#include <ert/util/test_util_abort.h>
#include <ert/job_queue/job_node.h>


void test_create() {
  job_queue_node_type * node = job_queue_node_alloc_simple("name" , "/tmp" , "/bin/ls" , 0 , NULL);
  test_assert_true( job_queue_node_is_instance( node ));
  job_queue_node_free( node );
}



void call_get_queue_index( void * arg ) {
  job_queue_node_type * node = job_queue_node_safe_cast( arg );
  job_queue_node_get_queue_index( node );
}




void test_queue_index() {
  job_queue_node_type * node = job_queue_node_alloc_simple( "name" , "/tmp" , "/bin/ls" , 0 , NULL );
  test_assert_util_abort("job_queue_node_get_queue_index" , call_get_queue_index , node );
}


void test_path_does_not_exist() {
  job_queue_node_type * node = job_queue_node_alloc_simple( "name" , "does-not-exist" , "/bin/ls" , 0 , NULL);
  test_assert_NULL( node );
}


int main( int argc , char ** argv) {
  util_install_signals();
  test_create();
  test_queue_index();
  test_path_does_not_exist();
}
