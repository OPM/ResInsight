/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_clamp.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/test_util.h>


int main(int argc , char ** argv) {
  double value;

  value = 0;
  util_clamp_double( &value , -1 , 1 );
  test_assert_double_equal( value , 0 );

  value = 0;
  util_clamp_double( &value , 1 , 2 );
  test_assert_double_equal( value , 1 );

  value = 0;
  util_clamp_double( &value , 2 , 1 );
  test_assert_double_equal( value , 1 );
  
  value = 0;
  util_clamp_double( &value , -2 , 0 );
  test_assert_double_equal( value , 0 );

  value = 0;
  util_clamp_double( &value , -2 , -1 );
  test_assert_double_equal( value , -1 );

  value = 0;
  util_clamp_double( &value , -1 , -2 );
  test_assert_double_equal( value , -1 );

}
