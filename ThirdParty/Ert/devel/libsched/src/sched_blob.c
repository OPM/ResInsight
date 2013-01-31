/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_blob.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <time.h>

#include <ert/util/buffer.h>
#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#include <ert/sched/sched_util.h>
#include <ert/sched/sched_blob.h>
#include <ert/sched/sched_time.h>





struct sched_blob_struct {
  char              * buffer;
  time_t              start_time;
  sched_time_type   * time_step;   /* Either a date into the 'future' - or a TSTEP. This is the end time of the blob.*/
};



static void sched_blob_append_buffer( sched_blob_type * blob , const char * new_buffer ) {
  blob->buffer = util_strcat_realloc( blob->buffer , new_buffer );
}


void sched_blob_append_token( sched_blob_type * blob , const char * token ) {
  char * new_buffer = util_calloc( (strlen(token) + 2) , sizeof * new_buffer );
  sched_blob_append_buffer( blob , new_buffer );
  free( new_buffer );
}



sched_blob_type * sched_blob_alloc() {
  sched_blob_type * blob = util_malloc( sizeof * blob );
  blob->buffer    = NULL;
  blob->time_step = NULL;
  return blob;
}


int sched_blob_get_size( const sched_blob_type * blob ) {
  if (blob->buffer == NULL)
    return 0;
  else
    return strlen( blob->buffer );
}



void sched_blob_free( sched_blob_type * blob ) {
  util_safe_free( blob->buffer );
  if (blob->time_step != NULL)
    sched_time_free( blob->time_step );
  free( blob );
}




void sched_blob_fprintf( const sched_blob_type * blob , FILE * stream ) {
  fprintf(stream , "%s" , blob->buffer );
}
