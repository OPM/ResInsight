/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'field_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <field_config.h>
#include <enkf_types.h>
#include <field.h>
#include <ecl_kw.h>
#include <fortio.h>
#include <ecl_grid.h>
#include <pgbox_config.h>
#include <pgbox.h>
#include <void_arg.h>




int main (int argc , char **argv) {
  const char * config_file = "../../librms/src/Testing/PERMX_%d";
  const char *EGRID_file = "../../librms/src/Testing/GRANE.EGRID";
  int nx , ny , nz , active_size;

  field_config_type * field_config;
  field_type * field1 , *field2;
  
  ecl_grid_type * ecl_grid  = ecl_grid_alloc(EGRID_file , true);
  ecl_grid_get_dims(ecl_grid , &nx , &ny , &nz , &active_size);


  field_config = field_config_alloc_parameter("PERMX" , nx , ny , nz , active_size , ecl_grid_get_index_map_ref(ecl_grid) , 0 , load_unique , 1 , (const char **) &config_file);
  field1        = field_alloc(field_config);
  field2        = field_alloc(field_config);
  
  field_fload(field1 , "../../librms/src/Testing/PERMX_1" , true);
  field_ROFF_export(field1 , "Testing/PERMX_1");

  field_fload(field2 , "Testing/PERMX_1" , true);
  field_ROFF_export(field2 , "Testing/PERMX_2");  

  if (field_cmp(field1 , field2)) 
    printf("EQUAL\n");
  else
    printf("DIFFERENT\n");
    

  field_config_free(field_config);
  ecl_grid_free(ecl_grid);
}
