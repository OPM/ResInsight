/*
   Copyright (C) 2012  Equinor ASA, Norway.

   The file 'ecl_rst_file.h' is part of ERT - Ensemble based Reservoir Tool.

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


#ifndef ERT_ECL_RST_FILE_H
#define ERT_ECL_RST_FILE_H

#include <ert/ecl/ecl_rsthead.hpp>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct ecl_rst_file_struct ecl_rst_file_type;


  ecl_rst_file_type * ecl_rst_file_open_read( const char * filename );
  ecl_rst_file_type * ecl_rst_file_open_write( const char * filename );
  ecl_rst_file_type * ecl_rst_file_open_append( const char * filename );
  ecl_rst_file_type * ecl_rst_file_open_write_seek( const char * filename , int report_step);
  void                ecl_rst_file_close( ecl_rst_file_type * rst_file );

  void                ecl_rst_file_start_solution( ecl_rst_file_type * rst_file );
  void                ecl_rst_file_end_solution( ecl_rst_file_type * rst_file );
  void                ecl_rst_file_fwrite_header( ecl_rst_file_type * rst_file , int seqnum, ecl_rsthead_type * rsthead_data );
  void                ecl_rst_file_add_kw(ecl_rst_file_type * rst_file , const ecl_kw_type * ecl_kw );
  offset_type         ecl_rst_file_ftell(const ecl_rst_file_type * rst_file );

#ifdef __cplusplus
}
#endif

#endif
