/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'mzran.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __MZRAN_H__
#define __MZRAN_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <util.h>

typedef struct mzran_struct mzran_type;

#define MZRAN_MAX_VALUE  4294967296
#define MZRAN_STATE_SIZE 16             /* Size of the seed buffer - in bytes. */


void              mzran_fscanf_state( void * __rng , FILE * stream );
unsigned int      mzran_forward(void * __rng);
void            * mzran_alloc( );
void              mzran_set_state(void * __rng , const char * seed_buffer);
double            mzran_get_double(mzran_type * rng);
int               mzran_get_int( mzran_type * rng, int max);
void              mzran_fprintf_state( const void * __rng , FILE * stream);
void              mzran_free( void * __rng );

#ifdef __cplusplus
}
#endif
#endif
