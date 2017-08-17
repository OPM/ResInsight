/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'msg.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_MSG_H
#define ERT_MSG_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/util/type_macros.h>


typedef struct msg_struct msg_type;



msg_type   * msg_alloc(const char * , bool debug);
void         msg_show(msg_type * );
void         msg_free(msg_type *  , bool);
void         msg_update(msg_type * , const char * );
void         msg_update_int(msg_type * , const char * , int );
void         msg_hide(msg_type *);
void         msg_clear_msg(msg_type * msg);


UTIL_SAFE_CAST_HEADER( msg );

#ifdef __cplusplus
}
#endif
#endif
