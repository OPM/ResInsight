/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'grane_fix.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <time.h>
#include <buffer.h>


static void fix_file(const char * filename, const char * target_path , buffer_type * buffer) {
  int  iens;
  char ** tmp;
  int num;
  util_split_string( filename , "." , &num , &tmp);
  util_sscanf_int( tmp[2] , &iens );
  
  {
    char * new_file      = util_alloc_sprintf("%s%c%s_%04d" , target_path , UTIL_PATH_SEP_CHAR , tmp[0] , iens);
    FILE * target_stream = util_fopen( new_file , "w");

    buffer_fread_realloc( buffer , filename );
    {
      char * ptr      =  buffer_get_data( buffer );
      int    elements = (buffer_get_size(buffer) - 12)/8;
      double *   data =  (double *) &ptr[12];
      for (int i=0; i < elements; i++)
        fprintf(target_stream , "%g\n",data[i]);
    }

    fclose( target_stream );
  }
  util_free_stringlist( tmp , num );
}


int main( int argc , char ** argv) {
  int iarg;
  buffer_type * buffer = buffer_alloc(199);
  
  for (iarg = 1; iarg < argc; iarg++) 
    fix_file( argv[iarg] , "init_files" , buffer);
    
}
