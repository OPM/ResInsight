/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'view_rft.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ecl_rft_file.h>
#include <ecl_util.h>
#include <util.h>





int main (int argc , char ** argv) {

  if (argc != 2)
    util_exit("Usage: rft.x BASENAME \n");
  {
    char * input_file = argv[1];
    char * file_name   = NULL;
    ecl_file_enum input_type;

    ecl_rft_file_type * rft_file;

    input_type = ecl_util_get_file_type( input_file , NULL , NULL);
    if (input_type == ECL_RFT_FILE)
      file_name = util_alloc_string_copy(input_file);
    else {
      char  * base;
      char  * path;
      char  * rft_file_formatted   ;
      char  * rft_file_unformatted ;

      util_alloc_file_components( input_file , &path , &base , NULL);
      rft_file_formatted   = ecl_util_alloc_filename(path , base , ECL_RFT_FILE , true  , -1);
      rft_file_unformatted = ecl_util_alloc_filename(path , base , ECL_RFT_FILE , false , -1);
      
      if (util_file_exists( rft_file_formatted ) && util_file_exists( rft_file_unformatted )) 
	file_name = util_alloc_string_copy( util_newest_file( rft_file_formatted , rft_file_unformatted));
      else if (util_file_exists( rft_file_formatted ))
	file_name = util_alloc_string_copy( rft_file_formatted );
      else if (util_file_exists( rft_file_unformatted ))
	file_name = util_alloc_string_copy( rft_file_unformatted );
      else 
	util_exit("Could not find RFT files: %s/%s \n",rft_file_formatted , rft_file_unformatted);
      
      free( rft_file_formatted );
      free( rft_file_unformatted );
      free( base );
      free( path );
    }
    
    rft_file = ecl_rft_file_alloc(file_name);
    ecl_rft_file_summarize( rft_file , true);
    ecl_rft_file_free( rft_file );
  }
}
  

