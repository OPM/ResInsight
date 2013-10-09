/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_active_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/enkf/active_list.h>



int main(int argc , char ** argv) {
  active_list_type * active_list1 = active_list_alloc(  );
  active_list_type * active_list2 = active_list_alloc(  );
  
  
  test_assert_true( active_list_is_instance( active_list1 ));
  
  test_assert_true( active_list_equal( active_list1 , active_list2 ));
  
  active_list_add_index( active_list1 , 11 );
  test_assert_false(active_list_equal( active_list1 , active_list2 ));

  active_list_add_index( active_list1 , 12 );
  test_assert_false(active_list_equal( active_list1 , active_list2 ));

  active_list_add_index( active_list2 , 11 );
  test_assert_false(active_list_equal( active_list1 , active_list2 ));

  active_list_add_index( active_list2 , 12 );
  test_assert_true(active_list_equal( active_list1 , active_list2 ));

  active_list_add_index( active_list2 , 13 );
  test_assert_false(active_list_equal( active_list1 , active_list2 ));
  
  active_list_add_index( active_list1 , 13 );
  test_assert_true(active_list_equal( active_list1 , active_list2 ));

  active_list_add_index( active_list2 , 27 );
  test_assert_false(active_list_equal( active_list1 , active_list2 ));
  active_list_copy( active_list1 , active_list2 );
  test_assert_true(active_list_equal( active_list1 , active_list2 ));

  active_list_free( active_list1 );
  active_list_free( active_list2 );
  exit(0);
}

