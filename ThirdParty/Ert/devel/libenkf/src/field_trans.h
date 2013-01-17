/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'field_trans.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __FIELD_TRANS_H__
#define __FIELD_TRANS_H__
#ifdef __cplusplus 
extern "C" {
#endif
#include <stdbool.h>
#include <stdio.h>


typedef  float  (field_func_type) ( float );
typedef  struct field_trans_table_struct field_trans_table_type;


void                     field_trans_table_fprintf(const field_trans_table_type * , FILE * );
void                     field_trans_table_free(field_trans_table_type * );
void                     field_trans_table_add(field_trans_table_type * , const char * , const char *  , field_func_type * );
field_trans_table_type * field_trans_table_alloc();
bool                     field_trans_table_has_key(field_trans_table_type *  , const char * );
field_func_type        * field_trans_table_lookup(field_trans_table_type *  , const char * );



#ifdef __cplusplus 
}
#endif
#endif
