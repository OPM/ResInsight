/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'job_lsb.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <dlfcn.h>

#include <ert/util/test_util.h>

#include <ert/job_queue/lsb.h>

/*
  This test should ideally be run twice in two different environments;
  with and without dlopen() access to the lsf libraries.
*/

int main( int argc , char ** argv) {
  lsb_type * lsb = lsb_alloc();

  test_assert_not_NULL( lsb );
  if (!lsb_ready(lsb)) {
    const stringlist_type * error_list = lsb_get_error_list( lsb );
    stringlist_fprintf(error_list , "\n", stdout);
  }

  if (dlopen( "libbat.so" , RTLD_NOW | RTLD_GLOBAL))
    test_assert_true( lsb_ready( lsb ));
  else
    test_assert_false( lsb_ready( lsb ));
                      
  lsb_free( lsb );
  exit(0);
}
