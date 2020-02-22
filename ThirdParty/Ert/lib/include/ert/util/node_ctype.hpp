/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'node_ctype.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_NODE_CTYPE_H
#define ERT_NODE_CTYPE_H
#ifdef __cplusplus
extern "C" {
#endif


/*
  value : means a scalar which has been packed into the container
          object.

  pointer: means a (typed) pointer which points to a memory location
           outside the container object (however the container can own
           the memory).

*/

typedef enum  {CTYPE_VOID_POINTER = 1,
	       CTYPE_INT_VALUE    = 2,
	       CTYPE_DOUBLE_VALUE = 3,
	       CTYPE_FLOAT_VALUE  = 4 ,
	       CTYPE_CHAR_VALUE   = 5 ,
	       CTYPE_BOOL_VALUE   = 6 ,
	       CTYPE_SIZE_T_VALUE = 7 ,
	       CTYPE_INVALID      = 100} node_ctype;


const char * node_ctype_name(node_ctype );
#ifdef __cplusplus
}
#endif
#endif
