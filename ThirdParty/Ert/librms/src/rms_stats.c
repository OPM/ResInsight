/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_stats.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>

#include <ert/util/util.h>

#include <ert/rms/rms_stats.h>
#include <ert/rms/rms_tagkey.h>
#include <ert/rms/rms_tag.h>
#include <ert/rms/rms_file.h>





void rms_stats_mean_std(rms_tagkey_type * mean , rms_tagkey_type * std , const char *parameter_name , int files , const char **filelist , bool log_transform) {
  int filenr;
  double norm = 1.0 / files;

  rms_tagkey_clear(mean);
  rms_tagkey_clear(std);


  printf("Loading: ");
  for (filenr = 0; filenr < files; filenr++) {
    printf("%s",filelist[filenr]); fflush(stdout);
    {
      rms_file_type *rms_file    = rms_file_alloc(filelist[filenr] , false);
      rms_tagkey_type * file_tag = rms_file_fread_alloc_data_tagkey(rms_file, "parameter" , "name" , parameter_name);
      
      if (log_transform)
        rms_tagkey_inplace_log10(file_tag);
      
      rms_tagkey_inplace_add_scaled(mean , file_tag , norm);
      rms_tagkey_inplace_sqr(file_tag);
      rms_tagkey_inplace_add_scaled(std , file_tag , norm);
      rms_tagkey_free(file_tag);
    
      rms_file_free(rms_file);
    }
    {
      int j;
      for (j = 0; j < strlen(filelist[filenr]); j++) fputc('\b' , stdout);
    }
  }
  printf("\n");
  
  {
    rms_tagkey_type * mean2;
    mean2 = rms_tagkey_copyc(mean);
    rms_tagkey_inplace_sqr(mean2);
    
    rms_tagkey_inplace_add_scaled(std , mean2 , -1.0);
    rms_tagkey_inplace_sqrt(std);
    rms_tagkey_free(mean2);
  }
}



void rms_stats_update_ens(const char *prior_path , const char *posterior_path , const char **file_list , const char *param_name , int ens_size , const double **X) {
  int iens , j;
  rms_tagkey_type ** prior;
  rms_tagkey_type  * post ;
  rms_tag_type     * dim_tag = NULL;

  if (!util_is_directory(posterior_path)) {
    fprintf(stderr,"%s: posterior_path:%s does not exist - aborting \n",__func__ , posterior_path);
    abort();
  }

  prior = malloc(ens_size * sizeof * prior);
  printf("Loading: ");
  for (iens = 0; iens < ens_size; iens++) {
    char * file_name         = util_alloc_filename(prior_path , file_list[iens] , NULL);
    printf("%s",file_name); fflush(stdout);
    {
      rms_file_type * rms_file = rms_file_alloc(file_name , false);
      prior[iens]              = rms_file_fread_alloc_data_tagkey(rms_file , "parameter" , "name" , param_name);
    
      if (iens == 0)
        dim_tag = rms_file_fread_alloc_tag(rms_file , "dimensions" , NULL , NULL);
    
      rms_file_free(rms_file);
    }
    for (j = 0; j < strlen(file_name); j++) fputc('\b' , stdout);
      
    
    free(file_name);
  }
  printf("\n");

  printf("Writing: ");
  post = rms_tagkey_copyc(prior[0]);
  for (iens = 0; iens < ens_size; iens++) {
    rms_tagkey_clear(post);
    
    for (j=0; j < iens; j++) 
      rms_tagkey_inplace_add_scaled(post , prior[j] , X[iens][j]);
    
    {
      char * file_name    = util_alloc_filename(posterior_path , file_list[iens] , NULL);
      rms_file_type *file = rms_file_alloc(file_name , false);
      FILE *stream        = rms_file_fopen_w(file);

      printf("%s",file_name); fflush(stdout);
      rms_file_init_fwrite(file , "parameter");
      rms_tag_fwrite(dim_tag , stream);
      rms_tag_fwrite_parameter(param_name , post , stream);
      rms_file_complete_fwrite(file);
      fclose(stream);
      rms_file_free(file);
      
      for (j = 0; j < strlen(file_name); j++) fputc('\b' , stdout);
      free(file_name);
    }
  }
  printf("\n");
  
  rms_tag_free(dim_tag);
  rms_tagkey_free(post);
  for (iens = 0; iens < ens_size; iens++) 
    rms_tagkey_free(prior[iens]);
  free(prior);
}

