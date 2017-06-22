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

//Same as pythons default log levels, but with different numeric values.
typedef enum {
  LOG_CRITICAL=0, //OOM.
  LOG_ERROR=1, //When something we really expected to work does not, e.g. IO failure.
  LOG_WARNING=2, //Important, but not error. E.g. combination of settings which can be intended, but probably are not.
  LOG_INFO=3, //Entering functions/parts of the code
  LOG_DEBUG=4 //Inside the for-loop, when you need the nitty gritty details. Think TRACE.
} message_level_type;


typedef struct log_struct log_type;

  FILE       * log_get_stream(log_type * logh );
  void         log_reopen( log_type * logh , const char * filename );
  log_type   * log_open(const char *filename, int log_level);
  void         log_add_message(log_type *logh, int message_level , FILE * dup_stream , char* message, bool free_message);
  void         log_add_message_str(log_type *logh, message_level_type message_level , const char* message);
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
