/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'well_segment.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <ert/ecl_well/well_segment.h>
#include <ert/ecl_well/well_const.h>

int main(int argc , char ** argv) {
  test_install_SIGNALS();
  double * rseg_data = util_calloc( 100 , sizeof * rseg_data );
  const double depth = 100;
  const double length = 20;
  const double total_length = 200;
  const double diameter = 10;

  rseg_data[ RSEG_DEPTH_INDEX ]  = depth;    
  rseg_data[ RSEG_LENGTH_INDEX ] = length;
  rseg_data[ RSEG_TOTAL_LENGTH_INDEX ] = total_length;
  rseg_data[ RSEG_DIAMETER_INDEX ] = diameter;  
  {
    int segment_id = 78;
    int outlet_segment_id = 100;
    int branch_nr = WELL_SEGMENT_BRANCH_MAIN_STEM_VALUE;
    well_segment_type * ws = well_segment_alloc(segment_id , outlet_segment_id , branch_nr, rseg_data);

    test_assert_true( well_segment_is_instance( ws ));
    test_assert_int_equal( 0 , well_segment_get_link_count( ws ));
    test_assert_NULL( well_segment_get_outlet( ws ));
    test_assert_int_equal( well_segment_get_outlet_id( ws ) , outlet_segment_id );
    test_assert_int_equal( well_segment_get_branch_id( ws ) , branch_nr );
    test_assert_int_equal( well_segment_get_id( ws ) , segment_id );

    test_assert_false( well_segment_nearest_wellhead( ws ));
    test_assert_true( well_segment_active( ws ));
    test_assert_true( well_segment_main_stem( ws ));

    test_assert_double_equal( depth  , well_segment_get_depth( ws ));
    test_assert_double_equal( length , well_segment_get_length( ws ));
    test_assert_double_equal( total_length , well_segment_get_total_length( ws ));
    test_assert_double_equal( diameter , well_segment_get_diameter( ws ));

    well_segment_free( ws );
  }

  {
    int outlet_segment_id = WELL_SEGMENT_OUTLET_END_VALUE;
    int branch_nr = 100;
    well_segment_type * ws = well_segment_alloc(12 , outlet_segment_id , branch_nr, rseg_data);
    
    test_assert_true( well_segment_nearest_wellhead( ws ));
    test_assert_false( well_segment_main_stem( ws ));
    well_segment_free( ws );
  }


  {
    int outlet_segment_id = WELL_SEGMENT_OUTLET_END_VALUE;
    int branch_nr = WELL_SEGMENT_BRANCH_INACTIVE_VALUE;
    well_segment_type * ws = well_segment_alloc(89 , outlet_segment_id , branch_nr, rseg_data);
    
    test_assert_false( well_segment_active( ws ));
    well_segment_free( ws );
  }

  {
    int branch_nr = WELL_SEGMENT_BRANCH_MAIN_STEM_VALUE;
    int outlet_id = 0;
    well_segment_type * outlet = well_segment_alloc(outlet_id , WELL_SEGMENT_OUTLET_END_VALUE , branch_nr, rseg_data);
    well_segment_type * ws = well_segment_alloc(100 , outlet_id , branch_nr, rseg_data);
    
    test_assert_true( well_segment_link( ws , outlet ));
    test_assert_ptr_equal( well_segment_get_outlet( ws ) , outlet );
    test_assert_int_equal( well_segment_get_link_count( outlet ) , 1 );
    test_assert_ptr_not_equal( ws , well_segment_get_outlet( ws ));

    well_segment_link_strict( ws , outlet );  // This relinks - not very logical; refcount gets wrong.
    well_segment_free( ws );
    well_segment_free( outlet );
  }

  {
    int branch_nr = WELL_SEGMENT_BRANCH_MAIN_STEM_VALUE;
    int outlet_id = 0;
    well_segment_type * outlet = well_segment_alloc(outlet_id , WELL_SEGMENT_OUTLET_END_VALUE , branch_nr , rseg_data);
    well_segment_type * ws = well_segment_alloc(100 , outlet_id + 1, branch_nr, rseg_data);
    
    test_assert_false( well_segment_link( ws , outlet ));
    test_assert_NULL( well_segment_get_outlet( ws ) );
    test_assert_int_equal( well_segment_get_link_count( outlet ) , 0 );
    
    well_segment_free( ws );
    well_segment_free( outlet );
  }
  free( rseg_data );
  exit(0);
}
