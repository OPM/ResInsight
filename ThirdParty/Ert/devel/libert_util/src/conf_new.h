/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'conf_new.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __CONF_NEW_H__
#define __CONF_NEW_H__

#include <stringlist.h>
#ifdef __cplusplus
extern "C" {
#endif

#define CONF_OK                         0
#define CONF_PARSE_ERROR                1
#define CONF_CIRCULAR_INCLUDE_ERROR     2
#define CONF_UNEXPECTED_EOF_ERROR       3 
#define CONF_UNABLE_TO_OPEN_FILE        4



typedef struct conf_struct      conf_type;
typedef struct conf_item_struct conf_item_type; 
typedef struct conf_spec_struct conf_spec_type;

typedef int (validator_ftype)(conf_type * conf);




#ifdef __cplusplus
}
#endif
#endif


