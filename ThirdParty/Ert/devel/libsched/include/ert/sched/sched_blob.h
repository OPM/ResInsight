/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_blob.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __SCHED_BLOB_H__
#define __SCHED_BLOB_H__
#ifdef __cplusplus 
extern "C" {
#endif
#include <stdlib.h>

typedef struct sched_blob_struct sched_blob_type;


void              sched_blob_append_token( sched_blob_type * blob , const char * token );
sched_blob_type * sched_blob_alloc( );
void              sched_blob_free( sched_blob_type * blob );
void              sched_blob_fprintf( const sched_blob_type * blob , FILE * stream );
int               sched_blob_get_size( const sched_blob_type * blob );

#ifdef __cplusplus 
}
#endif
#endif
