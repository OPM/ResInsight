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


int main(int argc , char ** argv) {
  bool ok;

  {
    latex_type * latex = latex_alloc( argv[1] , false );
    printf("input:%s \n",argv[1]);
    ok = latex_compile( latex , true , true );
    printf("OK: %d \n",ok);
    latex_free( latex );
  }

  if (ok) 
    exit(0);
  else
    exit(1);
}
