/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'util_contains_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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


#include <util.h>


void test_contains(const int * data , int len , int value , bool target_value , const char * msg) {
  if (util_sorted_contains_int( data , len , value ) == target_value) 
    printf("%60s : OK \n",msg);
  else
    printf("%60s : FAILED \n",msg);
}



int main( int argc , char ** argv) {
  test_contains( (const int[6]) {0 , 10 , 11 , 12 , 16 , 20} , 6 , 0  , true , "Test start equal" );
  test_contains( (const int[6]) {0 , 10 , 11 , 12 , 16 , 20} , 6 , 20 , true , "Test end equal" );

  test_contains( (const int[6]) {0 , 10 , 11 , 12 , 16 , 20} , 6 , 22 , false , "Test too large" );
  test_contains( (const int[6]) {0 , 10 , 11 , 12 , 16 , 20} , 6 , -22 , false , "Test too small" );

  test_contains( (const int[6]) {0 , 10 , 11 , 12 , 16 , 20} , 6 , 11 , true  , "Test contains - even" );
  test_contains( (const int[6]) {0 , 10 , 11 , 12 , 16 , 20} , 6 , 9  , false , "Test !contains - even" );

  test_contains( (const int[7]) {0 , 7, 10 , 11 , 12 , 16 , 20} , 7 , 11 , true  , "Test contains - odd" );
  test_contains( (const int[7]) {0 , 7 , 10 , 11 , 12 , 16 , 20} , 7 , 9  , false , "Test !contains - odd" );
}
