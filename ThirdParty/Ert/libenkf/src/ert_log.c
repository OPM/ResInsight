/*
   Copyright (C) 2014  Statoil ASA, Norway. 
    
   The file 'ert_log.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdarg.h>

#include <ert/util/util.h>

#include <ert/enkf/ert_log.h>
#include <ert/enkf/enkf_defaults.h>


static log_type * logh = NULL;               /* Handle to an open log file. */

/**
 * The logging uses log_level to determine if an incoming message is to be included in the log.
 * A high log_level setting will include more messages.
 */
void ert_log_init_log( int log_level , const char * log_file_name, bool verbose){
  logh = log_open( NULL , DEFAULT_LOG_LEVEL );
  
  log_set_level(logh, log_level);
  if (log_file_name)
    log_reopen( logh , log_file_name);

  if (verbose)
    printf("Activity will be logged to ..............: %s \n",log_get_filename( logh ));
  log_add_message(logh , 1 , NULL , "ert configuration loaded" , false);
}

void ert_log_add_message_py(int message_level, char* message){
    ert_log_add_message(message_level, NULL, message, false);
}

/**
 * Adding a message with a given message_level. A low message_level means "more important", as only messages with
 * message_level below the configured log_level will be included.
 */
void ert_log_add_message(int message_level , FILE * dup_stream , char* message, bool free_message) {
   if(logh==NULL)
     ert_log_init_log(1,NULL,true);
   log_add_message(logh, message_level, dup_stream, message, free_message);
}

/**
 * Adding a message with a given message_level. A low message_level means "more important", as only messages with
 * message_level below the configured log_level will be included.
 */
void ert_log_add_fmt_message(int message_level , FILE * dup_stream , const char * fmt , ...) {
    if (log_include_message(logh,message_level)) {
      char * message;
      va_list ap;
      va_start(ap , fmt);
      message = util_alloc_sprintf_va( fmt , ap );
      log_add_message( logh , message_level , dup_stream , message , true);
      va_end(ap);
    }
}

void ert_log_close(){
    if (log_is_open( logh ))
      log_add_message( logh , false , NULL , "Exiting ert application normally - all is fine(?)" , false);
    log_close( logh );
    logh = NULL;
}

bool ert_log_is_open(){
    if(logh==NULL)
        return false;
    return log_is_open(logh);
}

void ert_log_set_log_level(int log_level){
    if(logh==NULL)
        ert_log_init_log(1,NULL,true);
    log_set_level(logh, log_level);
}

int ert_log_get_log_level(){
  if(logh==NULL)
    ert_log_init_log(1,NULL,true);
  return log_get_level(logh);
}

const char * ert_log_get_filename() {
  if(logh==NULL)
    ert_log_init_log(1,NULL,true);
  return log_get_filename(logh);
}

log_type * ert_log_get_logh() {
  if(logh==NULL)
    ert_log_init_log(1,NULL,true);
  return logh;
}

void ert_log_open_empty(){
  logh = log_open(NULL, DEFAULT_LOG_LEVEL);
}
