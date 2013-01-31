/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'gruptree.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __GRUPTREE_H__
#define __GRUPTREE_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef struct gruptree_struct gruptree_type;

gruptree_type * gruptree_alloc();
gruptree_type * gruptree_copyc(const gruptree_type *);
gruptree_type * gruptree_fread_alloc(FILE *);
void            gruptree_fwrite(const gruptree_type *, FILE *);
void            gruptree_free(gruptree_type *);


void            gruptree_register_grup(gruptree_type *, const char *, const char *);
void            gruptree_register_well(gruptree_type *, const char *, const char *);
bool            gruptree_has_grup(const gruptree_type *, const char *);
char         ** gruptree_alloc_grup_well_list(gruptree_type *, const char *, int *);
void            gruptree_printf_grup_wells(gruptree_type *, const char *);

#ifdef __cplusplus
}
#endif
#endif
