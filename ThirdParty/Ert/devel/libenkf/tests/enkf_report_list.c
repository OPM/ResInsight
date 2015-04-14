/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'enkf_main.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <unistd.h>

#include <ert/util/test_util.h>

#include <ert/config/config_parser.h>
#include <ert/config/config_content.h>

#include <ert/enkf/ert_report_list.h>

int main(int argc , char ** argv) {
  config_parser_type * config = config_alloc();
  ert_report_list_type * report_list = ert_report_list_alloc( NULL , NULL );

  test_assert_not_NULL( report_list );
  ert_report_list_add_config_items( config );
  {
    config_content_type * content = config_parse( config , argv[1] , "--" , NULL, NULL , CONFIG_UNRECOGNIZED_IGNORE , true );
    test_assert_true( config_content_is_valid(content) );
    ert_report_list_init( report_list , content , NULL);
    config_content_free( content );
  }

  test_assert_int_equal( 167 , ert_report_list_get_latex_timeout( report_list ));
  test_assert_true( ert_report_list_get_init_large_report( report_list ));

  ert_report_list_free( report_list );
  exit(0);
}

