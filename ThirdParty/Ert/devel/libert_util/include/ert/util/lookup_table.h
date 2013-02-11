/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'lookup_table.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __LOOKUP_TABLE_H__
#define __LOOKUP_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/double_vector.h>

typedef struct lookup_table_struct lookup_table_type;


void                 lookup_table_set_data( lookup_table_type * lt , double_vector_type * x , double_vector_type * y , bool data_owner );
lookup_table_type  * lookup_table_alloc( double_vector_type * x , double_vector_type * y , bool data_owner);
lookup_table_type  * lookup_table_alloc_empty();
void                 lookup_table_append( lookup_table_type * lt , double x , double y);
void                 lookup_table_free( lookup_table_type * lt );
double               lookup_table_interp( lookup_table_type * lt , double x);
double               lookup_table_get_max_value(  lookup_table_type * lookup_table );
double               lookup_table_get_min_value(  lookup_table_type * lookup_table );
double               lookup_table_get_max_arg(  lookup_table_type * lookup_table );
double               lookup_table_get_max_arg(  lookup_table_type * lookup_table );
int                  lookup_table_get_size( const lookup_table_type * lt );

#ifdef __cplusplus
}
#endif
#endif
