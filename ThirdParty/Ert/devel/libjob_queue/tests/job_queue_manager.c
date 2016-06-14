/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'job_queue_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>

#include <ert/job_queue/job_queue.h>
#include <ert/job_queue/job_queue_manager.h>




void test_create() {
  job_queue_type * job_queue = job_queue_alloc( 100 , "OK" , "STATUS", "ERROR");
  job_queue_manager_type * manager = job_queue_manager_alloc( job_queue );
  
  test_assert_true( job_queue_manager_is_instance( manager ));
  
  job_queue_manager_free( manager );
  job_queue_free( job_queue );
}






int main( int argc , char ** argv) {
  test_create();
  exit(0);
}

