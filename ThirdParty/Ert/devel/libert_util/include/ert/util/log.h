/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'log.h' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#ifndef ERT_LOG_H
#define ERT_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdlib.h>

typedef struct log_struct log_type;

  FILE       * log_get_stream(log_type * logh );
  void         log_reopen( log_type * logh , const char * filename );
  log_type   * log_open(const char *filename, int log_level);
  void         log_add_message(log_type *logh, int message_level , FILE * dup_stream , char* message, bool free_message);
  void         log_add_fmt_message(log_type * logh , int message_level , FILE * dup_stream , const char * fmt , ...);
  int          log_get_level( const log_type * logh);
  void         log_set_level( log_type * logh , int new_level);
  void         log_close( log_type * logh );
  void         log_sync(log_type * logh);
  const char * log_get_filename( const log_type * logh );
  int          log_get_level( const log_type * logh);
  void         log_set_level( log_type * logh , int log_level);
  bool         log_is_open( const log_type * logh);
  bool         log_include_message(const log_type *logh , int message_level);


#ifdef __cplusplus
}
#endif
#endif
