/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'field_convert.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <field_config.h>
#include <field.h>
#include <ecl_grid.h>


void usage(const char * cmd) {
  printf("%s:  GRID_FILE OUTPATH field1  field2  field3 ....\n",cmd);
  exit(1);
}


int main(int argc , char ** argv) {
  if (argc < 4) usage(argv[0]);
  {
    const char  * grid_file = argv[1];
    const char  * out_path  = argv[2];
    const char ** file_list = (const char **) &argv[3];
    int   num_files         = argc - 3;
    int   ifile;
    field_file_format_type file_type;

    ecl_grid_type * ecl_grid = ecl_grid_alloc(grid_file , true);
    field_config_type *field_config = field_config_alloc_dynamic("XX" , NULL , NULL , ecl_grid); 
    field_type * field = field_alloc(field_config);
    field_config_enkf_OFF(field_config);
    
    util_make_path(out_path);
    file_type = field_config_manual_file_type("Export files to type: " , false);
    printf("num_files:%d \n",num_files);
    for (ifile = 0; ifile < num_files; ifile++) {
      char * base_name; 
      char * target_file;
      util_alloc_file_components(file_list[ifile] , NULL , &base_name , NULL);
      target_file = util_alloc_filename(out_path , base_name , field_config_default_extension(file_type , true));
      field_fload(field , file_list[ifile] , true);
      printf("Converting: %s -> %s \n",file_list[ifile] , target_file);
          field_export(field , target_file , file_type);
      free(target_file);
      free(base_name);
    }
		   

    field_free(field);
    field_config_free(field_config);
    ecl_grid_free(ecl_grid);
  }


}
