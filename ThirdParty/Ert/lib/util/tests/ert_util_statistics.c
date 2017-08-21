/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ert_util_statistics.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>
#include <ert/util/statistics.h>


void test_mean_std() {
  double_vector_type * d = double_vector_alloc(0,0);
  
  double_vector_append(d , 0 );
  double_vector_append(d , 1 );

  test_assert_double_equal( statistics_mean( d ) , 0.50 );
  test_assert_double_equal( statistics_std( d )  , 0.50 );
}


int main( int argc , char ** argv ) {
  test_mean_std();
}
