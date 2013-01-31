/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'surface.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SURFACE_H__
#define __SURFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/surface_config.h>

  typedef struct surface_struct surface_type;

  UTIL_SAFE_CAST_HEADER(surface);
  UTIL_SAFE_CAST_HEADER_CONST(surface);
  VOID_ALLOC_HEADER(surface);
  VOID_FREE_HEADER(surface);
  VOID_ECL_WRITE_HEADER(surface);
  VOID_COPY_HEADER(surface);
  VOID_USER_GET_HEADER(surface);
  VOID_WRITE_TO_BUFFER_HEADER(surface);
  VOID_READ_FROM_BUFFER_HEADER(surface);
  VOID_SERIALIZE_HEADER(surface);
  VOID_DESERIALIZE_HEADER(surface);
  VOID_SET_INFLATION_HEADER(surface);
  VOID_CLEAR_HEADER(surface);
  VOID_IADD_HEADER(surface);
  VOID_SCALE_HEADER(surface);
  VOID_IMUL_HEADER(surface);
  VOID_IADDSQR_HEADER(surface);
  VOID_ISQRT_HEADER(surface);
  VOID_INITIALIZE_HEADER(surface);

#ifdef __cplusplus
}
#endif

#endif
