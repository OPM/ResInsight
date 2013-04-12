/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_refcase_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/thread_pool.h>
#include <ert/util/stringlist.h>
#include <ert/util/arg_pack.h>

#include <ert/config/config.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/site_config.h>
#include <ert/enkf/ecl_refcase_list.h>

#include <ert/enkf/site_config.h>


int main(int argc , char ** argv) {
  const char * case1     = argv[1];
  const char * case_glob = argv[2];

  {
    ecl_refcase_list_type * refcase_list = ecl_refcase_list_alloc( );
    
    test_assert_false( ecl_refcase_list_has_case( refcase_list , "DoesNotExist" ));
    test_assert_NULL( ecl_refcase_list_get_case( refcase_list , "DoesNotExist"));
    
    test_assert_int_equal( ecl_refcase_list_add_matching( refcase_list , "DoesNotExist") , 0);
    ecl_refcase_list_add_case( refcase_list , "DoesNotExist" );
    test_assert_false( ecl_refcase_list_has_case( refcase_list , "DoesNotExist"));
    
    ecl_refcase_list_add_case( refcase_list , case1 );
    test_assert_true( ecl_refcase_list_has_case( refcase_list , case1));


    test_assert_not_NULL( refcase_list );
    test_assert_false( ecl_refcase_list_has_default( refcase_list ));
    test_assert_NULL( ecl_refcase_list_get_default( refcase_list ));

    test_assert_false( ecl_refcase_list_set_default( refcase_list , "DoesNotExist"));
    test_assert_false( ecl_refcase_list_has_default( refcase_list ));
    test_assert_NULL( ecl_refcase_list_get_default( refcase_list ));
    test_assert_int_equal( 1 , ecl_refcase_list_get_size( refcase_list ));
    
    test_assert_true( ecl_refcase_list_set_default( refcase_list , case1));
    test_assert_true( ecl_refcase_list_has_default( refcase_list ));
    test_assert_not_NULL( ecl_refcase_list_get_default( refcase_list ));
    test_assert_int_equal( 1 , ecl_refcase_list_get_size( refcase_list ));
    
    test_assert_false( ecl_refcase_list_set_default( refcase_list , "DoesNotExist"));
    test_assert_true( ecl_refcase_list_has_default( refcase_list ));
    test_assert_not_NULL( ecl_refcase_list_get_default( refcase_list ));
    test_assert_int_equal( 1 , ecl_refcase_list_get_size( refcase_list ));
    test_assert_NULL( ecl_refcase_list_iget_case( refcase_list , 100));
    
    ecl_refcase_list_free( refcase_list );
  }

  {
    ecl_refcase_list_type * refcase_list = ecl_refcase_list_alloc( );
    test_assert_int_equal( ecl_refcase_list_add_matching( refcase_list , case_glob ) , 11);
    test_assert_int_equal( 11 , ecl_refcase_list_get_size( refcase_list ));
    
    test_assert_true( ecl_refcase_list_set_default( refcase_list , case1));
    test_assert_true( ecl_refcase_list_has_default( refcase_list ));
    test_assert_not_NULL( ecl_refcase_list_get_default( refcase_list ));
    test_assert_int_equal( 11 , ecl_refcase_list_get_size( refcase_list ));
    
    test_assert_int_equal( ecl_refcase_list_add_matching( refcase_list , case_glob ) , 0);
    test_assert_int_equal( ecl_refcase_list_add_matching( refcase_list , case_glob ) , 0);
    {
      const ecl_sum_type * ecl_sum = ecl_refcase_list_iget_case( refcase_list , 0 );
      test_assert_not_NULL( ecl_sum );
    }
    test_assert_int_equal( 11 , ecl_refcase_list_get_size( refcase_list ));
    
    {
      stringlist_type * case_list = stringlist_alloc_new( );
      const int N = ecl_refcase_list_get_size( refcase_list );
      int i;
      for (i=0; i < N; i++)
        stringlist_append_ref( case_list , ecl_refcase_list_iget_pathcase( refcase_list , N - 1 - i ));

      {
        bool equal = true;
        for (i=0; i < N; i++)
          equal = equal && util_string_equal( stringlist_iget( case_list , i ) , ecl_refcase_list_iget_pathcase( refcase_list , i));
        
        test_assert_false( equal );
        stringlist_sort( case_list , util_strcmp_int);

        equal = true;
        for (i=0; i < N; i++)
          equal = equal && util_string_equal( stringlist_iget( case_list , i ) , ecl_refcase_list_iget_pathcase( refcase_list , i));        
        test_assert_true( equal );
      }

    }
    ecl_refcase_list_add_matching( refcase_list , "DoesNotExist*");
    test_assert_int_equal( 11 , ecl_refcase_list_get_size( refcase_list ));
    ecl_refcase_list_free( refcase_list );
  }


  exit(0);
}

