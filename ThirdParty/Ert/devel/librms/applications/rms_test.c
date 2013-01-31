/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/rms/rms_tagkey.h>
#include <ert/rms/rms_file.h>
#include <ert/rms/rms_stats.h>



void convert_test(const char * rms_file, const char * ecl_path) {
  rms_file_2eclipse(rms_file , ecl_path , false , 1);
}


int main (int argc , char **argv) {
  rms_tagkey_type *mean , *std;
  rms_tag_type    *dim_tag;
  bool log_transform = false;

  rms_file_type *file = rms_file_alloc(argv[1] , false);
  dim_tag = rms_file_fread_alloc_tag(file , "dimensions" , NULL , NULL);
  mean    = rms_file_fread_alloc_data_tagkey(file , "parameter" , "name" , "PERMX");
  std     = rms_tagkey_copyc(mean); 

  rms_file_free_data(file);
  rms_stats_mean_std(mean , std , "PERMX" , argc - 1 , (const char **) &argv[1] , log_transform);
  rms_file_set_filename(file , "Stats.ROFF" , false);

  {
    FILE *stream = rms_file_fopen_w(file);
    rms_file_init_fwrite(file , "parameter");
    rms_tag_fwrite(dim_tag , stream);
    rms_tag_fwrite_parameter("mean:PERMX" , mean , stream);
    rms_tag_fwrite_parameter("std:PERMX"  , std  , stream);
    rms_file_complete_fwrite(file);
    fclose(stream);
  }
  
  rms_tag_free(dim_tag);
  rms_tagkey_free(mean);
  rms_tagkey_free(std);
  rms_file_free_data(file);  
  rms_file_free(file);
  
  exit(1);

  {
    const int ens_size = 100;
    char **file_list;
    double **X;
    int i , j;
    file_list = malloc(ens_size * sizeof * file_list);
    for (i=0; i < ens_size; i++) {
      file_list[i] = malloc(100);
      sprintf(file_list[i] , "PERMX_%04d.INC" , i + 1);
    }
      

    X = malloc(ens_size * sizeof *X);
    for (i=0; i < ens_size; i++)
      X[i] = malloc(ens_size * sizeof *X[i]);
    
    for (i=0; i < ens_size; i++)
      for (j=0; j < ens_size; j++)
        X[i][j] = 0;
    
    for (i=0; i < ens_size; i++)
      X[i][i] = 1.0;
    
    rms_stats_update_ens("Posterior" , "Post2" , (const char **) file_list , "PERMX" , ens_size , (const double **) X);
    
    for (i=0; i < ens_size; i++) {
      free(X[i]);
      free(file_list[i]);
    }
    free(X);
    free(file_list);
  }

  return 0;
}

