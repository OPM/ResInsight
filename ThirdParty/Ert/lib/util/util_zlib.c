#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#include <ert/util/util.h>

/**
  This function reads data from the input pointer data, and writes a
  compressed copy into the target buffer zbuffer. On input data_size
  should be the *number of bytes* in data. compressed_size should be a
  reference to the size (in bytes) of zbuffer, on return this has been
  updated to reflect the new compressed size.
*/

void util_compress_buffer(const void * data , int data_size , void * zbuffer , unsigned long * compressed_size) {
  int compress_result;
  if (data_size > 0) {
    compress_result = compress((Bytef*)zbuffer , compressed_size , (const Bytef*)data , data_size);
    /**
       Have some not reproducible "one-in-a-thousand" problems with
       the return value from compress. It seemingly randomly returns:

         -2  == Z_STREAM_ERROR. 
         
       According to the documentation in zlib.h compress should only
       return one of the three values:

           Z_OK == 0   ||   Z_MEM_ERROR == -4   Z_BUF_ERROR == -5

       We mask the Z_STREAM_ERROR return value as Z_OK, with a
       FAT-AND_UGLY_WARNING, and continue with fingers crossed.
    */
    
    if (compress_result == Z_STREAM_ERROR) {
      fprintf(stderr,"*****************************************************************\n");
      fprintf(stderr,"**                       W A R N I N G                         **\n");
      fprintf(stderr,"** ----------------------------------------------------------- **\n");
      fprintf(stderr,"** Unrecognized return value:%d from compress(). Proceeding as **\n" , compress_result);
      fprintf(stderr,"** if all is OK ??  Cross your fingers!!                       **\n");
      fprintf(stderr,"*****************************************************************\n");
      compress_result = Z_OK;

      printf("data_size:%d   compressed_size:%ld \n",data_size , *compressed_size);
      util_abort("%s - kkk \n", __func__ );
    }
    
    if (compress_result != Z_OK) 
      util_abort("%s: returned %d - different from Z_OK - aborting\n",__func__ , compress_result);
  } else
    *compressed_size = 0;
}



/**
   This function allocates a new buffer which is a compressed version
   of the input buffer data. The input variable data_size, and the
   output * compressed_size are the size - *in bytes* - of input and
   output.
*/
void * util_alloc_compressed_buffer(const void * data , int data_size , unsigned long * compressed_size) {
  void * zbuffer = util_malloc(data_size );
  *compressed_size = data_size;
  util_compress_buffer(data , data_size , zbuffer , compressed_size);
  zbuffer = util_realloc(zbuffer , *compressed_size );
  return zbuffer;
}


/**
Layout on disk when using util_fwrite_compressed:

  /-------------------------------
  |uncompressed total size
  |size of compression buffer
  |----
  |compressed size
  |compressed block
  |current uncompressed offset
  |....
  |compressed size
  |compressed block
  |current uncompressed offset
  |....
  |compressed size
  |compressed block
  |current uncompressed offset
  \------------------------------

Observe that the functions util_fwrite_compressed() and
util_fread_compressed must be used as a pair, the files can **N O T**
be interchanged with normal calls to gzip/gunzip. To avoid confusion
it is therefor strongly advised NOT to give the files a .gz extension.

*/

void util_fwrite_compressed(const void * _data , int size , FILE * stream) {
  if (size == 0) {
    fwrite(&size , sizeof size , 1 , stream);
    return;
  }
  {
    const char * data = (const char *) _data;
    const int max_buffer_size      = 128 * 1048580; /* 128 MB */
    int       required_buffer_size = (int) ceil(size * 1.001 + 64);
    int       buffer_size , block_size;
    void *    zbuffer;
    
    buffer_size = util_int_min(required_buffer_size , max_buffer_size);
    do {
      zbuffer = malloc(buffer_size);
      if (zbuffer == NULL)
        buffer_size /= 2;
    } while(zbuffer == NULL);
    memset(zbuffer , 0 , buffer_size);
    block_size = (int) (floor(buffer_size / 1.002) - 64);
    
    {
      int header_write;
      header_write  = fwrite(&size        , sizeof size        , 1 , stream);
      header_write += fwrite(&buffer_size , sizeof buffer_size , 1 , stream);
      if (header_write != 2)
        util_abort("%s: failed to write header to disk: %s \n",__func__ , strerror(errno));
    }
    
    {
      int offset = 0;
      do {
        unsigned long compressed_size = buffer_size;
        int this_block_size           = util_int_min(block_size , size - offset);
        util_compress_buffer(&data[offset] , this_block_size , zbuffer , &compressed_size);
        fwrite(&compressed_size , sizeof compressed_size , 1 , stream);
        {
          int bytes_written = fwrite(zbuffer , 1 , compressed_size , stream);
          if (bytes_written < compressed_size) 
            util_abort("%s: wrote only %d/%ld bytes to compressed file  - aborting \n",__func__ , bytes_written , compressed_size);
        }
        offset += this_block_size;
        fwrite(&offset , sizeof offset , 1 , stream);
      } while (offset < size);
    }
    free(zbuffer);
  }
}

/**
  This function is used to read compressed data from file, observe
  that the file must have been created with util_fwrite_compressed()
  first. Trying to read a file compressed with gzip will fail.
*/

void util_fread_compressed(void *__data , FILE * stream) {
  unsigned char * data = (unsigned char *) __data;
  int buffer_size;
  int size , offset;
  void * zbuffer;
  
  fread(&size  , sizeof size , 1 , stream); 
  if (size == 0) return;


  fread(&buffer_size , sizeof buffer_size , 1 , stream);
  zbuffer = util_malloc(buffer_size );
  offset = 0;
  do {
    unsigned long compressed_size;
    unsigned long block_size = size - offset;
    int uncompress_result;
    fread(&compressed_size , sizeof compressed_size , 1 , stream);
    {
      int bytes_read = fread(zbuffer , 1 , compressed_size , stream);
      if (bytes_read < compressed_size) 
        util_abort("%s: read only %d/%d bytes from compressed file - aborting \n",__func__ , bytes_read , compressed_size);
      
    }
    uncompress_result = uncompress(&data[offset] , &block_size , (const Bytef*)zbuffer , compressed_size);
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
    
    offset += block_size;
    {
      int file_offset;
      fread(&file_offset , sizeof offset , 1 , stream); 
      if (file_offset != offset) 
        util_abort("%s: something wrong when reding compressed stream - aborting \n",__func__);
    }
  } while (offset < size);
  free(zbuffer);
}



/**
   Allocates storage and reads in from compressed data from disk. If the
   data on disk have zero size, NULL is returned.
*/

void * util_fread_alloc_compressed(FILE * stream) {
  long   current_pos = util_ftell(stream);
  char * data;
  int    size;

  fread(&size  , sizeof size , 1 , stream); 
  if (size == 0) 
    return NULL;
  else {
    util_fseek(stream , current_pos , SEEK_SET);
    data = (char*)util_calloc(size , sizeof * data );
    util_fread_compressed(data , stream);
    return data;
  }
}


/**
   Returns the **UNCOMPRESSED** size of a compressed section. 
*/

int util_fread_sizeof_compressed(FILE * stream) {
  long   pos = ftell(stream);
  int    size;

  fread(&size  , sizeof size , 1 , stream); 
  util_fseek(  stream , pos , SEEK_SET );
  return size;
}




void util_fskip_compressed(FILE * stream) {
  int size , offset;
  int buffer_size;
  fread(&size        , sizeof size        , 1 , stream);
  if (size == 0) return;

  
  fread(&buffer_size , sizeof buffer_size , 1 , stream);
  do {
    unsigned long compressed_size;
    fread(&compressed_size , sizeof compressed_size , 1 , stream);
    util_fseek(stream  , compressed_size , SEEK_CUR);
    fread(&offset , sizeof offset , 1 , stream);
  } while (offset < size);
}

