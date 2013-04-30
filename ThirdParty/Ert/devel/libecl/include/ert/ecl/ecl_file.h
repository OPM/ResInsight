/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'ecl_file.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef __ECL_FILE_H__
#define __ECL_FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <time.h>


#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file_kw.h>
#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_util.h>

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


#define ECL_FILE_FLAGS_ENUM_DEFS \
  {.value =   1 , .name="ECL_FILE_CLOSE_STREAM"}, \
  {.value =   2 , .name="ECL_FILE_WRITABLE"}
#define ECL_FILE_FLAGS_ENUM_SIZE 2




  typedef struct ecl_file_struct ecl_file_type;
  bool             ecl_file_load_all( ecl_file_type * ecl_file );
  void             ecl_file_push_block( ecl_file_type * ecl_file );
  void             ecl_file_pop_block( ecl_file_type * ecl_file );
  ecl_file_type  * ecl_file_open( const char * filename , int flags);
  ecl_file_type  * ecl_file_try_open( const char * filename , int flags);
  void             ecl_file_close( ecl_file_type * ecl_file );
  void             ecl_file_fortio_detach( ecl_file_type * ecl_file );
  void             ecl_file_free__(void * arg);
  ecl_kw_type    * ecl_file_icopy_named_kw( const ecl_file_type * ecl_file , const char * kw, int ith);
  ecl_kw_type    * ecl_file_icopy_kw( const ecl_file_type * ecl_file , int index);
  bool             ecl_file_has_kw( const ecl_file_type * ecl_file , const char * kw);
  int              ecl_file_get_num_named_kw(const ecl_file_type * ecl_file , const char * kw);
  int              ecl_file_get_size( const ecl_file_type * ecl_file );
  int              ecl_file_get_num_distinct_kw(const ecl_file_type * ecl_file);
  const char     * ecl_file_iget_distinct_kw(const ecl_file_type * ecl_file , int index);
  const char     * ecl_file_get_src_file( const ecl_file_type * ecl_file );
  int              ecl_file_iget_occurence( const ecl_file_type *  ecl_file , int index);
  ecl_version_enum ecl_file_get_ecl_version( const ecl_file_type * file );
  void             ecl_file_fwrite_fortio(const ecl_file_type * ec_file  , fortio_type * fortio , int offset);
  void             ecl_file_fwrite(const ecl_file_type * ecl_file , const char * , bool fmt_file );
  
  void             ecl_file_replace_kw( ecl_file_type * ecl_file , ecl_kw_type * old_kw , ecl_kw_type * new_kw , bool insert_copy);
  int              ecl_file_get_phases( const ecl_file_type * init_file );
  void             ecl_file_fprintf_kw_list( const ecl_file_type * ecl_file , FILE * stream );
  
  bool             ecl_file_writable( const ecl_file_type * ecl_file );
  int              ecl_file_get_flags( const ecl_file_type * ecl_file );
  bool             ecl_file_flags_set( const ecl_file_type * ecl_file , int flags);

  
  
  ecl_file_kw_type * ecl_file_iget_file_kw( const ecl_file_type * file , int global_index);
  ecl_file_kw_type * ecl_file_iget_named_file_kw( const ecl_file_type * file , const char * kw, int ith);
  ecl_kw_type      * ecl_file_iget_kw( const ecl_file_type * file , int global_index);
  ecl_type_enum      ecl_file_iget_type( const ecl_file_type * file , int global_index);
  int                ecl_file_iget_size( const ecl_file_type * file , int global_index);
  const char       * ecl_file_iget_header( const ecl_file_type * file , int global_index);
  ecl_kw_type      * ecl_file_iget_named_kw( const ecl_file_type * file , const char * kw, int ith);
  ecl_type_enum      ecl_file_iget_named_type( const ecl_file_type * file , const char * kw , int ith);
  int                ecl_file_iget_named_size( const ecl_file_type * file , const char * kw , int ith);
  
  
  bool               ecl_file_subselect_block( ecl_file_type * ecl_file , const char * kw , int occurence);
  bool               ecl_file_select_block( ecl_file_type * ecl_file , const char * kw , int occurence);
  void               ecl_file_select_global( ecl_file_type * ecl_file );
  //bool               ecl_file_writable( const ecl_file_type * ecl_file );
  bool               ecl_file_save_kw( const ecl_file_type * ecl_file , const ecl_kw_type * ecl_kw);
  bool               ecl_file_has_kw_ptr( const ecl_file_type * ecl_file , const ecl_kw_type * ecl_kw);

/*****************************************************************/
/*               R E S T A R T  F I L E S                        */

  time_t           ecl_file_iget_restart_sim_date( const ecl_file_type * restart_file , int occurence );
  int              ecl_file_get_restart_index( const ecl_file_type * restart_file , time_t sim_time);
  bool             ecl_file_has_report_step( const ecl_file_type * ecl_file , int report_step);
  bool             ecl_file_has_sim_time( const ecl_file_type * ecl_file , time_t sim_time);


  bool             ecl_file_select_rstblock_sim_time( ecl_file_type * ecl_file , time_t sim_time);
  bool             ecl_file_select_rstblock_report_step( ecl_file_type * ecl_file , int report_step);
  bool             ecl_file_iselect_rstblock( ecl_file_type * ecl_file , int seqnum_index );
  
  ecl_file_type  * ecl_file_open_rstblock_report_step( const char * filename , int report_step , int flags);
  ecl_file_type  * ecl_file_open_rstblock_sim_time( const char * filename , time_t sim_time , int flags); 
  ecl_file_type  * ecl_file_iopen_rstblock( const char * filename , int seqnum_index , int flags);

  
/*****************************************************************/
/* SUMMARY FILES */
  
  bool             ecl_file_select_smryblock( ecl_file_type * ecl_file , int ministep_nr );
  ecl_file_type  * ecl_file_open_smryblock( const char * filename , int ministep_nr , int flags);


  UTIL_IS_INSTANCE_HEADER( ecl_file );

#ifdef __cplusplus
}
#endif 
#endif
