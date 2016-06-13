/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'meas_data.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_MEAS_DATA_H
#define ERT_MEAS_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <ert/util/matrix.h>
#include <ert/util/hash.h>
#include <ert/util/bool_vector.h>
#include <ert/util/type_macros.h>

typedef struct meas_data_struct   meas_data_type;
typedef struct meas_block_struct  meas_block_type;

UTIL_IS_INSTANCE_HEADER( meas_data );
UTIL_SAFE_CAST_HEADER( meas_block );

bool               meas_block_iens_active( const meas_block_type * meas_block , int iens);
void               meas_block_iset( meas_block_type * meas_block , int iens , int iobs , double value);
double meas_block_iget( const meas_block_type * meas_block , int iens , int iobs);
double             meas_block_iget_ens_mean( meas_block_type * meas_block , int iobs );
double             meas_block_iget_ens_std( meas_block_type * meas_block , int iobs);
void               meas_block_deactivate( meas_block_type * meas_block , int iobs );
bool               meas_block_iget_active( const meas_block_type * meas_block , int iobs);
void meas_data_fprintf( const meas_data_type * matrix , FILE * stream);

void               meas_data_reset(meas_data_type * );
  meas_data_type *   meas_data_alloc( const bool_vector_type * ens_mask);
void               meas_data_free(meas_data_type * );
void               meas_data_add(meas_data_type * , int , double );
matrix_type      * meas_data_allocS(const meas_data_type * matrix);
int                meas_data_get_active_obs_size( const meas_data_type * matrix );
void               meas_data_deactivate(meas_data_type * meas_data, int index);
int                meas_data_get_active_ens_size( const meas_data_type * meas_data );
int                meas_data_get_nrobs( const meas_data_type * meas_data );
meas_block_type  * meas_data_add_block( meas_data_type * matrix , const char * obs_key , int report_step , int obs_size);
int                meas_data_get_num_blocks( const meas_data_type * meas_block );
meas_block_type  * meas_data_iget_block( const meas_data_type * matrix , int block_mnr);
const meas_block_type  * meas_data_iget_block_const( const meas_data_type * matrix , int block_nr );
int                meas_block_get_total_obs_size( const meas_block_type * meas_block );


#ifdef __cplusplus
}
#endif

#endif
