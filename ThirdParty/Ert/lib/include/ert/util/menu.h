/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'menu.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_MENU_H
#define ERT_MENU_H

typedef struct menu_struct menu_type;
typedef struct menu_item_struct menu_item_type;

typedef void (menu_func_type) (void *);
typedef void (arg_free_ftype) (void *);

menu_type      * menu_alloc(const char * , const char * , const char *);
void             menu_run(const menu_type * );
void             menu_free(menu_type * );
menu_item_type * menu_get_item(const menu_type * , char );
menu_item_type * menu_add_item(menu_type *, const char * , const char * , menu_func_type * , void * , arg_free_ftype * );
void             menu_add_separator(menu_type * );
void             menu_add_helptext(menu_type * , const char * );
menu_item_type * menu_get_item(const menu_type * , char );
void             menu_set_title(menu_type *, const char *);

void             menu_item_set_label( menu_item_type * , const char *);
void             menu_item_disable( menu_item_type * item );
void             menu_item_enable( menu_item_type * item );
void             menu_add_helptext(menu_type * menu, const char * label );

#endif
