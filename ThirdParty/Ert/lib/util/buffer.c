/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'buffer.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <ert/util/ert_api_config.h>
#include <ert/util/ssize_t.h>
#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/util/buffer.h>



/**
   This function implements a small buffer type. The whole point of
   this type is that it should work (more-or-less) as a drop in
   replacement of FILE * instances (at least for unformatted
   read/write).

   I.e. instead of

     int * p = util_malloc( sizeof * p * 10 );
     fread( p , sizeof * p , 10 , stream);

   To read ten integers from a FILE * instance we should be able to
   call

     int * p = util_malloc( sizeof * p * 10 );
     buffer_fread( buffer , p , sizeof * p , 10);

*/


#define BUFFER_TYPE_ID 661043


struct buffer_struct {
  UTIL_TYPE_ID_DECLARATION;
  char     * data;             /* The actual storage. */
  size_t     alloc_size;       /* The total byte size of the buffer. */
  size_t     content_size;     /* The extent of initialized data in the buffer - i.e. the meaningful content in the buffer. */
  size_t     pos;              /* The current byte position in the buffer.*/
};


/*****************************************************************/


UTIL_IS_INSTANCE_FUNCTION( buffer , BUFFER_TYPE_ID )
UTIL_SAFE_CAST_FUNCTION( buffer , BUFFER_TYPE_ID )


/**
   abort_on_error == true:
   -----------------------
   The function abort with util_abort() if the allocation fails.


   abort_on_error == false:
   ------------------------
   The function will SILENTLY fail if you ask for more memory than
   the system can provide.
*/


static void buffer_resize__(buffer_type * buffer , size_t new_size, bool abort_on_error) {
  if (abort_on_error) {
    buffer->data       = (char*)util_realloc(buffer->data , new_size );
    buffer->alloc_size = new_size;
  } else {
    void * tmp   = realloc(buffer->data , new_size);
    if (tmp != NULL) {
      buffer->data = (char*)tmp;
      buffer->alloc_size = new_size;
    }
  }
  buffer->content_size = util_size_t_min( buffer->content_size , new_size ); /* If the buffer has actually shrinked. */
  buffer->pos          = util_size_t_min( buffer->pos          , new_size);  /* If the buffer has actually shrinked. */
}


static buffer_type * buffer_alloc_empty( ) {
  buffer_type * buffer = (buffer_type*)util_malloc( sizeof * buffer );
  UTIL_TYPE_ID_INIT( buffer , BUFFER_TYPE_ID );
  buffer->data = NULL;

  buffer->alloc_size   = 0;
  buffer->content_size = 0;
  buffer->pos          = 0;
  return buffer;
}


buffer_type * buffer_alloc( size_t buffer_size ) {
  buffer_type * buffer = buffer_alloc_empty();
  buffer_resize__( buffer , buffer_size , true);
  return buffer;
}

/**
   Will resize the buffer storage to exactly fit the amount of content.
*/
void buffer_shrink_to_fit( buffer_type * buffer ) {
  buffer_resize__( buffer , buffer->content_size , true);
}


/**
   This function will allocate a buffer instance based on the input
   data. Observe that the buffer will 'steal' the input data pointer,
   in the process the data pointer might very well be realloced()
   leaving the original pointer invalid.

   All the content of the input data pointer will be assumed to be
   valid, i.e. the fields content_size and pos will be set to the
   value @buffer_size.

   When calling buffer_free() at a later stage the hijacked data will
   also be freed.
*/

buffer_type * buffer_alloc_private_wrapper(void * data , size_t buffer_size ) {
  buffer_type * buffer = buffer_alloc_empty();

  buffer->data         = (char*)data;        /* We have stolen the data pointer. */
  buffer->content_size = buffer_size;
  buffer->pos          = buffer_size;
  buffer->alloc_size   = buffer_size;

  return buffer;
}




/**
   This function will free the buffer data structure, but NOT the
   actual storage. Can typically be used when some other pointer has
   taken posession of the buffer content:

   buffer_type * buffer = buffer_alloc( );

   // Do thing with the buffer

   {
      void * thief_ptr = buffer_get_data( buffer );
      buffer_free_container( buffer );

      // Do stuff with thief_ptr
      // ....

      free( thief_ptr);
   }
*/

void buffer_free_container( buffer_type * buffer ) {
  free( buffer );
}


void buffer_free( buffer_type * buffer) {
  free( buffer->data );
  buffer_free_container( buffer );
}


/**
   This will reposition all the pointers to the start of the buffer.
   The actual data of the buffer will not be touched.
*/
void buffer_clear( buffer_type * buffer ) {
  buffer->content_size = 0;
  buffer->pos          = 0;
}


size_t buffer_fread(buffer_type * buffer,
                    void * target_ptr,
                    size_t item_size,
                    size_t items) {
  size_t remaining_size  = buffer->content_size - buffer->pos;
  size_t remaining_items = remaining_size / item_size;
  if (remaining_items < items)
    util_abort("%s: read beyond the length of the buffer (%d exceeds %d)\n",
               __func__, items, remaining_items);

  size_t read_bytes = items * item_size;

  memcpy(target_ptr, &buffer->data[buffer->pos], read_bytes);
  buffer->pos += read_bytes;

  return items;
}


/*****************************************************************/



size_t buffer_fwrite(buffer_type * buffer,
                     const void * src_ptr,
                     size_t item_size,
                     size_t items) {
  size_t remaining_size = buffer->alloc_size - buffer->pos;
  size_t target_size    = item_size * items;

  if (target_size > remaining_size) {
    buffer_resize__(buffer , buffer->pos + 2 * (item_size * items), true);
    remaining_size = buffer->alloc_size - buffer->pos;
  }

  size_t remaining_items = remaining_size / item_size;
  size_t write_items     = util_size_t_min( items , remaining_items );
  size_t write_bytes     = write_items * item_size;

  memcpy( &buffer->data[buffer->pos] , src_ptr , write_bytes );
  buffer->pos += write_bytes;

  if (write_items < items)
    util_abort("%s: failed to write %d elements to the buffer \n",__func__ , items);
  buffer->content_size = util_size_t_max(buffer->content_size , buffer->pos);
  return write_items;
}





/*****************************************************************/
/* Various (slighly) higher level functions                      */


void buffer_rewind(buffer_type * buffer ) {
  buffer_fseek( buffer , 0 , SEEK_SET);
}


void buffer_fseek(buffer_type * buffer , ssize_t offset , int whence) {
  ssize_t new_pos = 0;

  if (whence == SEEK_SET)
    new_pos = offset;
  else if (whence == SEEK_CUR)
    new_pos = buffer->pos + offset;
  else if (whence == SEEK_END)
    new_pos = buffer->content_size + offset;
  else
    util_abort("%s: unrecognized whence indicator - aborting \n",__func__);

  /**
      Observe that we can seek to the very end of the buffer. I.e. for
      a buffer with content_size == 20 we can seek to position 20.
  */

  if ((new_pos >= 0) && (new_pos <= buffer->content_size))
    buffer->pos = new_pos;
  else
    util_abort("%s: tried to seek to position:%ld - outside of bounds: [0,%d) \n",
               __func__ , new_pos , buffer->content_size);
}


void buffer_fskip(buffer_type * buffer, ssize_t offset) {
  buffer_fseek( buffer , offset , SEEK_CUR );
}


int buffer_fread_int(buffer_type * buffer) {
  int value = 0;
  int read = buffer_fread(buffer, &value, sizeof value, 1);
  if (read != 1)
      util_abort("%s: read mismatch, read %d expected to read %d\n",
                 __func__, read, 1);
  return value;
}


bool buffer_fread_bool(buffer_type * buffer) {
  bool value = false;
  buffer_fread(buffer, &value, sizeof value, 1);
  return value;
}


long int buffer_fread_long(buffer_type * buffer) {
  long value = 0L;
  buffer_fread(buffer, &value, sizeof value, 1);
  return value;
}


int buffer_fgetc(buffer_type * buffer) {
  if (buffer->pos == buffer->content_size)
    return EOF;

  unsigned char byte = 0;
  buffer_fread(buffer, &byte, sizeof byte, 1);
  return byte;
}

/**
   This function writes all the elements in the string including the
   terminating \0 character into the buffer. This should not be
   confused with buffer_fwrite_string() function which in addition
   prepends the string with an integer length.
*/

void buffer_fwrite_char_ptr(buffer_type * buffer , const char * string_ptr ) {
  buffer_fwrite(buffer , string_ptr , sizeof * string_ptr , strlen( string_ptr ) + 1);
}


void buffer_strcat(buffer_type * buffer , const char * string) {
  if (buffer->content_size == 0)
    buffer_fwrite_char_ptr( buffer , string );
  else {
    if (buffer->data[ buffer->content_size - 1] == '\0') {
      buffer_fseek( buffer , -1 , SEEK_END);
      buffer_fwrite_char_ptr( buffer , string );
    }
  }
}


/**
   Will append a \0 to the buffer; before appending the last character
   of the buffer will be checked, and no new \0 will be added if the
   buffer is already \0 terminated.
*/
static void buffer_terminate_char_ptr( buffer_type * buffer ) {
  if (buffer->data[ buffer->content_size - 1] != '\0')
    buffer_fwrite_char( buffer , '\0');
}


void buffer_fwrite_int(buffer_type * buffer , int value) {
  buffer_fwrite(buffer , &value , sizeof value , 1);
}


void buffer_fwrite_bool(buffer_type * buffer , bool value) {
  buffer_fwrite(buffer , &value , sizeof value , 1);
}


void buffer_fskip_time_t(buffer_type * buffer) {
  buffer_fseek( buffer , sizeof(time_t) , SEEK_CUR );
}


void buffer_fskip_int(buffer_type * buffer) {
  buffer_fseek( buffer , sizeof( int ) , SEEK_CUR );
}

void buffer_fskip_long(buffer_type * buffer) {
  buffer_fseek( buffer , sizeof( long ) , SEEK_CUR );
}

void buffer_fskip_bool(buffer_type * buffer) {
  buffer_fseek( buffer , sizeof( bool ) , SEEK_CUR );
}


time_t buffer_fread_time_t(buffer_type * buffer) {
  time_t value;
  buffer_fread(buffer , &value , sizeof value , 1);
  return value;
}


void buffer_fwrite_time_t(buffer_type * buffer , time_t value) {
  buffer_fwrite(buffer , &value , sizeof value , 1);
}


char buffer_fread_char(buffer_type * buffer) {
  char value;
  buffer_fread(buffer , &value , sizeof value , 1);
  return value;
}


void buffer_fwrite_char(buffer_type * buffer , char value) {
  buffer_fwrite(buffer , &value , sizeof value , 1);
}


double buffer_fread_double(buffer_type * buffer) {
  double value;
  buffer_fread(buffer , &value , sizeof value , 1);
  return value;
}


void buffer_fwrite_double(buffer_type * buffer , double value) {
  buffer_fwrite(buffer , &value , sizeof value , 1);
}






/*****************************************************************/
/*****************************************************************/

size_t buffer_get_offset(const buffer_type * buffer) {
  return buffer->pos;
}


size_t buffer_get_size(const buffer_type * buffer) {
  return buffer->content_size;
}


size_t buffer_get_string_size( const buffer_type * buffer ) {
  return strlen( buffer->data );
}

size_t buffer_get_alloc_size(const buffer_type * buffer) {
  return buffer->alloc_size;
}


size_t buffer_get_remaining_size(const buffer_type *  buffer) {
  return buffer->content_size - buffer->pos;
}

/**
    Returns a pointer to the internal storage of the buffer. Observe
    that this storage is volatile, and the return value from this
    function should not be kept around; alternatively you can use
    buffer_alloc_data_copy().
*/
void * buffer_get_data(const buffer_type * buffer) {
  return buffer->data;
}

void * buffer_iget_data(const buffer_type * buffer, size_t offset) {
  return &buffer->data[offset];
}



/**
   Returns a copy of the initialized (i.e. buffer->content_size)
   buffer content.
*/
void * buffer_alloc_data_copy(const buffer_type * buffer) {
  return util_alloc_copy(buffer->data , buffer->content_size );
}


/**
   This function will shift parts of the buffer data, either creating
   a hole in the buffer, or overwriting parts of the internal buffer.

   Example
   -------

   The buffer has content_size of 8, and allocated size of 12
   elements.

   -------------------------------------------------
   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | x | x | x | x |
   -------------------------------------------------


   buffer_memshift(buffer , 4 , 3)
   -------------------------------
   The part from offset four is moved three bytes to the right. The
   newly created 'hole' in the storage area has undefined values.

   -------------------------------------------------
   | 0 | 1 | 2 | 3 | x | x | x | 4 | 5 | 6 | 7 | x |
   -------------------------------------------------

   If you are shifting beyound the end of the buffer, it will be
   automatically resized.


   buffer_memshift(buffer , 2 , -4)
   --------------------------------

   -------------------------------------------------
   | 4 | 5 | 6 | 7 | 8 | x | x | x | x | x | x | x |
   -------------------------------------------------


   When shifting to the left, content is lost (without warning/error)
   when it is shifted beyond the start of the buffer.


   The current position in the buffer is not updated, unless it
   corresponds to a point beyond the (new) end of the buffer, in which
   case it is set to the new end of the buffer.
*/

void buffer_memshift(buffer_type * buffer , size_t offset, ssize_t shift) {
  /* Do we need to grow the buffer? */
  if (shift > 0) {
    if (buffer->alloc_size <= (buffer->content_size + shift)) {
      size_t new_size = 2 * (buffer->content_size + shift);
      buffer_resize__(buffer , new_size , true );
    }
  }

  {
    size_t move_size;
    if (shift < 0)
      if (labs(shift) > offset)
        offset = labs(shift);  /* We are 'trying' to left shift beyond the start of the buffer. */

    move_size = buffer->content_size - offset;
    memmove( &buffer->data[offset + shift] , &buffer->data[offset] , move_size );
    buffer->content_size += shift;
    buffer->pos           = util_size_t_min( buffer->pos , buffer->content_size);
  }
}


void buffer_replace_data(buffer_type * buffer , size_t offset , size_t old_size , const void * new_data , size_t new_size) {
  ssize_t shift = new_size - old_size;
  buffer_memshift( buffer , offset , shift );
  buffer_fseek( buffer , offset , SEEK_SET );
  buffer_fwrite( buffer , new_data , 1 , new_size );
}


void buffer_replace_string( buffer_type * buffer , size_t offset , size_t old_size , const char * new_string) {
  buffer_replace_data( buffer , offset , old_size , new_string , strlen(new_string));
}


/**
   This function will use the stdlib function strstr() to search for
   the string @expr in @buffer. The search will start at the current
   position in the buffer, if the string is found true is returned AND
   the internal pos is updated to point at the match.

   If the string is NOT found the function will return false, without
   touching internal state.
*/


bool buffer_strstr( buffer_type * buffer , const char * expr ) {
  bool match = false;

  if (strlen(expr) > 0) {
    char * match_ptr = strstr( &buffer->data[buffer->pos] , expr );
    if (match_ptr) {
      buffer->pos += match_ptr - &buffer->data[buffer->pos];
      match = true;
    }
  }
  return match;
}


bool buffer_strchr( buffer_type * buffer , int c) {
  /**
      If this condition is satisfied the assumption that buffer->data
      is a \0 terminated string certainly breaks down.
  */
  if ((buffer->content_size == 0) || (buffer->pos == buffer->content_size))
    return false;

  {
    bool match = false;
    size_t pos = buffer->pos;

    while (true) {
      if (buffer->data[pos] == c) {
        match = true;
        buffer->pos = pos;
        break;
      }
      pos++;
      if (pos == buffer->content_size)
        break;
    }

    return match;
  }
}




bool buffer_search_replace( buffer_type * buffer , const char * old_string , const char * new_string) {
  bool match = buffer_strstr( buffer , old_string );
  if (match) {
    size_t offset = buffer_get_offset( buffer ) + strlen( old_string );
    const int shift = strlen( new_string ) - strlen( old_string );
    if (shift != 0)
      buffer_memshift( buffer , offset , shift );

    buffer_fwrite( buffer , new_string , 1 , strlen(new_string));
    buffer_terminate_char_ptr( buffer );
  }
  return match;
}



void buffer_summarize(const buffer_type * buffer , const char * header) {
  printf("-----------------------------------------------------------------\n");
  if (header != NULL)
    printf("%s \n",header);
  printf("   Allocated size .....: %zd10 bytes \n",buffer->alloc_size);
  printf("   Content size .......: %zd10 bytes \n",buffer->content_size);
  printf("   Current position ...: %zd10 bytes \n",buffer->pos);
  printf("-----------------------------------------------------------------\n");
}


/*****************************************************************/
/*****************************************************************/
/*
  Here comes a couple of functions for loading/storing a buffer
  instance to a stream. Observe that when the buffer is stored to file
  it does not store any metadata (i.e. not even size) - only the raw
  buffer content.
*/

/**
   This is the lowest level: 'read buffer content from file'
   function. It will read 'byte_size' bytes from stream and fill the
   buffer with the data.

   When the function completes the buffer position is at the end of
   the buffer, i.e. it is ready for more calls to buffer_stream_fread;
   this is in contrast to the higher level functions
   buffer_fread_alloc() / buffer_fread_realloc() which reposition the
   buffer position to the beginning of the buffer.

   Before reading from the buffer with e.g. buffer_fread_int() the
   buffer must be repositioned with buffer_rewind().
*/


void buffer_stream_fread( buffer_type * buffer , size_t byte_size , FILE * stream) {
  size_t min_size = byte_size + buffer->pos;
  if (buffer->alloc_size < min_size)
    buffer_resize__(buffer , min_size , true);

  util_fread( &buffer->data[buffer->pos] , 1 , byte_size , stream , __func__);

  buffer->content_size += byte_size;
  buffer->pos          += byte_size;
}




/**
   This file will read in the full content of file, and allocate a
   buffer instance with that content. When the function completes the
   position is at the beginning of the buffer.
*/

void buffer_fread_realloc(buffer_type * buffer , const char * filename) {
  size_t file_size     = util_file_size( filename );
  FILE * stream        = util_fopen( filename , "r");

  buffer_clear( buffer );    /* Setting: content_size = 0; pos = 0;  */
  buffer_stream_fread( buffer , file_size , stream );
  buffer_rewind( buffer );   /* Setting: pos = 0; */
  fclose( stream );
}



buffer_type * buffer_fread_alloc(const char * filename) {
  buffer_type * buffer = buffer_alloc( 0 );
  buffer_fread_realloc( buffer , filename );
  return buffer;
}



/**
   Will write parts of the buffer to the stream. Will start at buffer
   position @offset and write @write_size bytes.

    o If @offset is invalid, i.e. less than zero or greater than
      buffer->content_size the function will fail hard.

    o If @write_size is greater than storage in the buffer the
      function will just write all the available data, but not complain
      any more.

    o @write_size == 0 that is interpreted as "write everything from offset".

    o @write_size < 0 is interpreted as : "Write everything except the
      abs(@write_size) last bytes.

   The return value is the number of bytes actually written.
*/

size_t buffer_stream_fwrite_n( const buffer_type * buffer , size_t offset , ssize_t write_size , FILE * stream ) {
  if (offset > buffer->content_size)
    util_abort("%s: invalid offset:%ld - valid range: [0,%ld) \n",__func__ , offset , offset);
  {
    ssize_t len;

    if (write_size > 0)             /* Normal - write @write_size bytes from offset */
      len = write_size;
    else if (write_size == 0)       /* Write everything from the offset */
      len = buffer->content_size - offset;
    else                            /* @write_size < 0 - write everything excluding the last abs(write_size) bytes. */
      len = buffer->content_size - offset - labs( write_size );

    if (len < 0)
      util_abort("%s: invalid length spesifier - tried to write %ld bytes \n",__func__ , len);

    util_fwrite( &buffer->data[offset] , 1 , len , stream , __func__);
    return len;
  }
}


void buffer_stream_fwrite( const buffer_type * buffer , FILE * stream ) {
  buffer_stream_fwrite_n( buffer , 0 , buffer->content_size , stream );
}


/* Assumes that the buffer contains a \0 terminated string - that is the resoponsability of the caller. */
void buffer_stream_fprintf( const buffer_type * buffer , FILE * stream ) {
  fprintf(stream , "%s" , buffer->data );
}


/**
   Dumps buffer content to a stream - without any metadata.
*/
void buffer_store(const buffer_type * buffer , const char * filename) {
  FILE * stream        = util_fopen(filename , "w");
  buffer_stream_fwrite( buffer , stream );
  fclose( stream );
}

/*
   The functions buffer_fread_string() and buffer_fwrite_string()
   should not be used; the embedded integer just creates chaos and
   should the sole responsability of the calling scope.
*/

/**
   Storing strings:
   ----------------

   When storing a string (\0 terminated char pointer) what is actually
   written to the buffer is

     1. The length of the string - as returned from strlen().
     2. The string content INCLUDING the terminating \0.


*/


/**
   This function will return a pointer to the current position in the
   buffer, and advance the buffer position forward until a \0
   terminater is found. If \0 is not found the thing will abort().

   Observe that the return value will point straight into the buffer,
   this is highly volatile memory, and in general it will be safer to
   use buffer_fread_alloc_string() to get a copy of the string.
*/

const char * buffer_fread_string(buffer_type * buffer) {
  int    string_length = buffer_fread_int( buffer );
  char * string_ptr    = &buffer->data[buffer->pos];
  char   c;
  buffer_fskip( buffer , string_length );
  c = buffer_fread_char( buffer );
  if (c != '\0')
    util_abort("%s: internal error - malformed string representation in buffer \n",__func__);
  return string_ptr;
}



char * buffer_fread_alloc_string(buffer_type * buffer) {
  return util_alloc_string_copy( buffer_fread_string( buffer ));
}

/**
   Observe that this function writes a leading integer string length.
*/
void buffer_fwrite_string(buffer_type * buffer , const char * string) {
  buffer_fwrite_int( buffer , strlen( string ));               /* Writing the length of the string */
  buffer_fwrite(buffer , string , 1 , strlen( string ) + 1);   /* Writing the string content ** WITH ** the terminating \0 */
}

#ifdef ERT_HAVE_ZLIB
#include <zlib.h>

/**
  Unfortunately the old RedHat3 computers have a zlib version which
  does not have the compressBound function. For that reason the
  compressBound function from a 1.2xx version of zlib is pasted in
  here verbatim:
  */


/* Snipped from zlib source code: */
static size_t __compress_bound (size_t sourceLen)
{
    return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + 11;
}


/**
  Return value is the size (in bytes) of the compressed buffer.
  */
size_t buffer_fwrite_compressed(buffer_type * buffer, const void * ptr , size_t byte_size) {
    size_t compressed_size = 0;
    bool abort_on_error    = true;
    buffer->content_size   = buffer->pos;   /* Invalidating possible buffer content coming after the compressed content; that is uninterpretable anyway. */

    if (byte_size > 0) {
        size_t remaining_size = buffer->alloc_size - buffer->pos;
        size_t compress_bound = __compress_bound( byte_size );
        if (compress_bound > remaining_size)
            buffer_resize__(buffer , remaining_size + compress_bound , abort_on_error);

        compressed_size = buffer->alloc_size - buffer->pos;
        util_compress_buffer( ptr , byte_size , &buffer->data[buffer->pos] , &compressed_size);
        buffer->pos          += compressed_size;
        buffer->content_size += compressed_size;
    }

    return compressed_size;
}


/**
  Return value is the size of the uncompressed buffer.
  */
size_t buffer_fread_compressed(buffer_type * buffer , size_t compressed_size , void * target_ptr , size_t target_size) {
    size_t remaining_size    = buffer->content_size - buffer->pos;
    size_t uncompressed_size = target_size;
    if (remaining_size < compressed_size)
        util_abort("%s: trying to read beyond end of buffer\n",__func__);


    if (compressed_size > 0) {
        int uncompress_result = uncompress((Bytef*)target_ptr , &uncompressed_size , (unsigned char *) &buffer->data[buffer->pos] , compressed_size);
        if (uncompress_result != Z_OK) {
            fprintf(stderr,"%s: ** Warning uncompress result:%d != Z_OK.\n" , __func__ , uncompress_result);
            /**
              According to the zlib documentation:

              1. Values > 0 are not errors - just rare events?
              2. The value Z_BUF_ERROR is not fatal - we let that pass?!
              */
            if (uncompress_result < 0 && uncompress_result != Z_BUF_ERROR)
                util_abort("%s: fatal uncompress error: %d \n",__func__ , uncompress_result);
        }
    } else
        uncompressed_size = 0;

    buffer->pos += compressed_size;
    return uncompressed_size;
}

#endif // ERT_HAVE_ZLIB
