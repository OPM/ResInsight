/*
   Copyright (C) 2015  Statoil ASA, Norway.

   The file 'rml_enkf_log.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef RML_ENKF_LOG_H
#define RML_ENKF_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

  typedef struct rml_enkf_log_struct rml_enkf_log_type;

  rml_enkf_log_type * rml_enkf_log_alloc();
  void rml_enkf_log_free(rml_enkf_log_type * rml_log);
  bool rml_enkf_log_get_clear_log( const rml_enkf_log_type * data );
  void rml_enkf_log_set_clear_log( rml_enkf_log_type * data , bool clear_log);
  void rml_enkf_log_set_log_file( rml_enkf_log_type * data , const char * log_file );
  const char * rml_enkf_log_get_log_file( const rml_enkf_log_type * data);
  void rml_enkf_log_open( rml_enkf_log_type * rml_log , int iteration_nr );
  void rml_enkf_log_close( rml_enkf_log_type * rml_log );
  void rml_enkf_log_line( rml_enkf_log_type * rml_log , const char * fmt , ...);
  bool rml_enkf_log_is_open( const rml_enkf_log_type * rml_log );

#ifdef __cplusplus
}
#endif
#endif

