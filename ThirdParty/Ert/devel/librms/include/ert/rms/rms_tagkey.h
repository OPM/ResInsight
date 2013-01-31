/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'rms_tagkey.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __RMS_TAGKEY_H__
#define __RMS_TAGKEY_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>

#include <ert/util/hash.h>

#include <ert/rms/rms_type.h>

#include <ert/ecl/ecl_util.h>

typedef struct rms_tagkey_struct rms_tagkey_type;


bool              rms_tagkey_cmp(const rms_tagkey_type * , const rms_tagkey_type * );
void              rms_tagkey_free(rms_tagkey_type *);
rms_tagkey_type * rms_tagkey_alloc_empty(bool);
rms_tagkey_type * rms_tagkey_alloc_complete(const char * , int , rms_type_enum , const void * , bool);
const char      * rms_tagkey_get_name(const rms_tagkey_type *);
rms_type_enum     rms_tagkey_get_rms_type(const rms_tagkey_type * );
ecl_type_enum     rms_tagkey_get_ecl_type(const rms_tagkey_type * );
void              rms_tagkey_manual_realloc_data(rms_tagkey_type * , int );
void              rms_tagkey_set_data(rms_tagkey_type * , const void * );

bool              rms_tagkey_char_eq(const rms_tagkey_type *, const char *);
void              rms_tagkey_free_(void *);
void            * rms_tagkey_copyc_(const void *);
void              rms_tagkey_load(rms_tagkey_type *, bool , FILE *, hash_type *);
void            * rms_tagkey_get_data_ref(const rms_tagkey_type *);
void              rms_tagkey_fwrite(const rms_tagkey_type * , FILE *);
void              rms_tagkey_fprintf(const rms_tagkey_type * , FILE *);
rms_tagkey_type * rms_tagkey_copyc(const rms_tagkey_type *);
int               rms_tagkey_get_size(const rms_tagkey_type *);


rms_tagkey_type * rms_tagkey_alloc_byteswap();
rms_tagkey_type * rms_tagkey_alloc_creationDate();
rms_tagkey_type * rms_tagkey_alloc_filetype(const char * );
rms_tagkey_type * rms_tagkey_alloc_dim(const char * , int );
rms_tagkey_type * rms_tagkey_alloc_parameter_name(const char * );

void rms_tagkey_assign(rms_tagkey_type * , const rms_tagkey_type *);
void rms_tagkey_apply(rms_tagkey_type * , double (f) (double));
void rms_tagkey_inplace_log10(rms_tagkey_type * );
void rms_tagkey_inplace_sqr(rms_tagkey_type *);
void rms_tagkey_inplace_sqrt(rms_tagkey_type *);
void rms_tagkey_inplace_mul(rms_tagkey_type * , const rms_tagkey_type *);
void rms_tagkey_inplace_add(rms_tagkey_type * , const rms_tagkey_type *);
void rms_tagkey_inplace_add_scaled(rms_tagkey_type * , const rms_tagkey_type * , double);
void rms_tagkey_scale(rms_tagkey_type * , double );
void rms_tagkey_clear(rms_tagkey_type *  );
int  rms_tagkey_get_sizeof_ctype(const rms_tagkey_type * );
void rms_tagkey_max_min(const rms_tagkey_type * , void *, void *);

#ifdef __cplusplus
}
#endif
#endif
