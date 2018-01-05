/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_time_interval.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <time.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/time_interval.h>
  


int main( int argc , char ** argv) {
  time_t start_time = util_make_date_utc(1,1,2000);
  time_t end_time   = util_make_date_utc(1,1,2010);
  time_t in         = util_make_date_utc( 1,1,2005);
  time_t before     = util_make_date_utc( 1,1,1995);
  time_t after      = util_make_date_utc( 1,1,2015);
  
  {
    time_interval_type * ti = time_interval_alloc( start_time , end_time );
    test_assert_not_NULL( ti );
    test_assert_false( time_interval_is_empty( ti ));

    test_assert_true( time_interval_contains( ti , start_time ));
    test_assert_true( time_interval_contains( ti , in ));
    test_assert_false( time_interval_contains( ti , before ));
    test_assert_false( time_interval_contains( ti , end_time ));
    test_assert_false( time_interval_contains( ti , after ));
    
    test_assert_false( time_interval_update( ti , end_time , start_time ));
    test_assert_true( time_interval_is_empty( ti ));

    test_assert_false( time_interval_contains( ti , start_time ));
    test_assert_false( time_interval_contains( ti , in ));
    test_assert_false( time_interval_contains( ti , before ));
    test_assert_false( time_interval_contains( ti , end_time ));
    test_assert_false( time_interval_contains( ti , after ));

    test_assert_true( time_interval_update( ti , start_time , end_time ));
    test_assert_false( time_interval_is_empty( ti ));
    
    time_interval_free( ti );
  }

  {
    time_interval_type * ti = time_interval_alloc( end_time , start_time );
    test_assert_not_NULL( ti );
    test_assert_true( time_interval_is_empty( ti ));
    test_assert_true( time_interval_update( ti , start_time , end_time ));
    test_assert_false( time_interval_is_empty( ti ));
    time_interval_free( ti );
  }

  {
    time_interval_type * t1 = time_interval_alloc( start_time , end_time );
    time_interval_type * t2 = time_interval_alloc( in , after );
    time_interval_type * t3 = time_interval_alloc( end_time , start_time );
    time_interval_type * t4 = time_interval_alloc( before , start_time );
    time_interval_type * t5 = time_interval_alloc( end_time , after );

    test_assert_true( time_interval_has_overlap( t1 , t2 ));

    test_assert_true( time_interval_is_empty( t3 ));
    test_assert_false( time_interval_has_overlap( t1 , t3 ));
    test_assert_false( time_interval_has_overlap( t3 , t1 ));
    test_assert_false( time_interval_has_overlap( t3 , t3 ));
    test_assert_false( time_interval_has_overlap( t4 , t5 ));
    test_assert_false( time_interval_has_overlap( t1 , t5 ));
    

    time_interval_free( t1 );
    time_interval_free( t2 );
  }

  {
    time_interval_type * ti = time_interval_alloc_open();

    test_assert_false( time_interval_is_empty( ti ));

    test_assert_true( time_interval_contains( ti , start_time ));
    test_assert_true( time_interval_contains( ti , in ));
    test_assert_true( time_interval_contains( ti , before ));
    test_assert_true( time_interval_contains( ti , end_time ));
    test_assert_true( time_interval_contains( ti , after ));

    test_assert_true( time_interval_update_start( ti , start_time ));
    test_assert_true( time_interval_contains( ti , start_time ));
    test_assert_true( time_interval_update_start( ti , in ));
    test_assert_false( time_interval_contains( ti , start_time ));
    test_assert_false( time_interval_is_empty( ti ));
    
    test_assert_false( time_interval_update_end( ti , start_time ));
    test_assert_true( time_interval_is_empty( ti ));
    test_assert_true( time_interval_update_end( ti , end_time ));
    test_assert_false( time_interval_is_empty( ti ));
    test_assert_false( time_interval_contains( ti , start_time ));
    
    time_interval_free( ti );
  }

  {
    time_interval_type * t1 = time_interval_alloc( start_time , end_time );

    test_assert_time_t_equal( start_time , time_interval_get_start( t1 ));
    test_assert_time_t_equal( end_time   , time_interval_get_end( t1 ));
    test_assert_false( time_interval_is_empty( t1 ));
    
    test_assert_false( time_interval_update_end( t1 , before ));
    test_assert_true( time_interval_is_empty( t1 ));
    test_assert_time_t_equal( start_time , time_interval_get_start( t1 ));
    test_assert_time_t_equal( before   , time_interval_get_end( t1 ));

    test_assert_true( time_interval_update_end( t1 , in ));
    test_assert_false( time_interval_is_empty( t1 ));
    test_assert_time_t_equal( start_time , time_interval_get_start( t1 ));
    test_assert_time_t_equal( in   , time_interval_get_end( t1 ));
    
    time_interval_free( t1 );
  }

  {
    time_interval_type * t1 = time_interval_alloc( start_time , in );
    time_interval_type * t2 = time_interval_alloc( in , end_time );
    time_interval_type * t3 = time_interval_alloc( start_time , end_time);

    test_assert_true( time_interval_is_adjacent( t1 , t2 ));
    test_assert_true( time_interval_is_adjacent( t2 , t1 ));
    
    test_assert_false( time_interval_is_adjacent( t1 , t3 ));
    test_assert_false( time_interval_is_adjacent( t3 , t1 ));
    test_assert_false( time_interval_is_adjacent( t2 , t3 ));
    test_assert_false( time_interval_is_adjacent( t3 , t2 ));
    
    time_interval_free( t1 );
    time_interval_free( t2 );
    time_interval_free( t3 );
  }



  {
    time_interval_type * t1 = time_interval_alloc( start_time , end_time );
    time_interval_type * t2 = time_interval_alloc( in , end_time );
    time_interval_type * t3 = time_interval_alloc( end_time , after );
    time_interval_type * t4 = time_interval_alloc( before , start_time );
    
    test_assert_true( time_interval_extend(t1 , t2 ));
    test_assert_time_t_equal( start_time , time_interval_get_start( t1 ));
    test_assert_time_t_equal( end_time   , time_interval_get_end( t1 ));

    test_assert_true( time_interval_extend(t1 , t3 ));
    test_assert_time_t_equal( start_time , time_interval_get_start( t1 ));
    test_assert_time_t_equal( after   , time_interval_get_end( t1 ));

    test_assert_true( time_interval_update_start(t1 , in ));
    test_assert_time_t_equal( in , time_interval_get_start( t1 ));

    
    test_assert_false( time_interval_extend(t1 , t4 ));
    test_assert_time_t_equal( in    , time_interval_get_start( t1 ));
    test_assert_time_t_equal( after , time_interval_get_end( t1 ));


    test_assert_true( time_interval_update_end(t4 , in ));
    test_assert_true( time_interval_extend(t1 , t4 ));
    test_assert_time_t_equal( before , time_interval_get_start( t1 ));
    test_assert_time_t_equal( after , time_interval_get_end( t1 ));


    time_interval_free( t1 );
    time_interval_free( t2 );
    time_interval_free( t3 );
    time_interval_free( t4 );
  }
  
  {
    time_interval_type * t1 = time_interval_alloc( start_time , end_time );
    time_interval_type * t2 = time_interval_alloc( in , end_time );
    time_interval_type * t3 = time_interval_alloc( end_time , after );
    time_interval_type * t4 = time_interval_alloc( before , start_time );
    
    test_assert_true( time_interval_intersect(t1 , t2 ));
    test_assert_time_t_equal( in , time_interval_get_start( t1 ));
    test_assert_time_t_equal( end_time , time_interval_get_end( t1 ));
    
    time_interval_free( t1 );
    time_interval_free( t2 );
    time_interval_free( t3 );
    time_interval_free( t4 );
  }

  {
    time_interval_type * t1 = time_interval_alloc( start_time , end_time );
    time_interval_type * t2 = time_interval_alloc( start_time , end_time );
    time_interval_type * t3 = time_interval_alloc( end_time , after );
    time_interval_type * t4 = time_interval_alloc_copy( t1 );

    test_assert_true( time_interval_equal( t1 , t2 ));
    test_assert_false( time_interval_equal( t1 , t3 ));

    test_assert_true( time_interval_equal( t4 , t2 ));
    test_assert_false( time_interval_equal( t4 , t3 ));

    test_assert_ptr_not_equal( t4 , t1 );
    time_interval_free( t1 );
    time_interval_free( t2 );
    time_interval_free( t3 );
    time_interval_free( t4 );
  }

  {
    time_interval_type * t1 = time_interval_alloc( start_time , end_time );
    
    test_assert_true( time_interval_arg_before( t1 , before ));
    test_assert_true( time_interval_arg_after( t1 , after));

    test_assert_false( time_interval_arg_before( t1 , start_time ));
    test_assert_false( time_interval_arg_before( t1 , in ));
    test_assert_false( time_interval_arg_before( t1 , after ));
    
    test_assert_false( time_interval_arg_after( t1 , start_time));
    test_assert_false( time_interval_arg_after( t1 , in));
    test_assert_true( time_interval_arg_after( t1 , after));
    
    time_interval_free( t1 );
  }
  
  exit(0);
}
