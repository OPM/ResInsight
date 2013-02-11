/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_static_kw.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef  __ECL_STATIC_KW_H__
#define  __ECL_STATIC_KW_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/util.h>

#include <ert/ecl/ecl_kw.h>

#include <ert/enkf/enkf_macros.h>


typedef struct ecl_static_kw_struct ecl_static_kw_type;


ecl_static_kw_type * ecl_static_kw_alloc();
void                 ecl_static_kw_free(ecl_static_kw_type *ecl_static_kw);
void                 ecl_static_kw_init(ecl_static_kw_type * , const ecl_kw_type * );
ecl_kw_type        * ecl_static_kw_ecl_kw_ptr(const ecl_static_kw_type * );
void               * ecl_static_kw_alloc__(const void *);

UTIL_SAFE_CAST_HEADER(ecl_static_kw);
UTIL_SAFE_CAST_HEADER_CONST(ecl_static_kw);
VOID_FREE_HEADER(ecl_static_kw);
VOID_FREE_DATA_HEADER(ecl_static_kw);
VOID_COPY_HEADER(ecl_static_kw);
VOID_ECL_WRITE_HEADER(ecl_static_kw);
VOID_READ_FROM_BUFFER_HEADER(ecl_static_kw);
VOID_WRITE_TO_BUFFER_HEADER(ecl_static_kw);

#ifdef __cplusplus
}
#endif
#endif
