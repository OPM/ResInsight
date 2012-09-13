/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_rft_node.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ECL_RFT_NODE_H__
#define __ECL_RFT_NODE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ecl_file.h>

typedef enum { RFT     = 1 , 
               PLT     = 2 , 
               SEGMENT = 3 /* Not really implemented */ } ecl_rft_enum;

typedef struct ecl_rft_node_struct ecl_rft_node_type;

int                 ecl_rft_node_lookup_ijk( const ecl_rft_node_type * rft_node , int i, int j , int k);
void                ecl_rft_node_fprintf_rft_obs(const ecl_rft_node_type * , double , const char * , const char * , double );
ecl_rft_node_type * ecl_rft_node_alloc(const ecl_file_type * file_map );
const char        * ecl_rft_node_get_well_name(const ecl_rft_node_type * );
void                ecl_rft_node_free(ecl_rft_node_type * );
void                ecl_rft_node_free__(void * );
void                ecl_rft_node_block2(const ecl_rft_node_type * , int  , const double * , const double * , int * , int * , int *);
void                ecl_rft_node_block(const ecl_rft_node_type *  , double , int , const double * , int * , int *  , int *);
time_t              ecl_rft_node_get_date(const ecl_rft_node_type * );
int                 ecl_rft_node_get_size(const ecl_rft_node_type * );
void                ecl_rft_node_summarize(const ecl_rft_node_type * , bool );
const char        * ecl_rft_node_get_well_name( const ecl_rft_node_type * rft_node );
void                ecl_rft_node_iget_ijk( const ecl_rft_node_type * rft_node , int index , int *i , int *j , int *k);

void ecl_rft_node_export_DEPTH(const ecl_rft_node_type * , const char * );
double ecl_rft_node_iget_pressure( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_depth( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_wrat( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_grat( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_orat( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_swat( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_sgas( const ecl_rft_node_type * rft_node , int index);

#ifdef __cplusplus
}
#endif
#endif

