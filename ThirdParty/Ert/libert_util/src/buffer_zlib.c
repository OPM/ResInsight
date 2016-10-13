/*
  This file is compiled as part of the buffer.c file; if the symbol
  ERT_HAVE_ZLIB is defined.  
*/
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
    int uncompress_result = uncompress(target_ptr , &uncompressed_size , (unsigned char *) &buffer->data[buffer->pos] , compressed_size);
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

