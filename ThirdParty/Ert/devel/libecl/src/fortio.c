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

#include <util.h>
#include <fortio.h>

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
  FILE             * stream;
  char             * filename;
  int                active_header;
  int                rec_nr;
  bool               endian_flip_header;  
  bool               fmt_file;    /* This is not really used by the fortio instance - but it is very convenient to store it here. */
  int                mode;
};



static fortio_type * fortio_alloc__(const char *filename , bool fmt_file , bool endian_flip_header) {
  fortio_type * fortio       = (fortio_type *) util_malloc(sizeof * fortio );
  fortio->filename           = util_alloc_string_copy(filename);
  fortio->endian_flip_header = endian_flip_header;
  fortio->active_header      = 0;
  fortio->rec_nr             = 0; 
  fortio->fmt_file           = fmt_file;
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
  long init_pos              = ftell(stream);
  bool is_fortran_stream     = false;
  int header , tail;
  bool cont;

  do {
    cont = false;
    if (__read_int(stream , &header , endian_flip)) {
      if (header >= 0) {
        if (fseek(stream , header , SEEK_CUR) == 0) {
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
  fseek(stream , init_pos , SEEK_SET);
  return is_fortran_stream;
}


/**
   This function tries (using some heuristic) to guess whether a
   particular file is a Fortran file. To complicate the matters
   further we make no assumptions regarding endian ness, if it is
   indeed determined that this is fortran file, the endian ness is
   returned by reference (if it is not recognized as a fortran file, 
   the returned endian ness will be garbage).

   The heuristic algorithm which is used is as follows:
   
    1. Read four bytes as an integer (header)
    2. Skip that number of bytes forward.
    3. Read four bytes again (tail).

   Now, when this is done we do the following tests:

    1. If header == tail. This is (probably) a fortran file, however
       if header == 0, we might have a normal file with two
       consequitive zeroes. In that case it is difficult to determine,
       and we continue.

    2. If header != tail we try to reinterpret header with an endian
       swap and read a new tail. If they are now equal we repeat test1, or
       return false (i.e. *not* a fortran file).
*/

bool fortio_is_fortran_file(const char * filename, bool * _endian_flip) {
  FILE * stream = util_fopen(filename , "rb");
  bool endian_flip = false;          
  bool is_fortran_stream = fortio_is_fortran_stream__(stream , endian_flip);
  if (!is_fortran_stream) {
    endian_flip = !endian_flip;
    is_fortran_stream = fortio_is_fortran_stream__(stream , endian_flip);
  }

  *_endian_flip = endian_flip;
  fclose(stream);
  return is_fortran_stream;
}


/**
   This function tries to determine automatically whether a certain
   file has endian flip or not. 

   Observe that the return value is whether we managed to determine the
   endian-ness or not, whereas the endian_flip flag is returned by
   reference.

   To be able to determine endianness the file *must* be a binary
   fortran file - this is essentially the return value.
*/

bool fortio_guess_endian_flip(const char * filename , bool * _endian_flip) {
  return fortio_is_fortran_file(filename , _endian_flip);
}





fortio_type * fortio_alloc_FILE_wrapper(const char *filename , bool endian_flip_header , bool fmt_file , FILE * stream) {
  fortio_type * fortio = fortio_alloc__(filename , fmt_file , endian_flip_header);
  fortio->stream = stream;
  return fortio;
}


fortio_type * fortio_open_reader(const char *filename , bool fmt_file , bool endian_flip_header) {
  fortio_type *fortio = fortio_alloc__(filename , fmt_file , endian_flip_header);
  
  if (fmt_file)
    fortio->stream = util_fopen(fortio->filename , READ_MODE_TXT);
  else
    fortio->stream = util_fopen(fortio->filename , READ_MODE_BINARY);
  fortio->mode = FORTIO_READ;
  return fortio;
}

fortio_type * fortio_open_writer(const char *filename , bool fmt_file , bool endian_flip_header ) {
  fortio_type *fortio = fortio_alloc__(filename , fmt_file , endian_flip_header);
  
  if (fmt_file)
    fortio->stream = util_fopen(fortio->filename , WRITE_MODE_TXT);
  else
    fortio->stream = util_fopen(fortio->filename , WRITE_MODE_BINARY);
  fortio->mode = FORTIO_WRITE;
  
  return fortio;
}


fortio_type * fortio_open_readwrite(const char *filename , bool fmt_file , bool endian_flip_header) {
  fortio_type *fortio = fortio_alloc__(filename , fmt_file , endian_flip_header);

  if (fmt_file)
    fortio->stream = util_fopen(fortio->filename , READ_WRITE_MODE_TXT);
  else
    fortio->stream = util_fopen(fortio->filename , READ_WRITE_MODE_BINARY);

  fortio->mode = FORTIO_READ + FORTIO_WRITE;
  return fortio;
}


fortio_type * fortio_open_append(const char *filename , bool fmt_file , bool endian_flip_header) {
  fortio_type *fortio = fortio_alloc__(filename , fmt_file , endian_flip_header);

  if (fmt_file)
    fortio->stream = util_fopen(fortio->filename , APPEND_MODE_TXT);
  else
    fortio->stream = util_fopen(fortio->filename , APPEND_MODE_BINARY);

  fortio->mode =  FORTIO_WRITE;
  return fortio;
}



static void fortio_free__(fortio_type * fortio) {
  if (fortio->filename != NULL) free(fortio->filename);
  free(fortio);
}

void fortio_free_FILE_wrapper(fortio_type * fortio) {
  fortio_free__( fortio );
}


void fortio_fclose(fortio_type *fortio) {
  fclose(fortio->stream);
  fortio_free__(fortio);
}


bool fortio_is_fortio_file(fortio_type * fortio) {
  FILE * stream = fortio->stream;
  int init_pos = ftell(stream);
  int elm_read;
  bool is_fortio_file = false;
  elm_read = fread(&fortio->active_header , sizeof(fortio->active_header) , 1 , fortio->stream);
  if (elm_read == 1) {
    int trailer;

    if (fortio->endian_flip_header)
      util_endian_flip_vector(&fortio->active_header , sizeof fortio->active_header , 1);

    if (fseek(stream , fortio->active_header , SEEK_CUR) == 0) {
      if (fread(&trailer , sizeof(fortio->active_header) , 1 , fortio->stream) == 1) {
        if (fortio->endian_flip_header)
          util_endian_flip_vector(&trailer , sizeof trailer , 1);
        
        if (trailer == fortio->active_header)
          is_fortio_file = true;
      }
    } 
  }

  fseek(stream , init_pos , SEEK_SET);
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
  elm_read = fread(&fortio->active_header , sizeof(fortio->active_header) , 1 , fortio->stream);
  if (elm_read == 1) {
    if (fortio->endian_flip_header)
      util_endian_flip_vector(&fortio->active_header , sizeof fortio->active_header , 1);

    fortio->rec_nr++;
    return fortio->active_header;
  } else 
    return -1;
}



// util_fread_int: read failed: No such file or directory
// 
// ****************************************************************************
// **                                                                        **
// **           A fatal error occured, and we have to abort.                 **
// **                                                                        **
// **  We now *try* to provide a backtrace, which would be very useful       **
// **  when debugging. The process of making a (human readable) backtrace    **
// **  is quite complex, among other things it involves several calls to the **
// **  external program addr2line. We have arrived here because the program  **
// **  state is already quite broken, so the backtrace might be (seriously)  **
// **  broken as well.                                                       **
// **                                                                        **
// ****************************************************************************
// Current executable : /private/joaho/EnKF/devel/EnKF/libecl/applications/summary.x
// --------------------------------------------------------------------------------
//  #00 util_abort                 (..) in /private/joaho/EnKF/devel/EnKF/libutil/src/util.c:4604
//  #01 util_fread_int             (..) in /private/joaho/EnKF/devel/EnKF/libutil/src/util.c:3423
//  #02 fortio_complete_read       (..) in libecl/src/fortio.c:275
//  #03 fortio_fread_record        (..) in libecl/src/fortio.c:297
//  #04 fortio_fread_buffer        (..) in libecl/src/fortio.c:313
//  #05 ecl_kw_fread_data          (..) in libecl/src/ecl_kw.c:745
//  #06 ecl_kw_fread_realloc       (..) in libecl/src/ecl_kw.c:996
//  #07 ecl_kw_fread_alloc         (..) in libecl/src/ecl_kw.c:1017
//  #08 ecl_file_fread_alloc_fortio(..) in /private/joaho/EnKF/devel/EnKF/libecl/src/ecl_file.c:206
//  #09 ecl_file_fread_alloc       (..) in /private/joaho/EnKF/devel/EnKF/libecl/src/ecl_file.c:265
//  #10 ecl_sum_data_fread__       (..) in /private/joaho/EnKF/devel/EnKF/libecl/src/ecl_sum_data.c:693
//  #11 ecl_sum_data_fread_alloc   (..) in /private/joaho/EnKF/devel/EnKF/libecl/src/ecl_sum_data.c:733
//  #12 ecl_sum_fread_alloc__      (..) in libecl/src/ecl_sum.c:58
//  #13 ecl_sum_fread_alloc_case__ (..) in libecl/src/ecl_sum.c:149
//  #14 main                       (..) in /private/joaho/EnKF/devel/EnKF/libecl/applications/view_summary.c:144
//  #15 ??                         (..) in ??:0
//  #16 _start                     (..) in ??:0



void fortio_complete_read(fortio_type *fortio) {
  int trailer;
  trailer = util_fread_int( fortio->stream );
  
  if (fortio->endian_flip_header)
    util_endian_flip_vector(&trailer , sizeof trailer , 1);
  
  if (trailer != fortio->active_header) {
    fprintf(stderr,"%s: fatal error reading record:%d in file: %s - aborting \n",__func__ , fortio->rec_nr , fortio->filename);
    util_abort("%s: Header: %d   Trailer: %d \n",__func__ , fortio->active_header , trailer);
  }
  fortio->active_header = 0;
}


/**
   This function reads one record from the fortio stream, and fills
   the buffer with the content. The return value is the number of bytes read.
*/

int fortio_fread_record(fortio_type *fortio, char *buffer) {
  fortio_init_read(fortio);
  {
          int record_size = fortio->active_header; /* This is reset in fortio_complete_read - must store it for the return. */
      util_fread(buffer , 1 , fortio->active_header , fortio->stream , __func__);
      fortio_complete_read(fortio);
      return record_size;
  }
}


/**
   This function fills the buffer with 'buffer_size' bytes from the
   fortio stream. The function works by repeated calls to
   fortio_read_record(), until the desired number of bytes of been
   read. The point of this is to handle the ECLIPSE system with blocks
   of e.g. 1000 floats (which then become one fortran record), in a
   transparent, low-level way.
*/

void fortio_fread_buffer(fortio_type * fortio, char * buffer , int buffer_size) {
  int bytes_read = 0;
  while (bytes_read < buffer_size) {
    char * buffer_ptr = &buffer[bytes_read];
    bytes_read += fortio_fread_record(fortio , buffer_ptr);
  }

  if (bytes_read > buffer_size) 
    util_abort("%s: hmmmm - something is broken. The individual records in %s did not sum up to the expected buffer size \n",__func__ , fortio->filename);
}


int fortio_fskip_record(fortio_type *fortio) {
  int record_size = fortio_init_read(fortio);
  fseek(fortio->stream , record_size , SEEK_CUR);
  fortio_complete_read(fortio);
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

  fortio_complete_read(src_stream);
  fortio_complete_write(target_stream);

  if (feof(src_stream->stream))
    *at_eof = true;
  else
    *at_eof = false;
}


/*****************************************************************/

void  fortio_init_write(fortio_type *fortio , int record_size) {
  int file_header;
  fortio->active_header = record_size;
  file_header = fortio->active_header;
  if (fortio->endian_flip_header)
    util_endian_flip_vector(&file_header , sizeof file_header , 1);
  
  util_fwrite_int( file_header , fortio->stream );
  fortio->rec_nr++;
}

void fortio_complete_write(fortio_type *fortio) {
  int file_header = fortio->active_header;
  if (fortio->endian_flip_header)
    util_endian_flip_vector(&file_header , sizeof file_header , 1);

  util_fwrite_int( file_header , fortio->stream );
  fortio->active_header = 0;
}


void fortio_fwrite_record(fortio_type *fortio, const char *buffer , int record_size) {
  fortio_init_write(fortio , record_size);
  util_fwrite( buffer , 1 , record_size , fortio->stream , __func__);
  fortio_complete_write(fortio);
}


void * fortio_fread_alloc_record(fortio_type * fortio) {
  void * buffer;
  fortio_init_read(fortio);
  buffer = util_malloc(fortio->active_header );
  util_fread(buffer , 1 , fortio->active_header , fortio->stream , __func__);
  fortio_complete_read(fortio);
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
    
    if (fseek(  stream , header , SEEK_CUR ) != 0) 
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
        else if ( tail != header ) 
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


long fortio_ftell( const fortio_type * fortio ) {
  return ftell( fortio->stream );
}


int fortio_fseek( fortio_type * fortio , long offset , int whence) {
  int fseek_return = fseek( fortio->stream , offset , whence );
  /*
    if fseek_return != 0 -> util_abort().
  */
  return fseek_return;
}


int fortio_fileno( fortio_type * fortio ) {
  return fileno( fortio->stream );
}


/*****************************************************************/
void          fortio_fflush(fortio_type * fortio) { fflush( fortio->stream); }
FILE        * fortio_get_FILE(const fortio_type *fortio)        { return fortio->stream; }
int           fortio_get_record_size(const fortio_type *fortio) { return fortio->active_header; }
//bool          fortio_endian_flip(const fortio_type *fortio)   { return fortio->endian_flip_header; }
bool          fortio_fmt_file(const fortio_type *fortio)        { return fortio->fmt_file; }
void          fortio_rewind(const fortio_type *fortio)          { rewind(fortio->stream); }
const char  * fortio_filename_ref(const fortio_type * fortio)   { return (const char *) fortio->filename; }
int           fortio_get_mode( const fortio_type * fortio )     { return fortio->mode; }
