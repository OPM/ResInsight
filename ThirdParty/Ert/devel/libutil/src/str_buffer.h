/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'str_buffer.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#ifdef __cplusplus
}
#endif
#endif

typedef struct str_buffer_struct str_buffer_type;


str_buffer_type * str_buffer_alloc(int );
str_buffer_type * str_buffer_alloc_with_string(const char *);
void              str_buffer_free(str_buffer_type *);
void              str_buffer_add_string(str_buffer_type *, const char *);
void              str_buffer_fprintf_substring(str_buffer_type * , int , int , FILE *);
const char      * str_buffer_get_char_ptr(const str_buffer_type *);
