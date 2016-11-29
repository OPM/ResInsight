/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_sprintf_escape.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/util/util.h>



int main(int argc , char ** argv) {
  test_assert_string_equal( NULL , util_alloc_sprintf_escape( NULL , 0));
  test_assert_string_equal( "String", util_alloc_sprintf_escape( "String" , 0));
  test_assert_string_equal( "String", util_alloc_sprintf_escape( "String" , 10));

  test_assert_string_equal( "S%%tring%%", util_alloc_sprintf_escape( "S%tring%" , 0));
  test_assert_string_equal( "S%%tring%%", util_alloc_sprintf_escape( "S%tring%" , 2));
  test_assert_string_equal( "S%%tring%%", util_alloc_sprintf_escape( "S%tring%" , 10));
  test_assert_string_equal( "S%%tring%" , util_alloc_sprintf_escape( "S%tring%" , 1));
  test_assert_string_equal( "%%%%" , util_alloc_sprintf_escape( "%%" , 0));
  test_assert_string_equal( "%%%%" , util_alloc_sprintf_escape( "%%%" , 1));

  exit(0);
}
