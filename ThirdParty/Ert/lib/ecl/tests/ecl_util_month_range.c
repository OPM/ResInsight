/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_util_month_range.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/time_t_vector.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>



void test_body( time_t_vector_type * date_list , int offset) {
  int i;
  for (i=offset; i < (time_t_vector_size( date_list ) - 1); i++) {
    int month,year;
    time_t current_date = time_t_vector_iget( date_list , i );
    test_assert_true( util_is_first_day_in_month_utc( current_date ));
    util_set_date_values_utc( current_date , NULL , &month , &year);
    if (i > offset) {
      time_t prev_date = time_t_vector_iget( date_list , i - 1 );
      int prev_month , prev_year;
      util_set_date_values_utc( prev_date , NULL , &prev_month , &prev_year);
      
      if (prev_year == year)
        test_assert_int_equal( month , prev_month + 1);
      else {
        test_assert_int_equal( month  , 1);
        test_assert_int_equal( prev_month  , 12);
        test_assert_int_equal( year - prev_year  , 1);
      }
    }
    test_assert_time_t_equal( current_date , util_make_pure_date_utc( current_date ));
  }
}


void test_append( const char * name , time_t_vector_type * date_list , time_t date1 , time_t date2, bool force_append_end) {
  int offset = time_t_vector_size( date_list );
  
  printf("%s ...",name); 
  fflush( stdout );

  ecl_util_append_month_range( date_list , date1 , date2 , force_append_end);

  // The start:
  test_assert_true( util_make_pure_date_utc(date1) <= time_t_vector_iget( date_list , offset));

  // The body:
  test_body( date_list , offset );

  // The tail:
  if ( force_append_end )
    test_assert_time_t_equal( time_t_vector_get_last( date_list ) , util_make_pure_date_utc(date2));
  else
    test_assert_true( util_is_first_day_in_month_utc( time_t_vector_get_last( date_list )));
  

  printf(" OK \n");
}


void test_init( const char * name , time_t_vector_type * date_list , time_t start_date , time_t end_date) {
  printf("%s ...",name); 
  fflush( stdout );
  {
    ecl_util_init_month_range( date_list , start_date , end_date );
    test_assert_time_t_equal( time_t_vector_get_first( date_list ) , util_make_pure_date_utc( start_date ));
    test_body( date_list , 1 );
    test_assert_time_t_equal( time_t_vector_get_last( date_list ) , util_make_pure_date_utc(end_date ));
  }
  printf(" OK \n");
}


int main(int argc , char ** argv) {
  time_t date1  = util_make_datetime_utc(0,2,0,1,1,2000);
  time_t date2  = util_make_datetime_utc(0,3,0,1,1,2005);
  time_t date3  = util_make_datetime_utc(0,4,0,10,1,2000);
  time_t date4  = util_make_datetime_utc(0,5,0,10,1,2005);
  
  

  {
    time_t_vector_type * date_list = time_t_vector_alloc(0 ,0 );
    test_append( "Append Test1" , date_list , date1 , date2 , true);
    test_append( "Append Test2" , date_list , date1 , date2 , false);
    test_append( "Append Test3" , date_list , date1 , date4 , true);
    test_append( "Append Test4" , date_list , date1 , date4 , false);
    test_append( "Append Test5" , date_list , date3 , date2 , true);
    test_append( "Append Test6" , date_list , date3 , date2 , false);
    test_append( "Append Test7" , date_list , date3 , date4 , true);
    test_append( "Append Test8" , date_list , date3 , date4 , false);
    time_t_vector_free( date_list );
  }
  {
    time_t_vector_type * date_list = time_t_vector_alloc(0 ,0 );
    
    test_init( "Init Test1" , date_list , date1 , date2 );
    test_init( "Init Test2" , date_list , date1 , date4 );
    test_init( "Init Test3" , date_list , date3 , date2 );
    test_init( "Init Test4" , date_list , date3 , date4 );

    time_t_vector_free( date_list );
  }

  exit(0);
}
