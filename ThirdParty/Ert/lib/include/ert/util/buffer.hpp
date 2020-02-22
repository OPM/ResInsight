/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'buffer.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_BUFFER_H
#define ERT_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <ert/util/ert_api_config.hpp>
#include <ert/util/type_macros.hpp>
#include <ert/util/ssize_t.hpp>



  typedef struct     buffer_struct buffer_type;

  buffer_type      * buffer_alloc( size_t buffer_size );
  buffer_type      * buffer_alloc_private_wrapper(void * data , size_t buffer_size );
  bool               buffer_search_replace( buffer_type * buffer , const char * old_string , const char * new_string);
  void               buffer_shrink_to_fit( buffer_type * buffer );
  void               buffer_memshift(buffer_type * buffer , size_t offset, ssize_t shift);
  bool               buffer_strstr( buffer_type * buffer , const char * expr );
  bool               buffer_strchr( buffer_type * buffer , int c);
  void               buffer_replace_string( buffer_type * buffer , size_t offset , size_t old_size , const char * new_string);
  void               buffer_replace_data(buffer_type * buffer , size_t offset , size_t old_size , const void * new_data , size_t new_size);

  void               buffer_free_container( buffer_type * buffer );
  void               buffer_free( buffer_type * buffer);
  size_t             buffer_fread(buffer_type * buffer , void * target_ptr , size_t item_size , size_t items);
  size_t             buffer_fwrite(buffer_type * buffer , const void * src_ptr , size_t item_size , size_t items);
  void               buffer_summarize(const buffer_type * buffer , const char *);

  void               buffer_fwrite_char_ptr(buffer_type * buffer , const char * string_ptr );
  void               buffer_strcat(buffer_type * buffer , const char * string);
  void               buffer_fwrite_char(buffer_type * buffer , char value);
  void               buffer_fwrite_int(buffer_type * buffer , int value);
  void               buffer_fskip_bool(buffer_type * buffer);
  void               buffer_fwrite_bool(buffer_type * buffer , bool value);
  int                buffer_fread_int(buffer_type * buffer );
  bool               buffer_fread_bool(buffer_type * buffer);
  long int           buffer_fread_long(buffer_type * buffer);
  void               buffer_fskip_long(buffer_type * buffer);
  void               buffer_store(const buffer_type * buffer , const char * filename);
  size_t             buffer_get_offset(const buffer_type * buffer);
  size_t             buffer_get_alloc_size(const buffer_type * buffer);
  size_t             buffer_get_size(const buffer_type * buffer);
  size_t             buffer_get_string_size( const buffer_type * buffer );
  size_t             buffer_get_remaining_size(const buffer_type *  buffer);
  void             * buffer_get_data(const buffer_type * buffer);
  void             * buffer_alloc_data_copy(const buffer_type * buffer);
  void             * buffer_iget_data(const buffer_type * buffer, size_t offset);
  void               buffer_stream_fwrite( const buffer_type * buffer , FILE * stream );
  int                buffer_fgetc( buffer_type * buffer );
  void               buffer_fseek(buffer_type * buffer , ssize_t offset , int whence);
  void               buffer_fskip(buffer_type * buffer, ssize_t offset);
  void               buffer_clear( buffer_type * buffer );

  void               buffer_fskip_int(buffer_type * buffer);
  void               buffer_fskip_time_t(buffer_type * buffer);
  time_t             buffer_fread_time_t(buffer_type * buffer);
  void               buffer_fwrite_time_t(buffer_type * buffer , time_t value);
  void               buffer_rewind(buffer_type * buffer );

  double             buffer_fread_double(buffer_type * buffer);
  void               buffer_fwrite_double(buffer_type * buffer , double value);

  size_t             buffer_stream_fwrite_n( const buffer_type * buffer , size_t offset , ssize_t write_size , FILE * stream );
  void               buffer_stream_fprintf( const buffer_type * buffer , FILE * stream );
  void               buffer_stream_fread( buffer_type * buffer , size_t byte_size , FILE * stream);
  buffer_type      * buffer_fread_alloc(const char * filename);
  void               buffer_fread_realloc(buffer_type * buffer , const char * filename);
  void               buffer_fprintf(const buffer_type * buffer, const char * fmt, FILE * stream);

#ifdef ERT_HAVE_ZLIB
  size_t             buffer_fwrite_compressed(buffer_type * buffer, const void * ptr , size_t byte_size);
  size_t             buffer_fread_compressed(buffer_type * buffer , size_t compressed_size , void * target_ptr , size_t target_size);
#endif


#include "buffer_string.h"

  UTIL_IS_INSTANCE_HEADER( buffer );
  UTIL_SAFE_CAST_HEADER( buffer );

#ifdef __cplusplus
}
#endif

#endif
