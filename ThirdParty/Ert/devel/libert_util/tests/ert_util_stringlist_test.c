/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_stringlist_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

//#include <test_util.h>
#include <stringlist.h>

void test_char() {
  const char * S1 = "S1";
  const char * S2 = "S2";
  const char * S3 = "S3";
  stringlist_type * s = stringlist_alloc_new();
  stringlist_append_ref( s , S1 );
  stringlist_append_ref( s , S2 );
  stringlist_append_ref( s , S3 );

  {
    char ** ref = stringlist_alloc_char_ref( s );
    char ** copy = stringlist_alloc_char_copy( s );
    int i;

    for (i=0; i < stringlist_get_size( s ); i++) {
      if (ref[i] != stringlist_iget(s , i))
        exit(1);

      if (strcmp( stringlist_iget( s , i ) , copy[i]) != 0)
        exit(1);

    }
  }
}



int main( int argc , char ** argv) {
  test_char();
  exit(0);
}
