/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'module_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <dlfcn.h>

#include <ert/util/rng.h>

#include <ert/analysis/analysis_module.h>


int check_module( rng_type * rng , const char * lib_name ) {
  analysis_module_load_status_enum  load_status;
  analysis_module_type * module = analysis_module_alloc_external__( rng , lib_name , false , &load_status);
  if (module != NULL) {
    printf("Module loaded successfully\n");
    analysis_module_free( module );
    return 0;
  } else {
    if (load_status == DLOPEN_FAILURE) {
      printf("\ndlerror(): %s\n\n",dlerror());
      printf("The runtime linker could not open the library:%s.\n", lib_name);
      printf("For the runtime linker to succesfully open your library\n");
      printf("at least one of two must be satisfied: \n\n");
      printf("  1. You give the FULL PATH to library - including .so extension\n\n");
      printf("  2. The path containing the library is in LD_LIBRARY_PATH.\n\n");
      printf("In addition all libraries needed by your module must be found\n");
    } else if (load_status == LOAD_SYMBOL_TABLE_NOT_FOUND) {
      printf("\nThe library %s was loaded successfully, however\n",lib_name);
      printf("the symbol table:\'%s\' was not found. You must make sure\n",EXTERNAL_MODULE_NAME);
      printf("that the \'analysis_table_type\' structure at the bottom\n");
      printf("of the source file is named exactly: \'analysis_table\'.\n");
      printf("See documentation of \'symbol_table\' in modules.txt.\n\n");
    }
  }
  return 1;
}



int main( int argc , char ** argv) {
  exit( check_module( NULL , argv[1] ) );
}
