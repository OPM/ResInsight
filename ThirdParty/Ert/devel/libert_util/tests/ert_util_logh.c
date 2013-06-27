/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ert_util_logh.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/util.h>
#include <ert/util/log.h>

#define LOG_FILE "/tmp/log.txt"

int main(int argc , char ** argv) {
  {
    log_type * logh = log_open( NULL , 0 );
    
    test_assert_false( log_is_open( logh ));
    log_reopen( logh , LOG_FILE );
    test_assert_true( log_is_open( logh ));
                
    log_close( logh );
  }
  
  {
    log_type * logh = log_open( LOG_FILE , 0 );
    test_assert_true( log_is_open( logh ));
    log_close( logh );
  }
  exit(0);
}
