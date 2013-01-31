/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_setname.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>

#include <ert/rms/rms_file.h>
#include <ert/rms/rms_tagkey.h>
#include <ert/rms/rms_stats.h>


int main (int argc , char **argv) {
  int i;
  char *name;
  int name_length;
  argv++;
  argc--;
  
  name        = argv[0];
  name_length = strlen(name) + 1;

  argv++;
  argc--;
  for (i = 0; i < argc; i++) {
    rms_file_type *file = rms_file_alloc(argv[i] , false);
    rms_tagkey_type *tagkey;
    rms_tag_type    *tag;
    rms_file_fread(file);

    tag = rms_file_get_tag_ref(file , "parameter" , NULL , NULL , true);
    tagkey = rms_tag_get_key(tag , "name");
    rms_tagkey_manual_realloc_data(tagkey , name_length);
    rms_tagkey_set_data(tagkey , name);
    rms_file_fwrite(file , "parameter");
    rms_file_free(file);
  }

  return 0;
}

