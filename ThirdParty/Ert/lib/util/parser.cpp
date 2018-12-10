/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'parser.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <ert/util/util.h>
#include <ert/util/parser.hpp>
#include <ert/util/buffer.hpp>

#define PARSER_ESCAPE_CHAR '\\'



struct basic_parser_struct
{
  char * splitters;         /* The string is split into tokens on the occurence of one of these characters - and they are removed. */
  char * specials;          /* This exactly like the splitters - but these characters are retained as tokens. */
  char * delete_set;        /* The chracters are just plain removed - but without any splitting on them. */
  char * quoters;
  char * comment_start;
  char * comment_end;
};



static void __verify_string_length( const char * s) {
  if ((s != NULL) && (strlen(s) == 0))
    util_abort("%s: invalid input to parser_set_xxx function - zero length string.\n",__func__);
}


void basic_parser_set_splitters( basic_parser_type * parser , const char * splitters ) {
  __verify_string_length( splitters );
  parser->splitters = util_realloc_string_copy( parser->splitters , splitters );
}


void basic_parser_set_quoters( basic_parser_type * parser , const char * quoters ) {
  __verify_string_length( quoters );
  parser->quoters = util_realloc_string_copy( parser->quoters , quoters );
}

void basic_parser_set_specials( basic_parser_type * parser , const char * specials ) {
  __verify_string_length( specials );
  parser->specials = util_realloc_string_copy( parser->specials , specials );
}


void basic_parser_set_delete_set( basic_parser_type * parser , const char * delete_set ) {
  __verify_string_length( delete_set );
  parser->delete_set = util_realloc_string_copy( parser->delete_set , delete_set );
}

void basic_parser_set_comment_start( basic_parser_type * parser , const char * comment_start ) {
  __verify_string_length( comment_start );
  parser->comment_start = util_realloc_string_copy( parser->comment_start , comment_start );
}


void basic_parser_set_comment_end( basic_parser_type * parser , const char * comment_end ) {
  __verify_string_length( comment_end );
  parser->comment_end = util_realloc_string_copy( parser->comment_end , comment_end );
}


basic_parser_type * basic_parser_alloc(
  const char * splitters,        /** Set to NULL if not interessting.            */
  const char * quoters,          /** Set to NULL if not interessting.            */
  const char * specials,         /** Set to NULL if not interessting.            */
  const char * delete_set,
  const char * comment_start,    /** Set to NULL if not interessting.            */
  const char * comment_end)      /** Set to NULL  if not interessting.           */
{
  basic_parser_type * parser = (basic_parser_type*)util_malloc(sizeof * parser);
  parser->splitters     = NULL;
  parser->delete_set    = NULL;
  parser->quoters       = NULL;
  parser->specials      = NULL;
  parser->comment_start = NULL;
  parser->comment_end   = NULL;

  basic_parser_set_splitters( parser , splitters );
  basic_parser_set_quoters( parser , quoters );
  basic_parser_set_specials( parser , specials );
  basic_parser_set_delete_set( parser , delete_set );
  basic_parser_set_comment_start( parser , comment_start );
  basic_parser_set_comment_end( parser , comment_end );

  if(comment_start == NULL && comment_end != NULL)
    util_abort("%s: Need to have comment_start when comment_end is set.\n", __func__);
  if(comment_start != NULL && comment_end == NULL)
    util_abort("%s: Need to have comment_end when comment_start is set.\n", __func__);

  return parser;
}



void basic_parser_free(basic_parser_type * parser) {

  free( parser->splitters    );
  free( parser->quoters       );
  free( parser->specials      );
  free( parser->comment_start );
  free( parser->comment_end   );
  free( parser->delete_set    );

  free( parser     );
}



static bool is_escape(const char c) {
  if( c == PARSER_ESCAPE_CHAR )
    return true;
  else
    return false;
}




static int length_of_initial_splitters(const char * buffer_position, const basic_parser_type * parser) {
  assert( buffer_position != NULL );
  assert( parser       != NULL );

  if( parser->splitters == NULL)
    return 0;
  else
    return strspn( buffer_position, parser->splitters );
}

static bool in_set(char c , const char * set) {
  if (set == NULL)
    return false;
  else {
    if (strchr( set , (int) c) != NULL)
      return true;
    else
      return false;
  }
}


static bool is_splitters( const char  c, const basic_parser_type * parser) {
  return in_set(c , parser->splitters);
}

static bool is_special( const char c, const basic_parser_type * parser) {
  return in_set(c , parser->specials);
}


static bool is_in_quoters( const char c,  const basic_parser_type * parser) {
  return in_set(c , parser->quoters);
}

static bool is_in_delete_set(const char c , const basic_parser_type * parser) {
  return in_set(c , parser->delete_set);
}


/**
  This finds the number of characters up til
  and including the next occurence of buffer[0].

  E.g. using this funciton on

  char * example = "1231abcd";

  should return 4.

  If the character can not be found, the function will fail with
  util_abort() - all quotation should be terminated (Joakim - WITH
  moustache ...). Observe that this function does not allow for mixed
  quotations, i.e. both ' and " might be vald as quaotation
  characters; but one quoted string must be wholly quoted with EITHER
  ' or ".

  Escaped occurences of the first character are
  not counted. E.g. if PARSER_ESCAPE_CHAR
  occurs in front of a new occurence of the first
  character, this is *NOT* regarded as the end.
*/

static int length_of_quotation( const char * buffer) {
  assert( buffer != NULL );
  {
  int  length  = 1;
  char target  = buffer[0];
  char current = buffer[1];

  bool escaped = false;
  while(current != '\0' &&  !(current == target && !escaped ))
  {
    escaped = is_escape(current);
    length += 1;
    current = buffer[length];
  }
  length += 1;

  if ( current == '\0') /* We ran through the whole string without finding the end of the quotation - abort HARD. */
    util_abort("%s: could not find quotation closing on %s \n",__func__ , buffer);


  return length;
  }
}



static int length_of_comment(  const char * buffer_position, const basic_parser_type * parser) {
  bool in_comment = false;
  int length = 0;

  if(parser->comment_start == NULL || parser->comment_end == NULL)
    length = 0;
  else {
    const char * comment_start     = parser->comment_start;
    int          len_comment_start = strlen( comment_start );
    if( strncmp( buffer_position, comment_start, len_comment_start) == 0) {
      in_comment = true;
      length     = len_comment_start;
    } else
      length = 0;
  }

  if( in_comment ) {
    const char * comment_end       = parser->comment_end;
    int          len_comment_end   = strlen( comment_end   );
    while(buffer_position[length] != '\0' && in_comment) {
      if( strncmp( &buffer_position[length], comment_end, len_comment_end) == 0)  {
        in_comment = false;
        length += len_comment_end;
      } else
        length += 1;
    }
  }
  return length;
}



static char * alloc_quoted_token( const char * buffer, int length,  bool strip_quote_marks) {
  char * token;
  if(!strip_quote_marks) {
    token = (char*)util_calloc( (length + 1) , sizeof * token );
    memmove(token, &buffer[0], length * sizeof * token );
    token[length] = '\0';
  } else  {
    token = (char*)util_calloc( (length - 1) , sizeof * token);
    memmove(token, &buffer[1], (length -1) * sizeof * token);
    token[length-2] = '\0';
    /**
      Removed escape char before any escaped quotation starts.
    */
    {
      char expr[3];
      char subs[2];
      expr[0] = PARSER_ESCAPE_CHAR;
      expr[1] = buffer[0];
      expr[2] = '\0';
      subs[0] = buffer[0];
      subs[1] = '\0';
      util_string_replace_inplace(&token, expr, subs);
    }
  }
  return token;
}




/**
    This does not care about the possible occurence of characters in
    the delete_set. That is handled when the token is inserted in the
    token list.
*/

static int length_of_normal_non_splitters( const char * buffer, const basic_parser_type * parser) {
  bool at_end  = false;
  int length   = 0;
  char current = buffer[0];

  while(current != '\0' && !at_end)   {
    length += 1;
    current = buffer[length];

    if( is_splitters( current, parser ) )  {
      at_end = true;
      continue;
    }
    if( is_special( current, parser ) )  {
      at_end = true;
      continue;
    }
    if( is_in_quoters( current, parser ) )  {
      at_end = true;
      continue;
    }
    if( length_of_comment(&buffer[length], parser) > 0)  {
      at_end = true;
      continue;
    }
  }

  return length;
}



static int length_of_delete( const char * buffer , const basic_parser_type * parser) {
  int length   = 0;
  char current = buffer[0];

  while(is_in_delete_set( current , parser ) && current != '\0') {
    length += 1;
    current = buffer[length];
  }
  return length;
}


/**
   Allocates a new stringlist.
*/
stringlist_type * basic_parser_tokenize_buffer(
  const basic_parser_type    * parser,
  const char           * buffer,
  bool                   strip_quote_marks)
{
  int position          = 0;
  int buffer_size       = strlen(buffer);
  int splitters_length  = 0;
  int comment_length    = 0;
  int delete_length     = 0;

  stringlist_type * tokens = stringlist_alloc_new();

  while( position < buffer_size ) {
    /**
      Skip initial splitters.
    */
    splitters_length = length_of_initial_splitters( &buffer[position], parser );
    if(splitters_length > 0) {
      position += splitters_length;
      continue;
    }


    /**
      Skip comments.
    */
    comment_length = length_of_comment( &buffer[position], parser);
    if(comment_length > 0) {
      position += comment_length;
      continue;
    }


    /**
       Skip characters which are just deleted.
    */

    delete_length = length_of_delete( &buffer[position] , parser );
    if (delete_length > 0) {
      position += delete_length;
      continue;
    }



    /**
       Copy the character if it is in the special set,
    */
    if( is_special( buffer[position], parser ) )  {
      char key[2];
      key[0] = buffer[position];
      key[1] = '\0';
      stringlist_append_copy( tokens, key );
      position += 1;
      continue;
    }

    /**
       If the character is a quotation start, we copy the whole quotation.
    */
    if( is_in_quoters( buffer[position], parser ) )  {
      int length   = length_of_quotation( &buffer[position] );
      char * token = alloc_quoted_token( &buffer[position], length, strip_quote_marks );
      stringlist_append_copy( tokens, token );
      position += length;
      free(token);
      continue;
    }

    /**
      If we are here, we are guaranteed that that
      buffer[position] is not:

      1. Whitespace.
      2. The start of a comment.
      3. A special character.
      4. The start of a quotation.
      5. Something to delete.

      In other words, it is the start of plain
      non-splitters. Now we need to find the
      length of the non-splitters until:

      1. Whitespace starts.
      2. A comment starts.
      3. A special character occur.
      4. A quotation starts.
    */

    {
      int length   = length_of_normal_non_splitters( &buffer[position], parser );
      char * token = (char*)util_calloc( (length + 1) , sizeof * token);
      int token_length;
      if (parser->delete_set == NULL) {
        token_length = length;
        memcpy( token , &buffer[position] , length * sizeof * token );
      } else {
        int i;
        token_length = 0;
        for (i = 0; i < length; i++) {
          char c = buffer[position + i];
          if ( !is_in_delete_set( c , parser)) {
            token[token_length] = c;
            token_length++;
          }
        }
      }


      if (token_length > 0) { /* We do not insert empty tokens. */
        token[token_length] = '\0';
        stringlist_append_copy( tokens, token );
      }
      free( token );

      position += length;
      continue;
    }
  }

  return tokens;
}



stringlist_type * basic_parser_tokenize_file(const basic_parser_type * parser, const char * filename, bool strip_quote_marks) {
  stringlist_type * tokens;
  char * buffer = util_fread_alloc_file_content( filename, NULL );
  tokens = basic_parser_tokenize_buffer( parser, buffer, strip_quote_marks );
  free(buffer);
  return tokens;
}


/*****************************************************************/
/* Below are some functions which do not actually tokenize, but use
   the comment/quote handling of the parser implementation for
   related tasks.
*/

static bool fseek_quote_end( char quoter , FILE * stream ) {
  int c;
  do {
    c = fgetc( stream );
  } while (c != quoter && c != EOF);

  if (c == quoter)
    return true;
  else
    return false;
}



static bool fgetc_while_equal( FILE * stream , const char * string , bool case_sensitive) {
  bool     equal        = true;
  long int current_pos  = util_ftell(stream);
  size_t string_index;
  for ( string_index = 0; string_index < strlen(string); string_index++) {
    int c = fgetc( stream );
    if (!case_sensitive)
      c = toupper( c );

    if (c != string[string_index]) {
      equal = false;
      break;
    }
  }

  if (!equal) /* OK - not equal - go back. */
    util_fseek( stream , current_pos , SEEK_SET);
  return equal;
}





/**
   The return value is whether the string could be found or not. If
   the string is found, the stream position is positionoed to point at
   (or immediatbely after) the string; if not the stream pointer is
   left at the initial position.

   This function is quite tolerant - it will accept (with a warning)
   unterminated comments and unterminated quotations.
*/

bool basic_parser_fseek_string(const basic_parser_type * parser , FILE * stream , const char * __string , bool skip_string, bool case_sensitive) {
  bool string_found        = false;
  char * string            = util_alloc_string_copy( __string );
  if (!case_sensitive)
    util_strupr( string );
  {
    long int initial_pos     = util_ftell( stream );   /* Store the inital position. */
    bool cont                = true;

    if (strstr( string , parser->comment_start ) != NULL)
      util_abort("%s: sorry the string contains a comment start - will never find it ... \n",__func__); /* A bit harsh ?? */

    do {
      int c = fgetc( stream );
      if (!case_sensitive) c = toupper( c );

      /* Special treatment of quoters - does not properly handle escaping of the quoters. */
      if (is_in_quoters( c , parser )) {
        long int quote_start_pos = util_ftell(stream);
        if (!fseek_quote_end( c , stream )) {
          util_fseek( stream ,  quote_start_pos , SEEK_SET);
          fprintf(stderr,"Warning: unterminated quotation starting at line: %d \n",util_get_current_linenr( stream ));
          util_fseek(stream , 0 , SEEK_END);
        }
        /*
           Now we are either at the first character following a
           terminated quotation, or at EOF.
        */
        continue;
      }

      /* Special treatment of comments: */
      if (c == parser->comment_start[0]) {
        /* OK - this might be the start of a comment - let us check further. */
        bool comment_start = fgetc_while_equal( stream , &parser->comment_start[1] , false);
        if (comment_start) {
          long int comment_start_pos = util_ftell(stream) - strlen( parser->comment_start );
          /* Start seeking for comment_end */
          if (!util_fseek_string(stream , parser->comment_end , true , true)) {
            /*
               No end comment end was found - what to do about that??
               The file is just positioned at the end - and the routine
               will exit at the next step - with a Warning.
            */
            util_fseek( stream , comment_start_pos , SEEK_SET);
            fprintf(stderr,"Warning: unterminated comment starting at line: %d \n",util_get_current_linenr( stream ));
            util_fseek(stream , 0 , SEEK_END);
          } continue;
          /* Now we are at the character following a comment end - or at EOF. */
        }
      }

      /*****************************************************************/

      /* Now c is a regular character - and we can start looking for our string. */
      if (c == string[0]) {  /* OK - we got the first character right - lets try in more detail: */
        bool equal = fgetc_while_equal( stream , &string[1] , case_sensitive);
        if (equal) {
          string_found = true;
          cont = false;
        }
      }

      if (c == EOF)
        cont = false;

    } while (cont);

    if (string_found) {
      if (!skip_string) {
        offset_type offset = (offset_type) strlen( string );
        util_fseek(stream , -offset , SEEK_CUR); /* Reposition to the beginning of 'string' */
      }
    } else
      util_fseek(stream , initial_pos , SEEK_SET);       /* Could not find the string reposition at initial position. */
  }
  free( string );
  return string_found;
}



/**
   This function takes an input buffer, and updates the buffer (in place) according to:

   1. Quoted content is copied verbatim (this takes presedence).
   2. All comment sections are removed.
   3. Delete characters are deleted.

*/


void basic_parser_strip_buffer(const basic_parser_type * parser , char ** __buffer) {
  char * src     = *__buffer;
  char * target  = (char*)util_calloc( ( strlen( *__buffer ) + 1) , sizeof * target );

  size_t src_position    = 0;
  int target_position = 0;
  while (src_position < strlen( src )) {
    int comment_length;
    int delete_length;

    /**
      Skip comments.
    */
    comment_length = length_of_comment( &src[src_position], parser);
    if(comment_length > 0)
    {
      src_position += comment_length;
      continue;
    }


    /**
       Skip characters which are just deleted.
    */
    delete_length = length_of_delete( &src[src_position] , parser );
    if (delete_length > 0) {
      src_position += delete_length;
      continue;
    }

    /*
      Quotations.
    */
    if( is_in_quoters( src[src_position], parser ) ) {
      int length   = length_of_quotation( &src[src_position] );
      char * token = alloc_quoted_token( &src[src_position], length, false );
      memcpy( &target[target_position] , &src[src_position] , length);
      free( token );
      src_position    += length;
      target_position += length;
      continue;
    }

    /**
       OK -it is a god damn normal charactar - copy it straight over:
    */
    target[target_position] = src[src_position];
    src_position    += 1;
    target_position += 1;
  }
  target[target_position] = '\0';
  target = (char*)util_realloc( target , sizeof * target * (target_position + 1) );

  free( src );
  *__buffer = target;
}



/*****************************************************************/
/**
   This file reads file content into a buffer, and then strips the
   buffer with parser_strip_buffer() and returns the 'cleaned up'
   buffer.

   This function is a replacement for the old
   util_fread_alloc_file_content() which no longer has support for a
   comment string.
*/

char * basic_parser_fread_alloc_file_content(const char * filename , const char * quote_set , const char * delete_set , const char * comment_start , const char * comment_end) {
  basic_parser_type * parser = basic_parser_alloc( NULL , quote_set , NULL , delete_set , comment_start , comment_end);
  char * buffer              = util_fread_alloc_file_content( filename , NULL);

  basic_parser_strip_buffer( parser , &buffer );
  basic_parser_free( parser );
  return buffer;
}
