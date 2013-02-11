/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'util_fmove_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <errno.h>

#include <util.h>




void test_init_txt( const char * name1 , const char * name2 ) {
  FILE * stream1 = util_fopen(name1 , "w");
  FILE * stream2 = util_fopen(name2 , "w");
  fprintf(stream1 , "01456789");
  fprintf(stream2 , "0123456789");
  fclose( stream1 );
  fclose( stream2 );
}


void test_init( const char * name1 , const char * name2 , int size , int offset , int shift ) {
  FILE * stream1 = util_fopen(name1 , "w");
  FILE * stream2 = util_fopen(name2 , "w");
  {
    int i;
    if (shift > 0) {
      for (i=0; i < size; i++) {
        int c = random( ) % 256;
        fputc( c , stream2 );
        
        if (i == offset) {
          for (int j=0; j < shift; j++) 
            fputc( 0 , stream1 );
        }
        fputc( c , stream1 );
      }
      if (i == offset) {
        for (int j=0; j < shift; j++) 
          fputc( 0 , stream1 );
      }
    } else {
      for (i=0; i < size; i++) {
        int c = random( ) % 256;
        fputc( c , stream2 );
        
        if ((i >= offset) || (i < (offset + shift)))
          fputc( c , stream1 );
      }
    }
  }
  
  fclose( stream1 );
  fclose( stream2 );
}


int test_fmove( const char * name1 , const char * name2 , int size , int offset , int shift ) {
  
  int fmove_value;
  FILE * stream = util_fopen( name2 , "r+");
  fmove_value = util_fmove( stream , offset , shift );
  fclose( stream );
  
  return fmove_value;
}


void run_test( int size , int offset , int shift , int target_value ) {
  const char * name1 = "/tmp/test1";
  const char * name2 = "/tmp/test2";
  
  test_init( name1 , name2 , size , offset , shift);
  if (test_fmove(name1 , name2 , size , offset , shift) == target_value) {
    if (target_value == 0) {
      if (util_files_equal( name1 , name2))
        printf("Files equal OK \n");
      else 
        printf("ERROR: Files different \n");
    } else
      printf("Return value OK \n");
  }
  unlink( name1 );
  unlink( name2 );
}


int main( int argc , char ** argv) {
  run_test( 10000000 , 10000 , -1000  , 0 );           // Normal negative shift
  run_test( 10000000 , 10000 ,  1000  , 0 );           // Normal positive shift
  run_test( 10000000 , 10000 , -1000  , 0 );           // Normal negative shift
  run_test( 10000000 , 10000 , -90000 , EINVAL );      // Negative shift before start of file
  run_test( 1000    , 10000 , -90000    , EINVAL );   // Offset beyond end of file
  run_test( 1000    , 1000  , 1000 , 0 );             // Positove shift at end of file
  run_test( 1000    , 1000  , -100 , 0 );             // Negative shift at end of file
}
