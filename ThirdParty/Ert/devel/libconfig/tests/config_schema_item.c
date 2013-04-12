/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'config_schema_item.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>

#include <ert/config/config_schema_item.h>



int main(int argc , char ** argv) {
  config_schema_item_type * schema_item = config_schema_item_alloc( "KW" , false );
  
  test_assert_int_equal( config_schema_item_iget_type( schema_item , 1 ) , CONFIG_STRING );
  test_assert_int_equal( config_schema_item_iget_type( schema_item , 2 ) , CONFIG_STRING );
  
  config_schema_item_iset_type( schema_item , 0 , CONFIG_INT );
  config_schema_item_iset_type( schema_item , 5 , CONFIG_BOOL );
  

  test_assert_int_equal( config_schema_item_iget_type( schema_item , 0 ) , CONFIG_INT );
  test_assert_int_equal( config_schema_item_iget_type( schema_item , 1 ) , CONFIG_STRING );
  test_assert_int_equal( config_schema_item_iget_type( schema_item , 2 ) , CONFIG_STRING );
  test_assert_int_equal( config_schema_item_iget_type( schema_item , 5 ) , CONFIG_BOOL );

  config_schema_item_set_default_type( schema_item , CONFIG_FLOAT );
  test_assert_int_equal( config_schema_item_iget_type( schema_item , 7 ) , CONFIG_FLOAT );
  
  config_schema_item_free( schema_item );
  exit(0);
}

