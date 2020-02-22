/*
   Copyright (C) 2011  Equinor ASA, Norway.

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

#ifndef ERT_ECL_RFT_NODE_H
#define ERT_ECL_RFT_NODE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/ecl/ecl_file_view.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_rft_cell.hpp>

typedef enum { RFT     = 1 ,
               PLT     = 2 ,
               SEGMENT = 3 /* Not really implemented */ } ecl_rft_enum;

typedef struct ecl_rft_node_struct ecl_rft_node_type;

void                      ecl_rft_node_inplace_sort_cells( ecl_rft_node_type * rft_node );
const ecl_rft_cell_type * ecl_rft_node_iget_cell_sorted( ecl_rft_node_type * rft_node , int index);
const ecl_rft_cell_type * ecl_rft_node_iget_cell( const ecl_rft_node_type * rft_node , int index);
const ecl_rft_cell_type * ecl_rft_node_lookup_ijk( const ecl_rft_node_type * rft_node , int i, int j , int k);
void                ecl_rft_node_fprintf_rft_obs(const ecl_rft_node_type * , double , const char * , const char * , double );
ecl_rft_node_type * ecl_rft_node_alloc(const ecl_file_view_type * rft_view );
void                ecl_rft_node_free(ecl_rft_node_type * );
void                ecl_rft_node_free__(void * );
time_t              ecl_rft_node_get_date(const ecl_rft_node_type * );
int                 ecl_rft_node_get_size(const ecl_rft_node_type * );
const char        * ecl_rft_node_get_well_name( const ecl_rft_node_type * rft_node );
void                ecl_rft_node_iget_ijk( const ecl_rft_node_type * rft_node , int index , int *i , int *j , int *k);

bool                ecl_rft_node_is_RFT( const ecl_rft_node_type * rft_node );
bool                ecl_rft_node_is_PLT( const ecl_rft_node_type * rft_node );
bool                ecl_rft_node_is_SEGMENT( const ecl_rft_node_type * rft_node );
bool                ecl_rft_node_is_MSW( const ecl_rft_node_type * rft_node );

double ecl_rft_node_iget_pressure( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_depth( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_wrat( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_grat( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_orat( const ecl_rft_node_type * rft_node , int index);

double ecl_rft_node_iget_swat( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_sgas( const ecl_rft_node_type * rft_node , int index);
double ecl_rft_node_iget_soil( const ecl_rft_node_type * rft_node , int index);
void   ecl_rft_node_fwrite(const ecl_rft_node_type * rft_node, fortio_type * fortio, ert_ecl_unit_enum unit_set);
double ecl_rft_node_get_days(const ecl_rft_node_type * rft_node );
int  ecl_rft_node_cmp( const ecl_rft_node_type * n1 , const ecl_rft_node_type * n2);
bool ecl_rft_node_lt(const ecl_rft_node_type * n1, const ecl_rft_node_type * n2);

void ecl_rft_node_append_cell( ecl_rft_node_type * rft_node , ecl_rft_cell_type * cell);
ecl_rft_node_type * ecl_rft_node_alloc_new(const char * well_name, const char * data_type_string, const time_t recording_date, const double days);

ecl_rft_enum ecl_rft_node_get_type(const ecl_rft_node_type * rft_node);

#ifdef __cplusplus
}
#endif
#endif

