/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_win64.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/rng.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_type.h>




int main( int argc , char ** argv) {
  int num_kw  = 1000;       // Total file size should roughly exceed 2GB
  int kw_size = 600000;
  ecl_kw_type * kw = ecl_kw_alloc("KW" , kw_size , ECL_INT );
  rng_type * rng = rng_alloc( MZRAN , INIT_DEFAULT );
  int i;
  offset_type file_size;
  for (i=0; i < kw_size; i++) 
    ecl_kw_iset_int( kw , i , rng_get_int( rng , 912732 ));

  {
    fortio_type * fortio = fortio_open_writer( "LARGE_FILE.UNRST" , false , ECL_ENDIAN_FLIP);
    for (i = 0; i < num_kw; i++) {
      printf("Writing keyword %d/%d to file:LARGE_FILE.UNRST \n",i+1 , num_kw );
      ecl_kw_fwrite( kw , fortio );
    }
    fortio_fclose( fortio );
  }

  /*{
    fortio_type * fortio = fortio_open_reader( "LARGE_FILE.UNRST" , false , ECL_ENDIAN_FLIP);
    for (i = 0; i < num_kw - 1; i++) {
       printf("SKipping keyword %d/%d from file:LARGE_FILE.UNRST \n",i+1 , num_kw );
       ecl_kw_fskip( fortio );
    }
    {
       ecl_kw_type * file_kw = ecl_kw_fread_alloc( fortio );
       if (ecl_kw_equal( kw , file_kw ))
          printf("Keyword read back from file correctly :-) \n");
        else
          printf("Fatal error - keyword different on return ...\n");
       ecl_kw_free( file_kw );
    }
    fortio_fclose( fortio );
  }
  */
  file_size = util_file_size( "LARGE_FILE.UNRST" );
  printf("File size: %lld \n",file_size);
  {
    fortio_type * fortio = fortio_open_reader( "LARGE_FILE.UNRST" , false , ECL_ENDIAN_FLIP);
    printf("Seeking to file end: ");
    fortio_fseek( fortio , file_size , SEEK_SET);
    fortio_fclose( fortio );
    printf("Seek OK \n");
  }
  

  printf("Doing ecl_file_open(..)\n");
  {
    ecl_file_type * file = ecl_file_open( "LARGE_FILE.UNRST" , 0);
    ecl_kw_type * file_kw = ecl_file_iget_named_kw( file , "KW" , num_kw - 1);
    if (ecl_kw_equal( kw , file_kw ))
      printf("Keyword read back from file correctly :-) \n");
    else
      printf("Fatal error - keyword different on return ...\n");
    ecl_file_close( file );
  }

  remove( "LARGE_FILE.UNRST" );
  
  exit(0);
}
