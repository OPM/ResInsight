/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'trans_func.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_TRANS_FUNC_H
#define ERT_TRANS_FUNC_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/arg_pack.h>

#include <ert/enkf/enkf_types.h>


typedef struct trans_func_struct  trans_func_type;
typedef double (transform_ftype) (double , const arg_pack_type *);
typedef bool   (validate_ftype)  (const trans_func_type * );

trans_func_type  * trans_func_fscanf_alloc( FILE * stream, const char * filename );
double             trans_func_eval( const trans_func_type * trans_func , double x);

void               trans_func_free( trans_func_type * trans_func );
void               trans_func_iset_double_param(trans_func_type  * trans_func , int param_index , double value );
bool               trans_func_set_double_param( trans_func_type  * trans_func , const char * param_name , double value );
void               trans_func_iset_int_param(trans_func_type  * trans_func , int param_index , int value );
bool               trans_func_set_int_param( trans_func_type  * trans_func , const char * param_name , int value );
bool               trans_func_use_log_scale(const trans_func_type  * trans_func );

#ifdef __cplusplus
}
#endif
#endif
