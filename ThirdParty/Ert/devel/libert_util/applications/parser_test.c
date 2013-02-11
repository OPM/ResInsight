/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'parser_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <parser.h>

int main(int argc, char ** argv)
{
  parser_type * parser = parser_alloc(" \t\n\r,", "'\"", "[]{}=", NULL , "--", "\n");

  if(argc < 2 )
  {
    printf("Usage: parser_test.x file.txt\n");
    return 1;
  }

  stringlist_type * tokens = parser_tokenize_file(parser, argv[1] , true);

  int num_tokens = stringlist_get_size(tokens);

  for(int i = 0; i < num_tokens; i++)
    printf("token[%d] : %s\n", i, stringlist_iget(tokens, i) );
  
  stringlist_free( tokens );
  parser_free( parser );
  return 0;
}
