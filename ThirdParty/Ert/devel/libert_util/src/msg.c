/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'msg.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/msg.h>
#include <ert/util/type_macros.h>

#define MSG_TYPE_ID 1999867

struct msg_struct {
  UTIL_TYPE_ID_DECLARATION;  
  char * prompt;
  char * msg;
  int    msg_len;
  bool   visible;
  bool   debug;
};


static void __msg_assert_visible(const msg_type * msg) {
  if (!msg->visible) 
    util_abort("%s: you must call msg_show() first - aborting.\n",__func__);
}


static void __blank_string(int len) {
    int i;
  for (i = 0; i < len; i++)
    fputc('\b' , stdout);

  for (i = 0; i < len; i++)
    fputc(' ' , stdout);

  for (i = 0; i < len; i++)
    fputc('\b' , stdout);

}


void msg_clear_msg(msg_type * msg) {
  __msg_assert_visible(msg);
  __blank_string(msg->msg_len);
  if (msg->msg != NULL) {
    free(msg->msg);
    msg->msg_len = 0;
    msg->msg     = NULL;
  }
}
  


static void msg_clear_prompt(const msg_type * msg) {
  __blank_string(strlen(msg->prompt));
}


void msg_hide(msg_type * msg) {
  msg_clear_msg(msg);
  msg_clear_prompt(msg);
  msg->visible = false;
}


void msg_set_prompt(msg_type * msg , const char * prompt) {
  msg->prompt = util_realloc_string_copy(msg->prompt , prompt);
}


void msg_print_msg(const msg_type * msg) {
  if (msg->msg != NULL) 
    printf("%s" , msg->msg);
  fflush(stdout);
}


void msg_show(msg_type * msg) {
  if (!msg->visible) {
    printf("%s" , msg->prompt);
    msg_print_msg(msg);
    msg->visible = true;
  }
}


void msg_update(msg_type * msg , const char * new_msg) {
  __msg_assert_visible(msg);
  if (!msg->debug)
    msg_clear_msg(msg);

  {
    msg->msg = util_realloc_string_copy(msg->msg , new_msg);
    if (new_msg == NULL)
      msg->msg_len = 0;
    else
      msg->msg_len = strlen(new_msg);
  }

  if (msg->debug)
    printf("%s\n",msg->msg);
  else
    msg_print_msg(msg);
}


void msg_update_int(msg_type * msg , const char * fmt , int value) {
  char buffer[16];
  sprintf(buffer , fmt , value);
  msg_update(msg , buffer);
}


UTIL_SAFE_CAST_FUNCTION( msg , MSG_TYPE_ID )

msg_type * msg_alloc(const char * prompt, bool debug) {
  msg_type * msg = util_malloc(sizeof * msg );
  UTIL_TYPE_ID_INIT( msg , MSG_TYPE_ID);
  msg->prompt = util_alloc_string_copy(prompt);
  
  msg->msg     = NULL;
  msg->msg_len = 0;
  msg->visible = false;
  msg->debug   = debug;
  return msg;
}


void msg_free(msg_type * msg , bool clear) {
  if (clear) 
    msg_hide(msg);
  else 
    printf("\n");

  free(msg->prompt);
  if (msg->msg != NULL)
    free(msg->msg);
  
  free(msg);
}
  


