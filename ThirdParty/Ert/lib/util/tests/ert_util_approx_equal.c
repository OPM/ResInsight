/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_approx_equal.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <math.h>

#include <ert/util/test_util.h>
#include <ert/util/stringlist.h>




int main( int argc , char ** argv) {

  test_assert_double_not_equal( -1.0 , 1.0 );
  test_assert_double_not_equal( 0.00000000002 , 0.000000000001 );
  test_assert_double_not_equal( 0.00000000002 , 0.000000000001 );

  test_assert_double_equal( 1.00000000002 , 1.000000000001 );
  test_assert_double_equal( 0.0 , 0.0 ); 
  test_assert_double_equal( 0.75 , asin( sin(0.75)));
  test_assert_double_equal(  2.25 , exp( log(2.25)));
  test_assert_double_equal(  2.25 , log( exp(2.25)));
  
  exit(0);
}
