/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'field_common.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __FIELD_COMMON_H__
#define __FIELD_COMMON_H__

/*
  Contains some headers which both field.c and field_config.c need -
  split like this to avoid circular dependencies.
*/



typedef struct field_config_struct field_config_type;
typedef struct field_struct        field_type;

field_type * field_alloc(const field_config_type * );
void         field_fload(field_type * , const char * );



#endif
