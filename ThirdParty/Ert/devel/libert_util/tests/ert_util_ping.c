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
  stringlist_type * server_list = stringlist_alloc_from_split( argv[1] , " ");
  argc = stringlist_get_size( server_list );
  
  if (argc >= 1)
    test_assert_true( util_ping( stringlist_iget( server_list , 0 )));

  if (argc >= 2)
    test_assert_false( util_ping( stringlist_iget( server_list , 1 )));
  
  if (argc >= 3)
    test_assert_false( util_ping( stringlist_iget( server_list , 2 )));
  
  exit(0);
}
