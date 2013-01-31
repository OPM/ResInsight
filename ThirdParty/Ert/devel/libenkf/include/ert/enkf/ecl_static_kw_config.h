/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_static_kw_config.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ECL_STATIC_KW_CONFIG_H__
#define __ECL_STATIC_KW_CONFIG_H__
#include <enkf_macros.h>

typedef struct ecl_static_kw_config_struct ecl_static_kw_config_type;



ecl_static_kw_config_type * ecl_static_kw_config_alloc(int , const char * , const char *);
int          		    ecl_static_kw_config_get_size       (const ecl_static_kw_config_type *);
const char   		  * ecl_static_kw_config_get_ensname_ref(const ecl_static_kw_config_type *);
void                        ecl_static_kw_config_free(ecl_static_kw_config_type *);


VOID_FREE_HEADER(ecl_static_kw_config);

#endif
