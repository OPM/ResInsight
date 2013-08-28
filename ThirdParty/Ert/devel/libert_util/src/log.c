/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'log.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <string.h> 
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#ifdef HAVE_FSYNC
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#include <ert/util/util.h>
#include <ert/util/log.h>

struct log_struct {
  char             * filename;
  FILE             * stream; 
  int                fd; 
  int                log_level;
#ifdef HAVE_PTHREAD
  pthread_mutex_t    mutex;
#endif
};



void log_reopen(log_type *logh , const char *filename) {
  if (logh->stream != NULL)  { /* Close the existing file descriptor. */
    size_t file_size;
    fclose( logh->stream );
    file_size = util_file_size( logh->filename );
    if (file_size == 0)
      remove( logh->filename ); /* Unlink the old log file if it had zero size. */ 
  }
  
  logh->filename = util_realloc_string_copy( logh->filename , filename );
#ifdef HAVE_PTHREAD
  pthread_mutex_lock( &logh->mutex );
#endif

  if (filename != NULL) {
    logh->stream = util_mkdir_fopen( filename , "a+");
    logh->fd     = fileno( logh->stream );
  } else {  /* It is ~OK to open a log with NULL filename, but then
               log_reopen() with a VALID filename must be
               called before it is actually used. */
    logh->stream = NULL;
    logh->fd     = -1;
  }

#ifdef HAVE_PTHREAD
  pthread_mutex_unlock( &logh->mutex );
#endif
}


const char * log_get_filename( const log_type * logh ) {
  return logh->filename;
}

int log_get_level( const log_type * logh) {
  return logh->log_level;
}


void log_set_level( log_type * logh , int log_level) {
  logh->log_level = log_level;
}



log_type * log_open( const char * filename , int log_level) {
  log_type   *logh;

  logh = util_malloc(sizeof *logh );
  
  logh->log_level     = log_level;
  logh->filename      = NULL;
  logh->stream        = NULL;
#ifdef HAVE_PTHREAD
  pthread_mutex_init( &logh->mutex , NULL );
#endif
  if (filename != NULL)
    log_reopen( logh , filename);
  
  return logh;
}



static bool log_include_message(const log_type *logh , int message_level) {
  if (message_level <= logh->log_level)
    return true;
  else
    return false;
}


/**
   If dup_stream != NULL the message (without the date/time header) is duplicated on this stream.
*/
void log_add_message(log_type *logh, int message_level , FILE * dup_stream , char* message, bool free_message) {
  if (logh->stream == NULL)
    util_abort("%s: logh->stream == NULL - must call log_reset_filename() first \n",__func__);

#ifdef HAVE_PTHREAD
  pthread_mutex_lock( &logh->mutex );
#endif
  {
    struct tm time_fields;
    time_t    epoch_time;
  
    if (log_include_message(logh,message_level)) {
      time(&epoch_time);
      util_localtime(&epoch_time , &time_fields);

      if (message != NULL)
        fprintf(logh->stream,"%02d/%02d - %02d:%02d:%02d  %s\n",time_fields.tm_mday, time_fields.tm_mon + 1, time_fields.tm_hour , time_fields.tm_min , time_fields.tm_sec , message);
      else
        fprintf(logh->stream,"%02d/%02d - %02d:%02d:%02d   \n",time_fields.tm_mday, time_fields.tm_mon + 1, time_fields.tm_hour , time_fields.tm_min , time_fields.tm_sec);

      /** We duplicate the message to the stream 'dup_stream'. */
      if ((dup_stream != NULL) && (message != NULL))
        fprintf(dup_stream , "%s\n", message);
      
      log_sync( logh );
    }
  }
#ifdef HAVE_PTHREAD
  pthread_mutex_unlock( &logh->mutex );
#endif
  if (free_message)
    free( message );
}






void log_add_fmt_message(log_type * logh , int message_level , FILE * dup_stream , const char * fmt , ...) {
  if (log_include_message(logh,message_level)) {
    char * message;
    va_list ap;
    va_start(ap , fmt);
    message = util_alloc_sprintf_va( fmt , ap );
    log_add_message( logh , message_level , dup_stream , message , true);
    va_end(ap);
  }
}






/**
   This function can be used to get low level to the FILE pointer of
   the stream. To 'ensure' that the data actually hits the disk
   you should call log_sync() after writing.

   It is your responsabiulity to avoid racing++ when using the
   log_get_stream() function.
*/

FILE * log_get_stream(log_type * logh ) {
  return logh->stream;
}


void log_sync(log_type * logh) {
#ifdef HAVE_FSYNC
  fsync( logh->fd );
#endif
  util_fseek( logh->stream , 0 , SEEK_END );
}



void log_close( log_type * logh ) {
  if ((logh->stream != stdout) && (logh->stream != stderr) && (logh->stream != NULL)) 
    fclose( logh->stream );  /* This closes BOTH the FILE * stream and the integer file descriptor. */
  
  util_safe_free( logh->filename );
  free( logh );
}


bool log_is_open( const log_type * logh) {
  if (logh->stream != NULL)
    return true;
  else
    return false;
}
