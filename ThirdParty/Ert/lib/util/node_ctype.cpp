/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'node_ctype.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdio.h>
#include <stdlib.h>

#include <ert/util/util.hpp>
#include <ert/util/node_ctype.hpp>


const char * node_ctype_name(node_ctype ctype) {
  const char * name;
  switch (ctype) {
  case(CTYPE_VOID_POINTER):
    name =  "void pointer";
    break;
  case(CTYPE_INT_VALUE):
    name =  "integer value";
    break;
  case(CTYPE_DOUBLE_VALUE):
    name =  "double value";
    break;
  case(CTYPE_FLOAT_VALUE):
    name =  "float_value";
    break;
  case(CTYPE_CHAR_VALUE):
    name =  "char value";
    break;
  case(CTYPE_BOOL_VALUE):
    name =  "bool value";
    break;
  case(CTYPE_SIZE_T_VALUE):
    name =  "size_t value";
    break;
  default:
    name = NULL;
    util_abort("%s: fatal internal error node_ctype:%d not recognized - aborting. \n", __func__ , ctype);
  }
  return name;
}
