/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'config_content_item.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>

#include <ert/util/test_util.h>
#include <ert/util/util.h>
#include <ert/util/hash.h>

#include <ert/config/config.h>
#include <ert/config/config_content_node.h>
#include <ert/config/config_schema_item.h>
#include <ert/config/config_path_elm.h>


int main(int argc , char ** argv) {
  const char * config_file = argv[1];
  config_type * config = config_alloc();
  
  config_add_schema_item( config , "SET" , true );
  config_add_schema_item( config , "NOTSET" , false );

  test_assert_true( config_parse( config , config_file , "--" , "INCLUDE" , NULL , CONFIG_UNRECOGNIZED_IGNORE , true ));

  test_assert_not_NULL( config_get_content_item( config , "SET" ));
  test_assert_NULL( config_get_content_item( config , "NOTSET" ) );
  test_assert_NULL( config_get_content_item( config , "UNKNOWN" ) );

  test_assert_true( config_has_schema_item( config , "SET" ));
  test_assert_true( config_has_schema_item( config , "NOTSET" ));
  test_assert_false( config_has_schema_item( config , "UNKNOWN" ));

  test_assert_true( config_has_content_item( config , "SET" ));
  test_assert_false( config_has_content_item( config , "NOTSET" ));
  test_assert_false( config_has_content_item( config , "UNKNOWN" ));
  
  

  
  exit(0);
}

