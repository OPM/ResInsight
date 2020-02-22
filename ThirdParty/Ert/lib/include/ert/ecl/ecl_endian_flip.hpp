/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ecl_endian_flip.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_ENDIAN_FLIP_H
#define ERT_ECL_ENDIAN_FLIP_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
   This header file checks if the ECLIPSE endianness and the hardware
   endianness are equal, and defines the macro ECL_ENDIAN_FLIP
   accordingly.

   All the ecl_xxx functions will use the ECL_ENDIAN_FLIP macro to
   determine whether the endian flip should be performed. When opening
   a fortio instance explicitly you can use the ECL_ENDIAN_FLIP macro
   to get the endianness correct (for ECLIPSE usage that is).
*/

#define ECLIPSE_BYTE_ORDER  __BIG_ENDIAN   // Alternatively: __LITTLE_ENDIAN

#ifdef BYTE_ORDER
  #if  BYTE_ORDER == ECLIPSE_BYTE_ORDER
    #define ECL_ENDIAN_FLIP false
  #else
    #define ECL_ENDIAN_FLIP true
  #endif
#else
  #ifdef WIN32
    #define ECL_ENDIAN_FLIP true    // Unconditional byte flip on Windows.
  #else
    #error: The macro BYTE_ORDER is not defined?
  #endif
#endif

#undef ECLIPSE_BYTE_ORDER


#ifdef __cplusplus
}
#endif
#endif
