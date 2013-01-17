/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'replace.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <subst_list.h>

int main(int argc, char ** argv)
{
  if(argc == 1 || argc % 2 != 0 ) 
    util_exit("Usage: replace.x from1 to1 from2 to2 ... fromN toN filename\n");
  
  {
    subst_list_type * subst_list =  subst_list_alloc( NULL );
    for(int i=1; i < argc-1; i += 2)
      subst_list_append_ref(subst_list, argv[i], argv[i+1] , NULL);
    
    subst_list_update_file(subst_list, argv[argc-1]);
    subst_list_free(subst_list);
  }
  return 0;
}
