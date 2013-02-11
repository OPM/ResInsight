/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gen_kw_common.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __GEN_KW_COMMON_H__
#define __GEN_KW_COMMON_H__

/*
  Contains some headers which both gen_kw.c and gen_kw_config.c need -
  split like this to avoid circular dependencies.
*/


typedef struct gen_kw_config_struct gen_kw_config_type;
typedef struct gen_kw_struct        gen_kw_type;

gen_kw_type * gen_kw_alloc(const gen_kw_config_type * );
void          gen_kw_fload(gen_kw_type * , const char *);



#endif
