/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'well_segment_collection.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/ecl/ecl_util.h>

#include <ert/ecl_well/well_const.h>
#include <ert/ecl_well/well_segment_collection.h>


int main(int argc , char ** argv) {
  test_install_SIGNALS();

  double * rseg_data = util_calloc( 100 , sizeof * rseg_data );
  well_segment_collection_type * sc = well_segment_collection_alloc();
  test_assert_not_NULL( sc );
  test_assert_int_equal( well_segment_collection_get_size( sc ) , 0 );

  {
    int outlet_segment_id = WELL_SEGMENT_OUTLET_END_VALUE;
    int branch_nr = WELL_SEGMENT_BRANCH_INACTIVE_VALUE;
    well_segment_type * ws = well_segment_alloc(89 , outlet_segment_id , branch_nr, rseg_data);
    
    well_segment_collection_add( sc , ws );
    test_assert_int_equal( well_segment_collection_get_size( sc ) , 1);
    test_assert_ptr_equal( well_segment_collection_iget( sc , 0 ) , ws );
    
    test_assert_false( well_segment_collection_has_segment( sc , 451 ));
    test_assert_true( well_segment_collection_has_segment( sc , 89 ));
    test_assert_ptr_equal( well_segment_collection_get( sc , 89 ) , ws );
  }

  {
    int outlet_segment_id = WELL_SEGMENT_OUTLET_END_VALUE;
    int branch_nr = WELL_SEGMENT_BRANCH_INACTIVE_VALUE;
    well_segment_type * ws = well_segment_alloc(90 , outlet_segment_id , branch_nr , rseg_data);
    
    well_segment_collection_add( sc , ws );
    test_assert_int_equal( well_segment_collection_get_size( sc ) , 2);
    test_assert_ptr_equal( well_segment_collection_iget( sc , 1 ) , ws );
    
    test_assert_false( well_segment_collection_has_segment( sc , 451 ));
    test_assert_true( well_segment_collection_has_segment( sc , 89 ));
    test_assert_true( well_segment_collection_has_segment( sc , 90 ));
    test_assert_ptr_equal( well_segment_collection_get( sc , 90 ) , ws );
    test_assert_NULL( well_segment_collection_get( sc , 76 ));
  }

  {
    int outlet_segment_id = WELL_SEGMENT_OUTLET_END_VALUE;
    int branch_nr = WELL_SEGMENT_BRANCH_INACTIVE_VALUE;
    well_segment_type * ws = well_segment_alloc(89 , outlet_segment_id , branch_nr, rseg_data);
    
    well_segment_collection_add( sc , ws );
    test_assert_int_equal( well_segment_collection_get_size( sc ) , 2);
    test_assert_ptr_equal( well_segment_collection_iget( sc , 0 ) , ws );
    
    test_assert_false( well_segment_collection_has_segment( sc , 451 ));
    test_assert_true( well_segment_collection_has_segment( sc , 89 ));
    test_assert_ptr_equal( well_segment_collection_get( sc , 89 ) , ws );
  }
  
  free( rseg_data );
  well_segment_collection_free( sc );
  
  exit(0);
}
