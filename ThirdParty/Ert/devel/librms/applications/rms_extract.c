/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_extract.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/util.h>

#include <ert/rms/rms_file.h>
#include <ert/rms/rms_tagkey.h>
#include <ert/rms/rms_stats.h>


void split_name(const char * arg, char **_old_name , char **_new_name) {
  char * new_name;
  char * old_name;
  int i;
  int old_name_len = 0;

  for (i=0; i < strlen(arg); i++) {
    if (arg[i] == '=')
      old_name_len = i;
  }

  if (old_name_len > 0) {
    old_name = util_alloc_substring_copy(arg , 0 , old_name_len);
    new_name = util_alloc_string_copy(&arg[old_name_len + 1]);
  } else {
    old_name = util_alloc_string_copy(arg);
    new_name = old_name;
  }
  
  *_old_name = old_name;
  *_new_name = new_name;
}




int main (int argc , char **argv) {
  {
    if (argc <= 2) {
      fprintf(stderr,"rms_extract.x filename tag1(=new_tag1)  tag2 ... \n");
      abort();
    }
  }
  {
    const char * filename = argv[1];
    int i;
    rms_tag_type   * dim_tag;
    rms_file_type *file = rms_file_alloc(filename , false);
    printf("Skal laste inn file: %s \n",filename);
    rms_file_fread(file);
    dim_tag = rms_file_get_dim_tag_ref(file);
    
    for (i = 2; i < argc; i++) {
      char * new_name;
      char * old_name;
      char * new_file;
      
      split_name(argv[i] , &old_name , &new_name);
      printf("Exctracting %s -> %s \n" , old_name , new_name); fflush(stdout);
      new_file = util_alloc_filename(NULL , new_name , "ROFF");
      
      {
        rms_tag_type   * tag;
        rms_file_type  * out_file = rms_file_alloc(new_file , false);
        FILE *stream = rms_file_fopen_w(out_file);
        rms_file_init_fwrite(out_file , "parameter");
        rms_tag_fwrite(dim_tag , stream);
        
        tag = rms_file_get_tag_ref(file , "parameter" , "name" , old_name , true);
        rms_tag_fwrite_parameter(new_name, rms_tag_get_datakey(tag) , stream);
        rms_file_complete_fwrite(out_file);
        fclose(stream);
      }
      if (new_name == old_name) 
        free(new_name);
      else {
        free(new_name);
        free(old_name);
      }
    }
    rms_file_free(file);
    return 0;
  }
}

