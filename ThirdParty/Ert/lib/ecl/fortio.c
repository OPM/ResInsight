/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'fortio.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/ecl/fortio.h>


#define FORTIO_ID  345116

extern int errno;

/**
The fortio struct is implemented to handle fortran io. The problem is
that when a Fortran program writes unformatted data to file in a
statemente like:

   integer array(100)
   write(unit) array

it actually writes a head and tail in addition to the actual
data. The header and tail is a 4 byte integer, which value is the
number of bytes in the immediately following record. I.e. what is
actually found on disk after the Fortran code above is:

  | 400 | array ...... | 400 |

Where the "400" head and tail is the number of bytes in the following
record. Fortran IO handles this transparently, but when mixing with
other programming languages care must be taken. This file implements
functionality to read and write these fortran generated files
transparently. The three functions:

  1. fortio_fopen()
  2. fortio_fread_record()
  3. fortio_fwrite_record()

together constitute something very similar to fopen() , fread() and
fwrite() from the standard library.
*/


#define READ_MODE_TXT          "r"
#define READ_MODE_BINARY       "rb"
#define WRITE_MODE_TXT         "w"
#define WRITE_MODE_BINARY      "wb"
#define READ_WRITE_MODE_TXT    "r+"
#define READ_WRITE_MODE_BINARY "r+b"
#define APPEND_MODE_TXT        "a"
#define APPEND_MODE_BINARY     "ab"


struct fortio_struct {
  UTIL_TYPE_ID_DECLARATION;
  FILE             * stream;
  char             * filename;
  bool               endian_flip_header;
  bool               fmt_file;    /* This is not really used by the fortio instance - but it is very convenient to store it here. */
  const char *       fopen_mode;
  bool               stream_owner;

  /*
    The internal variable read_size is used in the functions
    fortio_fseek() and fortio_read_at_eof() - if-and-only-if - the
    file is opened in read only mode.

    Observe that the semantics of the fortio_fseek() function depends
    on whether the file is writable.
  */
  bool               writable;
  offset_type        read_size;
};


UTIL_IS_INSTANCE_FUNCTION( fortio , FORTIO_ID );
UTIL_SAFE_CAST_FUNCTION( fortio, FORTIO_ID );

static fortio_type * fortio_alloc__(const char *filename , bool fmt_file , bool endian_flip_header , bool stream_owner , bool writable) {
  fortio_type * fortio       = util_malloc(sizeof * fortio );
  UTIL_TYPE_ID_INIT( fortio, FORTIO_ID );
  fortio->filename           = util_alloc_string_copy(filename);
  fortio->endian_flip_header = endian_flip_header;
  fortio->fmt_file           = fmt_file;
  fortio->stream_owner       = stream_owner;
  fortio->writable           = writable;
  fortio->read_size = 0;

  return fortio;
}



/**
   Helper function for fortio_is_fortran_stream__().
*/

static bool __read_int(FILE * stream , int * value, bool endian_flip) {
  /* This fread() can legitemately fail - can not use util_fread() here. */
  if (fread(value , sizeof * value , 1 , stream) == 1) {
    if (endian_flip)
      util_endian_flip_vector(value , sizeof * value , 1);
    return true;
  } else
    return false;
}


/**
   Helper function for fortio_is_fortran_file(). Checks whether a
   particular stream is formatted according to fortran io, for a fixed
   endian ness.
*/

static bool fortio_is_fortran_stream__(FILE * stream , bool endian_flip) {
  const bool strict_checking = true;          /* True: requires that *ALL* records in the file are fortran formatted */
  offset_type init_pos              = util_ftell(stream);
  bool is_fortran_stream     = false;
  int header , tail;
  bool cont;

  do {
    cont = false;
    if (__read_int(stream , &header , endian_flip)) {
      if (header >= 0) {
        if (util_fseek(stream , (offset_type) header , SEEK_CUR) == 0) {
          if (__read_int(stream , &tail , endian_flip)) {
            cont = true;
            /*
               OK - now we have read a header and a tail - it might be
               a fortran file.
            */
            if (header == tail) {
              if (header != 0) {
                /* This is (most probably) a fortran file */
                is_fortran_stream = true;
                if (strict_checking)
                  cont = true;
                else
                  cont = false;
              }
              /* Header == tail == 0 - we don't make any inference on this. */
            } else {
              /* Header != tail => this is *not* a fortran file */
              cont = false;
              is_fortran_stream = false;
            }
          }
        }
      }
    }
  } while (cont);
  util_fseek(stream , init_pos , SEEK_SET);
  return is_fortran_stream;
}


/**
   This function tries (using some heuristic) to guess whether a
   particular file is a Fortran file.

   The heuristic algorithm which is used is as follows:

    1. Read four bytes as an integer (header)
    2. Skip that number of bytes forward.
    3. Read four bytes again (tail).

   Now, when this is done we do the following test:

   If header == tail. This is (probably) a fortran file, however if
   header == 0, we might have a normal file with two consequitive
   zeroes. In that case it is difficult to determine, and we continue.

*/

bool fortio_looks_like_fortran_file(const char * filename, bool endian_flip) {
  FILE * stream = util_fopen(filename , "rb");
  bool is_fortran_stream = fortio_is_fortran_stream__(stream , endian_flip);
  fclose(stream);
  return is_fortran_stream;
}


static void fortio_init_size(fortio_type * fortio) {
  fortio->read_size = util_fd_size( fortio_fileno( fortio ));
}




fortio_type * fortio_alloc_FILE_wrapper(const char *filename , bool endian_flip_header , bool fmt_file , bool writable , FILE * stream) {
  fortio_type * fortio = fortio_alloc__(filename , fmt_file , endian_flip_header , false , writable);
  fortio->stream = stream;
  return fortio;
}

/*****************************************************************/
/*
  Observe that the stream open functions accept a failure, and call
  the fopen() function driectly.
*/

static const char * fortio_fopen_read_mode( bool fmt_file ) {
  if (fmt_file)
    return READ_MODE_TXT;
  else
    return READ_MODE_BINARY;
}

static FILE * fortio_fopen_read( const char * filename , bool fmt_file ) {
  FILE * stream;
  const char * mode = fortio_fopen_read_mode( fmt_file );
  stream = fopen(filename , mode);
  return stream;
}


static const char * fortio_fopen_write_mode( bool fmt_file ) {
  if (fmt_file)
    return WRITE_MODE_TXT;
  else
    return WRITE_MODE_BINARY;
}

static FILE * fortio_fopen_write( const char * filename , bool fmt_file ) {
  FILE * stream;
  const char * mode = fortio_fopen_write_mode( fmt_file );
  stream = fopen(filename , mode);
  return stream;
}




static const char * fortio_fopen_readwrite_mode( bool fmt_file ) {
  if (fmt_file)
    return READ_WRITE_MODE_TXT;
  else
    return READ_WRITE_MODE_BINARY;
}

static FILE * fortio_fopen_readwrite( const char * filename , bool fmt_file ) {
  FILE * stream;
  const char * mode = fortio_fopen_readwrite_mode( fmt_file );
  stream = fopen(filename , mode);
  return stream;
}


static const char * fortio_fopen_append_mode( bool fmt_file ) {
  if (fmt_file)
    return APPEND_MODE_TXT;
  else
    return APPEND_MODE_BINARY;
}


static FILE * fortio_fopen_append( const char * filename , bool fmt_file ) {
  FILE * stream;
  const char * mode = fortio_fopen_append_mode( fmt_file );
  stream = fopen(filename , mode);
  return stream;
}



/*****************************************************************/



fortio_type * fortio_open_reader(const char *filename , bool fmt_file , bool endian_flip_header) {
  FILE * stream = fortio_fopen_read( filename , fmt_file );
  if (stream) {
    fortio_type *fortio = fortio_alloc__(filename , fmt_file , endian_flip_header , true , false);
    fortio->stream = stream;
    fortio->fopen_mode = fortio_fopen_read_mode( fmt_file );
    fortio_init_size( fortio );
    return fortio;
  } else
    return NULL;
}




fortio_type * fortio_open_writer(const char *filename , bool fmt_file , bool endian_flip_header ) {
  FILE * stream = fortio_fopen_write( filename , fmt_file );
  if (stream) {
    fortio_type *fortio = fortio_alloc__(filename , fmt_file , endian_flip_header, true , true);
    fortio->stream = stream;
    fortio->fopen_mode = fortio_fopen_write_mode( fmt_file );
    fortio_init_size( fortio );
    return fortio;
  } else
    return NULL;
}



fortio_type * fortio_open_readwrite(const char *filename , bool fmt_file , bool endian_flip_header) {
  FILE * stream = fortio_fopen_readwrite( filename , fmt_file );
  if (stream) {
    fortio_type *fortio = fortio_alloc__(filename , fmt_file , endian_flip_header , true , true);
    fortio->stream = stream;
    fortio->fopen_mode = fortio_fopen_readwrite_mode( fmt_file );
    fortio_init_size( fortio );
    return fortio;
  } else
    return NULL;
}


fortio_type * fortio_open_append(const char *filename , bool fmt_file , bool endian_flip_header) {
  FILE * stream = fortio_fopen_append( filename , fmt_file );
  if (stream) {
    fortio_type *fortio = fortio_alloc__(filename , fmt_file , endian_flip_header , true , true);

    fortio->stream = stream;
    fortio->fopen_mode = fortio_fopen_append_mode( fmt_file );
    fortio_init_size( fortio );

    return fortio;
  } else
    return NULL;
}

/*****************************************************************/

bool fortio_fclose_stream( fortio_type * fortio ) {
  if (fortio->stream_owner) {
    if (fortio->stream) {
      int fclose_return = fclose( fortio->stream );
      fortio->stream = NULL;
      if (fclose_return == 0)
        return true;
      else
        return false;
    } else
      return false; // Already closed.
  } else
    return false;
}


bool fortio_fopen_stream( fortio_type * fortio ) {
  if (fortio->stream == NULL) {
    fortio->stream = fopen( fortio->filename , fortio->fopen_mode );
    if (fortio->stream)
      return true;
    else
      return false;
  } else
    return false;
}


bool fortio_stream_is_open( const fortio_type * fortio ) {
  if (fortio->stream)
    return true;
  else
    return false;
}


bool fortio_assert_stream_open( fortio_type * fortio ) {
  if (fortio->stream)
    return true;
  else {
    fortio_fopen_stream( fortio );
    return fortio_stream_is_open( fortio );
  }
}


/*****************************************************************/


static void fortio_free__(fortio_type * fortio) {
  util_safe_free(fortio->filename);
  free(fortio);
}

void fortio_free_FILE_wrapper(fortio_type * fortio) {
  fortio_free__( fortio );
}


void fortio_fclose(fortio_type *fortio) {
  if (fortio->stream) {
    fclose(fortio->stream);
    fortio->stream = NULL;
  }

  fortio_free__(fortio);
}


bool fortio_is_fortio_file(fortio_type * fortio) {
  offset_type init_pos = fortio_ftell(fortio);
  int elm_read;
  bool is_fortio_file = false;
  int record_size;
  elm_read = fread(&record_size , sizeof(record_size) , 1 , fortio->stream);
  if (elm_read == 1) {
    int trailer;

    if (fortio->endian_flip_header)
      util_endian_flip_vector(&record_size , sizeof record_size , 1);

    if (fortio_fseek(fortio , (offset_type) record_size , SEEK_CUR) == 0) {
      if (fread(&trailer , sizeof(record_size) , 1 , fortio->stream) == 1) {
        if (fortio->endian_flip_header)
          util_endian_flip_vector(&trailer , sizeof trailer , 1);

        if (trailer == record_size)
          is_fortio_file = true;
      }
    }
  }

  fortio_fseek(fortio , init_pos , SEEK_SET);
  return is_fortio_file;
}


/**
  This function reads the header (i.e. the number of bytes in the
  following record), stores that internally in the fortio struct, and
  also returns it. If the function fails to read a header (i.e. EOF)
  it will return -1.
*/

int fortio_init_read(fortio_type *fortio) {
  int elm_read;
  int record_size;

  elm_read = fread(&record_size , sizeof(record_size) , 1 , fortio->stream);
  if (elm_read == 1) {
    if (fortio->endian_flip_header)
      util_endian_flip_vector(&record_size , sizeof record_size , 1);

    return record_size;
  } else
    return -1;
}


bool fortio_data_fskip(fortio_type* fortio, const int element_size, const int element_count, const int block_count) {
  int headers = block_count * 4;
  int trailers = block_count * 4;
  int bytes_to_skip = headers + trailers + (element_size * element_count);

  return fortio_fseek(fortio, bytes_to_skip, SEEK_CUR);
}


void fortio_data_fseek(fortio_type* fortio, offset_type data_offset, size_t data_element, const int element_size, const int element_count, const int block_size) {
    if(data_element >= element_count) {
        util_abort("%s: Element index is out of range: 0 <= %d < %d \n", __func__, data_element, element_count);
    }
    {
      int block_index = data_element / block_size;
      int headers = (block_index + 1) * 4;
      int trailers = block_index * 4;
      offset_type bytes_to_skip = data_offset + headers + trailers + (data_element * element_size);

      fortio_fseek(fortio, bytes_to_skip, SEEK_SET);
    }
}

int fortio_fclean(fortio_type * fortio) {
  long current_pos = ftell(fortio->stream);
  if(current_pos == -1)
    return -1;

  int flush_status = fflush(fortio->stream);
  if(flush_status != 0)
    return flush_status;

  return fseek(fortio->stream, current_pos, SEEK_SET);
}

bool fortio_complete_read(fortio_type *fortio , int record_size) {
  int trailer;
  size_t read_count = fread(&trailer , sizeof trailer , 1 , fortio->stream );

  if (read_count == 1) {
    if (fortio->endian_flip_header)
      util_endian_flip_vector(&trailer , sizeof trailer , 1);

    if (record_size == trailer)
      return true;
  }

  return false;
}


/**
   This function reads one record from the fortio stream, and fills
   the buffer with the content. The return value is the number of
   bytes read; the function will return -1 on failure.
*/

static int fortio_fread_record(fortio_type *fortio , char *buffer) {
  int record_size = fortio_init_read(fortio);
  if (record_size >= 0) {
    size_t items_read = fread(buffer , 1 , record_size , fortio->stream);
    if (items_read == record_size) {
      bool complete_ok = fortio_complete_read(fortio , record_size);
      if (!complete_ok)
        record_size = -1;
    } else
      record_size = -1;  /* Failure */
  }
  return record_size;
}


/**
   This function fills the buffer with 'buffer_size' bytes from the
   fortio stream. The function works by repeated calls to
   fortio_read_record(), until the desired number of bytes of been
   read. The point of this is to handle the ECLIPSE system with blocks
   of e.g. 1000 floats (which then become one fortran record), in a
   transparent, low-level way.
*/

bool fortio_fread_buffer(fortio_type * fortio, char * buffer , int buffer_size) {
  int total_bytes_read = 0;

  while (true) {
    char * buffer_ptr = &buffer[total_bytes_read];
    int bytes_read = fortio_fread_record(fortio , buffer_ptr);

    if (bytes_read < 0)
      break;
    else {
      total_bytes_read += bytes_read;
      if (total_bytes_read >= buffer_size)
        break;
    }
  }

  if (total_bytes_read == buffer_size)
    return true;

  if (total_bytes_read < buffer_size)
    return false;

  util_abort("%s: internal inconsistency: buffer_size:%d  read %d bytes \n",__func__ , buffer_size , total_bytes_read);
  return false;
}


int fortio_fskip_record(fortio_type *fortio) {
  int record_size = fortio_init_read(fortio);
  fortio_fseek(fortio , (offset_type) record_size , SEEK_CUR);
  fortio_complete_read(fortio , record_size);
  return record_size;
}

void fortio_fskip_buffer(fortio_type * fortio, int buffer_size) {
  int bytes_skipped = 0;
  while (bytes_skipped < buffer_size)
    bytes_skipped += fortio_fskip_record(fortio);

  if (bytes_skipped > buffer_size)
    util_abort("%s: hmmmm - something is broken. The individual records in %s did not sum up to the expected buffer size \n",__func__ , fortio->filename);
}


void fortio_copy_record(fortio_type * src_stream , fortio_type * target_stream , int buffer_size , void * buffer , bool *at_eof) {
  int bytes_read;
  int record_size = fortio_init_read(src_stream);
  fortio_init_write(target_stream , record_size);

  bytes_read = 0;
  while (bytes_read < record_size) {
    int bytes;
    if (record_size > buffer_size)
      bytes = buffer_size;
    else
      bytes = record_size - bytes_read;

    util_fread(buffer , 1 , bytes , src_stream->stream     , __func__);
    util_fwrite(buffer , 1 , bytes , target_stream->stream , __func__);

    bytes_read += bytes;
  }

  fortio_complete_read(src_stream , record_size);
  fortio_complete_write(target_stream , record_size);

  if (feof(src_stream->stream))
    *at_eof = true;
  else
    *at_eof = false;
}


/*****************************************************************/

void  fortio_init_write(fortio_type *fortio , int record_size) {
  int file_header;
  file_header = record_size;
  if (fortio->endian_flip_header)
    util_endian_flip_vector(&file_header , sizeof file_header , 1);

  util_fwrite_int( file_header , fortio->stream );
}

void fortio_complete_write(fortio_type *fortio , int record_size) {
  int file_header = record_size;
  if (fortio->endian_flip_header)
    util_endian_flip_vector(&file_header , sizeof file_header , 1);

  util_fwrite_int( file_header , fortio->stream );
}


void fortio_fwrite_record(fortio_type *fortio, const char *buffer , int record_size) {
  fortio_init_write(fortio , record_size);
  util_fwrite( buffer , 1 , record_size , fortio->stream , __func__);
  fortio_complete_write(fortio , record_size);
}


void * fortio_fread_alloc_record(fortio_type * fortio) {
  void * buffer;
  int record_size = fortio_init_read(fortio);
  buffer = util_malloc( record_size );
  util_fread(buffer , 1 , record_size , fortio->stream , __func__);
  fortio_complete_read(fortio , record_size);
  return buffer;
}



static fortio_status_type fortio_check_record( FILE * stream , bool endian_flip , int * record_size) {
  int read_count;
  int header, tail;
  fortio_status_type status;

  read_count = fread( &header , sizeof header , 1 , stream );
  if (read_count == 0)
    status = FORTIO_EOF;
  else {
    if (endian_flip)
      util_endian_flip_vector(&header , sizeof header , 1);

    if (util_fseek(  stream , (offset_type) header , SEEK_CUR ) != 0)
      /* The fseek() failed - i.e. the data section was not sufficiently long. */
      status = FORTIO_MISSING_DATA;
    else {
      read_count = fread( &tail , sizeof tail , 1 , stream );
      if (read_count == 1) {
        if (endian_flip)
          util_endian_flip_vector(&tail , sizeof tail , 1);

        if (tail == header)
          /* All OK */
          status = FORTIO_OK;
        else
          /* The numerical value of the tail did not agree with the header. */
          status = FORTIO_HEADER_MISMATCH;
      } else
        /* The file ended before we could read the tail mark. */
        status = FORTIO_MISSING_TAIL;
    }
  }

  *record_size = header;
  return status;
}


fortio_status_type fortio_check_buffer( FILE * stream , bool endian_flip , size_t buffer_size ) {
  size_t current_size = 0;
  fortio_status_type record_status;
  while (true) {
    int record_size;
    record_status = fortio_check_record( stream , endian_flip , &record_size );
    current_size += record_size;
    if (record_status != FORTIO_OK)
      break;
  }
  if (record_status == FORTIO_EOF) {
    /* We are at the end of file - see if we have read in enough data. */
    if (buffer_size == current_size)
      return FORTIO_OK;
    else
      return FORTIO_EOF;
  } else
    return record_status;
}



fortio_status_type fortio_check_file( const char * filename , bool endian_flip) {
  if (util_file_exists( filename )) {
    size_t file_size = util_file_size( filename );
    if (file_size == 0)
      return FORTIO_EOF;
    else {
      fortio_status_type record_status;
      {
        FILE * stream = util_fopen( filename , "r");
        do {
          int record_size;
          record_status = fortio_check_record( stream , endian_flip , &record_size);
        } while ( record_status == FORTIO_OK );
        fclose( stream );
      }
      if (record_status == FORTIO_EOF) /* Normal exit from loop at EOF */
        return FORTIO_OK;
      else
        return record_status;
    }
  } else
    return FORTIO_NOENTRY;  /* This is an application error. */
}


offset_type fortio_ftell( const fortio_type * fortio ) {
  return util_ftell( fortio->stream );
}


static bool fortio_fseek__(fortio_type * fortio , offset_type offset , int whence) {
  int fseek_return = util_fseek( fortio->stream , offset , whence );
  if (fseek_return == 0)
    return true;
  else
    return false;

}

/*
  The semantics of this function depends on the readbale flag of the
  fortio structure:

    writable == true: Ordinary fseek() semantics which can potentially
       grow the file.

    writable == false: The function will only seek within the range of
       the file, and fail if you try to seek beyond the EOF marker.

*/


bool fortio_fseek( fortio_type * fortio , offset_type offset , int whence) {
  if (fortio->writable)
    return fortio_fseek__( fortio , offset , whence );
  else {
    offset_type new_offset = 0;

    switch (whence) {
    case( SEEK_CUR ):
      new_offset = fortio_ftell( fortio ) + offset;
      break;
    case (SEEK_END):
      new_offset = fortio->read_size + offset;
      break;
    case (SEEK_SET):
      new_offset = offset;
      break;
    default:
      util_abort("%s: invalid seek flag \n",__func__);
    }

    if (new_offset <= fortio->read_size)
      return fortio_fseek__( fortio , new_offset , SEEK_SET );
    else
      return false;
  }
}

bool fortio_ftruncate( fortio_type * fortio , offset_type size) {
  fortio_fseek( fortio , size , SEEK_SET);
  return util_ftruncate( fortio->stream , size);
}


bool fortio_ftruncate_current( fortio_type * fortio ) {
  offset_type size = fortio_ftell( fortio );
  return util_ftruncate( fortio->stream , size);
}



int fortio_fileno( fortio_type * fortio ) {
  return fileno( fortio->stream );
}




/*
  It is massively undefined behaviour to call this function for a file
  which has been updated; in that case the util_fd_size() function
  will return the size of the file *when it was opened*.
*/

bool fortio_read_at_eof( fortio_type * fortio ) {

  if (fortio_ftell(fortio) == fortio->read_size)
    return true;
  else
    return false;

}

/*
  When this function is called the underlying file is unlinked, and
  the entry will be removed from the filsystem. Subsequent calls which
  write to this file will still (superficially) succeed.
*/

void fortio_fwrite_error(fortio_type * fortio) {
  if (fortio->writable)
    util_unlink( fortio->filename );
}


/*****************************************************************/
void          fortio_fflush(fortio_type * fortio) { fflush( fortio->stream); }
FILE        * fortio_get_FILE(const fortio_type *fortio)        { return fortio->stream; }
//bool          fortio_endian_flip(const fortio_type *fortio)   { return fortio->endian_flip_header; }
bool          fortio_fmt_file(const fortio_type *fortio)        { return fortio->fmt_file; }
void          fortio_rewind(const fortio_type *fortio)          { util_rewind(fortio->stream); }
const char  * fortio_filename_ref(const fortio_type * fortio)   { return (const char *) fortio->filename; }


