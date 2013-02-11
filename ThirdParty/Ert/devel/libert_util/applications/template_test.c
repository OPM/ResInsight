/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'template_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <template.h>



bool test_path( const char * src_file , const char * cmp_file ) {
  const char * target_file = "/tmp/target.txt";
  template_type * template = template_alloc( src_file , true , NULL );
  printf("Instantiating template: %s ...... " , src_file);
  template_instantiate( template , target_file  , NULL , true );
  if (util_files_equal( target_file , cmp_file ))
    printf("OK\n");
  else {
    printf("ERROR \n");
    printf("Compare files: %s and %s \n",cmp_file , target_file );
    exit(1);
  }
  template_free( template );
}



int main( int argc , char ** argv) {
  test_path("template_test_data/template1.txt" , "template_test_data/result1.txt");
  test_path("template_test_data/template2.txt" , "template_test_data/result2.txt");
  test_path("template_test_data/template3.txt" , "template_test_data/result3.txt");
}
