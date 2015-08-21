/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'config_test.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stringlist.h>
#include <hash.h>
#include <config.h>


int main(void) {
  const char * config_file = "config_test_input";
  config_parser_type * config = config_alloc();
  config_schema_item_type * item;
  
  item = config_add_schema_item(config , "KEY1" , true  , true);
  item = config_add_schema_item(config , "KEY2" , true  , false);
  config_schema_item_set_argc_minmax(item , 1 , 4 , 4 , (const config_item_types [4]) {CONFIG_EXECUTABLE , CONFIG_EXISTING_FILE , CONFIG_BOOLEAN , CONFIG_BOOLEAN});


  item = config_add_schema_item(config , "FATHER"  , false , false);
  {
    stringlist_type * children = stringlist_alloc_argv_ref( (const char *[2]) {"CHILD1" , "CHILD2"} , 2);
    config_schema_item_set_required_children(item , children);
    stringlist_free(children);
  }
  item = config_add_schema_item(config , "CHILD1"  , false , false);
  config_schema_item_set_argc_minmax(item , 1 , 1 , 1 , (const config_item_types [1]) {CONFIG_INT});
  
  config_parse(config , config_file , "--" , "INCLUDE" , NULL , true, true);
  


  {
    stringlist_type * sl = config_alloc_complete_stringlist(config , "KEY1");
    char * s = stringlist_alloc_joined_string(sl , "|");
    printf("KEY1 -> \"%s\" \n",s);
    printf("CONFIG_IGET:%s\n" , config_iget(config , "KEY2" , 0 , 0));
    free(s);
    stringlist_free(sl);
  }
  
  
  config_free(config);
}
