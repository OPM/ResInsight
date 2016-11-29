/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'fwd_step_log.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef FWD_STEP_LOG_H
#define FWD_STEP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

  typedef struct fwd_step_log_struct fwd_step_log_type;

  fwd_step_log_type * fwd_step_log_alloc();
  void fwd_step_log_free(fwd_step_log_type * fwd_step_log);
  bool fwd_step_log_get_clear_log( const fwd_step_log_type * data );
  void fwd_step_log_set_clear_log( fwd_step_log_type * data , bool clear_log);
  void fwd_step_log_set_log_file( fwd_step_log_type * data , const char * log_file );
  const char * fwd_step_log_get_log_file( const fwd_step_log_type * data);
  void fwd_step_log_open( fwd_step_log_type * fwd_step_log );
  void fwd_step_log_close( fwd_step_log_type * fwd_step_log );
  void fwd_step_log_line( fwd_step_log_type * fwd_step_log , const char * fmt , ...);
  bool fwd_step_log_is_open( const fwd_step_log_type * fwd_step_log );

#ifdef __cplusplus
}
#endif
#endif

