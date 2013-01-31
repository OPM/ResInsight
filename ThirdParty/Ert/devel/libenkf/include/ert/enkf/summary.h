/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'summary.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SUMMARY_H__
#define __SUMMARY_H__
#include <ert/util/double_vector.h>

#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_file.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/summary_config.h>






double    summary_get(const summary_type * summary, int report_step , state_enum state);

VOID_HAS_DATA_HEADER(summary);
UTIL_SAFE_CAST_HEADER(summary);
UTIL_SAFE_CAST_HEADER_CONST(summary);
VOID_ALLOC_HEADER(summary);
VOID_FREE_HEADER(summary);
VOID_COPY_HEADER(summary);
VOID_FORWARD_LOAD_HEADER(summary);
VOID_FORWARD_LOAD_VECTOR_HEADER(summary);
VOID_USER_GET_HEADER(summary);
VOID_USER_GET_VECTOR_HEADER(summary);
VOID_WRITE_TO_BUFFER_HEADER(summary);
VOID_READ_FROM_BUFFER_HEADER(summary);
VOID_SERIALIZE_HEADER(summary)
VOID_DESERIALIZE_HEADER(summary)
VOID_SET_INFLATION_HEADER(summary);
VOID_CLEAR_HEADER(summary);

VOID_IADD_HEADER(summary);
VOID_SCALE_HEADER(summary);
VOID_IMUL_HEADER(summary);
VOID_IADDSQR_HEADER(summary);
VOID_ISQRT_HEADER(summary);
#endif
