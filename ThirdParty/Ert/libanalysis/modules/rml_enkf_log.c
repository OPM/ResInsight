/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'rml_enkf_log.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <rml_enkf_log.h>

#define DEFAULT_LOG_FILE               "rml_enkf.out"
#define DEFAULT_CLEAR_LOG              true

struct rml_enkf_log_struct {
  bool      clear_log;
  char    * log_file;
  FILE    * log_stream;
};


rml_enkf_log_type * rml_enkf_log_alloc() {
  rml_enkf_log_type * rml_log = util_malloc( sizeof * rml_log );
  rml_log->log_file = NULL;
  rml_log->log_stream = NULL;
  rml_enkf_log_set_clear_log( rml_log , DEFAULT_CLEAR_LOG );

  rml_enkf_log_set_log_file( rml_log, DEFAULT_LOG_FILE );
  return rml_log;
}

bool rml_enkf_log_get_clear_log( const rml_enkf_log_type * data ) {
  return data->clear_log;
}

void rml_enkf_log_set_clear_log( rml_enkf_log_type * data , bool clear_log) {
  data->clear_log = clear_log;
}

void rml_enkf_log_set_log_file( rml_enkf_log_type * data , const char * log_file ) {
  data->log_file = util_realloc_string_copy( data->log_file , log_file );
}

const char * rml_enkf_log_get_log_file( const rml_enkf_log_type * data) {
  return data->log_file;
}


void rml_enkf_log_free(rml_enkf_log_type * rml_log) {
  rml_enkf_log_close( rml_log );
  util_safe_free( rml_log->log_file );
  free( rml_log );
}


void rml_enkf_log_open( rml_enkf_log_type * rml_log , int iteration_nr ) {
  if (rml_log->log_file) {
    if ( iteration_nr == 0) {

      if (rml_log->clear_log)
        rml_log->log_stream = util_mkdir_fopen( rml_log->log_file , "w");
      else
        rml_log->log_stream = util_mkdir_fopen( rml_log->log_file , "a");

    } else
      rml_log->log_stream = util_fopen( rml_log->log_file , "a");
  }
}


bool rml_enkf_log_is_open( const rml_enkf_log_type * rml_log ) {
  if (rml_log->log_stream)
    return true;
  else
    return false;
}


void rml_enkf_log_close( rml_enkf_log_type * rml_log ) {
  if (rml_log->log_stream)
    fclose( rml_log->log_stream );

  rml_log->log_stream = NULL;
}


void rml_enkf_log_line( rml_enkf_log_type * rml_log , const char * fmt , ...) {
  if (rml_log->log_stream) {
    va_list ap;
    va_start(ap , fmt);
    vfprintf( rml_log->log_stream , fmt , ap );
    va_end( ap );
  }
}

