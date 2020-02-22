/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'fortio.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_FORTIO_H
#define ERT_FORTIO_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.hpp>

typedef enum {
  FORTIO_NOENTRY         = 0,    /* File does not exists at all - application error. */
  FORTIO_EOF             = 1,    /* The file / record is empty */
  FORTIO_OK              = 2,    /* The file / record is OK with: [32 bit header | data | 32 bit footer] */
  FORTIO_MISSING_DATA    = 3,
  FORTIO_MISSING_TAIL    = 4,
  FORTIO_HEADER_MISMATCH = 5
} fortio_status_type;


typedef struct fortio_struct fortio_type;

  fortio_status_type fortio_check_buffer( FILE * stream , bool endian_flip , size_t buffer_size );
  fortio_status_type fortio_check_file( const char * filename , bool endian_flip);
  bool               fortio_looks_like_fortran_file(const char *  , bool );
  void               fortio_copy_record(fortio_type * , fortio_type * , int , void * , bool *);
  fortio_type *      fortio_open_reader(const char *, bool fmt_file , bool endian_flip_header);
  fortio_type *      fortio_open_writer(const char *, bool fmt_file , bool endian_flip_header);
  fortio_type *      fortio_open_readwrite(const char *, bool fmt_file , bool endian_flip_header);
  fortio_type *      fortio_open_append(const char *filename , bool fmt_file , bool endian_flip_header);
  fortio_type *      fortio_alloc_FILE_wrapper(const char * , bool , bool , bool , FILE * );
  void               fortio_free_FILE_wrapper(fortio_type *);
  void               fortio_fclose(fortio_type *);
  int                fortio_init_read(fortio_type *);
  bool               fortio_complete_read(fortio_type *, int record_size);
  void               fortio_init_write(fortio_type * , int);
  void               fortio_complete_write(fortio_type * , int record_size);
  void               fortio_fskip_buffer(fortio_type *, int );
  int                fortio_fskip_record(fortio_type *);
  bool               fortio_fread_buffer(fortio_type * , char * buffer, int buffer_size);
  void               fortio_fwrite_record(fortio_type * , const char * buffer, int buffer_size);
  FILE        *      fortio_get_FILE(const fortio_type *);
  void               fortio_fflush(fortio_type * ) ;
  bool               fortio_ftruncate_current( fortio_type * fortio);
  bool               fortio_is_fortio_file(fortio_type * );
  void               fortio_rewind(const fortio_type *fortio);
  const char  *      fortio_filename_ref(const fortio_type * );
  bool               fortio_fmt_file(const fortio_type *);
  offset_type        fortio_ftell( const fortio_type * fortio );
  bool               fortio_fseek( fortio_type * fortio , offset_type offset , int whence);
  bool               fortio_data_fskip(fortio_type* fortio, const int element_size, const int element_count, const int block_count);
  void               fortio_data_fseek(fortio_type* fortio, offset_type data_offset, size_t data_element, const int element_size, const int element_count, const int block_size);
  int                fortio_fileno( fortio_type * fortio );
  bool               fortio_ftruncate( fortio_type * fortio , offset_type size);
  int                fortio_fclean(fortio_type * fortio);

  bool               fortio_fclose_stream( fortio_type * fortio );
  bool               fortio_fopen_stream( fortio_type * fortio );
  bool               fortio_stream_is_open( const fortio_type * fortio );
  bool               fortio_assert_stream_open( fortio_type * fortio );
  bool               fortio_read_at_eof( fortio_type * fortio );
  void               fortio_fwrite_error(fortio_type * fortio);

UTIL_IS_INSTANCE_HEADER( fortio );
UTIL_SAFE_CAST_HEADER( fortio );

#ifdef __cplusplus
}
#endif
#endif
