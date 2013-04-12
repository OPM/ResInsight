/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'ert_util_latex_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>

#include <ert/util/latex.h>
#include <ert/util/test_util.h>
#include <ert/util/util.h>

void make_file( const char * filename ) {
  FILE * stream = util_fopen( filename , "w");
  
  fclose(stream);
}


void test_link( const latex_type * latex , const char * link , const char * target) {
  char * latex_link = util_alloc_filename( latex_get_runpath( latex ) , link , NULL);
  char * latex_target = util_alloc_link_target( latex_link );
  
  test_assert_true( util_same_file( target , latex_target));
  
  free( latex_link );
  free( latex_target);
}



void test_latex_link( latex_type * latex ) {
  const char * path = "/tmp/linkFarm";
  const char * file1 = util_alloc_filename( path , "File1" , NULL );
  const char * file2 = util_alloc_filename( path , "File2" , NULL );
  const char * file3 = util_alloc_filename( path , "File3" , NULL );
  

  util_make_path( path );
  make_file( file1 );
  make_file( file2 );
  make_file( file3 );
  
  latex_link_path( latex , path );
  latex_link_directory_content( latex , path );
  
  test_link( latex , "File1" , file1);
  test_link( latex , "File2" , file2);
  test_link( latex , "File3" , file3);
  test_link( latex , "linkFarm" , path);
  
  util_clear_directory( path , true , true );
}



int main(int argc , char ** argv) {
  bool ok;

  {
    bool in_place = false;
    latex_type * latex = latex_alloc( argv[1] , in_place);
    ok = latex_compile( latex , true , true , true);
    test_assert_true( in_place == latex_compile_in_place( latex ));
    latex_free( latex );
    test_assert_true( ok );
  }


  {
    latex_type * latex = latex_alloc( argv[1] , false );
    test_latex_link( latex );
    latex_free( latex );
  }

  {
    bool in_place = true;
    latex_type * latex = latex_alloc( argv[1] , in_place);
    test_assert_true( in_place == latex_compile_in_place( latex ));
    test_latex_link( latex );
    latex_free( latex );
  }
}
