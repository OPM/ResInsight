/*
   Copyright (C) 2017  Statoil ASA, Norway.

   The file 'ecl_type.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <string.h>

#include <ert/util/util.h>
#include <ert/ecl/ecl_type.h>

/*****************************************************************/
/* The string names for the different ECLIPSE low-level
   types.
*/
#define ECL_TYPE_NAME_CHAR     "CHAR"
#define ECL_TYPE_NAME_C010     "C010"
#define ECL_TYPE_NAME_FLOAT    "REAL"
#define ECL_TYPE_NAME_INT      "INTE"
#define ECL_TYPE_NAME_DOUBLE   "DOUB"
#define ECL_TYPE_NAME_BOOL     "LOGI"
#define ECL_TYPE_NAME_MESSAGE  "MESS"


ecl_data_type ecl_type_create(const ecl_type_enum type, const size_t element_size) {
    ecl_data_type ecl_type = ecl_type_create_from_type(type);

    if(ecl_type.element_size != element_size)
        util_abort(
                "%s: element_size mismatch for type %d, was: %d, expected: %d\n",
                __func__, type, element_size, ecl_type.element_size);

    return ecl_type;
}

ecl_data_type ecl_type_create_from_type(const ecl_type_enum type) {
    switch(type) {
    case(ECL_CHAR_TYPE):
      return ECL_CHAR;
    case(ECL_INT_TYPE):
      return ECL_INT;
    case(ECL_FLOAT_TYPE):
      return ECL_FLOAT;
    case(ECL_DOUBLE_TYPE):
      return ECL_DOUBLE;
    case(ECL_BOOL_TYPE):
      return ECL_BOOL;
    case(ECL_MESS_TYPE):
      return ECL_MESS;
    case(ECL_C010_TYPE):
      return ECL_C010;
    default:
      util_abort("%s: invalid ecl_type: %d\n", __func__, type);
      return ECL_INT; /* Dummy */
    }
}

ecl_type_enum ecl_type_get_type(const ecl_data_type ecl_type) {
    return ecl_type.type;
}

size_t ecl_type_get_element_size(const ecl_data_type ecl_type) {
    return ecl_type.element_size;
}

const char * ecl_type_get_name(const ecl_data_type ecl_type) {
  switch (ecl_type.type) {
  case(ECL_CHAR_TYPE):
    return ECL_TYPE_NAME_CHAR ;
  case(ECL_C010_TYPE):
    return ECL_TYPE_NAME_C010;
  case(ECL_FLOAT_TYPE):
    return ECL_TYPE_NAME_FLOAT;
  case(ECL_DOUBLE_TYPE):
    return ECL_TYPE_NAME_DOUBLE;
  case(ECL_INT_TYPE):
    return ECL_TYPE_NAME_INT;
  case(ECL_BOOL_TYPE):
    return ECL_TYPE_NAME_BOOL;
  case(ECL_MESS_TYPE):
    return ECL_TYPE_NAME_MESSAGE;
  default:
    util_abort("Internal error in %s - internal eclipse_type: %d not recognized - aborting \n",__func__ , ecl_type.type);
    return NULL; /* Dummy */
  }
}

ecl_data_type ecl_type_create_from_name( const char * type_name ) {
  if (strncmp( type_name , ECL_TYPE_NAME_FLOAT , ECL_TYPE_LENGTH) == 0)
    return ECL_FLOAT;
  else if (strncmp( type_name , ECL_TYPE_NAME_INT , ECL_TYPE_LENGTH) == 0)
    return ECL_INT;
  else if (strncmp( type_name , ECL_TYPE_NAME_DOUBLE , ECL_TYPE_LENGTH) == 0)
    return ECL_DOUBLE;
  else if (strncmp( type_name , ECL_TYPE_NAME_CHAR , ECL_TYPE_LENGTH) == 0)
    return ECL_CHAR;
  else if (strncmp( type_name , ECL_TYPE_NAME_C010 , ECL_TYPE_LENGTH) == 0)
    return ECL_C010;
  else if (strncmp( type_name , ECL_TYPE_NAME_MESSAGE , ECL_TYPE_LENGTH) == 0)
    return ECL_MESS;
  else if (strncmp( type_name , ECL_TYPE_NAME_BOOL , ECL_TYPE_LENGTH) == 0)
    return ECL_BOOL;
  else {
    util_abort("%s: unrecognized type name:%s \n",__func__ , type_name);
    return ECL_INT; /* Dummy */
  }
}


int ecl_type_get_sizeof_ctype_fortio(const ecl_data_type ecl_type) {
  if(ecl_type_is_char(ecl_type) || ecl_type_is_C010(ecl_type))
      return ecl_type.element_size - 1;
  else
      return ecl_type_get_sizeof_ctype(ecl_type);
}

int ecl_type_get_sizeof_ctype(const ecl_data_type ecl_type) {
   return ecl_type.element_size;
}

bool ecl_type_is_numeric(const ecl_data_type ecl_type) {
    return (ecl_type_is_int(ecl_type) ||
            ecl_type_is_float(ecl_type) ||
            ecl_type_is_double(ecl_type));
}

bool ecl_type_is_equal(const ecl_data_type ecl_type1,
                       const ecl_data_type ecl_type2) {
    return (ecl_type1.type         == ecl_type2.type &&
            ecl_type1.element_size == ecl_type2.element_size);
}

bool ecl_type_is_char(const ecl_data_type ecl_type) {
    return (ecl_type.type == ECL_CHAR_TYPE);
}

bool ecl_type_is_int(const ecl_data_type ecl_type) {
    return (ecl_type.type == ECL_INT_TYPE);
}

bool ecl_type_is_float(const ecl_data_type ecl_type) {
    return (ecl_type.type == ECL_FLOAT_TYPE);
}

bool ecl_type_is_double(const ecl_data_type ecl_type) {
    return (ecl_type.type == ECL_DOUBLE_TYPE);
}

bool ecl_type_is_mess(const ecl_data_type ecl_type) {
    return (ecl_type.type == ECL_MESS_TYPE);
}
 
bool ecl_type_is_bool(const ecl_data_type ecl_type) {
    return (ecl_type.type == ECL_BOOL_TYPE);
}

bool ecl_type_is_C010(const ecl_data_type ecl_type) {
    return (ecl_type.type == ECL_C010_TYPE);
}
