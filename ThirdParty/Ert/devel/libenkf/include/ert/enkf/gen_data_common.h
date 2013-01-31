/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_data_common.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __GEN_DATA_COMMON_H__
#define __GEN_DATA_COMMON_H__

/*
  Contains some headers which both gen_data.c and gen_data_config.c need -
  split like this to avoid circular dependencies.
*/


typedef struct gen_data_config_struct gen_data_config_type;
typedef struct gen_data_struct        gen_data_type;

gen_data_type * gen_data_alloc(const gen_data_config_type * );
void            gen_data_fload(gen_data_type * , const char * );



#endif
