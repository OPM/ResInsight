/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'eol-fix.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#include <util.h>



void eol_fix_file(const char * filename) {
  char * tmp_file = util_alloc_tmp_file("/tmp" , "eol-fix" , true);
  util_copy_file(filename , tmp_file );
  {
    FILE * src    = util_fopen(tmp_file , "r");
    FILE * target = util_fopen(filename , "w");

    do {
      int c = fgetc(src);
      if (c != '\r')
	fputc(c , target);
    } while (!feof(src));

    fclose(src);
    fclose(target);
  }
  free(tmp_file);
}



int main (int argc , char **argv) {
  eol_fix_file(argv[1]);
  return 0;
}
