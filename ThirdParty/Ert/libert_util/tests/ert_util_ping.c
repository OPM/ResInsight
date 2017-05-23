/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_ping.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/stringlist.h>
#include <ert/util/util.h>



int main( int argc , char ** argv) {
  test_install_SIGNALS();
  test_assert_true( util_ping("localhost" ));
  test_assert_true( util_ping("127.0.0.1" ));
  test_assert_false( util_ping("does.not.exist"));

  exit(0);
}
