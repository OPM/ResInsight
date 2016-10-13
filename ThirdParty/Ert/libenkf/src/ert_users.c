/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ert_users.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <enkf_main.h>
#include <set.h>
#include <unistd.h>


int main (int argc , char ** argv) {
  char hostname[256];
  const char * executable = argv[1];
  
  gethostname( hostname , 255 );
  printf("%s : " , hostname);
  {
    set_type * user_set = set_alloc_empty();
    enkf_main_list_users( user_set , executable );
    
    if (set_get_size( user_set ) > 0) 
      set_fprintf(user_set , " " , stdout );
    else
      printf("No users.");

    printf("\n");
    
    set_free( user_set );
  }
}




