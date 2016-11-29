/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'sched_util.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <string.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/buffer.h>

#include <ert/sched/sched_util.h>

/**
   This file implements small utility functions used by the rest of
   the libsched library.
*/


static const int strip_comment = 1;
static const int strip_space   = 2;


static char * strip_line_alloc(const char * line) {
  const char  comment_char = '-';
  const char *space   = " \t";
  const int strip_mode = strip_comment + strip_space;
  char * new_line = NULL;
  int offset, length,pos;
  bool cont , quote_on , dash_on;   /* The quote_on variable protects againts premature termination based on '/' in path specifications. */
  
  /* Strip intial white-space */
  if (strip_mode & strip_space)
    offset   = strspn(line , space);
  else
    offset = 0;

  
  dash_on  = false;
  quote_on = false;
  cont     = true;
  length   = 0;
  if (line[offset] != '\0') {
    pos = offset;
    do {
      if (line[pos] == '\'' || line[pos] == '"')
       quote_on = !quote_on;
      
      if (strip_mode & strip_comment) {
       if (!quote_on) {
         if (line[pos] == comment_char) {
           if (dash_on) {
             cont   = false;
             length = pos - offset - 1;
           } else 
             dash_on = true;
         } else
           dash_on = false;
       }
      }

      if (cont) {
       if (pos == (strlen(line) - 1)) {
         length = pos - offset + 1;
         cont = false;
       }
      }
      
      if (cont)
       pos++;
    } while (cont);

    /*
      Remove trailing space:
    */

    if (strip_mode & strip_space) {
      if (offset + length > 0) {
       while (line[offset + length - 1] == ' ') 
         length--;
      }
    }

    if (length > 0) 
      new_line = util_realloc_substring_copy(NULL , &line[offset] , length);
    else 
      new_line = NULL;
    
  } 
  
  return new_line;
}


/*****************************************************************/



char * sched_util_alloc_line(FILE *stream , bool *at_eof) {
  char *tmp_line = util_fscanf_alloc_line(stream , at_eof);
  char * line    = strip_line_alloc( tmp_line );
  
  free(tmp_line);
  return line;
}






/**
 * We parse up to the terminating '/' - but it is NOT included in the returned string 

   The num_tokens variable is only used to fill up with defaults at the end.
*/
stringlist_type * sched_util_alloc_line_tokens( const stringlist_type * tokens , bool untyped , int num_tokens , int * __token_index ) {
  /** First part - identify the right start/end of the token list */
  stringlist_type * line_tokens = NULL;
  int token_index  = *__token_index;
  int token_length = stringlist_get_size( tokens );
  int line_start;
  int line_end;
  bool at_eokw = false;
  {
    line_start = token_index;
    const char * current_token;
    {
      bool at_eol = false;
      do {
        current_token = stringlist_iget( tokens , token_index );
        if (strcmp( current_token , "/" ) == 0) 
          at_eol = true;
        
        token_index++;
        
        // The schedule file is not correctly terminated with a "/".
        if (token_index == token_length) 
          at_eol = true;

      } while (!at_eol);
    }
    line_end = token_index;
    if ((line_end - line_start) == 1) 
      /* 
         This line *only* contained a terminating '/'. This marks the
         end of the kewyord.
      */
      at_eokw = true;
  }

  
  /* Second part - filling in with defaults+++ */
  if (!at_eokw) {
    line_tokens = stringlist_alloc_new( );
    if (untyped) {  /* In case of untyped we basically do nothing with the content - even keeping the trailing '/'. */
      int it;
      for (it = line_start; it < line_end; it++) {
        const char * token          = stringlist_iget( tokens , it );
        stringlist_append_copy(line_tokens , token );
      }
    } else {
      int it;
      for (it = line_start; it < (line_end - 1); it++) {
        const char * token          = stringlist_iget( tokens , it );
        char       * dequoted_token = util_alloc_dequoted_copy( token );
        
        if (util_string_equal( dequoted_token , SCHED_KW_DEFAULT_ITEM ))                /* The item is just '*'  */
          stringlist_append_copy(line_tokens , SCHED_KW_DEFAULT_ITEM );
        else {
          char repeated_value[32];
          long int items;
          if (sscanf(dequoted_token , "%ld*%s" , &items , repeated_value) == 2) {       /* It is a '5*8.60' item - i.e. the value 8.60 repeated five times. */
            int counter = 0;
            do {
              stringlist_append_copy(line_tokens , repeated_value);
              counter++;
            } while ( counter < items );
          } else {
            char * star_ptr = (char *) dequoted_token;
            items           = strtol(dequoted_token , &star_ptr , 10);                  /* The item is a repeated default: '5*'  */
            if (star_ptr != token && util_string_equal( star_ptr , SCHED_KW_DEFAULT_ITEM )) {
              for (int i=0; i < items; i++)
                stringlist_append_copy( line_tokens , SCHED_KW_DEFAULT_ITEM );
            } else                                                                     /* The item is a non-default value. */
              stringlist_append_copy(line_tokens , dequoted_token );
          }
        }
        free( dequoted_token );
      }
    }
  }

  
  /** Skip trailing garbage */
  sched_util_skip_trailing_tokens(  tokens , &token_index );
  sched_util_skip_newline( tokens , &token_index );
  
  /* Append default items at the end until we have num_tokens length. */
  if (line_tokens != NULL) {
    while (stringlist_get_size( line_tokens ) < num_tokens)
      stringlist_append_copy( line_tokens , SCHED_KW_DEFAULT_ITEM );
  }
  
  *__token_index = token_index;
  return line_tokens;
}




void sched_util_fprintf_default(int width , FILE * stream) {
  fprintf(stream , "1*");
  for (int i=0; i < (width - 2); i++) 
    fputc(' ' , stream);
}


/**
   All the sched_util_fprintf_xxx() functions start by putting out one
   ' ', to ensure against overlapping fields.
*/


void sched_util_fprintf_dbl(bool def, double value , int width , int dec , FILE *stream) {
  fputc(' ' , stream);
  if (def) 
    sched_util_fprintf_default( width , stream);
  else 
    util_fprintf_double(value , width , dec , 'f' , stream);
}



void sched_util_fprintf_int(bool def, int value , int width , FILE *stream) {
  fputc(' ' , stream);
  if (def) {
    sched_util_fprintf_default( width , stream);
  } else 
    util_fprintf_int(value , width , stream);
}


/*
  The formatting is ridicolusly inflexible - don't touch this shit.
*/

void sched_util_fprintf_qst(bool def, const char *s , int width , FILE *stream) {
  fputc(' ' , stream);
  if (def) {
    sched_util_fprintf_default( width , stream );
  } else {
    for (int i=0; i < (width - strlen(s)); i++) 
      fputc(' ' , stream);
    
    fprintf(stream , "\'%s\'" , s);
  }
}



/**
   The atof / atoi functions accept either 'NULL' or '*' as default
   values, in that case numeric 0 is returned.
*/

double sched_util_atof(const char *token) {
  if (token != NULL) {
    double value = 0;
    if (!util_string_equal( token , SCHED_KW_DEFAULT_ITEM)) {
      if (!util_sscanf_double(token , &value))
        util_abort("%s: failed to parse:\"%s\" as floating point number. \n",__func__ , token);
    }
    return value;
  } else
    return 0.0;
}



int sched_util_atoi(const char *token) {
  if (token != NULL) {
    int value = 0;
    if (!util_string_equal( token , SCHED_KW_DEFAULT_ITEM)) {
      if (!util_sscanf_int(token , &value))
        util_abort("%s: failed to parse:\"%s\" as integer \n",__func__ , token);
    }
    return value;
  } else
    return 0;
}



/* Simple utility function used for debugging. */
void sched_util_fprintf_tokenlist(int num_token , const char ** token_list , const bool * def) {
  int i;
  for (i = 0; i < num_token; i++) {
    if (def[i])
      fprintf(stdout , " \'*\' " );
    else
      fprintf(stdout , " \'%s\' " , token_list[i]);
  }
  fprintf(stdout , "\n");
}



/**
   This should repeatedly skip tokens until the token_index points to
   a newline. Should return with token_index pointing at the first newline character.
*/

void sched_util_skip_trailing_tokens( const stringlist_type * tokens , int * __token_index ) {
  int token_index = *__token_index;
  int len         = stringlist_get_size( tokens );
  while ( (token_index < len) && (!stringlist_iequal( tokens , token_index , "\n"))) {
    token_index++;
  } 
  *__token_index = token_index;
}


void sched_util_skip_newline( const stringlist_type * tokens , int * __token_index ) {
  int token_index = *__token_index;
  int len         = stringlist_get_size( tokens );
  while ( (token_index < len) && (stringlist_iequal( tokens , token_index , "\n"))) {
    token_index++;
  } 
  *__token_index = token_index;
}





void sched_util_init_default(const stringlist_type * line_tokens , bool * def) {
  int i;
  for (i = 0; i < stringlist_get_size( line_tokens ); i++) {
    if (util_string_equal( stringlist_iget( line_tokens , i ) , SCHED_KW_DEFAULT_ITEM))
      def[i] = true;
    else
      def[i] = false;
  }
}
