/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_type.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <ert/rms/rms_type.h>

/*****************************************************************/
/* A microscopic (purely internal) type object only used 
   for storing the hash type_map */
/*****************************************************************/




void rms_type_free(void *rms_t) {
  free( (__rms_type *) rms_t);
}


static __rms_type * rms_type_set(__rms_type *rms_t , rms_type_enum rms_type , int sizeof_ctype) {
  rms_t->rms_type     = rms_type;
  rms_t->sizeof_ctype = sizeof_ctype;
  return rms_t;
}


__rms_type * rms_type_alloc(rms_type_enum rms_type, int sizeof_ctype) {
  __rms_type *rms_t   = malloc(sizeof *rms_t);
  rms_type_set(rms_t , rms_type , sizeof_ctype);
  return rms_t;
}


const void * rms_type_copyc(const void *__rms_t) {
  const __rms_type *rms_t = (const __rms_type *) __rms_t;
  __rms_type *new_t = rms_type_alloc(rms_t->rms_type , rms_t->sizeof_ctype);
  return new_t;
}
