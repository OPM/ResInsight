/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'config_error.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/util.h>

#include <ert/config/config_error.h>

int main(int argc , char ** argv) {
  config_error_type * config_error = config_error_alloc();

  {
    config_error_type * error_copy = config_error_alloc_copy( config_error );

    test_assert_true( config_error_equal( config_error , error_copy ));
    test_assert_ptr_not_equal( config_error , error_copy );
    
    config_error_free( error_copy );
  }
  
  config_error_free( config_error );
  exit(0);
}

