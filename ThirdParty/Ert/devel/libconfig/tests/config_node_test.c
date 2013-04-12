/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'config_node_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdbool.h>

#include <ert/config/config.h>



int main(int argc , char ** argv) {
  const char * config_file = argv[1];
  config_type * config = config_alloc();
  bool OK;
  {
    config_schema_item_type * item  = config_add_schema_item(config , "APPEND" , false );
    config_schema_item_set_argc_minmax( item , 1 , 1);
  }
  config_add_schema_item(config , "NEXT"   , false );
  
  OK = config_parse(config , config_file , "--" , NULL , NULL , false , true );
  
  if (OK) {
    if (config_get_content_size( config ) == 4) {
      const config_content_node_type * node0 = config_iget_content_node( config , 0 );
      if (strcmp( config_content_node_get_kw( node0 ) , "APPEND") == 0) {
        if (config_content_node_get_size(node0) == 1) {
          const config_content_node_type * node3 = config_iget_content_node( config , 3 );
          if (strcmp( config_content_node_get_kw( node3 ) , "NEXT") == 0) {
            if (config_content_node_get_size(node3) == 2) {
              exit(0);
            } else printf("Size error node3\n");
          } else printf("kw error node3 \n");
        } else printf("Size error node0\n");
      } else printf("kw error node0 kw:%s \n", config_content_node_get_kw( node0 ));
    } else printf("Size error \n");
  } else printf("Parse error");
  
  exit(1);
}

