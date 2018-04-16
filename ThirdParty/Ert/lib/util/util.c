/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'util.c' is part of ERT - Ensemble based Reservoir Tool.

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

/**
  This file contains a large number of utility functions for memory
  handling, string handling and file handling. Observe that all these
  functions are just that - functions - there is no associated state
  with any of these functions.

  The file util_path.c is included in this, and contains path
  manipulation functions which explicitly use the PATH_SEP variable.
*/

#include <string.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>

#include <fcntl.h>
#include <limits.h>

#include <ert/util/ert_api_config.h>
#include "ert/util/build_config.h"

#include <errno.h>

#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>

#ifdef HAVE_FNMATCH
#include <fnmatch.h>
#else
#include <Windows.h>
#include <Shlwapi.h>
#endif

#ifdef ERT_HAVE_SPAWN
#include <unistd.h>
#include <sys/wait.h>
#endif

#ifdef HAVE__USLEEP
#include <unistd.h>
#endif

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#ifdef HAVE_FTRUNCATE
#include <unistd.h>
#include <sys/types.h>
#else
#include <io.h>
#endif


#ifdef HAVE_WINDOWS_GETCWD
#include <direct.h>
#endif


#include <stdint.h>
#if UINTPTR_MAX == 0xFFFFFFFF
#define ARCH32
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFF
#define ARCH64
#else
#error "Could not determine if this is a 32 bit or 64 bit computer?"
#endif

#include <ert/util/util.h>
#include <ert/util/buffer.h>


/*
   Macros for endian flipping. The macros create a new endian-flipped
   value, and should be used as:

     flipped_value = FLIP32( value )

   The macros are not exported and only available through the function
   util_endian_flip_vector().
*/


#define FLIP16(var) (((var >> 8) & 0x00ff) | ((var << 8) & 0xff00))

#define FLIP32(var) (( (var >> 24) & 0x000000ff) | \
                      ((var >>  8) & 0x0000ff00) | \
                      ((var <<  8) & 0x00ff0000) | \
                      ((var << 24) & 0xff000000))

#define FLIP64(var)  (((var >> 56) & 0x00000000000000ff) | \
                      ((var >> 40) & 0x000000000000ff00) | \
                      ((var >> 24) & 0x0000000000ff0000) | \
                      ((var >>  8) & 0x00000000ff000000) | \
                      ((var <<  8) & 0x000000ff00000000) | \
                      ((var << 24) & 0x0000ff0000000000) | \
                      ((var << 40) & 0x00ff000000000000) | \
                      ((var << 56) & 0xff00000000000000))




static uint16_t util_endian_convert16( uint16_t u ) {
  return (( u >> 8U ) & 0xFFU) | (( u & 0xFFU) >> 8U);
}


static uint32_t util_endian_convert32( uint32_t u ) {
  const uint32_t m8  = (uint32_t) 0x00FF00FFUL;
  const uint32_t m16 = (uint32_t) 0x0000FFFFUL;

  u = (( u >> 8U ) & m8)   | ((u & m8) << 8U);
  u = (( u >> 16U ) & m16) | ((u & m16) << 16U);
  return u;
}


static uint64_t util_endian_convert64( uint64_t u ) {
  const uint64_t m8  = (uint64_t) 0x00FF00FF00FF00FFULL;
  const uint64_t m16 = (uint64_t) 0x0000FFFF0000FFFFULL;
  const uint64_t m32 = (uint64_t) 0x00000000FFFFFFFFULL;


  u = (( u >> 8U ) & m8)   | ((u & m8) << 8U);
  u = (( u >> 16U ) & m16) | ((u & m16) << 16U);
  u = (( u >> 32U ) & m32) | ((u & m32) << 32U);
  return u;
}


static uint64_t util_endian_convert32_64( uint64_t u ) {
  const uint64_t m8  = (uint64_t) 0x00FF00FF00FF00FFULL;
  const uint64_t m16 = (uint64_t) 0x0000FFFF0000FFFFULL;


  u = (( u >> 8U ) & m8)   | ((u & m8) << 8U);
  u = (( u >> 16U ) & m16) | ((u & m16) << 16U);
  return u;
}



void util_endian_flip_vector(void *data, int element_size , int elements) {
  int i;
  switch (element_size) {
  case(1):
    break;
  case(2):
    {
      uint16_t *tmp16 = (uint16_t *) data;

      for (i = 0; i <elements; i++)
        tmp16[i] = util_endian_convert16(tmp16[i]);
      break;
    }
  case(4):
    {
#ifdef ARCH64
      /*
        In the case of a 64 bit CPU the fastest way to swap 32 bit
        variables will be by swapping two elements in one operation;
        this is provided by the util_endian_convert32_64() function. In the case
        of binary ECLIPSE files this case is quite common, and
        therefore worth supporting as a special case.
      */
      uint64_t *tmp64 = (uint64_t *) data;

      for (i = 0; i <elements/2; i++)
        tmp64[i] = util_endian_convert32_64(tmp64[i]);

      if ( elements & 1 ) {
        // Odd number of elements - flip the last element as an ordinary 32 bit swap.
        uint32_t *tmp32 = (uint32_t *) data;
        tmp32[ elements - 1] = util_endian_convert32( tmp32[elements - 1] );
      }
      break;
#else
      uint32_t *tmp32 = (uint32_t *) data;

      for (i = 0; i <elements; i++)
        tmp32[i] = util_endian_convert32(tmp32[i]);

      break;
#endif
    }
  case(8):
    {
      uint64_t *tmp64 = (uint64_t *) data;

      for (i = 0; i <elements; i++)
        tmp64[i] = util_endian_convert64(tmp64[i]);
      break;
    }
  default:
    fprintf(stderr,"%s: current element size: %d \n",__func__ , element_size);
    util_abort("%s: can only endian flip 1/2/4/8 byte variables - aborting \n",__func__);
  }
}

void util_endian_flip_vector_old(void *data, int element_size , int elements) {
  int i;
  switch (element_size) {
  case(1):
    break;
  case(2):
    {
      uint16_t *tmp_int = (uint16_t *) data;

      for (i = 0; i <elements; i++)
        tmp_int[i] = FLIP16(tmp_int[i]);
      break;
    }
  case(4):
    {
      uint32_t *tmp_int = (uint32_t *) data;

      for (i = 0; i <elements; i++)
        tmp_int[i] = FLIP32(tmp_int[i]);

      break;
    }
  case(8):
    {
      uint64_t *tmp_int = (uint64_t *) data;

      for (i = 0; i <elements; i++)
        tmp_int[i] = FLIP64(tmp_int[i]);
      break;
    }
  default:
    fprintf(stderr,"%s: current element size: %d \n",__func__ , element_size);
    util_abort("%s: can only endian flip 1/2/4/8 byte variables - aborting \n",__func__);
  }
}

#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif


/*****************************************************************/


static bool EOL_CHAR(char c) {
  if (c == '\r' || c == '\n')
    return true;
  else
    return false;
}

#undef strncpy // This is for some reason needed in RH3

/*
  The difference between /dev/random and /dev/urandom is that the
  former will block if the entropy pool is close to empty:

    util_fread_dev_random() : The 'best' quality random numbers, but
       runtime can be quite long.

    util_fread_dev_urandom(): Potentially lower quality random
       numbers, but deterministic runtime.
*/

void util_fread_dev_random(int buffer_size , char * buffer) {
  FILE * stream = util_fopen("/dev/random" , "r");
  if (fread(buffer , 1 , buffer_size , stream) != buffer_size)
    util_abort("%s: failed to read:%d bytes from /dev/random \n",__func__ , buffer_size);

  fclose(stream);
}


void util_fread_dev_urandom(int buffer_size , char * buffer) {
  FILE * stream = util_fopen("/dev/urandom" , "r");
  if (fread(buffer , 1 , buffer_size , stream) != buffer_size)
    util_abort("%s: failed to read:%d bytes from /dev/random \n",__func__ , buffer_size);

  fclose(stream);
}


unsigned int util_dev_urandom_seed( ) {
  unsigned int seed;
  util_fread_dev_urandom( sizeof seed, (char*)&seed );
  return seed;
}

unsigned int util_clock_seed( ) {
  int sec,min,hour;
  int mday,year,month;
  time_t now = time( NULL );


  util_set_datetime_values_utc(now , &sec , &min , &hour , &mday , &month , &year);
  {
    unsigned int seed = clock( );
    int i,j,k;
    for (i=0; i < 2*min + 2; i++) {
      for (j=0; j < 13*mday + 17; j++) {
        for (k=0; k < (hour + year + 17) * month + 13; k++) {
          seed *= (sec + min + mday);
        }
      }
    }
    return seed;
  }
}


/**
   Kahan summation is a technique to retain numerical precision when
   summing a long vector of values. See:
   http://en.wikipedia.org/wiki/Kahan_summation_algorithm
*/

double util_kahan_sum(const double *data, size_t N) {
  double S = data[0];
  double C = 0;
  double Y,T;
  size_t i;

  for (i=1; i < N; i++) {
    Y = data[i] - C;
    T = S + Y;
    C = (T - S) - Y;
    S = T;
  }
  return S;
}


bool util_float_approx_equal__( float d1 , float d2, float rel_eps, float abs_eps) {
  if ((fabsf(d1) + fabsf(d2)) == 0)
    return true;
  else {
    float diff = fabsf(d1 - d2);
    if ((abs_eps > 0) && (diff > abs_eps))
      return false;
    {
      float sum  = fabsf(d1) + fabsf(d2);
      float rel_diff = diff / sum;

      if ((rel_eps > 0) && (rel_diff > rel_eps))
        return false;
    }
    return true;
  }
}


/*
  If an epsilon value is identically equal to zero that comparison
  will be ignored.
*/

bool util_double_approx_equal__( double d1 , double d2, double rel_eps, double abs_eps) {
  if ((fabs(d1) + fabs(d2)) == 0)
    return true;
  else {
    double diff = fabs(d1 - d2);
    if ((abs_eps > 0) && (diff > abs_eps))
      return false;
    {
      double sum  = fabs(d1) + fabs(d2);
      double rel_diff = diff / sum;

      if ((rel_eps > 0) && (rel_diff > rel_eps))
        return false;
    }
    return true;
  }
}


bool util_double_approx_equal( double d1 , double d2) {
  double epsilon = 1e-6;
  return util_double_approx_equal__( d1 , d2 , epsilon , 0.0 );
}


char * util_alloc_substring_copy(const char *src , int offset , int N) {
  char *copy;
  if ((N + offset) < strlen(src)) {
    copy = (char*)util_calloc(N + 1 , sizeof * copy );
    strncpy(copy , &src[offset] , N);
    copy[N] = '\0';
  } else
    copy = util_alloc_string_copy(&src[offset]);
  return copy;
}


char * util_alloc_dequoted_copy(const char *s) {
  char first_char = s[0];
  char last_char  = s[strlen(s) - 1];
  char *next;
  int offset , len;

  if ((first_char == '\'') || (first_char == '\"'))
    offset = 1;
  else
    offset = 0;

  if ((last_char == '\'') || (last_char == '\"'))
    len = strlen(s) - offset - 1;
  else
    len = strlen(s) - offset;

  next = util_alloc_substring_copy(s , offset , len);
  return next;
}



/**
   The input string is freed, and a new storage
   without quotes is returned.
*/
char * util_realloc_dequoted_string(char *s) {
  char first_char = s[0];
  char last_char  = s[strlen(s) - 1];
  char *next;
  int offset , len;

  if ((first_char == '\'') || (first_char == '\"'))
    offset = 1;
  else
    offset = 0;

  if ((last_char == '\'') || (last_char == '\"'))
    len = strlen(s) - offset - 1;
  else
    len = strlen(s) - offset;

  next = util_alloc_substring_copy(s , offset , len);
  free(s);
  return next;
}

void util_strupr(char *s) {
  int i;
  for (i=0; i < strlen(s); i++)
    s[i] = toupper(s[i]);
}


char * util_alloc_strupr_copy(const char * s) {
  char * c = util_alloc_string_copy(s);
  util_strupr(c);
  return c;
}


/**
    Replaces all occurences of c1 in s with c2.
*/
void util_string_tr(char * s, char c1, char c2) {
  int i;
  for (i=0; i < strlen(s);i++)
    if (s[i] == c1) s[i] = c2;
}


void util_rewind_line(FILE *stream) {
  bool at_eol = false;
  int c;

  do {
    if (util_ftell(stream) == 0)
      at_eol = true;
    else {
      util_fseek(stream , -1 , SEEK_CUR);
      c = fgetc(stream);
      at_eol = EOL_CHAR(c);
      if (!at_eol)
        util_fseek(stream , -1 , SEEK_CUR);
    }
  } while (!at_eol);
}



/**
   This function will reposition the stream pointer at the the first
   occurence of 'string'. If 'string' is found the function will
   return true, otherwise the function will return false, and stream
   pointer will be at the original position.

   If skip_string == true the stream position will be positioned
   immediately after the 'string', otherwise it will be positioned at
   the beginning of 'string'.

   If case_sensitive is true we require exact match with respect to
   case, otherwise we accept any case combination.
*/

bool util_fseek_string(FILE * stream , const char * __string , bool skip_string , bool case_sensitive) {
  bool string_found  = false;
  char * string      = util_alloc_string_copy(__string);

  if (!case_sensitive)
    util_strupr( string );
  {
    int len              = strlen( string );
    long int initial_pos = util_ftell( stream );   /* Store the inital position. */
    bool cont            = true;
    do {
      int c = fgetc( stream );
      if (!case_sensitive)
        c = toupper( c );

      if (c == string[0]) {  /* OK - we got the first character right - lets try in more detail: */
        long int current_pos  = util_ftell(stream);
        bool     equal        = true;
        int string_index;
        for (string_index = 1; string_index < len; string_index++) {
          c = fgetc( stream );
          if (!case_sensitive)
            c = toupper( c );

          if (c != string[string_index]) {
            equal = false;
            break;
          }
        }

        if (equal) {
          string_found = true;
          cont = false;
        } else /* Go back to current pos and continue searching. */
          util_fseek(stream , current_pos , SEEK_SET);

      }
      if (c == EOF)
        cont = false;
    } while (cont);


    if (string_found) {
      if (!skip_string) {
        offset_type offset = (offset_type) strlen(string);
        util_fseek(stream , -offset , SEEK_CUR); /* Reposition to the beginning of 'string' */
      }
    } else
      util_fseek(stream , initial_pos , SEEK_SET);       /* Could not find the string reposition at initial position. */

  }
  free( string );
  return string_found;
}



/**
  This function will allocate a character buffer, and read file
  content all the way up to 'stop_string'. If the stop_string is not
  found, the function will return NULL, and the file pointer will be
  unchanged.

  If include_stop_string is true the returned string will end with
  stop_string, and the file pointer will be positioned right AFTER
  stop_string, otherwise the file_pointer will be positioned right
  before stop_string.
*/



char * util_fscanf_alloc_upto(FILE * stream , const char * stop_string, bool include_stop_string) {
  long int start_pos = util_ftell(stream);
  if (util_fseek_string(stream , stop_string , include_stop_string , true)) {   /* Default case sensitive. */
    long int end_pos = util_ftell(stream);
    int      len     = end_pos - start_pos;
    char * buffer    = (char*)util_calloc( (len + 1) ,  sizeof * buffer );

    util_fseek(stream , start_pos , SEEK_SET);
    util_fread( buffer , 1 , len , stream , __func__);
    buffer[len] = '\0';

    return buffer;
  } else
    return NULL;   /* stop_string not found */
}



static char * util_fscanf_alloc_line__(FILE *stream , bool *at_eof , char * line) {
  int init_pos = util_ftell(stream);
  char * new_line;
  int len;
  char end_char;
  bool cont;
  bool dos_newline;

  len  = 0;
  cont = true;
  {
    char c;
    do {
      c = fgetc(stream);
      if (c == EOF)
        cont = false;
      else {
        if (EOL_CHAR(c))
          cont = false;
        else
          len++;
      }
    } while (cont);
    if (c == '\r')
      dos_newline = true;
    else
      dos_newline = false;
    end_char = c;
  }

  if (util_fseek(stream , init_pos , SEEK_SET) != 0)
    util_abort("%s: fseek failed: %d/%s \n",__func__ , errno , strerror(errno));

  new_line = (char*)util_realloc(line , len + 1 );
  util_fread(new_line , sizeof * new_line , len , stream , __func__);
  new_line[len] = '\0';


  /*
    Skipping the end of line marker(s).
  */

  fgetc(stream);
  if (dos_newline)
    fgetc(stream);

  if (at_eof != NULL) {
    if (end_char == EOF)
      *at_eof = true;
    else
      *at_eof = false;
  }

  if (new_line != NULL) {
    char * strip_line = util_alloc_strip_copy(new_line);
    free(new_line);

    return strip_line;
  } else
    return NULL;
}


char * util_fscanf_alloc_line(FILE *stream , bool *at_eof) {
  return util_fscanf_alloc_line__(stream , at_eof , NULL);
}


char * util_fscanf_realloc_line(FILE *stream , bool *at_eof , char *line) {
  return util_fscanf_alloc_line__(stream , at_eof , line);
}




/**
   WIndows does not have the usleep() function, on the other hand
   Sleep() function in windows has millisecond resolution, instead of
   seconds as in linux.
*/

void util_usleep( unsigned long micro_seconds ) {
#ifdef HAVE__USLEEP
  usleep( micro_seconds );
#else
  #ifdef ERT_WINDOWS
  {
    int milli_seconds = micro_seconds / 1000;
    Sleep( milli_seconds );
  }
#endif
#endif
}

void util_yield() {
#if defined(WITH_PTHREAD) && (defined(HAVE_YIELD_NP) || defined(HAVE_YIELD))
  #ifdef HAVE_YIELD_NP
    pthread_yield_np();
  #else
    #ifdef HAVE_YIELD
    pthread_yield();
    #endif
  #endif
#else
  util_usleep(1000);
#endif
}

static char * util_getcwd(char * buffer , int size) {
#ifdef HAVE_POSIX_GETCWD
  return getcwd( buffer , size );
#endif

#ifdef HAVE_WINDOWS_GETCWD
  return _getcwd( buffer , size );
#endif
}


char * util_alloc_cwd(void) {
  char * result_ptr;
  char * cwd;
  int buffer_size = 128;
  do {
    cwd = (char*)util_calloc(buffer_size , sizeof * cwd );
    result_ptr = util_getcwd(cwd , buffer_size - 1);
    if (result_ptr == NULL) {
      if (errno == ERANGE) {
        buffer_size *= 2;
        free(cwd);
      }
    }
  } while ( result_ptr == NULL );
  cwd = (char*)util_realloc(cwd , strlen(cwd) + 1 );
  return cwd;
}



bool util_is_cwd( const char * path ) {
  bool is_cwd = false;
  stat_type path_stat;

  if (util_stat(path , &path_stat) == 0) {
    if (S_ISDIR( path_stat.st_mode )) {
      char * cwd = util_alloc_cwd();
#ifdef ERT_WINDOWS
      /*
        The windows stat structure has the inode element, but it is
        not set. Actually - this is a property of the filesystem, and
        not the operating system - the whole check is probably broken?
      */
      util_abort("%s: Internal error - function not properly implmented on Windows \n",__func__);
#else
      stat_type cwd_stat;
      util_stat(cwd , &cwd_stat);
      if (cwd_stat.st_ino == path_stat.st_ino)
        is_cwd = true;
#endif
      free( cwd );
    }
  }
  return is_cwd;
}



/*
   Homemade realpath() for not existing path or platforms without
   realpath().
*/


static char * util_alloc_cwd_abs_path( const char * path ) {
  if (util_is_abs_path( path ))
    return util_alloc_string_copy( path );
  else {
    char * cwd       = util_alloc_cwd( );
    char * abs_path  = (char*)util_realloc( cwd , strlen( cwd ) + 1 + strlen( path ) + 1 );
    strcat( abs_path , UTIL_PATH_SEP_STRING );
    strcat( abs_path , path );
    return abs_path;
  }
}



/**
   Manual realpath() implementation to be used on platforms without
   realpath() support. Will remove /../, ./ and extra //. Will not
   handle symlinks.
*/


#define BACKREF ".."
#define CURRENT "."

char * util_alloc_realpath__(const char * input_path) {
  char * abs_path  = util_alloc_cwd_abs_path( input_path );
  char * real_path = (char*)util_malloc( strlen(abs_path) + 2 );
  real_path[0] = '\0';


  {
    char ** path_list;
    char ** path_stack;
    int     path_len;

    util_path_split( abs_path , &path_len , &path_list );
    path_stack = util_malloc( path_len * sizeof * path_stack );
    for (int i=0; i < path_len; i++)
      path_stack[i] = NULL;

    {
      int stack_size = 0;

      for (int path_index=0; path_index < path_len; path_index++) {
        const char * path_elm = path_list[path_index];

         if (strcmp( path_elm , CURRENT) == 0)
          continue;

        /* Backref - pop from stack. */
        if (strcmp(path_elm , BACKREF ) == 0) {
          if (stack_size > 0) {
            memmove(path_stack, &path_stack[1] , (stack_size - 1) * sizeof * path_stack);
            stack_size--;
          }
          continue;
        }

        /* Normal path element - push onto stack. */
        memmove(&path_stack[1], path_stack, stack_size * sizeof * path_stack);
        path_stack[0] = path_elm;
        stack_size++;
      }

      /* Build up the new string. */
      if (stack_size > 0) {
        for (int pos = stack_size - 1; pos >= 0; pos--) {
          const char * path_elm = path_stack[pos];
          if (pos == (stack_size- 1)) {
#ifdef ERT_WINDOWS
            // Windows:
            //   1) If the path starts with X: - just do nothing
            //   2) Else add \\ - for a UNC path.
            if (path_elm[1] != ':') {
              strcat(real_path, UTIL_PATH_SEP_STRING);
              strcat(real_path, UTIL_PATH_SEP_STRING);
            }
#else
            // Posix: just start with a leading '/'
            strcat(real_path, UTIL_PATH_SEP_STRING);
#endif
            strcat( real_path , path_elm);
          } else {
            strcat(real_path, UTIL_PATH_SEP_STRING);
            strcat(real_path , path_elm);
          }
        }
      }
    }
    free( path_stack );
    util_free_stringlist( path_list , path_len );
  }

  free(abs_path);
  return real_path;
}

#undef BACKREF
#undef CURRENT



/**
   The util_alloc_realpath() will fail hard if the @input_path does
   not exist. If the path might-not-exist you should use
   util_alloc_abs_path() instead.
*/


char * util_alloc_realpath(const char * input_path) {
#ifdef HAVE_REALPATH
  char * buffer   = (char*)util_calloc(PATH_MAX + 1 , sizeof * buffer );
  char * new_path = NULL;

  new_path = realpath( input_path , buffer);
  if (new_path == NULL)
    util_abort("%s: input_path:%s - failed: %s(%d) \n",__func__ , input_path , strerror(errno) , errno);
  else
    new_path = (char*)util_realloc(new_path , strlen(new_path) + 1);

  return new_path;
#else
  /* We do not have the realpath() implementation. Must first check if
     the entry exists; and if not we abort. If the entry indeed exists
     we call the util_alloc_cwd_abs_path() function: */
#ifdef ERT_HAVE_SYMLINK
  ERROR - What the fuck; have symlinks and not realpath()?!
#endif
  if (!util_entry_exists( input_path ))
    util_abort("%s: input_path:%s does not exist - failed.\n",__func__ , input_path);

  return util_alloc_realpath__( input_path );
#endif
}




/**
   If @path points to an existing entry the standard realpath()
   function is used otherwise an absolute path is created after the
   following simple algorithm:

    1. If @path starts with '/' the path is assumed to be absolute, and
       just returned.

    2. Else cwd is prepended to the path.

   In the manual realpath() neither "/../" nor symlinks are resolved. If path == NULL the function will return cwd().
*/

char * util_alloc_abs_path( const char * path ) {
  if (path == NULL)
    return util_alloc_cwd();
  else {
    if (util_entry_exists( path ))
      return util_alloc_realpath( path );
    else
      return util_alloc_cwd_abs_path( path );
  }
}



/**
   Both path arguments must be absolute paths; if not a copy of the
   input path will be returned. Neither of the input arguments can
   have "/../" elements - that will just fuck things up.

   root_path can be NULL - in which case cwd is used.
*/

char * util_alloc_rel_path( const char * __root_path , const char * path) {
  char * root_path;
  if (__root_path == NULL)
    root_path = util_alloc_cwd();
  else
    root_path = util_alloc_string_copy( __root_path );

  if (util_is_abs_path(root_path) && util_is_abs_path(path)) {
    const char * back_path = "..";
    char * rel_path = util_alloc_string_copy("");  // In case strcmp(root_path , path) == 0 the empty string "" will be returned
    char ** root_path_list;
    char ** path_list;
    int     root_path_length , path_length , back_length;

    /* 1. Split both input paths into list of path elements. */
    util_path_split( root_path, &root_path_length  , &root_path_list );
    util_path_split( path     , &path_length       , &path_list      );

    {
      /* 2: Determine the number of common leading path elements. */
      int common_length = 0;
      while (true) {
        if (strcmp(root_path_list[common_length] , path_list[common_length]) == 0)
          common_length++;
        else
          break;

        if (common_length == util_int_min( root_path_length , path_length))
          break;
      }

      /* 3: Start building up the relative path with leading ../ elements. */
      back_length = root_path_length - common_length;
      if (back_length > 0) {
        int i;
        for (i=0; i < back_length; i++) {
          rel_path = util_strcat_realloc( rel_path , back_path );
          rel_path = util_strcat_realloc( rel_path , UTIL_PATH_SEP_STRING );
        }
      }

      /* 4: Add the remaining elements from the input path. */
      {
        int i;
        for (i=common_length; i < path_length; i++) {
          rel_path = util_strcat_realloc( rel_path , path_list[i] );
          if (i != (path_length - 1))
            rel_path = util_strcat_realloc( rel_path , UTIL_PATH_SEP_STRING );
        }
      }
    }

    util_free_stringlist( root_path_list , root_path_length );
    util_free_stringlist( path_list , path_length );
    free( root_path );

    return rel_path;
  } else {
    /*
       One or both the input arguments do not correspond to an
       absolute path; just return a copy of the input back.
    */
    free( root_path );
    return util_alloc_string_copy( path );
  }
}

/*
  This function will return a new string where all "../" and "./"
  occurences have been normalized away. The function is based on pure
  string scanning, and will not consider the filesystem at
  all.
*/

char * util_alloc_normal_path( const char * input_path ) {
  if (util_is_abs_path(input_path))
    return util_alloc_realpath__( input_path );

  char * realpath = util_alloc_realpath__(input_path);
  return util_alloc_rel_path( NULL , realpath );
}









void util_fskip_lines(FILE * stream , int lines) {
  bool cont = true;
  int line_nr = 0;
  do {
    bool at_eof = false;
    char c;
    do {
      c = fgetc(stream);
      if (c == EOF)
        at_eof = true;
    } while (c != '\r' && c != '\n' && !at_eof);

    /*
       If we have read a \r this is quite probably a DOS formatted
       file, and we read another character in the anticipation that it
       is a \n character.
    */
    if (c == '\r') {
      c = fgetc( stream );
      if (c == EOF)
        at_eof = true;
      else {
        if (c != '\n')
          util_fseek( stream , -1 , SEEK_CUR );
      }
    }

    line_nr++;
    if (line_nr == lines || at_eof) cont = false;
  } while (cont);
}


/*
  The last line(s) without content are not counted, i.e.

  File:
  ----------
  |Line1
  |Line2
  |
  |Line4
  |empty1
  |empty2
  |empty3

  will return a value of four.
*/

int util_forward_line(FILE * stream , bool * at_eof) {
  bool at_eol = false;
  int col = 0;
  *at_eof     = false;

  do {
    char c = fgetc(stream);
    if (c == EOF) {
      *at_eof = true;
      at_eol  = true;
    } else {
      if (EOL_CHAR(c)) {
        at_eol = true;
        c = fgetc(stream);
        if (c == EOF)
          *at_eof = true;
        else {
          if (!EOL_CHAR(c))
            util_fseek(stream , -1 , SEEK_CUR);
        }
      } else
        col++;
    }
  } while (!at_eol);
  return col;
}



bool util_char_in(char c , int set_size , const char * set) {
  int i;
  bool in = false;
  for (i=0; i < set_size; i++)
    if (set[i] == c)
      in = true;

  return in;
}


/**
    Returns true if ALL the characters in 's' return true from
    isspace().
*/
bool util_string_isspace(const char * s) {
  int  index = 0;
  while (index < strlen(s)) {
    if (!isspace( s[index] ))
      return false;
    index++;
  }
  return true;
}


void util_fskip_token(FILE * stream) {
  char * tmp = util_fscanf_alloc_token(stream);
  if (tmp != NULL)
    free(tmp);
}



static void util_fskip_chars__(FILE * stream , const char * skip_set , bool complimentary_set , bool *at_eof) {
  bool cont     = true;
  do {
    int c = fgetc(stream);
    if (c == EOF) {
      *at_eof = true;
      cont = false;
    } else {
      if (strchr(skip_set , c) == NULL) {
        /* c is not in skip_set */
        if (!complimentary_set)
          cont = false;
      } else {
        /* c is in skip_set */
        if (complimentary_set)
          cont = false;
      }
    }
  } while (cont);
  if (!*at_eof)
    util_fseek(stream , -1 , SEEK_CUR);
}


void util_fskip_chars(FILE * stream , const char * skip_set , bool *at_eof) {
  util_fskip_chars__(stream , skip_set , false , at_eof);
}

void util_fskip_cchars(FILE * stream , const char * skip_set , bool *at_eof) {
  util_fskip_chars__(stream , skip_set , true , at_eof);
}


static void util_fskip_space__(FILE * stream , bool complimentary_set , bool *at_eof) {
  bool cont     = true;
  do {
    int c = fgetc(stream);
    if (c == EOF) {
      *at_eof = true;
      cont = false;
    } else {
      if (!isspace(c)) {
        /* c is not in space set. */
        if (!complimentary_set)
          cont = false;
      } else {
        /* c is in skip_set */
        if (complimentary_set)
          cont = false;
      }
    }
  } while (cont);
  if (!*at_eof)
    util_fseek(stream , -1 , SEEK_CUR);
}


void util_fskip_space(FILE * stream ,  bool *at_eof) {
  util_fskip_space__(stream , false , at_eof);
}




/**
    This functions reads a token[1] from stream, allocates storage for
    the it, and returns the newly allocated storage. Observe that the
    function does *NOT* read past end of line. If no token can be the
    function will return NULL.

    Example:
    --------

    File:
     ________________
    /
    | This is a file
    | Line2
    | Line3
    \________________


   bool at_eof = 0;
   char * token;
   while (!at_eof) {
      token = util_fscanf_alloc_token(stream);
      if (token != NULL) {
         printf("Have read token:%s \n",token);
         free(token);
      } else {
         printf("Have reached EOL/EOF \n");
         util_forward_line(stream , &at_eof);
      }
   }

   This will produce the output:
   Have read token: This
   Have read token: is
   Have read token: a
   Have read token: file
   Have reached EOL/EOF
   Have read token: Line2
   Have reached EOL/EOF
   Have read token: Line3
   Have reached EOL/EOF


   [1]: A token is defined as a sequence of characters separated by
    white-space or tab.
*/




char * util_fscanf_alloc_token(FILE * stream) {
  const char * space_set = " \t";
  bool cont;
  char * token = NULL;
  char c;


  cont = true;

  /* Skipping initial whitespace */
  do {
    int pos = util_ftell(stream);
    c = fgetc(stream);
    if (EOL_CHAR(c)) {
      /* Going back to position at newline */
      util_fseek(stream , pos , SEEK_SET);
      cont = false;
    } else if (c == EOF)
      cont = false;
    else if (!util_char_in(c , 2 , space_set)) {
      util_fseek(stream , pos , SEEK_SET);
      cont = false;
    }
  } while (cont);
  if (EOL_CHAR(c)) return NULL;
  if (c == EOF)    return NULL;



  /* At this point we are guranteed to return something != NULL */
  cont = true;
  {
    int length = 0;
    long int token_start = util_ftell(stream);

    do {
      c = fgetc(stream);
      if (c == EOF)
        cont = false;
      else if (EOL_CHAR(c))
        cont = false;
      else if (util_char_in(c , 2 , space_set))
        cont = false;
      else
        length++;
    } while (cont);
    if (EOL_CHAR(c)) util_fseek(stream , -1 , SEEK_CUR);

    token = (char*)util_calloc(length + 1 , sizeof * token );
    util_fseek(stream , token_start , SEEK_SET);
    {
      int i;
      for (i = 0; i < length; i++)
        token[i] = fgetc(stream);
      token[length] = '\0';
    }
  }
  return token;
}



/**
  This function parses a string literal (hopfully) containing a
  represantation of a double. The return value is true|false depending
  on the success of the parse operation, the parsed value is returned
  by reference.


  Example:
  --------
  const char * s = "78.92"
  double value;

  if (util_sscanf_double(s , &value))
    printf("%s is a valid double\n");
  else
    printf("%s is NOT a valid double\n");

*/

bool util_sscanf_double(const char * buffer , double * value) {
  if(!buffer)
    return false;

  bool value_OK = false;
  char * error_ptr;

  double tmp_value = strtod(buffer , &error_ptr);
  /*
    Skip trailing white-space
  */
  while (error_ptr[0] != '\0' && isspace(error_ptr[0]))
    error_ptr++;

  if (error_ptr[0] == '\0') {
    value_OK = true;
    if (value != NULL)
      *value = tmp_value;
  }
  return value_OK;
}


/**
   Base 8
*/

bool util_sscanf_octal_int(const char * buffer , int * value) {
  if(!buffer)
    return false;

  bool value_OK = false;
  char * error_ptr;

  int tmp_value = strtol(buffer , &error_ptr , 8);

  /*
    Skip trailing white-space
  */

  while (error_ptr[0] != '\0' && isspace(error_ptr[0]))
    error_ptr++;

  if (error_ptr[0] == '\0') {
    value_OK = true;
    if (value != NULL)
      *value = tmp_value;
  }
  return value_OK;
}



/**
   Takes a char buffer as input, and parses it as an integer. Returns
   true if the parsing succeeded, and false otherwise. If parsing
   succeeded, the integer value is returned by reference.
*/

bool util_sscanf_int(const char * buffer , int * value) {
  if(!buffer)
      return false;

  bool value_OK = false;
  char * error_ptr;

  int tmp_value = strtol(buffer , &error_ptr , 10);

  /*
    Skip trailing white-space
  */

  while (error_ptr[0] != '\0' && isspace(error_ptr[0]))
    error_ptr++;

  if (error_ptr[0] == '\0') {
    value_OK = true;
    if(value != NULL)
      *value = tmp_value;
  }
  return value_OK;
}

/*
  If any (or both) s1 or s2 is equal to NULL - the function will
  return false.
*/

bool util_string_equal(const char * s1 , const char * s2 ) {
  if (s1 == NULL || s2 == NULL)
    return false;

  /* Now we know that BOTH s1 and s2 are != NULL */
  if (strcmp(s1,s2) == 0)
    return true;
  else
    return false;
}



/**
   This function parses the string 's' for an integer. The return
   value is a pointer to the first character which is not an integer
   literal, or NULL if the whole string is read.

   The actual integer value is returned by reference. In addition a
   bool 'OK' is returned by reference, observe that that the bool OK
   is checked on function entry, and must point to true then.

   The somewhat contrived interface is to facilitate repeated calls on
   the same string to get out all the integers, typically to be used
   together with util_skip_sep().

   Example
   -------

   s = "1, 10, 78, 67";
         |   |   |   NULL
         1   2   3   3

   The vertical bars indicate the return values.

*/


const char * util_parse_int(const char * s , int * value, bool *OK) {
  if (*OK) {
    char * error_ptr;
    *value = strtol(s , &error_ptr , 10);
    if (error_ptr == s) *OK = false;
    return error_ptr;
  } else
    return NULL;
}



/**
   This function will skip the characters in s which are in the string
   'sep_set' and return a pointer to the first character NOT in
   'sep_set'; this is basically strspn() functionality. But it will
   update a reference bool OK if no characters are skipped -
   i.e. there should be some characters to skip. Typically used
   together with util_parse_int():


   Example
   -------

   const char * s = "1, 6 , 79 , 89 , 782";
   const char * current_ptr = s;
   bool OK = true;
   while (OK) {
      int value;
      current_ptr = util_parse_int(current_ptr , &value , &OK);
      if (OK)
         printf("Found:%d \n",value);
      current_ptr = util_skip_sep(current_ptr , " ," , &OK);
   }


*/

const char * util_skip_sep(const char * s, const char * sep_set, bool *OK) {
  if (*OK) {
    int sep_length = strspn(s , sep_set);
    if (sep_length == 0)
      *OK = false;
    return &s[sep_length];
  } else
    return NULL;
}





/**
   This function will parse string containing an integer, and an
   optional suffix. The valid suffixes are KB,MB and GB (any case is
   allowed); if no suffix is appended the buffer is assumed to contain
   a memory size already specified in bytes.


   Observe that __this_function__ allows an arbitrary number of spaces
   between between the integer literal and the suffix string; however
   this might be tricky when parsing. It is probably best to disallow
   these spaces?

   "1GB", "1 GB", "1    gB"

   are all legitimate. The universal factor used is 1024:

      KB => *= 1024
      MB => *= 1024 * 1024;
      GB => *= 1024 * 1024 * 1024;

   Observe that if the functions fails to parse/interpret the string
   it will return false, and set the reference value to 0. However it
   will not fail with an abort. Overflows are *NOT* checked for.
*/


bool util_sscanf_bytesize(const char * buffer, size_t *size) {
  if(!buffer) {
    if(size)
      *size = 0;
    return false;
  }

  size_t value;
  char * suffix_ptr;
  size_t KB_factor = 1024;
  size_t MB_factor = 1024 * 1024;
  size_t GB_factor = 1024 * 1024 * 1024;
  size_t factor    = 1;
  bool   parse_OK  = true;

  value = strtol(buffer , &suffix_ptr , 10);
  if (suffix_ptr[0] != '\0') {
    while (isspace(suffix_ptr[0]))
      suffix_ptr++;
    {
      char * upper = util_alloc_strupr_copy(suffix_ptr);
      if (strcmp(upper,"KB") == 0)
        factor = KB_factor;
      else if (strcmp(upper,"MB") == 0)
        factor = MB_factor;
      else if (strcmp(upper , "GB") == 0)
        factor = GB_factor;
      else
        parse_OK = false;
      /* else - failed to parse - returning false. */
      free(upper);
    }
  }

  if (size != NULL) {
    if (parse_OK)
      *size = value * factor;
    else
      *size = 0;
  }

  return parse_OK;
}


/**
   Checks if c is in the character class: [-0-9], and optionally "."
*/

static int isnumeric( int c , bool float_mode) {
  if (isdigit( c ))
    return true;
  else {
    if (c == '-')
      return true;
    else if (float_mode && (c == '.'))
      return true;
    else
      return false;
  }
}

/**
   Will compare two strings by using a mix of ordinary strcmp() and
   numerical comparison. The algorithm works like this:

   while (equal) {
      1. Scan both characters until character difference is found:
      2. Both characters numeric?

           Yes: use strtol()/strtod() to read embedded numeric string,
                and do a numerical comparison.

           No:  normal strcmp( ) of the two strings.

       Whether a character is numeric is tested with isnumeric()
       function.
   }

   Caveats:

     o Algorithm will not interpret "-" as a minus sign, only as a
       character. This might lead to unexpected results if you want to
       compare strings with embedded signed numeric literals, see
       example three below.

   Examples:

   S1           S2            util_strcmp_numeric()    strcmp()
   ------------------------------------------------------------
   "001"        "1"           0                        -1
   "String-10"  "String-3"    1                        -1
   "String-8"   "String1"     -1                       -1
   ------------------------------------------------------------
*/


static int util_strcmp_numeric__( const char * s1 , const char * s2, bool float_mode) {
  /*
     Special case to handle e.g.

         util_strcmp_numeric("100.00000" , "100")

     which should compare as equal. The normal algorithm will compare
     equal characterwise all the way until one string ends, and the
     shorter string will be returned as less than the longer - without
     ever invoking the numerical comparison.
  */
  {
    double num1 , num2;
    char * end1;
    char * end2;

    if (float_mode) {
      num1 = strtod( s1 , &end1 );
      num2 = strtod( s2 , &end2 );
    } else {
      num1 = 1.0 * strtol( s1 , &end1  , 10);
      num2 = 1.0 * strtol( s2 , &end2  , 10);
    }

    if ( (*end2 == '\0') && (*end1 == '\0')) {
      /* The whole string has been read and converted to a number - for both strings. */
      if (num1 < num2)
        return -1;
      else if (num1 > num2)
        return 1;
      else
        return 0;
    }
  }


  /*
     At least one of the strings is not a pure numerical literal and
     we start on a normal comparison.
  */
  {
    int cmp       = 0;
    bool complete = false;
    int  len1     = strlen( s1 );
    int  len2     = strlen( s2 );
    int  offset1  = 0;
    int  offset2  = 0;


    while (!complete) {
      while( true ) {
        if ((offset1 == len1) || (offset2 == len2)) {
          /* OK - at least one of the strings is at the end;
             we set the cmp value and return. */

          if ((offset1 == len1) && (offset2 == len2))
            /* We are the the end of both strings - they compare as equal. */
            cmp = 0;
          else if (offset1 == len1)
            cmp = -1;  // s1 < s2
          else
            cmp = 1;   // s1 > s2
          complete = true;
          break;
        }

        if (s1[offset1] != s2[offset2])
          /* We have reached a point of difference, jump out of this
             loop and continue with more detailed comparison further
             down. */
          break;

        // Strings are still equal - proceed one character further.
        offset1++;
        offset2++;
      }

      if (!complete) {
        /*
           Both offset values point to a valid character value, but the
           two character values are different:

           1. Both characters are numeric - we use strtol() to read the
              two integers and perform a numeric comparison.

           2. One or none of the characters are numeric, we just use
              ordinary strcmp() on the substrings starting at the current
              offset.
        */

        if (isnumeric( s1[ offset1 ] , float_mode) && isnumeric( s2[ offset2 ] , float_mode)) {
          char * end1;
          char * end2;
          double num1,num2;
          /*
             The numerical comparison is based on double variables
             irrespective of the value of @float_mode.
          */
          if (float_mode) {
            num1 = strtod( &s1[ offset1 ] , &end1 );
            num2 = strtod( &s2[ offset2 ] , &end2 );
          } else {
            num1 = 1.0 * strtol( &s1[ offset1 ] , &end1  , 10);
            num2 = 1.0 * strtol( &s2[ offset2 ] , &end2  , 10);
          }

          if (num1 != num2) {
            if (num1 < num2)
              cmp = -1;
            else
              cmp = 1;
            complete = true;
          } else {
            /* The two numeric values are identical and we must
               continue. Start by using the end pointers to determine
               new offset values. If the end pointers are NULL that
               means the whole string has been devoured, and we can set
               the offset to point to the end of the string.
            */
            if (end1 == NULL)
              offset1 = len1;
            else
              offset1 = end1 - s1;

            if (end2 == NULL)
              offset2 = len2;
            else
              offset2 = end2 - s2;
          }
        } else {
          /*
             We have arrived at a point of difference in the string, and
             perform 100% standard strcmp().
          */
          cmp = strcmp( &s1[offset1] , &s2[offset2] );
          complete = true;
        }
      }
    }
    return cmp;
  }
}

/**
   Will interpret XXXX.yyyy as a fractional number.
*/
int util_strcmp_float( const char * s1 , const char * s2) {
  return util_strcmp_numeric__( s1 , s2 , true );
}


/*
   Will interpret XXXX.yyyy as two integers.
*/

int util_strcmp_int( const char * s1 , const char * s2) {
  return util_strcmp_numeric__( s1 , s2 , false );
}



/**
    Successfully parses:

      1 , T (not 't') , True (with any case) => true
      0 , F (not 'f') , False(with any case) => false

    Otherwise, set _value to false and return false.
*/
bool util_sscanf_bool(const char * buffer , bool * _value) {
  if(!buffer) {
    if(_value)
      *_value = false;
    return false;
  }

  bool parse_OK = false;
  bool value    = false; /* Compiler shut up */

  if (strcmp(buffer,"1") == 0) {
    parse_OK = true;
    value = true;
  } else if (strcmp(buffer , "0") == 0) {
    parse_OK = true;
    value = false;
  } else if (strcmp(buffer , "T") == 0) {
    parse_OK = true;
    value = true;
  } else if (strcmp(buffer , "F") == 0) {
    parse_OK = true;
    value = false;
  } else {
    char * local_buffer = util_alloc_string_copy(buffer);
    util_strupr(local_buffer);

    if (strcmp(local_buffer , "TRUE") == 0) {
      parse_OK = true;
      value = true;
    } else if (strcmp(local_buffer , "FALSE") == 0) {
      parse_OK = true;
      value = false;
    }

    free(local_buffer);
  }
  if (_value != NULL)
    *_value = value;
  return parse_OK;
}


bool util_fscanf_bool(FILE * stream , bool * value) {
  char buffer[256];
  if (fscanf(stream , "%s" , buffer) == 1)
    return util_sscanf_bool( buffer , value );

  return false;
}


/**
   Takes a stream as input. Reads one string token from the stream,
   and tries to interpret the token as an integer with the function
   util_sscanf_int(). Returns true if the parsing succeeded, and false
   otherwise. If parsing succeded, the integer value is returned by
   reference.

   If the parsing fails the stream is repositioned at the location it
   had on entry to the function.
*/


bool util_fscanf_int(FILE * stream , int * value) {
  long int start_pos = util_ftell(stream);
  char * token       = util_fscanf_alloc_token(stream);

  bool   value_OK = false;
  if (token != NULL) {
    value_OK = util_sscanf_int(token , value);
    if (!value_OK)
      util_fseek(stream , start_pos , SEEK_SET);
    free(token);
  }
  return value_OK;
}










/**
   This function counts the number of lines from the current position
   in the file, to the end of line. The file pointer is repositioned
   at the end of the counting.
*/


int util_count_file_lines(FILE * stream) {
  long int init_pos = util_ftell(stream);
  int lines = 0;
  bool at_eof = false;
  do {
    int col = util_forward_line(stream , &at_eof);
    if (col > 0) lines++;
  } while (!at_eof);
  util_fseek(stream , init_pos , SEEK_SET);
  return lines;
}


int util_count_content_file_lines(FILE * stream) {
  int lines       = 0;
  int empty_lines = 0;
  int col         = 0;
  char    c;

  do {
    c = fgetc(stream);
    if (EOL_CHAR(c)) {
      if (col == 0)
        empty_lines++;
      else {
        lines       += empty_lines + 1;
        empty_lines  = 0;
      }
      col = 0;
      c = fgetc(stream);
      if (! feof(stream) ) {
        if (!EOL_CHAR(c)){
          util_fseek(stream , -1 , SEEK_CUR);
        }
      }else if (c == EOF){
        lines++;
      }

    } else if (c == EOF){
      lines++;
    }
    else {
      if (c != ' ')
        col++;
    }
  } while (! feof(stream) );
  if (col == 0)
    /*
      Not counting empty last line.
    */
    lines--;
  return lines;
}


/******************************************************************/


/**
    buffer_size is _only_ for a return (by reference) of the size of
    the allocation. Can pass in NULL if that size is not interesting.
*/

char * util_fread_alloc_file_content(const char * filename , int * buffer_size) {
  size_t file_size = util_file_size(filename);
  char * buffer = (char*)util_calloc(file_size + 1 , sizeof * buffer );
  {
    FILE * stream = util_fopen(filename , "r");
    util_fread( buffer , 1 , file_size , stream , __func__);
    fclose(stream);
  }
  if (buffer_size != NULL) *buffer_size = file_size;

  buffer[file_size] = '\0';
  return buffer;
}


/**
   If abort_on_error = true the function will abort if the read/write
   fails (although the write will try the disk_full hack). If
   abort_on_error == false the function will just return false if the
   write fails, in this case the calling scope must do the right
   thing.
*/


bool util_copy_stream(FILE *src_stream , FILE *target_stream , size_t buffer_size , void * buffer , bool abort_on_error) {
  while ( ! feof(src_stream)) {
    int bytes_read;
    int bytes_written;
    bytes_read = fread (buffer , 1 , buffer_size , src_stream);

    if (bytes_read < buffer_size && !feof(src_stream)) {
      if (abort_on_error)
        util_abort("%s: error when reading from src_stream - aborting \n",__func__);
      else
        return false;
    }

    if (abort_on_error)
      util_fwrite( buffer , 1 , bytes_read , target_stream , __func__);
    else {
      bytes_written = fwrite(buffer , 1 , bytes_read , target_stream);
      if (bytes_written < bytes_read)
        return false;
    }
  }

  return true;
}


bool util_copy_file__(const char * src_file , const char * target_file, size_t buffer_size , void * buffer , bool abort_on_error) {
  if (util_same_file(src_file , target_file)) {
    fprintf(stderr,"%s Warning: trying to copy %s onto itself - nothing done\n",__func__ , src_file);
    return false;
  } else {
    {
      FILE * src_stream      = util_fopen(src_file     , "r");
      FILE * target_stream   = util_fopen(target_file  , "w");
      bool result = util_copy_stream(src_stream , target_stream , buffer_size , buffer , abort_on_error);

      fclose(src_stream);
      fclose(target_stream);

#ifdef HAVE_CHMOD
#ifdef HAVE_MODE_T
      {
        stat_type stat_buffer;
        mode_t src_mode;

        stat( src_file , &stat_buffer );
        src_mode = stat_buffer.st_mode;
        chmod( target_file , src_mode );
      }
#endif
#endif

      return result;
    }
  }
}



bool util_copy_file(const char * src_file , const char * target_file) {
  const bool abort_on_error = true;
  void * buffer   = NULL;
  size_t buffer_size = util_size_t_max( 32 , util_file_size(src_file) );  /* The copy stream function will hang if buffer size == 0 */
  do {
    buffer = malloc(buffer_size);
    if (buffer == NULL) buffer_size /= 2;
  } while ((buffer == NULL) && (buffer_size > 0));

  if (buffer_size == 0)
    util_abort("%s: failed to allocate any memory ?? \n",__func__);

  {
    bool result = util_copy_file__(src_file , target_file , buffer_size , buffer , abort_on_error);
    free(buffer);
    return result;
  }
}

void util_move_file(const char * src_file , const char * target_file) {
  if (util_file_exists( src_file )) {
    if (util_copy_file( src_file , target_file))
      remove( src_file );
  }
}


void util_move_file4( const char * src_name , const char * target_name , const char *src_path , const char * target_path) {
  char * src;
  char * target;

  if (src_path != NULL)
    src = util_alloc_filename( src_path , src_name , NULL);
  else
    src = util_alloc_string_copy( src_name );

  if (target_name == NULL)
    target_name = src_name;

  if (target_path == NULL)
    target_path = src_path;

  if (target_path != NULL)
    target = util_alloc_filename( target_path , target_name , NULL);
  else
    target = util_alloc_string_copy( target_name );


  util_move_file( src , target );

  free( target );
  free( src );
}



/**
   Only the file _content_ is considered - file metadata is ignored.
*/

bool util_files_equal( const char * file1 , const char * file2 ) {
  bool equal = true;
  const int buffer_size = 4096;
  char * buffer1 = (char*)util_calloc( buffer_size , sizeof * buffer1 );
  char * buffer2 = (char*)util_calloc( buffer_size , sizeof * buffer2 );

  FILE * stream1 = util_fopen( file1 , "r" );
  FILE * stream2 = util_fopen( file2 , "r" );

  do {
    int count1 = fread( buffer1 , 1 , buffer_size , stream1 );
    int count2 = fread( buffer2 , 1 , buffer_size , stream2 );

    if (count1 != count2)
      equal = false;
    else {
      if (memcmp( buffer1 , buffer2 , count1 ) != 0)
        equal = false;
    }

    if (feof(stream1)) {
      if (feof(stream2))
        break;
      else
        equal = false;
    }
  } while (equal);
  fclose( stream1 );
  fclose( stream2 );

  free( buffer1 );
  free( buffer2 );
  return equal;
}


static void util_fclear_region( FILE * stream , long offset , long region_size) {
  util_fseek( stream , offset , SEEK_SET );
  {
     int i;
     for ( i=0; i < region_size; i++)
        fputc( 0 , stream );
  }
}


static void util_fmove_block(FILE * stream , long offset , long shift , char * buffer , int buffer_size) {
  util_fseek( stream , offset , SEEK_SET );
  {
    int bytes_read = fread( buffer , sizeof * buffer , buffer_size , stream );
    util_fseek( stream , offset + shift , SEEK_SET );
    fwrite( buffer , sizeof * buffer , bytes_read , stream );
  }
}


/**
   This function will modify a file in place by moving the part from
   @offset and to the end of the file @shift bytes. If @shift is
   positive a 'hole' will be created in the file, if @shift is
   negative a part of the file will be overwritten:

     Original        : "ABCDEFGHIJKILMNOPQ"
     fmove( 10 , 5 ) : "ABCDEFGHIJ     KILMNOPQ"
     fmove( 10 , -5) : "ABCDEKILMNOPQ"

  If offset is positioned at the end of the file a section initialized
  to zero will be appended to the file. If @offset is beyond the end
  of the file the function will return EINVAL.

  For this function to work the file must be opened in read-write mode
  (i.e. r+).
*/

int util_fmove( FILE * stream , long offset , long shift) {
  long file_size;
  // Determine size of file.
  {
    long init_pos = util_ftell( stream );
    util_fseek( stream , 0 , SEEK_END);
    file_size = util_ftell( stream );
    util_fseek( stream , init_pos , SEEK_SET );
  }

  // Validate offset and shift input values.
  if ((offset > file_size) || (offset < 0))
    return EINVAL;

  if (offset + shift < 0)
    return EINVAL;

  if (shift != 0) {
    int buffer_size = 1024 * 1024 * 4;  /* 4MB buffer size. */
    char * buffer = (char*)util_calloc( buffer_size , sizeof * buffer );

    /* Shift > 0: We are opening up a hole in the file. */
    if (shift > 0) {
      long pos = offset;
      while (( pos + buffer_size ) < file_size)
        pos += buffer_size;

      while (pos >= offset) {
        util_fmove_block( stream , pos , shift , buffer , buffer_size );
        pos -= buffer_size;
      }
      util_fclear_region( stream , offset , shift );
    } else {
      /* Shift < 0: We are overwriting a part of the file internally. */
      long pos = offset;
      while (pos <= file_size) {
        util_fmove_block( stream , pos , shift , buffer , buffer_size );
        pos += buffer_size;
      }

      // Make sure the file actually shrinks.
      util_ftruncate( stream  , file_size + shift );
    }
    free( buffer );
  }
  return 0;
}


/*
  Windows *might* have both the symbols _access() and access(), but we prefer
  the _access() symbol as that seems to be preferred by Windows. We therefor do
  the #HAVE_WINDOWS__ACCESS check first.
*/

#ifdef HAVE_WINDOWS__ACCESS

bool util_access(const char * entry, int mode) {
  return (_access(entry, mode) == 0);
}

#else

#ifdef HAVE_POSIX_ACCESS
bool util_access(const char * entry, mode_t mode) {
  return (access(entry, mode) == 0);
}
#endif

#endif


/**
   Currently only checks if the entry exists - this will return true
   if path points to directory.
*/
bool util_file_exists(const char *filename) {
  return util_entry_exists( filename );
}



/**
   Checks if there is an entry in the filesystem - i.e.  ordinary
   file, directory , symbolic link, ... with name 'entry'; and returns
   true/false accordingly.
*/

bool util_entry_exists( const char * entry ) {
  return util_access(entry, F_OK);
}
/*****************************************************************/



/**
   The semantics for all the is_xxx functions is follows:

   1. Call stat(). If stat fails with ENOENT the entry does not exist
      at all - return false.

   2. If stat() has succeeded check the type of the entry, and return
      true|false if it is of the wanted type.

   3. If stat() fails with error != ENOENT => util_abort().

*/





bool util_is_directory(const char * path) {
  stat_type stat_buffer;

  if (util_stat(path , &stat_buffer) == 0)
    return S_ISDIR(stat_buffer.st_mode);
  else if (errno == ENOENT)
    /*Path does not exist at all. */
    return false;
  else {
    util_abort("%s: stat(%s) failed: %s \n",__func__ , path , strerror(errno));
    return false; /* Dummy to shut the compiler warning */
  }
}



bool util_is_file(const char * path) {
  stat_type stat_buffer;

  if (util_stat(path , &stat_buffer) == 0)
    return S_ISREG(stat_buffer.st_mode);
  else if (errno == ENOENT)
    /*Path does not exist at all. */
    return false;
  else {
    util_abort("%s: stat(%s) failed: %s \n",__func__ , path , strerror(errno));
    return false; /* Dummy to shut the compiler warning */
  }
}



/**
   Will return false if the path does not exist.
*/

#ifdef ERT_HAVE_SPAWN
bool util_is_executable(const char * path) {
  if (util_file_exists(path)) {
    stat_type stat_buffer;
    util_stat(path , &stat_buffer);
    if (S_ISREG(stat_buffer.st_mode))
      return (stat_buffer.st_mode & S_IXUSR);
    else
      return false; /* It is not a file. */
  } else  /* Entry does not exist - return false. */
    return false;
}


/*
   Will not differtiate between files and directories.
*/
bool util_entry_readable( const char * entry ) {
  stat_type buffer;
  if (util_stat( entry , &buffer ) == 0)
    return buffer.st_mode & S_IRUSR;
  else
    return false;  /* If stat failed - typically not existing entry - we return false. */
}


bool util_file_readable( const char * file ) {
  if (util_entry_readable( file ) && util_is_file( file ))
    return true;
  else
    return false;
}

bool util_entry_writable( const char * entry ) {
  stat_type buffer;
  if (util_stat( entry , &buffer ) == 0)
    return buffer.st_mode & S_IWUSR;
  else
    return false;  /* If stat failed - typically not existing entry - we return false. */
}



#else
  // Windows: executable status based purely on extension ....

bool util_is_executable(const char * path) {
  char * ext;
  util_alloc_file_components( path , NULL , NULL , &ext);
  if (ext != NULL)
    if (strcmp(ext , "exe") == 0)
      return true;

  return false;
}

/* If it exists on windows it is readable ... */
bool util_entry_readable( const char * entry ) {
  stat_type buffer;
  if (util_stat( entry , &buffer ) == 0)
    return true;
  else
    return false;  /* If stat failed - typically not existing entry - we return false. */
}

/* If it exists on windows it is readable ... */
bool util_entry_writable( const char * entry ) {
  stat_type buffer;
  if (util_stat( entry , &buffer ) == 0)
    return true;
  else
    return false;  /* If stat failed - typically not existing entry - we return false. */
}



#endif



static int util_get_path_length(const char * file) {
  if (util_is_directory(file))
    return strlen(file);
  else {
    const char * last_slash = strrchr(file , UTIL_PATH_SEP_CHAR);
    if (last_slash == NULL)
      return 0;
    else
      return last_slash - file;
  }
}



static int util_get_base_length(const char * file) {
  int path_length   = util_get_path_length(file);
  const char * base_start;
  const char * last_point;
  long character_index;

  if (path_length == strlen(file))
    return 0;
  else if (path_length == 0)
    base_start = file;
  else
    base_start = &file[path_length + 1];

  last_point  = strrchr(base_start , '.');
  character_index = last_point - base_start;

  if (last_point == NULL || character_index == 0)
    return strlen(base_start);
  else
    return last_point - base_start;

}



/**
   This function splits a filename into three parts:

    1. A leading path.
    2. A base name.
    3. An extension.

   In the calling scope you should pass in references to pointers to
   char for the fields, you are interested in:

   Example:
   --------

   char * path;
   char * base;
   char * ext;

   util_alloc_file_components("/path/to/some/file.txt" , &path , &base , &ext);
   util_alloc_file_components("/path/to/some/file.txt" , &path , &base , NULL);

   In the second example we were not interested in the extension, and
   just passed in NULL. Before use in the calling scope it is
   essential to check the values of base, path and ext:

   util_alloc_file_components("/path/to/some/file" , &path , &base , &ext);
   if (ext != NULL)
      printf("File: has extension: %s \n",ext);
   else
      printf("File does *not* have an extension \n");

   The memory allocated in util_alloc_file_components must be freed by
   the calling unit.

   Observe the following:

    * It is easy to be fooled by the optional existence of an extension
      (badly desgined API).

    * The function is **NOT** based purely on string parsing, but also
      on checking stat() output to check if the argument you send in
      is an existing directory (that is done through the
      util_get_path_length() function).


      Ex1: input is an existing directory:
      ------------------------------------
      util_alloc_file_components("/some/existing/path" , &path , &base , &ext)

        path -> "/some/existing/path"
        base -> NULL
        ext  -> NULL



      Ex2: input is NOT an existing directory:
      ------------------------------------
      util_alloc_file_components("/some/random/not_existing/path" , &path , &base , &ext)

      path -> "/some/random/not_existing"
        base -> "path"
        ext  -> NULL

*/

void util_alloc_file_components(const char * file, char **_path , char **_basename , char **_extension) {
  char *path      = NULL;
  char *basename  = NULL;
  char *extension = NULL;

  int path_length = util_get_path_length(file);
  int base_length = util_get_base_length(file);
  int ext_length ;
  int slash_length = 1;

  if (path_length > 0)
    path = util_alloc_substring_copy(file , 0 , path_length);
  else
    slash_length = 0;


  if (base_length > 0)
    basename = util_alloc_substring_copy(file , path_length + slash_length , base_length);


  ext_length = strlen(file) - (path_length + base_length + 1);
  if (ext_length > 0)
    extension = util_alloc_substring_copy(file , (path_length + slash_length + base_length + 1) , ext_length);

  if (_extension != NULL)
    *_extension = extension;
  else
    if (extension != NULL) free(extension);


  if (_basename != NULL)
    *_basename = basename;
  else
    if (basename != NULL) free(basename);


  if (_path != NULL)
    *_path = path;
  else
    if (path != NULL) free(path);


}






size_t util_file_size(const char *file) {
  size_t file_size;

  {
    int fildes = open(file , O_RDONLY);
    if (fildes == -1)
      util_abort("%s: failed to open:%s - %s \n",__func__ , file , strerror(errno));

    file_size = util_fd_size( fildes );
    close(fildes);
  }

  return file_size;
}



size_t util_fd_size(int fd) {
  stat_type buffer;

  util_fstat(fd, &buffer);

  return buffer.st_size;
}









bool util_ftruncate(FILE * stream , long size) {
  int fd = fileno( stream );
  int int_return;

#ifdef HAVE_FTRUNCATE
  int_return = ftruncate( fd , size );
#else
  int_return = _chsize( fd , size );
#endif

  if (int_return == 0)
    return true;
  else
    return false;
}





/*
  The windows stat structure has the inode element, but it is not
  set. Actually - this is a property of the filesystem, and not the
  operating system - this check is probably broken on two levels:

   1. One should really check the filesystem involved - whether it
      supports inodes.

   2. The code below will compile on windows, but for a windows
      filesystem it will yield rubbish.
*/

bool util_same_file(const char * file1 , const char * file2) {
#ifdef ERT_HAVE_UNISTD
  stat_type buffer1 , buffer2;
  int stat1,stat2;

  stat1 = util_stat(file1, &buffer1);   // In the case of symlinks the stat call will stat the target file and not the link.
  stat2 = util_stat(file2, &buffer2);

  if ((stat1 == 0) && (stat1 == stat2)) {
    if (buffer1.st_ino == buffer2.st_ino)
      return true;
    else
      return false;
  } else
    return false;  // Files which do not exist are no equal!
#else
  if (util_file_exists(file1) && util_file_exists(file2)) {
    char * abs_path1 = util_alloc_abs_path(file1);
    char * abs_path2 = util_alloc_abs_path(file2);
    bool same_file = util_string_equal(abs_path1, abs_path2);
    free(abs_path1);
    free(abs_path2);
      return same_file;
  }
  else
    return false;    
#endif
}







void util_unlink_existing(const char *filename) {
  if (util_file_exists(filename))
    remove(filename);
}


bool util_fmt_bit8_stream(FILE * stream ) {
  const int    min_read      = 256; /* Critically small */
  const double bit8set_limit = 0.00001;
  const int    buffer_size   = 131072;
  long int start_pos         = util_ftell(stream);
  bool fmt_file;
  {
    double bit8set_fraction;
    int N_bit8set = 0;
    int elm_read,i;
    char *buffer = (char*)util_calloc(buffer_size , sizeof * buffer );

    elm_read = fread(buffer , 1 , buffer_size , stream);
    if (elm_read < min_read)
      util_abort("%s: file is too small to automatically determine formatted/unformatted status \n",__func__);

    for (i=0; i < elm_read; i++)
      N_bit8set += (buffer[i] & (1 << 7)) >> 7;


    free(buffer);

    bit8set_fraction = 1.0 * N_bit8set / elm_read;
    if (bit8set_fraction < bit8set_limit)
      fmt_file =  true;
    else
      fmt_file = false;
  }
  util_fseek(stream , start_pos , SEEK_SET);
  return fmt_file;
}



bool util_fmt_bit8(const char *filename ) {
  FILE *stream;
  bool fmt_file = true;

  if (util_file_exists(filename)) {
    stream   = fopen(filename , "r");
    fmt_file = util_fmt_bit8_stream(stream);
    fclose(stream);
  } else
    util_abort("%s: could not find file: %s - aborting \n",__func__ , filename);

  return fmt_file;
}



/*
  time_t        st_atime;    time of last access
  time_t        st_mtime;    time of last modification
  time_t        st_ctime;    time of last status change
*/

/*
  The returned value is the difference in file mtime from the file2 to
  file1 (what a mess). I.e. if file1 is newer than file2 the returned
  value will be positive.
*/




double util_file_difftime(const char *file1 , const char *file2) {
  stat_type b1, b2;
  int f1,f2;
  time_t t1,t2;

  f1 = open(file1 , O_RDONLY);
  util_fstat(f1, &b1);
  t1 = b1.st_mtime;
  close(f1);

  f2 = open(file2 , O_RDONLY);
  util_fstat(f2, &b2);
  t2 = b2.st_mtime;
  close(f2);

  return difftime(t1 , t2);
}


time_t util_file_mtime(const char * file) {
  time_t mtime = -1;
  int fd = open( file , O_RDONLY);
  if (fd != -1) {
    stat_type f_stat;
    util_fstat(fd , &f_stat );
    mtime = f_stat.st_mtime;
    close( fd );
  }
  return mtime;
}




/**
   Will check if the st_mtime (i.e. last modification) of the file is
   after the time given by @t0.
*/

bool util_file_newer( const char * file , time_t t0) {
  time_t mtime = util_file_mtime( file );
  if (difftime(mtime , t0) > 0)
    return true;
  else
    return false;
}

/**
   Will check if the st_mtime (i.e. last modification) of the file is
   before the time given by @t0.
*/


bool util_file_older( const char * file , time_t t0) {
  time_t mtime = util_file_mtime( file );
  if (difftime(mtime , t0) < 0)
    return true;
  else
    return false;
}


bool util_before( time_t t , time_t limit) {
  if (difftime(limit , t) > 0)
    return true;
  else
    return false;
}


bool util_after( time_t t , time_t limit) {
  if (difftime(limit , t) < 0)
    return true;
  else
    return false;
}



/**
    This function will return a pointer to the newest of the two
    files. If one of the files does not exist - the other is
    returned. If none of the files exist - NULL is returned.
*/

char * util_newest_file(const char *file1 , const char *file2) {
  if (util_file_exists(file1)) {
    if (util_file_exists(file2)) {
      /* Actual comparison of two existing files. */
      if (util_file_difftime(file1 , file2) < 0)
        return (char *) file1;
      else
        return (char *) file2;
    } else
      return (char *)file1;   /* Only file1 exists. */
  } else {
    if (util_file_exists(file2))
      return (char *) file2; /* Only file2 exists. */
    else
      return NULL;   /* None of the files exist. */
  }
}


bool util_file_update_required(const char *src_file , const char *target_file) {
  if (util_file_difftime(src_file , target_file) < 0)
    return true;
  else
    return false;
}




static void __util_set_timevalues_utc(time_t t , int * sec , int * min , int * hour , int * mday , int * month , int * year) {
  struct tm ts;

  util_time_utc(&t , &ts);
  if (sec   != NULL) *sec   = ts.tm_sec;
  if (min   != NULL) *min   = ts.tm_min;
  if (hour  != NULL) *hour  = ts.tm_hour;
  if (mday  != NULL) *mday  = ts.tm_mday;
  if (month != NULL) *month = ts.tm_mon  + 1;
  if (year  != NULL) *year  = ts.tm_year + 1900;
}


static bool util_make_datetime_utc__(int sec, int min, int hour , int mday , int month , int year, bool force_set, time_t * t);

/*
  This function takes a time_t instance as input, and
  returns the the time broken down in sec:min:hour  mday:month:year.

  The return values are by pointers - you can pass in NULL to any of
  the fields.
*/




/*
  This function takes a time_t instance as input, and
  returns the the time broken down in sec:min:hour  mday:month:year.

  The return values are by pointers - you can pass in NULL to any of
  the fields.
*/
void util_set_datetime_values_utc(time_t t , int * sec , int * min , int * hour , int * mday , int * month , int * year) {
  __util_set_timevalues_utc(t , sec , min , hour , mday , month , year);
}


void util_set_date_values_utc(time_t t , int * mday , int * month , int * year) {
  __util_set_timevalues_utc(t , NULL , NULL , NULL , mday , month , year);
}



bool util_is_first_day_in_month_utc( time_t t) {
  int mday;
  __util_set_timevalues_utc(t , NULL , NULL , NULL , &mday ,NULL, NULL);
  if (mday == 1)
    return true;
  else
    return false;
}


/*
  Expects date in the order YYYY-MM-DD.
*/

bool util_sscanf_isodate(const char * date_token , time_t * t) {
  int day, month, year;

  if (date_token && sscanf(date_token , "%d-%d-%d" , &year , &month , &day) == 3)
    return util_make_datetime_utc__(0,0,0,day , month , year , false, t);

  if (t)
    *t = -1;
  return false;
}




/**
   If the parsing fails the time_t pointer is set to -1;
*/
bool util_sscanf_date_utc(const char * date_token , time_t * t) {
  int day   , month , year;
  char sep1 , sep2;

  if (date_token && sscanf(date_token , "%d%c%d%c%d" , &day , &sep1 , &month , &sep2 , &year) == 5) {
    if (t)
      *t = util_make_date_utc(day , month , year );
    return true;
  } else {
    if (t)
      *t = -1;
    return false;
  }
}


bool util_sscanf_percent(const char * percent_token, double * value) {
  if(!percent_token)
    return false;

  char * percent_ptr;
  double double_val = strtod( percent_token, &percent_ptr);

  if (0 == strcmp(percent_ptr, "%")) {
    *value = double_val;
    return true;
  } else
    return false;
}


bool util_fscanf_date_utc(FILE *stream , time_t *t)  {
  int init_pos = util_ftell(stream);
  char * date_token = util_fscanf_alloc_token(stream);
  bool return_value = util_sscanf_date_utc(date_token , t);
  if (!return_value) util_fseek(stream , init_pos , SEEK_SET);
  free(date_token);
  return return_value;
}

/*
   The date format is HARD assumed to be

   dd/mm/yyyy

   Where mm is [1..12]
   yyyy ~2000

*/


void util_fprintf_date_utc(time_t t , FILE * stream) {
  int mday,year,month;

  util_set_datetime_values_utc(t , NULL , NULL , NULL , &mday , &month , &year);
  fprintf(stream , "%02d/%02d/%4d", mday,month,year);
}


char * util_alloc_date_string_utc( time_t t ) {
  int mday,year,month;

  util_set_datetime_values_utc(t , NULL , NULL , NULL , &mday , &month , &year);
  return util_alloc_sprintf("%02d/%02d/%4d", mday,month,year);
}

char * util_alloc_date_stamp_utc( ) {
  time_t now = time( NULL );
  return util_alloc_date_string_utc( now );
}



void util_inplace_forward_seconds_utc(time_t * t , double seconds) {
  (*t) += seconds;
}

void util_inplace_forward_days_utc(time_t * t , double days) {
  util_inplace_forward_seconds_utc( t , days * 3600 * 24 );
}


/**
   This function computes the difference in time between times time1
   and time0: time1 - time0. The return value is the difference in
   seconds (straight difftime output). Observe that the ordering of
   time_t arguments is switched with respect to the difftime
   arguments.

   In addition the difference can be broken down in days, hours,
   minutes and seconds if the appropriate pointers are passed in.
*/


double util_difftime(time_t start_time , time_t end_time , int * _days , int * _hours , int * _minutes , int *_seconds) {
  int sec_min  = 60;
  int sec_hour = 3600;
  int sec_day  = 24 * 3600;
  double dt = difftime(end_time , start_time);
  double dt0 = dt;
  int days , hours, minutes , seconds;

  days = (int) floor(dt / sec_day );
  dt  -= days * sec_day;

  hours = (int) floor(dt / sec_hour);
  dt   -= hours * sec_hour;

  minutes = (int) floor(dt / sec_min);
  dt     -= minutes * sec_min;

  seconds = (int) dt;

  if (_seconds != NULL) *_seconds = seconds;
  if (_minutes != NULL) *_minutes = minutes;
  if (_hours   != NULL) *_hours   = hours;
  if (_days    != NULL) *_days    = days;

  return dt0;
}



/* Is this dst safe ??? */
double util_difftime_days(time_t start_time , time_t end_time) {
  double dt = difftime(end_time , start_time);
  return dt / (24 * 3600);
}


double util_difftime_seconds( time_t start_time , time_t end_time) {
  double dt = difftime(end_time , start_time);
  return dt;
}


/*
  Observe that this routine does the following transform before calling mktime:

  1. month -> month - 1;
  2. year  -> year  - 1900;

  Then it is on the format which mktime expects.

*/

char * util_get_timezone() {
#if defined(HAVE_TZNAME)
  return tzname[0];
#elif defined(HAVE_WINDOWS_TZNAME)
  return _tzname[0];
#endif
}


/*
  The underlying timegm() function will happily accept dates like
  December 33.th 2012 - which is wrapped around to 2.nd of January
  2013. Such wrap-araounds are not accepted by this function, which
  will return false in that case.

  The time_t output is by reference, and will be set to the return
  value of timegm() irrespective of the true/false return value.
*/


static bool util_make_datetime_utc__(int sec, int min, int hour , int mday , int month , int year, bool force_set, time_t * t) {
  bool valid = false;
  struct tm ts;
  ts.tm_sec    = sec;
  ts.tm_min    = min;
  ts.tm_hour   = hour;
  ts.tm_mday   = mday;
  ts.tm_mon    = month - 1;
  ts.tm_year   = year - 1900;
  ts.tm_isdst  = -1;    /* Negative value means mktime tries to determine automagically whether Daylight Saving Time is in effect. */
  {

#ifdef HAVE_TIMEGM
    time_t work_t = timegm( &ts );
#else
    time_t work_t = _mkgmtime( &ts );
#endif

    if ((ts.tm_sec  == sec) &&
        (ts.tm_min  == min) &&
        (ts.tm_hour == hour) &&
        (ts.tm_mday == mday) &&
        (ts.tm_mon == (month - 1)) &&
        (ts.tm_year == (year - 1900)))
      valid = true;

    if (t) {
      if (valid || force_set)
        *t = work_t;
    }
  }
  return valid;
}


bool util_make_datetime_utc_validated(int sec, int min, int hour , int mday , int month , int year, time_t * t) {
  return util_make_datetime_utc__( sec,min,hour,mday,month,year,false, t);
}

time_t util_make_datetime_utc(int sec, int min, int hour , int mday , int month , int year) {
  time_t t;
  util_make_datetime_utc__( sec,min,hour,mday,month,year,true, &t);
  return t;
}




time_t util_make_date_utc(int mday , int month , int year) {
  return util_make_datetime_utc(0 , 0 , 0 , mday , month , year);
}



time_t util_make_pure_date_utc(time_t t) {
  int day,month,year;
  util_set_date_values_utc( t , &day , &month , &year);
  return util_make_date_utc( day , month , year );
}




/*****************************************************************/

void util_set_strip_copy(char * copy , const char *src) {
  const char null_char  = '\0';
  const char space_char = ' ';
  int  src_index   = 0;
  int target_index = 0;
  while (src[src_index] == space_char)
    src_index++;

  while (src[src_index] != null_char && src[src_index] != space_char) {
    copy[target_index] = src[src_index];
    src_index++;
    target_index++;
  }
  copy[target_index] = null_char;
}


/**
   The function will allocate a new copy of src where leading and
   trailing whitespace has been stripped off. If the source string is
   all blanks a string of length one - only containing \0 is returned,
   i.e. not NULL.

   If src is NULL the function will return NULL. The incoming source
   string is not modified, see the function util_realloc_strip_copy()
   for a similar function implementing realloc() semantics.
*/


char * util_alloc_strip_copy(const char *src) {
  char * target;
  int strip_length = 0;
  int end_index   = strlen(src) - 1;
  while (end_index >= 0 && src[end_index] == ' ')
    end_index--;

  if (end_index >= 0) {

    int start_index = 0;
    while (src[start_index] == ' ')
      start_index++;
    strip_length = end_index - start_index + 1;
    target = (char*)util_calloc(strip_length + 1 , sizeof * target );
    memcpy(target , &src[start_index] , strip_length);
  } else
    /* A blank string */
    target = (char*)util_calloc(strip_length + 1 , sizeof * target );

  target[strip_length] = '\0';
  return target;
}



char * util_realloc_strip_copy(char *src) {
  if (src == NULL)
    return NULL;
  else {
    char * strip_copy = util_alloc_strip_copy(src);
    free(src);
    return strip_copy;
  }
}


char ** util_alloc_stringlist_copy(const char **src, int len) {
  if (src != NULL) {
    int i;
    char ** copy = (char**)util_calloc(len , sizeof * copy );
    for (i=0; i < len; i++)
      copy[i] = util_alloc_string_copy(src[i]);
    return copy;
  } else
    return NULL;
}


/**
   This function reallocates the stringlist pointer, making room for
   one more char *, this newly allocated slot is then set to point to
   (a copy of) the new string. The newly reallocated char ** instance
   is the return value from this function.

   Example:
   --------
   char ** stringlist  = (char *[2]) {"One" , "Two"};
   char  * three_string = "Three";

   stringlist = util_stringlist_append_copy(stringlist , 2 , three_string);

   This function does allocate memory - but does not have *alloc* in
   the name - hmmmm....??
*/




char ** util_stringlist_append_copy(char ** string_list, int size , const char * append_string) {
  return util_stringlist_append_ref(string_list , size , util_alloc_string_copy(append_string));
}


/**
   This is nearly the same as util_stringlist_append_copy(), but for
   this case only a refernce to the new string is appended.

   Slightly more dangerous to use ...
*/

char ** util_stringlist_append_ref(char ** string_list, int size , const char * append_string) {
  string_list = (char**)util_realloc(string_list , (size + 1) * sizeof * string_list );
  string_list[size] = (char *) append_string;
  return string_list;
}



char * util_alloc_string_copy(const char *src ) {
  if (src != NULL) {
    int byte_size = (strlen(src) + 1) * sizeof * src;
    char * copy   = (char*)util_calloc( byte_size , sizeof * copy );
    memcpy( copy , src , byte_size );
    return copy;
  } else
    return NULL;
}



char * util_realloc_string_copy(char * old_string , const char *src ) {
  if (src != NULL) {
    char *copy = (char*)util_realloc(old_string , (strlen(src) + 1) * sizeof *copy );
    strcpy(copy , src);
    return copy;
  } else {
    if (old_string != NULL)
      free(old_string);
    return NULL;
  }
}


char * util_realloc_substring_copy(char * old_string , const char *src , int len) {
  if (src != NULL) {
    int str_len;
    char *copy;
    if (strlen(src) < len)
      str_len = strlen(src);
    else
      str_len = len;

    copy = (char*)realloc(old_string , (str_len + 1) * sizeof *copy);
    strncpy(copy , src , str_len);
    copy[str_len] = '\0';

    return copy;
  } else
    return NULL;
}


/**
   This function check that a pointer is different from NULL, and
   frees the memory if that is the case.
*/


void util_safe_free(void *ptr) {
   if (ptr != NULL) free(ptr);
}




/**
   This function checks whether a string matches a pattern with
   wildcard(s). The pattern can consist of plain string parts (which
   must match verbatim), and an arbitrary number of '*' which will
   match an arbitrary number (including zero) of arbitrary characters.

   Examples:
   ---------

   util_string_match("Bjarne" , "Bjarne")    ==> True

   util_string_match("Bjarne" , "jarn")      ==> False

   util_string_match("Bjarne" , "*jarn*")    ==> True

   util_string_match("Bjarne" , "B*e")       ==> True

   util_string_match("Bjarne" , "B*n")       ==> False

   util_string_match("Bjarne" , "*")         ==> True

   util_string_match("Bjarne" , "B***jarne") ==> True

   util_string_match("Bjarne" , "B*r*e")     ==> True

*/



bool util_string_match(const char * string , const char * pattern) {
  const   char    wildcard    = '*';
  const   char   *wildcard_st = "*";

  if (strcmp(wildcard_st , pattern) == 0)
    return true;
  else {
    bool          match = true;
    char **       sub_pattern;
    int           num_patterns;
    const char *  string_ptr;
    util_split_string( pattern , wildcard_st , &num_patterns , &sub_pattern );

    if (pattern[0] == '*')
      string_ptr = strstr(string , sub_pattern[0]);
    else
      string_ptr = (strncmp(string , sub_pattern[0] , strlen(sub_pattern[0])) == 0) ? (char * ) string : NULL;

    if (string_ptr != NULL) {
      /* Inital part matched */
      int i;
          string_ptr += strlen( sub_pattern[0] );
      for (i=1; i < num_patterns; i++) {
        const char * match_ptr = strstr(string_ptr , sub_pattern[i]);
        if (match_ptr != NULL)
          string_ptr = match_ptr + strlen( sub_pattern[i] );
        else {
          match = false;
          break;
        }
      }

      /*
         We have exhausted the complete pattern - matching all the way.
         Does it match at the end?
      */
      if (match) {
        if (strlen(string_ptr) > 0) {
          /*
             There is more left at the end of the string; if the pattern
             ends with '*' that is OK, otherwise the match result is
             FALSE.
          */
          if (pattern[(strlen(pattern) - 1)] != wildcard)
            match = false;
        }
      }

    } else
      match = false;

    util_free_stringlist( sub_pattern , num_patterns);
    return match;
  }
}

/**
   Will check the input string 's' and return true if it contains
   wildcard characters (i.e. '*'), and false otherwise.
*/

bool util_string_has_wildcard( const char * s) {
  const char wildcard = '*';
  if (strchr(s , wildcard) != NULL)
    return true;
  else
    return false;
}



void util_free_stringlist(char **list , int N) {
  int i;
  if (list != NULL) {
    for (i=0; i < N; i++) {
      util_safe_free( list[i] );
    }
    free(list);
  }
}



char * util_strstr_int_format(const char * string ) {
  // the function itself is essentially removing the const from *string
  // so do it explicitly here to make C++ happy
  char * percent_ptr = strchr((char*)string , '%');

  if (percent_ptr) {

    percent_ptr++;
    if (percent_ptr[0] == 'd')
      return percent_ptr++;
    else {
      if (percent_ptr[0] == '0') {

        while (isdigit(percent_ptr[0]))
          percent_ptr++;

        if (percent_ptr[0] == 'd')
          return percent_ptr++;
        else
          return NULL;

      }
      return NULL;
    }
  }
  return percent_ptr;
}


int util_int_format_count(const char * string ) {
  int count = 0;
  const char * str = util_strstr_int_format(string);
  while (str) {
    count++;
    str = util_strstr_int_format( str );
  }
  return count;
}



/**
   Will free a list of strings where the last element is NULL. Will
   go completely canacas if the list is not NULL terminated.
*/

void util_free_NULL_terminated_stringlist(char ** string_list) {
  if (string_list != NULL) {
    int i = 0;
    while (true) {
      if (string_list[i] == NULL)
        break;
      else
        free( string_list[i] );
      i++;
    }
    free( string_list );
  }
}




/**
   This function will reallocate the string s1 to become the sum of s1
   and s2. If s1 == NULL it will just return a copy of s2.

   Observe that due to the use realloc() the s1 input argument MUST BE
   the return value from a malloc() call; this is not intuitive and
   the function should be discontinued.
*/

char * util_strcat_realloc(char *s1 , const char * s2) {
  if (s1 == NULL)
    s1 = util_alloc_string_copy(s2);
  else {
    if (s2 != NULL) {
      int new_length = strlen(s1) + strlen(s2) + 1;
      s1 = (char*)util_realloc( s1 , new_length );
      strcat(s1 , s2);
    }
  }
  return s1;
}


char * util_alloc_string_sum(const char ** string_list , int N) {
  int i , len;
  char * buffer;
  len = 0;
  for (i=0; i < N; i++) {
    if (string_list[i] != NULL)
      len += strlen(string_list[i]);
  }
  buffer = (char*)util_calloc(len + 1 , sizeof * buffer );
  buffer[0] = '\0';
  for (i=0; i < N; i++) {
    if (string_list[i] != NULL)
      strcat(buffer , string_list[i]);
  }
  return buffer;
}


/*****************************************************************/


/**
  Allocates a new string consisting of all the elements in item_list,
  joined together with sep as separator. Elements in item_list can be
  NULL, this will be replaced with the empty string.
*/


char * util_alloc_joined_string(const char ** item_list , int len , const char * sep) {
  if (len <= 0)
    return NULL;
  else {
    char * joined_string;
    int sep_length   = strlen(sep);
    int total_length = 0;
    int eff_len = 0;
    int i;
    for (i=0; i < len; i++)
      if (item_list[i] != NULL) {
        total_length += strlen(item_list[i]);
        eff_len++;
      }

    if (eff_len > 0) {
      total_length += (eff_len - 1) * sep_length + 1;
      joined_string = (char*)util_calloc(total_length , sizeof * joined_string );
      joined_string[0] = '\0';
      for (i=0; i < len; i++) {
        if (item_list[i] != NULL) {
          if (i > 0)
            strcat(joined_string , sep);
          strcat(joined_string , item_list[i]);
        }
      }
      return joined_string;
    } else
      return NULL;
  }
}


/**
  New string is allocated by joining the elements in item_list, with
  "\n" character as separator; an extra "\n" is also added at the end
  of the list.
*/
char * util_alloc_multiline_string(const char ** item_list , int len) {
  char * multiline_string = util_alloc_joined_string(item_list , len , UTIL_NEWLINE_STRING);
  multiline_string = (char*)util_realloc(multiline_string , (strlen(multiline_string) + strlen(UTIL_NEWLINE_STRING) + 1) * sizeof * multiline_string );
  strcat(multiline_string , UTIL_NEWLINE_STRING);
  return multiline_string;
}


/**
   sep_set = string with various characters, i.e. " \t" to split on.
*/

void util_split_string(const char *line , const char *sep_set, int *_tokens, char ***_token_list) {
  int offset;
  int tokens , token , token_length;
  char **token_list;

  offset = strspn(line , sep_set);
  tokens = 0;
  do {
    /*
      if (line[offset] == '\"') {
      seek for terminating ".
      }
    */
    token_length = strcspn(&line[offset] , sep_set);
    if (token_length > 0)
      tokens++;

    offset += token_length;
    offset += strspn(&line[offset] , sep_set);
  } while (line[offset] != '\0');

  if (tokens > 0) {
    token_list = (char**)util_calloc(tokens , sizeof * token_list );
    offset = strspn(line , sep_set);
    token  = 0;
    do {
      token_length = strcspn(&line[offset] , sep_set);
      if (token_length > 0) {
        token_list[token] = util_alloc_substring_copy(line , offset , token_length);
        token++;
      } else
        token_list[token] = NULL;

      offset += token_length;
      offset += strspn(&line[offset] , sep_set);
    } while (line[offset] != '\0');
  } else
    token_list = NULL;

  *_tokens     = tokens;
  *_token_list = token_list;
}


/**
   This function will split the input string in two parts, it will
   split on occurence of one or several of the characters in
   sep_set.


   o If split_on_first is true it will split on the first occurence of
     split_set, and otherwise it will split on the last:

       util_binary_split_string("A:B:C:D , ":" , true  , ) => "A"     & "B:C:D"
       util_binary_split_string("A:B:C:D , ":" , false , ) => "A:B:C" & "D"


   o Characters in the split_set at the front and back are discarded
     _BEFORE_ the actual splitting process.

       util_binary_split_string(":A:B:C:D:" , ":" , true , )   => "A"     & "B:C:D"
       util_binary_split_string(":A:B:C:D:" , ":" , false , )  => "A:B:C" & "D"


   o If no split is found the whole content is in first_part, and
     second_part is NULL. If the input string == NULL, both return
     strings will be NULL. Observe that because leading split
     characters are removed before the splitting starts:

        util_binary_split_string(":ABCD" , ":" , true , )   => "ABCD" & NULL

*/


void util_binary_split_string(const char * __src , const char * sep_set, bool split_on_first , char ** __first_part , char ** __second_part) {
  char * first_part = NULL;
  char * second_part = NULL;
  char * src;

  if (__src != NULL) {
    int offset = 0;
    int len;
    /* 1: Remove leading split characters. */
    while ((offset < strlen(__src)) && (strchr(sep_set , __src[offset]) != NULL))
      offset++;
    len = strlen( __src ) - offset;
    if (len > 0) {
      int tail_pos = strlen( __src ) - 1;
      /* 2: Remove trailing split characters. */
      while (strchr(sep_set , __src[tail_pos]) != NULL)
        tail_pos--;
      len = 1 + tail_pos - offset;

      src = util_alloc_substring_copy(__src , offset , len);
    } else
      src = NULL;

    /*
       OK - we have removed all leading (or trailing) separators, and we have
       a valid string which we can continue with.
    */
    if (src != NULL) {
      int pos;
      int start_pos , delta , end_pos;
      if (split_on_first) {
        start_pos = 0;
        delta     = 1;
        end_pos   = strlen(src);
      } else {
        start_pos = strlen(src) - 1;
        delta     = -1;
        end_pos   = -1;
      }

      pos = start_pos;
      while ((pos != end_pos) && (strchr(sep_set , src[pos]) == NULL))
        pos += delta;
      /*
         OK - now we have either iterated through the whole string - or
         we hav found a character in the sep_set.
      */
      if (pos == end_pos) {
        /* There was no split. */
        first_part = util_alloc_string_copy( src );
        second_part   = NULL;
      } else {
        int sep_start = 0;
        int sep_end   = 0;
        if (split_on_first)
          sep_start = pos;
        else
          sep_end = pos;
        /* Iterate through the separation string - can be e.g. many " " */
        while ((pos != end_pos) && (strchr(sep_set , src[pos]) != NULL))
          pos += delta;

        if (split_on_first) {
          sep_end = pos;
          first_part = util_alloc_substring_copy(src , 0 , sep_start);

          if (sep_end == end_pos)
            second_part = NULL;
          else
            second_part = util_alloc_string_copy( &src[sep_end] );
        } else {
          sep_start = pos;
          if (sep_start == end_pos) {
            // ":String" => (NULL , "String")
            first_part = NULL;
            second_part = util_alloc_string_copy( &src[sep_end+1] );
          } else {
            first_part  = util_alloc_substring_copy( src , 0 , sep_start + 1);
            second_part = util_alloc_string_copy( &src[sep_end + 1]);
          }
        }
      }
      free(src);
    }
  }
  *__first_part  = first_part;
  *__second_part = second_part;
}


void util_binary_split_string_from_max_length(const char * __src , const char * sep_set, int max_length , char ** __first_part , char ** __second_part) {
  char * first_part = NULL;
  char * second_part = NULL;
  if (__src != NULL) {
    char * src;
    int pos;
    /* Removing leading separators. */
    pos = 0;
    while ((pos < strlen(__src)) && (strchr(sep_set , __src[pos]) != NULL))
      pos += 1;
    if (pos == strlen(__src))  /* The string consisted ONLY of separators. */
      src = NULL;
    else
      src = util_alloc_string_copy(&__src[pos]);

    /*Remove trailing separators. */
    pos = strlen(__src) - 1;
    while ((pos >= 0) && (strchr(sep_set , __src[pos]) != NULL))
      pos -= 1;
    if (pos < 0)
      src = NULL;
    else
      src = util_alloc_substring_copy(__src , 0 , pos + 1);


    /*
       OK - we have removed all leading (or trailing) separators, and we have
       a valid string which we can continue with.
    */
    if (src != NULL) {
      int pos;
      int start_pos , delta , end_pos;
      start_pos = max_length;
      delta     = -1;
      end_pos   = -1;
      pos = start_pos;
      while ((pos != end_pos) && (strchr(sep_set , src[pos]) == NULL))
        pos += delta;
      /*
         OK - now we have either iterated through the whole string - or
         we hav found a character in the sep_set.
      */
      if (pos == end_pos) {
        /* There was no split. */
        first_part = util_alloc_string_copy( src );
        second_part   = NULL;
      } else {
        int sep_start = 0;
        int sep_end   = 0;
        sep_end = pos;
        /* Iterate through the separation string - can be e.g. many " " */
        while ((pos != end_pos) && (strchr(sep_set , src[pos]) != NULL))
          pos += delta;


        sep_start = pos;
        if (sep_start == end_pos) {
          // ":String" => (NULL , "String")
          first_part = NULL;
          second_part = util_alloc_string_copy( &src[sep_end+1] );
        } else {
          first_part  = util_alloc_substring_copy( src , 0 , sep_start + 1);
          second_part = util_alloc_string_copy( &src[sep_end + 1]);
        }
      }
      free(src);
    }
  }
  *__first_part  = first_part;
  *__second_part = second_part;
}



/**
   The function will, in-place, update all occurencese: expr->subs.
   The return value is the number of substitutions which have been
   performed. buffer must be \0 terminated.
*/


int static util_string_replace_inplace__(char ** _buffer , const char * expr , const char * subs) {
  char * buffer            = *_buffer;
  int    buffer_size       = strlen( buffer ) + 1;   /* The variable buffer_size is the TOTAL size - including the terminating \0. */
  int len_expr             = strlen( expr );
  int len_subs             = strlen( subs );
  int    size              = strlen(buffer);
  int    offset            = 0;
  int    match_count       = 0;

  char  * match = NULL;
  do {
    match = strstr(&buffer[offset] ,  expr);

    if (match != NULL) {
      /*
         Can not use pointer arithmetic here - because the underlying
         buffer pointer might be realloced.
      */
      {
        int    start_offset  = match             - buffer;
        int    end_offset    = match + len_expr  - buffer;
        int    target_offset = match + len_subs - buffer;
        int    new_size      = size  + len_subs - len_expr;
        if (new_size >= (buffer_size - 1)) {
          buffer_size += buffer_size + 2*len_subs;
          buffer = (char*)util_realloc( buffer , buffer_size );
        }
        {
          char * target    = &buffer[target_offset];
          char * match_end = &buffer[end_offset];
          memmove(target , match_end , 1 + size - end_offset);
        }

        memcpy(&buffer[start_offset] , subs , len_subs);
        offset = start_offset + len_subs;
        size   = new_size;
        match_count++;
      }
    }
  } while (match != NULL && offset < strlen(buffer));


  *_buffer      = buffer;
  return match_count;
}



int util_string_replace_inplace(char ** _buffer , const char * expr , const char * subs) {
  return util_string_replace_inplace__(_buffer , expr , subs );
}




/**
  This allocates a copy of buff_org where occurences of the string expr are replaced with subs.
*/
char * util_string_replace_alloc(const char * buff_org, const char * expr, const char * subs)
{
  int buffer_size   = strlen(buff_org) * 2;
  char * new_buffer = (char*)util_calloc(buffer_size ,  sizeof * new_buffer );
  memcpy(new_buffer , buff_org , strlen(buff_org) + 1);
  util_string_replace_inplace__( &new_buffer , expr , subs);

  {
    int size = strlen(new_buffer);
    new_buffer = (char*)util_realloc(new_buffer, (size + 1) * sizeof * new_buffer);
  }

  return new_buffer;
}



/**
   This allocates a copy of buff_org where occurences of expr[i] are replaced with subs[i] for i=1,..,num_expr.
*/
char * util_string_replacen_alloc(const char * buff_org, int num_expr, const char ** expr, const char ** subs)
{
  int buffer_size   = strlen(buff_org) * 2;
  char * new_buffer = (char*)util_calloc(buffer_size , sizeof * new_buffer );
  memcpy(new_buffer , buff_org , strlen(buff_org) + 1);
  {
          int i;
  for( i=0; i<num_expr; i++)
    util_string_replace_inplace__( &new_buffer , expr[i] , subs[i]);
  }

  {
          int size = strlen(new_buffer);
          new_buffer = (char*)util_realloc(new_buffer, (size + 1) * sizeof * new_buffer);
  }

  return new_buffer;
}



/**
  This will alloc a copy of buff_org were char's in the last strings are removed.
*/
char * util_string_strip_chars_alloc(const char * buff_org, const char * chars)
{
  int len_org = strlen(buff_org);
  int pos_org = 0;
  int pos_new = 0;

  char * buff_new = (char*)util_calloc( (len_org +1) ,  sizeof * buff_new);

  while(pos_org < len_org)
  {
    int pos_off = strcspn(buff_org + pos_org, chars);
    if(pos_off > 0)
    {
      memmove(buff_new + pos_new, buff_org + pos_org, pos_off * sizeof * buff_new);
      pos_org += pos_off;
      pos_new += pos_off;
    }

    pos_off = strspn(buff_org + pos_org, chars);
    if(pos_off > 0)
    {
      pos_org += pos_off;
    }
  }
  buff_new[pos_new + 1] = '\0';
  buff_new = (char*)util_realloc(buff_new, (pos_new + 1) * sizeof buff_new);

  return buff_new;
}



/*****************************************************************/



void util_float_to_double(double *double_ptr , const float *float_ptr , int size) {
  int i;
  for (i=0; i < size; i++)
    double_ptr[i] = float_ptr[i];
}


void util_double_to_float(float *float_ptr , const double *double_ptr , int size) {
  int i;
  for (i=0; i < size; i++)
    float_ptr[i] = (float) double_ptr[i];
}


/*****************************************************************/

/*
   The util_fwrite_string / util_fread_string are BROKEN when it comes
   to NULL / versus an empty string "":

    1. When writing 'NULL' to disk what is actually found on the disk
       is the sequence "0".

    2. When writing the empty string - i.e. "" - what hits the disk is
       the sequence "-1\0"; i.e. the -1 is used as a magic flag to
       indicate the empty string.
*/



void util_fwrite_string(const char * s, FILE *stream) {
  int len = 0;
  if (s != NULL) {
    len = strlen(s);
    if (len == 0)
      util_fwrite_int(-1 , stream);  /* Writing magic string for "" */
    else
      util_fwrite(&len , sizeof len , 1       , stream , __func__);
    util_fwrite(s    , 1          , len + 1 , stream , __func__);
  } else
    util_fwrite(&len , sizeof len , 1       , stream , __func__);
}



char * util_fread_alloc_string(FILE *stream) {
  int len;
  char *s = NULL;
  util_fread(&len , sizeof len , 1 , stream , __func__);
  if (len > 0) {
    s = (char*)util_calloc(len + 1 , sizeof * s );
    util_fread(s , 1 , len + 1 , stream , __func__);
  } else if (len == -1) /* Magic length for "" */ {
    s = (char*)util_calloc(1 , sizeof * s );
    util_fread(s , 1 , 1 , stream , __func__);
  }
  return s;
}


char * util_fread_realloc_string(char * old_s , FILE *stream) {
  int len;
  char *s = NULL;
  util_fread(&len , sizeof len , 1 , stream , __func__);
  if (len > 0) {
    s = (char*)util_realloc(old_s , len + 1 );
    util_fread(s , 1 , len + 1 , stream , __func__);
  } else if (len == -1) /* Magic length for "" */ {
    s = (char*)util_realloc(s , 1 );
    util_fread(s , 1 , 1 , stream , __func__);
  }
  return s;
}


void util_fskip_string(FILE *stream) {
  int len;
  util_fread(&len , sizeof len , 1 , stream , __func__);
  if (len == 0)
    return;                                  /* The user has written NULL with util_fwrite_string(). */
  else if (len == -1)
    util_fseek( stream , 1 , SEEK_CUR);           /* Magic length for "" - skip the '\0' */
  else
    util_fseek(stream , len + 1 , SEEK_CUR);      /* Skip the data in a normal string. */
}


void util_fwrite_offset( offset_type value , FILE * stream ) { UTIL_FWRITE_SCALAR(value , stream); }
void util_fwrite_bool     (bool value , FILE * stream)   { UTIL_FWRITE_SCALAR(value , stream); }
void util_fwrite_int      (int value , FILE * stream)    { UTIL_FWRITE_SCALAR(value , stream); }
void util_fwrite_time_t   (time_t value , FILE * stream)    { UTIL_FWRITE_SCALAR(value , stream); }
void util_fwrite_size_t   (size_t value , FILE * stream)    { UTIL_FWRITE_SCALAR(value , stream); }
void util_fwrite_long  (long value , FILE * stream)    { UTIL_FWRITE_SCALAR(value , stream); }
void util_fwrite_double(double value , FILE * stream) { UTIL_FWRITE_SCALAR(value , stream); }

void util_fwrite_int_vector   (const int * value    , int size , FILE * stream, const char * caller) { util_fwrite(value , sizeof * value, size , stream, caller); }
void util_fwrite_double_vector(const double * value , int size , FILE * stream, const char * caller) { util_fwrite(value , sizeof * value, size , stream, caller); }
void util_fwrite_char_vector  (const char * value   , int size , FILE * stream, const char * caller) { util_fwrite(value , sizeof * value, size , stream, caller); }

void util_fread_char_vector(char * ptr , int size , FILE * stream , const char * caller) {
  util_fread(ptr , sizeof * ptr , size , stream , caller);
}


double util_fread_double(FILE * stream) {
  double file_value;
  UTIL_FREAD_SCALAR(file_value , stream);
  return file_value;
}

int util_fread_int(FILE * stream) {
  int file_value;
  UTIL_FREAD_SCALAR(file_value , stream);
  return file_value;
}

time_t util_fread_time_t(FILE * stream) {
  time_t file_value;
  UTIL_FREAD_SCALAR(file_value , stream);
  return file_value;
}


long util_fread_long(FILE * stream) {
  long file_value;
  UTIL_FREAD_SCALAR(file_value , stream);
  return file_value;
}


bool util_fread_bool(FILE * stream) {
  bool file_value;
  UTIL_FREAD_SCALAR(file_value , stream);
  return file_value;
}


void util_fskip_int(FILE * stream) {
  util_fseek( stream , sizeof (int) , SEEK_CUR);
}

void util_fskip_long(FILE * stream) {
  util_fseek( stream , sizeof (long) , SEEK_CUR);
}

void util_fskip_bool(FILE * stream) {
  util_fseek( stream , sizeof (bool) , SEEK_CUR);
}


/*****************************************************************/

time_t util_time_t_min(time_t a , time_t b) {
  return (a < b) ? a : b;
}

time_t util_time_t_max(time_t a , time_t b) {
  return (a > b) ? a : b;
}

size_t util_size_t_min(size_t a , size_t b) {
  return (a < b) ? a : b;
}

size_t util_size_t_max(size_t a , size_t b) {
  return (a > b) ? a : b;
}

int util_int_min(int a , int b) {
  return (a < b) ? a : b;
}

double util_double_min(double a , double b) {
  return (a < b) ? a : b;
}

float util_float_min(float a , float b) {
  return (a < b) ? a : b;
}

int util_int_max(int a , int b) {
  return (a > b) ? a : b;
}

long int util_long_max(long int a , long int b) {
  return (a > b) ? a : b;
}

double util_double_max(double a , double b) {
  return (a > b) ? a : b;
}

float util_float_max(float a , float b) {;
  return (a > b) ? a : b;
}

void util_update_int_max_min(int value , int * max , int * min) {
  *min = util_int_min( value , *min);
  *max = util_int_max( value , *max);
}

void util_update_float_max_min(float value , float * max , float * min) {
  *min = util_float_min(value , *min);
  *max = util_float_max(value , *max);
}

void util_update_double_max_min(double value , double * max , double * min) {
  *min = util_double_min(value , *min);
  *max = util_double_max(value , *max);
}

void util_clamp_double(double * value , double limit1 , double limit2) {
  double min = util_double_min( limit1 , limit2 );
  double max = util_double_max( limit1 , limit2 );

  *value = util_double_max( *value , min );
  *value = util_double_min( *value , max );
}



/**
   Scans through a vector of doubles, and finds min and max
   values. They are returned by reference.
*/

void util_double_vector_max_min(int N , const double *vector, double *_max , double *_min) {
  double min =  1e100; /* How should this be done ??? */
  double max = -1e100;
  int i;
  for (i = 0; i < N; i++) {
    if (vector[i] > max)
      max = vector[i];

    /* Can not have else here - because same item might succed on both tests. */

    if (vector[i] < min)
      min = vector[i];
  }
  *_max = max;
  *_min = min;
}



double util_double_vector_mean(int N, const double * vector) {
  double mean = 0.0;
  int i;
  for(i=0; i<N; i++)
    mean = mean + vector[i];

  return mean / N;
}



double util_double_vector_stddev(int N, const double * vector) {
  if(N <= 1)
    return 0.0;
  {
          double   stddev         = 0.0;
          double   mean           = util_double_vector_mean(N, vector);
          double * vector_shifted = (double*)util_calloc(N , sizeof *vector_shifted);

          {
          int i;
          for(i=0; i<N; i++)
                vector_shifted[i] = vector[i] - mean;

          for(i=0; i<N; i++)
                stddev = stddev + vector_shifted[i] * vector_shifted[i];
          }
          free(vector_shifted);

          return sqrt( stddev / (N-1));
  }
}


/*****************************************************************/






/**
   This function will update *value so that on return ALL bits which
   are set in bitmask, are also set in value. No other bits in *value
   should be modified - i.e. it is a logical or.
*/

void util_bitmask_on(int * value , int mask) {
  int tmp = *value;
  tmp = (tmp | mask);
  *value = tmp;
}


FILE * util_fopen__(const char * filename , const char * mode) {
  return fopen(filename, mode);
}

FILE * util_fopen(const char * filename , const char * mode) {
  FILE * stream = util_fopen__(filename , mode);
  if (stream == NULL)
    util_abort("%s: failed to open:%s with mode:\'%s\' - error:%s(%d) \n",__func__ , filename , mode , strerror(errno) , errno);

  return stream;
}



/**
   This micro function is only provided for the convenience of java
   wrapping; if you wonder "What on earth should I use this function
   for" - you can just forget about it.
*/

void util_fclose( FILE * stream ) {
  fclose( stream );
}



/**
   This function will open 'filename' with mode 'mode'. If the mode is
   for write or append (w|a) and the open fails with ENOENT we will
   try to make the path compponent.

   So - the whole point about this function is that for writing it
   should be possible to safely call:

     util_mkdir_fopen("/some/path/to/file.txt" , "w");

   without first enusring that /some/path/to exists.
*/

FILE * util_mkdir_fopen( const char * filename , const char * mode ) {
  FILE* stream = fopen( filename , mode);
  if (stream == NULL) {
    if (errno == ENOENT) {
      if (mode[0] == 'w' || mode[0] == 'a') {
        char * path;
        util_alloc_file_components(filename , &path , NULL , NULL);
        if (path != NULL) {
          util_make_path( path );
          free( path );
        }
      }
    }
    /* Let the eventual util_abort() come in the main util_fopen function. */
    return util_fopen( filename , mode );
  } else
    return stream;
}



void util_fwrite(const void *ptr , size_t element_size , size_t items, FILE * stream , const char * caller) {
  int items_written = fwrite(ptr , element_size , items , stream);
  if (items_written != items)
    util_abort("%s/%s: only wrote %d/%d items to disk - aborting: %s(%d) .\n",caller , __func__ , items_written , items , strerror(errno) , errno);
}


void util_fread(void *ptr , size_t element_size , size_t items, FILE * stream , const char * caller) {
  int items_read = fread(ptr , element_size , items , stream);
  if (items_read != items)
    util_abort("%s/%s: only read %d/%d items from disk - aborting.\n %s(%d) \n",caller , __func__ , items_read , items , strerror(errno) , errno);
}


#undef ABORT_READ
#undef ABORT_WRITE


/*****************************************************************/

void * util_realloc(void * old_ptr , size_t new_size ) {
  /* The realloc documentation as ambigous regarding realloc() with size 0 - WE return NULL. */
  if (new_size == 0) {
    if (old_ptr != NULL)
      free(old_ptr);
    return NULL;
  } else {
    void * tmp = realloc(old_ptr , new_size);
    if (tmp == NULL)
      util_abort("%s: failed to realloc %zu bytes - aborting \n",__func__ , new_size);
    return tmp;
  }
}


/**
   This function is a super-thin wrapper around malloc. It allocates
   the number of bytes you ask for. If the return value from malloc()
   is NULL the routine will abort().

   If you are actually interested in handling malloc() failures in a
   decent way, you should not use this routine.
*/



static void * util_malloc__(size_t size) {
  void * data;
  if (size == 0)
    /*
       Not entirely clear from documentation what you get when you
       call malloc( 0 ); this code will return NULL in that case.
    */
    data = NULL;
  else {
    data = malloc( size );
    if (data == NULL)
      util_abort("%s: failed to allocate %zu bytes - aborting \n",__func__ , size);

    /*
       Initializing with something different from zero - hopefully
       errors will pop up more easily this way?
    */
    memset(data , 255 , size);
  }
  return data;
}

void * util_malloc(size_t size) {
  return util_malloc__( size );
}


void * util_calloc( size_t elements , size_t element_size ) {
  return util_malloc( elements * element_size );
}


/**
   Allocates byte_size bytes of storage, and initializes content with
   the value found in src.
*/

void * util_alloc_copy(const void * src , size_t byte_size ) {
  if (byte_size == 0 && src == NULL)
    return NULL;
  {
    void * next = util_malloc(byte_size );
    memcpy(next , src , byte_size);
    return next;
  }
}



void * util_realloc_copy(void * org_ptr , const void * src , size_t byte_size ) {
  if (byte_size == 0 && src == NULL)
    return util_realloc( org_ptr , 0 );
  {
    void * next = util_realloc(org_ptr , byte_size );
    memcpy(next , src , byte_size);
    return next;
  }
}

void util_free(void * ptr) {
  free( ptr );
}





/*****************************************************************/


/**
   These small functions write formatted values onto a stream. The
   main point about these functions is to avoid creating small one-off
   format strings. The character base_fmt should be 'f' or 'g'
*/

void util_fprintf_double(double value , int width , int decimals , char base_fmt , FILE * stream) {
  char * fmt = util_alloc_sprintf("%c%d.%d%c" , '%' , width , decimals , base_fmt);
  fprintf(stream , fmt , value);
  free(fmt);
}


void util_fprintf_int(int value , int width , FILE * stream) {
  char fmt[32];
  sprintf(fmt , "%%%dd" , width);
  fprintf(stream , fmt , value);
}



void util_fprintf_string(const char * s , int width , string_alignement_type alignement , FILE * stream) {
  char fmt[32];
  int i;
  if (alignement == left_pad) {
    i = 0;
    if (width > strlen(s)) {
      for (i=0; i < (width - strlen(s)); i++)
        fputc(' ' , stream);
    }
    fprintf(stream , "%s", s);
  } else if (alignement == right_pad) {
    sprintf(fmt , "%%-%ds" , width);
    fprintf(stream , fmt , s);
  } else {
    int total_pad  = width - strlen(s);
    int front_pad  = total_pad / 2;
    int back_pad   = total_pad - front_pad;
    int i;
    util_fprintf_string(s , front_pad + strlen(s) , left_pad , stream);
    for (i=0; i < back_pad; i++)
      fputc(' ' , stream);
  }
}





/**
   This function allocates a string acoording to the fmt
   specification, and arguments. The arguments (except the format) are
   entered as a variable length argument list, and the function is
   basically a thin wrapper around vsnprintf().

   Example of usage:

   char * s = util_alloc_sprintf("/%s/File:%04d/%s" , "prefix" , 67 , "Suffix");

   => s = /prefix/File:0067/Suffix

   Observe that when it is based in vsnprintf() essentially no
   error-checking is performed.
*/

char * util_alloc_sprintf_va(const char * fmt , va_list ap) {
  char *s = NULL;
  int length;
  va_list tmp_va;

  UTIL_VA_COPY(tmp_va , ap);

  length = vsnprintf(NULL , 0 , fmt , tmp_va);
  s = (char*)util_calloc(length + 1 , sizeof * s );
  vsprintf(s , fmt , ap);
  return s;
}


char * util_alloc_sprintf(const char * fmt , ...) {
  char * s;
  va_list ap;
  va_start(ap , fmt);
  s = util_alloc_sprintf_va(fmt , ap);
  va_end(ap);
  return s;
}


char * util_alloc_sprintf_escape(const char * src , int max_escape) {
  if (src == NULL)
    return NULL;

  if (max_escape == 0)
    max_escape = strlen( src );

  {
    const int src_len = strlen( src );
    char * target = (char*)util_calloc( max_escape + strlen(src) + 1 , sizeof * target);

    int escape_count = 0;
    int src_offset = 0;
    int target_offset = 0;

    while (true) {
      if (src[src_offset] == '%') {
        if (escape_count < max_escape) {
          target[target_offset] = '%';
          target_offset++;
          escape_count++;
        }
      }
      target[target_offset] = src[src_offset];
      target_offset++;
      src_offset++;
      if (src_offset == src_len)
        break;
    }
    target[target_offset] = '\0';
    target = (char*)util_realloc( target , (target_offset + 1) * sizeof * target);
    return target;
  }
}


/*
char * util_realloc_sprintf(char * s , const char * fmt , ...) {
  va_list ap;
  va_start(ap , fmt);
  printf("1: %s  s:%s / p:%p \n",__func__ , s , s );
  {
    int length;
    va_list tmp_va;
    va_copy(tmp_va , ap);
    length = vsnprintf(s , 0 , fmt , tmp_va);
    s = util_realloc(s , length + 1 , __func__);
    printf("2: %s  s:%s / p:%p  length:%d\n",__func__ , s , s );
  }
  vsprintf(s , fmt , ap);
  va_end(ap);
  return s;
}
*/

char * util_realloc_sprintf(char * s , const char * fmt , ...) {
  char * new_s;
  va_list ap;
  va_start(ap , fmt);

  new_s = util_alloc_sprintf_va( fmt , ap );
  util_safe_free(s);

  va_end(ap);
  return new_s;
}







/**
  This function is intended to be installed as a signal
  handler, so we can get a traceback from signals like SIGSEGV.

  To install the signal handler:

  #include <signal.h>
  ....
  ....
  signal(SIGSEGV , util_abort_signal);


  The various signals can be found in: /usr/include/bits/signum.h
*/


void util_abort_signal(int signal) {
  util_abort("Program received signal:%d\n" , signal);
}


void util_install_signals(void) {
#ifdef HAVE_SIGBUS
  signal(SIGBUS  , util_abort_signal);
#endif

  signal(SIGSEGV , util_abort_signal);    /* Segmentation violation, i.e. overwriting memory ... */
  signal(SIGTERM , util_abort_signal);    /* If killing the enkf program with SIGTERM (the default kill signal) you will get a backtrace.
                                             Killing with SIGKILL (-9) will not give a backtrace.*/
  signal(SIGABRT , util_abort_signal);    /* Signal abort. */
  signal(SIGILL  , util_abort_signal);    /* Signal illegal instruction. */
  signal(SIGFPE  , util_abort_signal);    /* Floating point exception */
}

/*
   Will install the util_abort signal handler for all signals which
   have not been modified from the default state.
*/

static void update_signal( int signal_nr ) {
  /* Redefining sighandler_t in case it isn't defined (Windows).
     This is harmless on other platforms. */
  typedef void (*sighandler_t)(int);

  sighandler_t current_handler = signal(signal_nr , SIG_DFL);
  if (current_handler == SIG_DFL)
    signal( signal_nr , util_abort_signal );
  else
    signal( signal_nr , current_handler );
}



void util_update_signals(void) {
#ifdef HAVE_SIGBUS
  update_signal(SIGBUS );
#endif

  update_signal(SIGSEGV );
  update_signal(SIGTERM );
  update_signal(SIGABRT );
  update_signal(SIGILL  );
  update_signal(SIGFPE  );
}


void util_exit(const char * fmt , ...) {
  va_list ap;
  va_start(ap , fmt);
  vfprintf(stderr , fmt , ap);
  va_end(ap);
  exit(1);
}






/**
   This function is quite dangerous - it will always return something;
   it is the responsability of the calling scope to check that it
   makes sense. Will return 0 on input NULL.
*/

int util_get_type( void * data ) {
  if (data == NULL)
    return 0;
  else
    return ((int *) data)[0];
}











/**
   This function finds the current linenumber (by counting '\n'
   characters) in a currently open stream. It is an extremely
   primitive routine, and should only be used exceptionally.

   Observe that it implements "natural" counting, starting at line_nr
   1, and not line_nr 0.
*/

int util_get_current_linenr(FILE * stream) {
  long init_pos = util_ftell(stream);
  int line_nr   = 0;
  util_fseek( stream , 0L , SEEK_SET);
  {
    int char_nr;
    int c;
    for (char_nr = 0; char_nr < init_pos; char_nr++) {
      c = fgetc(stream);
      if (c == '\n')
        line_nr++;
    }
  }
  return line_nr;
}





const char * util_enum_iget( int index , int size , const util_enum_element_type * enum_defs , int * value) {
  if ((index < 0) || (index >= size)) {
    *value = -1;
    return NULL;
  } else {
    const util_enum_element_type elm = enum_defs[ index ];
    *value = elm.value;
    return elm.name;
  }
}





/*****************************************************************/

/*
  Will check if the vector @data contains the element @value. The
  @data vector should be sorted in increasing order prior to calling
  this function. If the vector is not correctly sorted this will be
  crash and burn.

  If the element is found the index is returned, otherwise the value -1
  is returned.
*/

#define CONTAINS(TYPE) int util_sorted_contains_ ## TYPE(const TYPE * data , int size , TYPE value) { \
  if ((data[0] == value))                               \
     return 0;                                          \
  else if ((data[size - 1] == value))                   \
     return size - 1;                                   \
  else if ((value < data[0]) || (value > data[size-1])) \
     return -1;                                         \
  else {                                                \
     int index1 = 0;                                    \
     int index2 = size - 1;                             \
     while (true) {                                     \
        int center_index = (index1 + index2) / 2;       \
        TYPE center_value = data[center_index];         \
        if (center_value == value)                      \
           return center_index;                         \
        else {                                          \
          if (value < center_value)                     \
            index2 = center_index;                      \
          else                                          \
            index1 = center_index;                      \
          if ((index2 - index1) == 1)                   \
            return -1;                                  \
        }                                               \
     }                                                  \
  }                                                     \
}

CONTAINS(int)
CONTAINS(time_t)
CONTAINS(size_t)
#undef CONTAINS

/*****************************************************************/




int util_fnmatch( const char * pattern , const char * string ) {
#ifdef HAVE_FNMATCH
  return fnmatch( pattern , string , 0 );
#else
#pragma comment(lib , "shlwapi.lib")
  bool match = PathMatchSpec( string , pattern ); // shlwapi
  if (match)
    return 0;
  else
    return 1;

#endif
}

/*****************************************************************/
/* Conditional compilation; this last section includes several
   functions which are included if certain features like e.g. posix_spawn()
   are present. */

// Observe that there is some really ugly #ifdef HAVE_REALPATH in the
// main code.



void util_time_utc( time_t * t , struct tm * ts ) {
#ifdef HAVE_GMTIME_R
  gmtime_r( t , ts );
#else
  struct tm * ts_shared = localtime( t );
  memcpy( ts , ts_shared , sizeof * ts );
#endif
}

#ifdef HAVE_ROUND
int util_roundf( float x ) { return roundf(x); }
int util_round( double x ) { return round(x); }
#else
int util_roundf( float x ) { return (int) (x + 0.5); }
int util_round( double x ) { return (int) (x + 0.5); }
#endif

bool util_is_abs_path(const char * path) {
#ifdef ERT_WINDOWS
  if ((path[0] == '/') || (path[0] == '\\'))
    return true;
  else 
    if ((isalpha(path[0]) && (path[1] == ':')))
      return true;

  return false;

#else

  if (path[0] == UTIL_PATH_SEP_CHAR)
    return true;
  else
    return false;

#endif
}

static int util_mkdir( const char * path ) {
#ifdef HAVE_POSIX_MKDIR
  return mkdir( path , UTIL_DEFAULT_MKDIR_MODE );
#endif

#ifdef HAVE_WINDOWS_MKDIR
  return _mkdir( path );
#endif
}


void util_make_path(const char *_path) {
  char *active_path;
  char *path = (char *) _path;
  int current_pos = 0;

  if (!util_is_directory(path)) {
    int i = 0;
    active_path = (char*)util_calloc(strlen(path) + 1 , sizeof * active_path );
    do {
      int n = strcspn(path , UTIL_PATH_SEP_STRING);
      if (n < strlen(path))
        n += 1;
      path += n;
      i++;
      strncpy(active_path , _path , n + current_pos);
      active_path[n+current_pos] = '\0';
      current_pos += n;

      if (!util_is_directory(active_path)) {
        if (util_mkdir(active_path) != 0) {
          bool fail = false;
          switch (errno) {
          case(EEXIST):
            if (util_is_directory(active_path))
              fail = false;
            break;
          default:
            fail = true;
            break;
          }
          if (fail)
            util_abort("%s: failed to make directory:%s - aborting\n: %s(%d) \n",__func__ , active_path , strerror(errno), errno);
        }
      }

    } while (strlen(active_path) < strlen(_path));
    free(active_path);
  }
}


/**
   This function will allocate a unique filename with a random part in
   it. If the the path corresponding to the first argument does not
   exist it is created.

   If the value include_pid is true, the pid of the calling process is
   included in the filename, the resulting filename will be:

      path/prefix-pid-RANDOM

   if include_pid is false the resulting file will be:

      path/prefix-RANDOM

   Observe that IFF the prefix contains any path separator character
   they are translated to "_".
*/



char * util_alloc_tmp_file(const char * path, const char * prefix , bool include_pid ) {
  // Should be reimplemented to use mkstemp()
  const int pid_digits    = 6;
  const int random_digits = 6;
  const int random_max    = 1000000;

#ifdef HAVE_PID_T
  const int pid_max     = 1000000;
  pid_t  pid            = getpid() % pid_max;
#else
  int    pid            = 0;
#endif

  char * file           = (char*)util_calloc(strlen(path) + 1 + strlen(prefix) + 1 + pid_digits + 1 + random_digits + 1 , sizeof * file );
  char * tmp_prefix     = util_alloc_string_copy( prefix );

  if (!util_is_directory(path))
    util_make_path(path);
  util_string_tr( tmp_prefix ,  UTIL_PATH_SEP_CHAR , '_');  /* removing path seps. */

  do {
    long int rand_int = rand() % random_max;
    if (include_pid)
      sprintf(file , "%s%c%s-%d-%ld" , path , UTIL_PATH_SEP_CHAR , tmp_prefix , pid , rand_int);
    else
      sprintf(file , "%s%c%s-%ld" , path , UTIL_PATH_SEP_CHAR , tmp_prefix , rand_int);
  } while (util_file_exists(file));

  free( tmp_prefix );
  return file;
}

/**
   This file allocates a filename consisting of a leading path, a
   basename and an extension. Both the path and the extension can be
   NULL, but not the basename.

   Observe that this function does pure string manipulation; there is
   no input check on whether path exists, if baseneme contains "."
   (or even a '/') and so on.
*/

char * util_alloc_filename(const char * path , const char * basename , const char * extension) {
  char * file;
  int    length = strlen(basename) + 1;

  if (path && strlen(path))
    length += strlen(path) + 1;

  if (extension && strlen(extension))
    length += strlen(extension) + 1;

  file = (char*) util_calloc(length , sizeof * file );
  file[0] = '\0';
  
  if (path && strlen(path)) {
    strcat(file, path);
    strcat(file, UTIL_PATH_SEP_STRING );
  }
  strcat(file, basename);
  if (extension && strlen(extension)) {
    strcat(file, ".");
    strcat(file, extension);
  }

  return file;
}


char * util_realloc_filename(char * filename , const char * path , const char * basename , const char * extension) {
  util_safe_free(filename);
  return util_alloc_filename( path , basename , extension );
}




#ifdef HAVE_PROC
bool util_proc_alive(pid_t pid) {
  char proc_path[16];
  sprintf(proc_path , "/proc/%d" , pid);
  return util_is_directory(proc_path);
}
#endif

int util_proc_mem_free(void) {
  FILE *stream = util_fopen("/proc/meminfo" , "r");
  int mem;
  util_fskip_lines(stream , 1);
  util_fskip_token(stream);
  util_fscanf_int(stream , &mem);
  fclose(stream);
  return mem;
}




char * util_split_alloc_dirname( const char * input_path ) {
  char * path;
  util_alloc_file_components( input_path , &path , NULL , NULL);
  return path;
}


char * util_split_alloc_filename( const char * input_path ) {
  char * filename = NULL;
  {
    char * basename;
    char * extension;

    util_alloc_file_components( input_path , NULL , &basename , &extension);

    if (basename)
      filename = util_alloc_filename( NULL , basename , extension );

    util_safe_free( basename );
    util_safe_free( extension );
  }

  return filename;
}



void util_path_split(const char *line , int *_tokens, char ***_token_list) {
  util_split_string( line , UTIL_PATH_SEP_STRING , _tokens , _token_list);
}

char * util_alloc_parent_path( const char * path) {
  int     path_ncomp;
  char ** path_component_list;
  char *  parent_path = NULL;

  if (path) {
    bool is_abs = util_is_abs_path( path );
    char * work_path;

    if (strstr(path , "..")) {
      if (is_abs)
        work_path = util_alloc_realpath__( path );
      else {
        char * abs_path = util_alloc_realpath__( path );
        char * cwd = util_alloc_cwd();
        work_path = util_alloc_rel_path( cwd , abs_path );
        free( abs_path );
        free( cwd );
      }
    } else
      work_path = util_alloc_string_copy( path );

    util_path_split( work_path , &path_ncomp , &path_component_list );
    if (path_ncomp > 0) {
      int current_length = 4;
      int ip;

      parent_path = (char*)util_realloc( parent_path , current_length * sizeof * parent_path);
      parent_path[0] = '\0';

      for (ip=0; ip < path_ncomp - 1; ip++) {
        const char * ipath = path_component_list[ip];
        int min_length = strlen(parent_path) + strlen(ipath) + 1;

        if (min_length >= current_length) {
          current_length = 2 * min_length;
          parent_path = (char*)util_realloc( parent_path , current_length * sizeof * parent_path);
        }

        if (is_abs || (ip > 0))
          strcat( parent_path , UTIL_PATH_SEP_STRING );
        strcat( parent_path , ipath );
      }
    }
    util_free_stringlist( path_component_list , path_ncomp );
    free( work_path );
  }
  return parent_path;
}

#ifdef ERT_HAVE_UNISTD

int util_type_get_id( const void * data ) {
  int type_id = ((const int*) data)[0];
  return type_id;
}

int util_chdir(const char * path) {
  return chdir( path );
}
#endif

#ifdef HAVE_WINDOWS_CHDIR
#include <direct.h>

int util_chdir(const char * path) {
  return _chdir( path );
}

#endif



bool util_chdir_file( const char * filename ) {
  if (!util_is_file( filename ))
    return false;

  bool chdir_OK = false;
  char * path;
  char * abs_path;
  util_alloc_file_components( filename , &path, NULL , NULL );
  abs_path = util_alloc_abs_path( path );
  if (util_is_directory( abs_path ))
    chdir_OK = (0 == util_chdir( abs_path ));

  free( abs_path );
  free( path );
  return chdir_OK;
}
