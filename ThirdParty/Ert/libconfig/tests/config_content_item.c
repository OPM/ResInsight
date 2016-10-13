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

#include <ert/config/config_parser.h>
#include <ert/config/config_content_node.h>
#include <ert/config/config_schema_item.h>
#include <ert/config/config_path_elm.h>


int main(int argc , char ** argv) {
  const char * config_file = argv[1];
  config_parser_type * config = config_alloc();

  config_add_schema_item( config , "SET" , true );
  config_add_schema_item( config , "NOTSET" , false );

  {
    config_content_type * content = config_parse( config , config_file , "--" , "INCLUDE" , NULL , NULL , CONFIG_UNRECOGNIZED_IGNORE , true );
    test_assert_true( config_content_is_instance( content ));
    test_assert_true(config_content_is_valid( content ));

    test_assert_true( config_content_has_item( content , "SET" ));
    test_assert_false( config_content_has_item( content , "NOTSET" ) );
    test_assert_false( config_content_has_item( content , "UNKNOWN" ) );

    test_assert_true( config_has_schema_item( config , "SET" ));
    test_assert_true( config_has_schema_item( config , "NOTSET" ));
    test_assert_false( config_has_schema_item( config , "UNKNOWN" ));

    config_content_free( content );
  }

  exit(0);
}

