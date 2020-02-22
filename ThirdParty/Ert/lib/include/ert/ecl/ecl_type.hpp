/*
   Copyright (C) 2017  Equinor ASA, Norway.

   The file 'ecl_type.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_TYPE_H
#define ERT_ECL_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>

/*
  The type of an eclipse keyword is carried by a struct
  ecl_type_struct which contains a type enum, and the size in bytes of
  one such element. These structs are for the most part handled with
  value semantics, and created with macros ECL_INT, ECL_FLOAT and so
  on.

  The macros in C use designated initializers, whereas the C++ macros
  use a constructor, for this reason this file has two slightly
  different code paths for C and C++.
*/

#define ECL_STRING8_LENGTH   8
#define ECL_TYPE_LENGTH      4

typedef enum {
  ECL_CHAR_TYPE   = 0,
  ECL_FLOAT_TYPE  = 1,
  ECL_DOUBLE_TYPE = 2,
  ECL_INT_TYPE    = 3,
  ECL_BOOL_TYPE   = 4,
  ECL_MESS_TYPE   = 5,
  ECL_STRING_TYPE = 7
} ecl_type_enum;

#define ECL_TYPE_ENUM_DEFS {.value = 0 , .name = "ECL_CHAR_TYPE"}, \
{.value = 1 , .name = "ECL_FLOAT_TYPE"} ,                          \
{.value = 2 , .name = "ECL_DOUBLE_TYPE"},                          \
{.value = 3 , .name = "ECL_INT_TYPE"},                             \
{.value = 4 , .name = "ECL_BOOL_TYPE"},                            \
{.value = 5 , .name = "ECL_MESS_TYPE"},                            \
{.value = 7 , .name = "ECL_STRING_TYPE"}


/*
  Character data in ECLIPSE files comes as an array of fixed-length
  string. Each of these strings is 8 characters long. The type name,
  i.e. 'REAL', 'INTE', ... , come as 4 character strings.
*/

#define ECL_STRING8_LENGTH   8  // 'Normal' 8 characters 'CHAR' type.
#define ECL_TYPE_LENGTH      4

struct ecl_type_struct {
  const ecl_type_enum type;
  const size_t element_size;
};

#ifdef __cplusplus

#define ECL_INT ecl_data_type{ ECL_INT_TYPE, sizeof(int)}
#define ECL_FLOAT ecl_data_type{ ECL_FLOAT_TYPE, sizeof(float)}
#define ECL_DOUBLE ecl_data_type{ ECL_DOUBLE_TYPE, sizeof(double)}
#define ECL_BOOL ecl_data_type{ ECL_BOOL_TYPE, sizeof(bool)}
#define ECL_CHAR ecl_data_type{ ECL_CHAR_TYPE, ECL_STRING8_LENGTH + 1}
#define ECL_MESS ecl_data_type{ ECL_MESS_TYPE, 0}
#define ECL_STRING(size) ecl_data_type{ECL_STRING_TYPE, (size) + 1}

}

#else

#define ECL_CHAR (ecl_data_type) {.type = ECL_CHAR_TYPE, .element_size = ECL_STRING8_LENGTH + 1}
#define ECL_INT (ecl_data_type) {.type = ECL_INT_TYPE, .element_size = sizeof(int)}
#define ECL_FLOAT (ecl_data_type) {.type = ECL_FLOAT_TYPE, .element_size = sizeof(float)}
#define ECL_DOUBLE (ecl_data_type) {.type = ECL_DOUBLE_TYPE, .element_size = sizeof(double)}
#define ECL_BOOL (ecl_data_type) {.type = ECL_BOOL_TYPE, .element_size = sizeof(bool)}
#define ECL_MESS (ecl_data_type) {.type = ECL_MESS_TYPE, .element_size = 0}
#define ECL_STRING(size) (ecl_data_type) {.type = ECL_STRING_TYPE, .element_size = (size) + 1}

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ecl_type_struct ecl_data_type;

ecl_data_type      ecl_type_create_from_name(const char *);
ecl_data_type      ecl_type_create(const ecl_type_enum, const size_t);
ecl_data_type      ecl_type_create_from_type(const ecl_type_enum);

ecl_type_enum      ecl_type_get_type(const ecl_data_type);
char *             ecl_type_alloc_name(const ecl_data_type);

int                ecl_type_get_sizeof_ctype(const ecl_data_type);
int                ecl_type_get_sizeof_iotype(const ecl_data_type);

bool               ecl_type_is_equal(const ecl_data_type, const ecl_data_type);

bool               ecl_type_is_numeric(const ecl_data_type);
bool               ecl_type_is_alpha(const ecl_data_type);
bool               ecl_type_is_char(const ecl_data_type);
bool               ecl_type_is_int(const ecl_data_type);
bool               ecl_type_is_float(const ecl_data_type);
bool               ecl_type_is_double(const ecl_data_type);
bool               ecl_type_is_mess(const ecl_data_type);
bool               ecl_type_is_bool(const ecl_data_type);
bool               ecl_type_is_string(const ecl_data_type);

// Temporary fixup for OPM.
char * ecl_type_get_name(const ecl_data_type);

#ifdef __cplusplus
}
#endif


#endif
