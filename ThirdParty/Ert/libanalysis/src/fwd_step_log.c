/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'fwd_step_log.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <stdio.h>
#include <stdlib.h>

#include <ert/util/util.h>

#include <ert/analysis/fwd_step_log.h>

#define DEFAULT_LOG_FILE               "fwd_step.out"
#define DEFAULT_CLEAR_LOG              false

struct fwd_step_log_struct {
  bool      clear_log;
  char    * log_file;
  FILE    * log_stream;
};


fwd_step_log_type * fwd_step_log_alloc() {
  fwd_step_log_type * fwd_step_log = util_malloc( sizeof * fwd_step_log );
  fwd_step_log->log_file = NULL;
  fwd_step_log->log_stream = NULL;
  fwd_step_log_set_log_file( fwd_step_log , DEFAULT_LOG_FILE);
  fwd_step_log_set_clear_log( fwd_step_log , DEFAULT_CLEAR_LOG );
  return fwd_step_log;
}

bool fwd_step_log_get_clear_log( const fwd_step_log_type * data ) {
  return data->clear_log;
}

void fwd_step_log_set_clear_log( fwd_step_log_type * data , bool clear_log) {
  data->clear_log = clear_log;
}

void fwd_step_log_set_log_file( fwd_step_log_type * data , const char * log_file ) {
  data->log_file = util_realloc_string_copy( data->log_file , log_file );
}

const char * fwd_step_log_get_log_file( const fwd_step_log_type * data) {
  return data->log_file;
}


void fwd_step_log_free(fwd_step_log_type * fwd_step_log) {
  fwd_step_log_close( fwd_step_log );
  util_safe_free( fwd_step_log->log_file );
  free( fwd_step_log );
}


void fwd_step_log_open( fwd_step_log_type * fwd_step_log ) {
  if (fwd_step_log->log_file) {
    if (fwd_step_log->clear_log)
      fwd_step_log->log_stream = util_mkdir_fopen( fwd_step_log->log_file , "w");
    else
      fwd_step_log->log_stream = util_mkdir_fopen( fwd_step_log->log_file , "a");
  }
}


bool fwd_step_log_is_open( const fwd_step_log_type * fwd_step_log ) {
  if (fwd_step_log->log_stream)
    return true;
  else
    return false;
}


void fwd_step_log_close( fwd_step_log_type * fwd_step_log ) {
  if (fwd_step_log->log_stream)
    fclose( fwd_step_log->log_stream );

  fwd_step_log->log_stream = NULL;
}


void fwd_step_log_line( fwd_step_log_type * fwd_step_log , const char * fmt , ...) {
  if (fwd_step_log->log_stream) {
    va_list ap;
    va_start(ap , fmt);
    vfprintf( fwd_step_log->log_stream , fmt , ap );
    va_end( ap );
  }
}


