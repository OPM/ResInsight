/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'grdecl_to_ascii.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <assert.h>
#include <parser.h>
#include <util.h>


int main(int argc, char ** argv)
{
  if(argc < 2)
    util_exit("usage: grdecl_to_ascii file_1.grdecl file_2.grdecl ... file_N.grdecl.\n");

  parser_type * parser = parser_alloc(" \t\r\n", NULL , NULL , NULL , "--", "\n");

  for(int i=1; i<argc; i++)
  {
    char * basename;
    char * filename;
    FILE * stream;

    stringlist_type * tokens = parser_tokenize_file(parser , argv[i], false);
    int num_tokens = stringlist_get_size(tokens);

    util_alloc_file_components( argv[i], NULL, &basename, NULL);
    assert(basename != NULL);
    filename = util_alloc_filename(NULL, basename, "ascii");
    stream = util_fopen(filename, "w");

    for(int i=0; i<num_tokens; i++)
    {
      double value;
      const char * value_str = stringlist_iget(tokens, i);
      if( util_sscanf_double(value_str, &value) )
        fprintf(stream, "%f\n", value); 
    }

    fclose(stream);
    free(filename);
    free(basename);
  }

  parser_free(parser);
  
  return 0;
}
