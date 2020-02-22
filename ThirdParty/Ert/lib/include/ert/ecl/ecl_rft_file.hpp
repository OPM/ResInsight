/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'ecl_rft_file.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_ECL_RFT_FILE_H
#define ERT_ECL_RFT_FILE_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdbool.h>

#include <ert/util/stringlist.hpp>

#include <ert/ecl/ecl_rft_node.hpp>
#include <ert/util/vector.hpp>
#include <ert/util/int_vector.hpp>

typedef struct ecl_rft_file_struct ecl_rft_file_type;

char                 * ecl_rft_file_alloc_case_filename(const char * case_input );
const char           * ecl_rft_file_get_filename( const ecl_rft_file_type * rft_file );
ecl_rft_file_type    * ecl_rft_file_alloc_case(const char * case_input );
bool                   ecl_rft_file_case_has_rft( const char * case_input );
ecl_rft_file_type    * ecl_rft_file_alloc(const char * );
void                   ecl_rft_file_free(ecl_rft_file_type * );
void                   ecl_rft_file_block(const ecl_rft_file_type *  , double , const char * , int , const double * , int * , int * , int *);
void                   ecl_rft_file_fprintf_rft_obs(const ecl_rft_file_type  * , double , const char * , const char *, const char * , double);
ecl_rft_node_type    * ecl_rft_file_get_node(const ecl_rft_file_type * , const char * );


int                       ecl_rft_file_get_size__( const ecl_rft_file_type * rft_file, const char * well_pattern , time_t recording_time);
int                       ecl_rft_file_get_size( const ecl_rft_file_type * rft_file);
ecl_rft_node_type       * ecl_rft_file_get_well_time_rft( const ecl_rft_file_type * rft_file , const char * well , time_t recording_time);
ecl_rft_node_type       * ecl_rft_file_iget_node( const ecl_rft_file_type * rft_file , int index);
ecl_rft_node_type       * ecl_rft_file_iget_well_rft( const ecl_rft_file_type * rft_file , const char * well, int index);
bool                      ecl_rft_file_has_well( const ecl_rft_file_type * rft_file , const char * well);
int                       ecl_rft_file_get_well_occurences( const ecl_rft_file_type * rft_file , const char * well);
stringlist_type         * ecl_rft_file_alloc_well_list(const ecl_rft_file_type * rft_file );
int                       ecl_rft_file_get_num_wells( const ecl_rft_file_type * rft_file );
void                      ecl_rft_file_free__( void * arg);
void                      ecl_rft_file_update(const char * rft_file_name,  ecl_rft_node_type ** nodes,int num_nodes, ert_ecl_unit_enum unit_set);

#ifdef __cplusplus
}
#endif
#endif
