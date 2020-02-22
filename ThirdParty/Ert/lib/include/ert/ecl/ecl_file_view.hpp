/*
   Copyright (C) 2016  Equinor ASA, Norway.

   This file is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_ECL_FILE_VIEW_H
#define ERT_ECL_FILE_VIEW_H

#include <stdlib.h>
#include <stdbool.h>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_file_kw.hpp>
#include <ert/ecl/ecl_type.hpp>


#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ECL_FILE_CLOSE_STREAM  =  1 ,  /*
                                    This flag will close the underlying FILE object between each access; this is
                                    mainly to save filedescriptors in cases where many ecl_file instances are open at
                                    the same time. */
  //
  ECL_FILE_WRITABLE      =  2    /*
                                    This flag opens the file in a mode where it can be updated and modified, but it
                                    must still exist and be readable. I.e. this should not compared with the normal:
                                    fopen(filename , "w") where an existing file is truncated to zero upon successfull
                                    open.
                                 */
} ecl_file_flag_type;


typedef struct ecl_file_view_struct ecl_file_view_type;
typedef struct ecl_file_transaction_struct ecl_file_transaction_type;


  bool ecl_file_view_flags_set( const ecl_file_view_type * file_view, int query_flags);
  bool ecl_file_view_check_flags( int state_flags , int query_flags);

  ecl_file_view_type      * ecl_file_view_alloc( fortio_type * fortio , int * flags , inv_map_type * inv_map , bool owner );
  int                       ecl_file_view_get_global_index( const ecl_file_view_type * ecl_file_view , const char * kw , int ith);
  void                      ecl_file_view_make_index( ecl_file_view_type * ecl_file_view );
  bool                      ecl_file_view_has_kw( const ecl_file_view_type * ecl_file_view, const char * kw);
  ecl_file_kw_type        * ecl_file_view_iget_file_kw( const ecl_file_view_type * ecl_file_view , int global_index);
  ecl_file_kw_type        * ecl_file_view_iget_named_file_kw( const ecl_file_view_type * ecl_file_view , const char * kw, int ith);
  ecl_kw_type             * ecl_file_view_iget_kw( const ecl_file_view_type * ecl_file_view , int index);
  void                      ecl_file_view_index_fload_kw(const ecl_file_view_type * ecl_file_view, const char* kw, int index, const int_vector_type * index_map, char* buffer);
  int                       ecl_file_view_find_kw_value( const ecl_file_view_type * ecl_file_view , const char * kw , const void * value);
  const char              * ecl_file_view_iget_distinct_kw( const ecl_file_view_type * ecl_file_view , int index);
  int                       ecl_file_view_get_num_distinct_kw( const ecl_file_view_type * ecl_file_view );
  int                       ecl_file_view_get_size( const ecl_file_view_type * ecl_file_view );
  ecl_data_type             ecl_file_view_iget_data_type( const ecl_file_view_type * ecl_file_view , int index);
  int                       ecl_file_view_iget_size( const ecl_file_view_type * ecl_file_view , int index);
  const char              * ecl_file_view_iget_header( const ecl_file_view_type * ecl_file_view , int index);
  ecl_kw_type             * ecl_file_view_iget_named_kw( const ecl_file_view_type * ecl_file_view , const char * kw, int ith);
  ecl_data_type             ecl_file_view_iget_named_data_type( const ecl_file_view_type * ecl_file_view , const char * kw , int ith);
  int                       ecl_file_view_iget_named_size( const ecl_file_view_type * ecl_file_view , const char * kw , int ith);
  void      ecl_file_view_replace_kw( ecl_file_view_type * ecl_file_view , ecl_kw_type * old_kw , ecl_kw_type * new_kw , bool insert_copy);
  bool      ecl_file_view_load_all( ecl_file_view_type * ecl_file_view );
  void      ecl_file_view_add_kw( ecl_file_view_type * ecl_file_view , ecl_file_kw_type * file_kw);
  void      ecl_file_view_free( ecl_file_view_type * ecl_file_view );
  void      ecl_file_view_free__( void * arg );
  int       ecl_file_view_get_num_named_kw(const ecl_file_view_type * ecl_file_view , const char * kw);
  void      ecl_file_view_fwrite( const ecl_file_view_type * ecl_file_view , fortio_type * target , int offset);
  int       ecl_file_view_iget_occurence( const ecl_file_view_type * ecl_file_view , int global_index);
  void      ecl_file_view_fprintf_kw_list(const ecl_file_view_type * ecl_file_view , FILE * stream);
  ecl_file_view_type * ecl_file_view_add_blockview(ecl_file_view_type * ecl_file_view , const char * header, int occurence);
  ecl_file_view_type * ecl_file_view_add_blockview2(ecl_file_view_type * ecl_file_view , const char * start_kw, const char * end_kw, int occurence);
  ecl_file_view_type * ecl_file_view_add_restart_view(ecl_file_view_type * file_view , int seqnum_index, int report_step , time_t sim_time, double sim_days);
  ecl_file_view_type * ecl_file_view_alloc_blockview(const ecl_file_view_type * ecl_file_view , const char * header, int occurence);
  ecl_file_view_type * ecl_file_view_alloc_blockview2(const ecl_file_view_type * ecl_file_view , const char * start_kw, const char * end_kw, int occurence);

  void ecl_file_view_add_child( ecl_file_view_type * parent , ecl_file_view_type * child);
  bool ecl_file_view_drop_flag( ecl_file_view_type * file_view , int flag);
  void ecl_file_view_add_flag( ecl_file_view_type * file_view , int flag);

  int    ecl_file_view_seqnum_index_from_sim_time( ecl_file_view_type * parent_map , time_t sim_time);
  bool   ecl_file_view_has_sim_time( const ecl_file_view_type * ecl_file_view , time_t sim_time);
  int    ecl_file_view_find_sim_time(const ecl_file_view_type * ecl_file_view , time_t sim_time);
  double ecl_file_view_iget_restart_sim_days(const ecl_file_view_type * ecl_file_view , int seqnum_index);
  time_t ecl_file_view_iget_restart_sim_date(const ecl_file_view_type * ecl_file_view , int seqnum_index);
  bool   ecl_file_view_has_report_step( const ecl_file_view_type * ecl_file_view , int report_step);

  ecl_file_view_type * ecl_file_view_add_summary_view( ecl_file_view_type * file_view , int report_step );
  const char *         ecl_file_view_get_src_file( const ecl_file_view_type * file_view );
  void                 ecl_file_view_fclose_stream( ecl_file_view_type * file_view );

  void                 ecl_file_view_write_index(const ecl_file_view_type * file_view, FILE * ostream);
  ecl_file_view_type * ecl_file_view_fread_alloc( fortio_type * fortio , int * flags , inv_map_type * inv_map, FILE * istream );

  ecl_file_transaction_type * ecl_file_view_start_transaction(ecl_file_view_type * file_view);
  void                        ecl_file_view_end_transaction( ecl_file_view_type * file_view, ecl_file_transaction_type * transaction);

#ifdef __cplusplus
}
#endif

#endif
