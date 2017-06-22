/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'template_loop.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/ert_api_config.h>

#include <ctype.h>
#include <sys/types.h>
#include <regex.h>

#include <ert/util/parser.h>
#include <ert/util/stringlist.h>
#include <ert/util/template.h>
#include <ert/util/template_type.h>

#define END_REGEXP           "[{]%[[:space:]]+endfor[[:space:]]+%[}]"
#define LOOP_REGEXP          "[{]%[[:space:]]+for[[:space:]]+([$]?[[:alpha:]][[:alnum:]]*)[[:space:]]+in[[:space:]]+[[]([^]]*)[]][[:space:]]+%[}]"

#define LOOP_OPTIONS REG_EXTENDED
#define END_OPTIONS  REG_EXTENDED

#define DOLLAR  '$'
/*
  This file implements a simple looping construct in the templates. The support
  is strongly based on the POSIX regexp functionality; and this file will not
  be compiled unless that support is present.
*/

struct loop_struct {
  int               opentag_offset;
  int               opentag_length;

  int               endtag_offset;
  int               endtag_length;

  int               body_offset;
  int               body_length;

  bool              replace_substring; 
  int               var_length;
  char            * loop_var;
  stringlist_type * items;
};

static void regcompile(regex_t * reg , const char * reg_string , int flags) {
  int error = regcomp(reg , reg_string , flags);
  if (error) {
    int error_size = 256;
    char error_buffer[256];
    regerror(error , reg , error_buffer , error_size);
    util_exit("Compile of %s failed: %s \n",LOOP_REGEXP, error_buffer);
  }
}




static loop_type * loop_alloc( const char * buffer , int global_offset , regmatch_t tag_offset , regmatch_t __var_offset , regmatch_t __items_offset) {
  loop_type * loop  = util_malloc( sizeof * loop);

  loop->opentag_offset  = global_offset + tag_offset.rm_so;
  loop->opentag_length  = tag_offset.rm_eo - tag_offset.rm_so;
  
  loop->endtag_offset  = -1;
  loop->endtag_length  = -1;
  
  loop->body_offset = loop->opentag_offset + loop->opentag_length;
  loop->body_length = -1;
  {
    int var_offset = global_offset + __var_offset.rm_so;
    int var_length = __var_offset.rm_eo - __var_offset.rm_so;
    loop->loop_var   = util_alloc_substring_copy( buffer , var_offset , var_length );
    loop->var_length = strlen( loop->loop_var );
    if (loop->loop_var[0] == DOLLAR)
      loop->replace_substring = true;
    else
      loop->replace_substring = false;
  }
  {
    int items_offset = global_offset + __items_offset.rm_so;
    int items_length = __items_offset.rm_eo - __items_offset.rm_so;
    char * items_string = util_alloc_substring_copy( buffer , items_offset , items_length );
    basic_parser_type * parser = basic_parser_alloc("," , NULL , NULL , NULL , NULL , NULL); // Does not tolerate spaces around the splitting ","

    loop->items          = basic_parser_tokenize_buffer( parser , items_string , false);

    basic_parser_free( parser );
    free( items_string );
  }
  return loop;
}

static void loop_set_endmatch( loop_type * loop , int global_offset , regmatch_t end_offset) {
  loop->endtag_offset = global_offset + end_offset.rm_so;
  loop->endtag_length = end_offset.rm_eo - end_offset.rm_so;
  loop->body_length   = loop->endtag_offset - loop->opentag_offset - loop->opentag_length;
}


static void loop_free( loop_type * loop ) {
  free( loop->loop_var );
  stringlist_free( loop->items );
  free( loop );
}




static void replace_1var( buffer_type * var_expansion , int shift , int write_offset , int shift_offset , const char * value) {
  buffer_memshift( var_expansion , shift_offset , shift );
  buffer_fseek( var_expansion , write_offset , SEEK_SET );
  buffer_fwrite( var_expansion , value , strlen(value) , 1);
}


static void loop_eval( const loop_type * loop , const char * body , buffer_type * var_expansion , int ivar) {
  buffer_clear( var_expansion );
  buffer_fwrite_char_ptr( var_expansion , body );
  {
    const char * value = stringlist_iget( loop->items , ivar );
    int value_length = strlen( value );
    int shift        = value_length - loop->var_length;
    int search_offset = 0;
    
    
    while (true) {
      char * data = buffer_get_data( var_expansion );
      char * match_ptr = strstr( &data[search_offset] , loop->loop_var );
      
      if (match_ptr == NULL) 
        break;
      else {

        /* Check that the match is either at the very start of the
           string, or alternatively NOT preceeded by alphanumeric
           character. If the variable starts with a '$' we ignore this
           test.
        */  
        if (!loop->replace_substring) {
          if (match_ptr != &data[search_offset]) { char
              pre_char = match_ptr[-1]; if (isalnum( pre_char )) {
              search_offset = match_ptr - data + 1;
              
              if (search_offset >= strlen(data))
                break;
              else
                continue;
            }
          }


          /* 
             Check that the match is at the very end of the string, or
             alternatively followed by a NON alphanumeric character. */
          if (strlen(match_ptr) > loop->var_length) {
            char end_char = match_ptr[ loop->var_length ];
            if (isalnum( end_char )) 
              break;
          }
        }
          
        /* OK - this is a valid match; update the string buffer. */
        {
          int write_offset = match_ptr - data;
          int shift_offset = write_offset + loop->var_length;
          
          replace_1var( var_expansion , shift , write_offset , shift_offset , value );
          search_offset = write_offset + loop->var_length;
        }
      }
    }
  }
}


int template_eval_loop( const template_type * template , buffer_type * buffer , int global_offset , loop_type * loop) {
  int flags = 0;
  int NMATCH = 3;
  regmatch_t match_list_loop[NMATCH];
  regmatch_t match_list_end[NMATCH];
  int search_offset = loop->opentag_offset + loop->opentag_length;
  {
    int end_match , loop_match;
    {
      char * search_data = buffer_get_data( buffer );  // This pointer must be refetched because the buffer can realloc and move it.
      loop_match  = regexec( &template->start_regexp , &search_data[search_offset] , NMATCH , match_list_loop , flags );
      end_match   = regexec( &template->end_regexp   , &search_data[search_offset] , NMATCH , match_list_end , flags );
    }

    if (end_match == REG_NOMATCH)
      return -1; // This for loop does not have a matching {% endfor %}

    if (loop_match == 0) {
      if (match_list_loop[0].rm_so < match_list_end[0].rm_so) {
        // This is a nested sub loop - evaluate that recursively.
        char * search_data = buffer_get_data( buffer );
        loop_type * sub_loop = loop_alloc( search_data , search_offset , match_list_loop[0] , match_list_loop[1] , match_list_loop[2]);
        global_offset = template_eval_loop( template , buffer , global_offset , sub_loop );
      }
    } 

    /*****************************************************************/
    /* We have completed the sub loops; the buffer has (possibly)
       changed so we must search for the end again - to get the right
       offset. */
    {
      char * search_data = buffer_get_data( buffer );
      search_offset = global_offset;
      end_match   = regexec( &template->end_regexp   , &search_data[search_offset] , NMATCH , match_list_end , flags );
      if (end_match == REG_NOMATCH)
        util_exit("Fatal error - have lost a {% endfor %} marker \n");
    }
    
    /* This loop is the inner loop - expand it. */
    loop_set_endmatch( loop , search_offset , match_list_end[0]);
    {
      buffer_type * loop_expansion = buffer_alloc( loop->body_length * stringlist_get_size( loop->items) );
      {
        char * src_data = buffer_get_data( buffer );
        char * body = util_alloc_substring_copy( src_data , loop->body_offset , loop->body_length );
        buffer_type * var_expansion = buffer_alloc( 0 );
        int ivar;
        
        for (ivar =0; ivar < stringlist_get_size( loop->items ); ivar++) {
          loop_eval(loop , body , var_expansion , ivar );
          buffer_strcat( loop_expansion , buffer_get_data( var_expansion ));
        }
        
        buffer_free( var_expansion );
        util_safe_free( body );
      }
      {
        int tag_length = loop->endtag_offset + loop->endtag_length - loop->opentag_offset;
        int offset = loop->endtag_offset + loop->endtag_length;
        int shift = buffer_get_string_size( loop_expansion ) - tag_length;
        
        buffer_memshift( buffer , offset , shift );
        buffer_fseek( buffer , loop->opentag_offset , SEEK_SET );
        buffer_fwrite( buffer , buffer_get_data( loop_expansion ) , 1 , buffer_get_string_size( loop_expansion ));
        
        global_offset = loop->opentag_offset + buffer_get_string_size( loop_expansion );
      }
      buffer_free( loop_expansion );
    }
    loop_free( loop );
  }
  return global_offset;
}




void template_init_loop_regexp( template_type * template ) {
  regcompile( &template->start_regexp , LOOP_REGEXP , LOOP_OPTIONS );
  regcompile( &template->end_regexp , END_REGEXP , END_OPTIONS );
}


void template_eval_loops( const template_type * template , buffer_type * buffer ) {
  int NMATCH = 10;
  regmatch_t match_list[NMATCH];
  {
    int global_offset = 0;
    while( true ) {
      char * src_data = buffer_get_data( buffer );
      int match = regexec(&template->start_regexp , &src_data[global_offset] , NMATCH , match_list , 0);
      if (match == 0) {
        loop_type * loop = loop_alloc( src_data , global_offset , match_list[0] , match_list[1] , match_list[2]);
        global_offset = template_eval_loop( template , buffer , global_offset , loop );
        if (global_offset < 0) {
          fprintf(stderr,"** Warning ** : found {%% for .... %%} loop construct without mathcing {%% endfor %%}\n");
          break;
        }
      } else if (match == REG_NOMATCH) 
        break;
    }
  }
}
