/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'fs_types.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdlib.h>

#include <ert/util/util.h>

#include <ert/enkf/fs_types.h>



fs_driver_impl fs_types_lookup_string_name(const char * driver_name) {
  if (strcmp(driver_name , "PLAIN") == 0)
    return PLAIN_DRIVER_ID;
  else if (strcmp(driver_name , "BLOCK_FS") == 0)
    return BLOCK_FS_DRIVER_ID;
  else {
    util_abort("%s: could not determine driver type for input:%s \n",__func__ , driver_name);
    return INVALID_DRIVER_ID;
  }
}



const char * fs_types_get_driver_name(fs_driver_enum driver_type) {
  switch( driver_type ) {
  case(DRIVER_PARAMETER):
    return "PARAMETER";
    break;
  case(DRIVER_STATIC):
    return "STATIC";
    break;
  case(DRIVER_DYNAMIC_FORECAST):
    return "FORECAST";
    break;
  case(DRIVER_DYNAMIC_ANALYZED):
    return "ANALYZED";
    break;
  case(DRIVER_INDEX):
    return "INDEX";
    break;
  default:
    util_abort("%s: driver_id:%d not recognized. \n",__func__ , driver_type );
    return NULL;
  }
}
