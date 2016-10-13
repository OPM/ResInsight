/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_enkf_config_node_gen_data.c' is part of ERT -
   Ensemble based Reservoir Tool.
    
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


#include <ert/util/test_util.h>

#include <ert/enkf/enkf_config_node.h>


void test_create() {
  enkf_config_node_type * node = enkf_config_node_alloc_GEN_PARAM("key" , false, ASCII , ASCII , "init%d" , "out.txt");
  enkf_config_node_free( node );
}



int main( int argc , char **argv ) {
  test_create();
  exit(0);
}
