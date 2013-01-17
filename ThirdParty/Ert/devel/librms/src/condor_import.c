/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'condor_import.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <string.h>
#include <stdio.h>
#include <rms_file.h>
#include <rms_tag.h>
#include <rms_tagkey.h>
#include <util.h>



int main(int argc , char **argv) {
  int nx,ny,nz;
  char  * condor_file , *rms_file_name , *file_base, *path;
  float * data;
  char  * param_name = "PARAM";
  if (argc <= 4) {
    fprintf(stderr,"%s: nx ny nz condor-file \n",argv[0]);
    exit(1);
  }

  nx = atoi(argv[1]);
  ny = atoi(argv[2]);
  nz = atoi(argv[3]);
  condor_file = argv[4];
  data = util_malloc(nx*ny*nz * sizeof * data , __func__);
  
  {
    FILE *stream = util_fopen(condor_file , "r");
    int condor_index, rms_index;
    int i,j,k;
    float * tmp_data = util_malloc(nx*ny*nz * sizeof * data , __func__);
    for (i=0; i < nx*ny*nz; i++)
      fscanf(stream , "%g" , &tmp_data[i]);
    fclose(stream);

    rms_index = -1;
    for (i=0; i < nx; i++)
      for (j=0; j < ny; j++)
	for (k= (nz - 1); k >= 0; k--) {
	  rms_index    += 1;
	  condor_index  = i + j*nx + k*nx*ny;
	  /*k + j*nz + i*ny*nz;*/
	  data[rms_index] = tmp_data[condor_index];
	}
    
    free(tmp_data);
  }
  
  util_alloc_file_components(condor_file , &path , &file_base , NULL);
  if (path != NULL) {
    rms_file_name = util_malloc(strlen(path) + strlen(file_base) + 7, __func__);
    sprintf(rms_file_name , "%s/%s.ROFF" , path , file_base);
  } else {
    rms_file_name = util_malloc(strlen(file_base) + 7, __func__);
    sprintf(rms_file_name , "%s.ROFF" , file_base);
  }

  {
    rms_file_type * rms_file = rms_file_alloc(rms_file_name , false);
    FILE *stream = rms_file_fopen_w(rms_file);
    rms_tag_type    * dim_tag = rms_tag_alloc_dimensions(nx , ny , nz);
    rms_tagkey_type * data_key = rms_tagkey_alloc_complete("data" , nx*ny*nz , rms_float_type , data , true); 

    rms_file_init_fwrite(rms_file , "parameter");
    rms_tag_fwrite(dim_tag , stream);
    rms_tag_fwrite_parameter(param_name , data_key , stream);
    rms_file_complete_fwrite(rms_file);
    
    fclose(stream);
    rms_file_free(rms_file);
    rms_tagkey_free(data_key);
    rms_tag_free(dim_tag);
  }
  printf("Have written file: %s \n",rms_file_name);
  
  
  free(rms_file_name);
}
