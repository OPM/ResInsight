/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'file_open.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <util.h>
#include <stdlib.h>


int main( int argc , char ** argv ) {
  char hostname[256];
  gethostname( hostname , 255 );
  printf("%s : " , hostname);
  for (int iarg = 1; iarg < argc; iarg++) {
    uid_t * uid_list;
    int     num_users;
    
    printf("%s :",argv[iarg]);
    uid_list = util_alloc_file_users( argv[iarg] , &num_users);
    for (int i = 0; i < num_users; i++) {
      struct passwd * pwd = getpwuid( uid_list[i] );
      if (pwd != NULL)
        printf(" %s", pwd->pw_name);
      else
        printf(" %d",uid_list[ i ]);
    }
    
    if (num_users == 0)
      printf(" <Not open>");
    printf("\n");
    
    util_safe_free( uid_list );
  }
}
