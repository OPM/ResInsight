/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ert_util_binary_split.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

void test_split(const char * test_string , bool split_on_first , const char * true1, const char * true2) {
  char * part1;
  char * part2;

  
  util_binary_split_string( test_string , ":" , split_on_first , &part1 , &part2 );
  test_assert_string_equal( true1 , part1 );
  test_assert_string_equal( true2 , part2 );

  util_binary_split_string( test_string , ":;" , split_on_first , &part1 , &part2 );
  test_assert_string_equal( true1 , part1 );
  test_assert_string_equal( true2 , part2 );

  util_binary_split_string( test_string , ";" , split_on_first , &part1 , &part2 );
  test_assert_string_equal( test_string , part1 );
  test_assert_string_equal( NULL , part2 );
}

int main(int argc , char ** argv) {
  
  test_split( "Hello:Hello"  , true  , "Hello" , "Hello");
  test_split( "ABC:DEF:GEH"  , true  , "ABC" , "DEF:GEH");
  test_split( "ABC:DEF:GEH"  , false , "ABC:DEF" , "GEH");
  test_split( "ABC:DEF:GEH:" , false , "ABC:DEF" , "GEH");
  test_split( "ABC:DEF:GEH:" , true  , "ABC", "DEF:GEH");
  test_split( "ABCDEFGEH"    , false , "ABCDEFGEH" , NULL);
  test_split( "ABCDEFGEH:"   , false , "ABCDEFGEH" , NULL);
  test_split( ":ABCDEFGEH"   , false , "ABCDEFGEH" , NULL);
  test_split( NULL , false , NULL , NULL);
  
  exit(0);
}
