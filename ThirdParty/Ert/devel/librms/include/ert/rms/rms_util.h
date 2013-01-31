/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_util.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __RMS_UTIL_H__
#define __RMS_UTIL_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <ert/ecl/ecl_util.h>

#include <ert/rms/rms_type.h>

#define RMS_INACTIVE_DOUBLE -999.00
#define RMS_INACTIVE_FLOAT  -999.00
#define RMS_INACTIVE_INT    -999


rms_type_enum rms_util_convert_ecl_type(ecl_type_enum );
int           rms_util_global_index_from_eclipse_ijk(int, int, int, int, int, int);
void          rms_util_translate_undef(void * , int , int , const void * , const void * );
void          rms_util_set_fortran_data(void *, const void * , int , int , int  , int);
void          rms_util_read_fortran_data(const void *, void * , int , int , int , int);
void          rms_util_fskip_string(FILE *);
int           rms_util_fread_strlen(FILE *);
bool          rms_util_fread_string(char * ,  int , FILE *);
void          rms_util_fwrite_string(const char * string , FILE *stream);
void          rms_util_fwrite_comment(const char *  , FILE *);
void          rms_util_fwrite_newline(FILE *stream);

#ifdef __cplusplus
}
#endif
#endif
