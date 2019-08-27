/*
   Copyright (C) 2011  Equinor ASA, Norway.

   The file 'parser.h' is part of ERT - Ensemble based Reservoir Tool.

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

#ifndef ERT_PARSER_H
#define ERT_PARSER_H
#include <ert/util/stringlist.hpp>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct basic_parser_struct basic_parser_type;


/**
  GENERAL OVERVIEW

  The parser_type is used to create a series of "tokens"
  from a file or string buffer. In it's simplest form,
  we define a token as a subset of a string separated by
  by some split characters.

  For example, if we define the normal space (i.e. " ") as
  the only split character, "tokenizing" the string
  "I like     beer  " would give the following result:

  Token number 0 is "I"
  Token number 1 is "like"
  Token number 2 is "beer"

  Note that all the white space (i.e. split characters) have been
  removed.



  COMMENTS

  The parser can ignore comments when tokenzing
  a file or buffer. To enable this feature, allocate
  the parser_type with comment_start and comment_end
  different from NULL. For example if we set both
  comment_start and comment_end to "##", tokenizing
  "I ## really  ## like beer" would give:

  Token number 0 is "I"
  Token number 1 is "like"
  Token number 2 is "beer"



  SPECIAL CHARACTERS

  Some times it can be useful to define a set of characters which
  behave like white space in the sense that they separate tokens in
  the source, but they do not get dropped. For example, letting "=" be
  a special character, tokenzing "key=value" would give:

  Token number 0 is "key"
  Token number 1 is "="
  Token number 2 is "value"

  The special characters are given in the "specials" string when
  allocating the parser.



  QUOTERS

  When parsing user input, the user often wants to provide e.g. a
  filename with a white-space character in it. To support this, the
  parser can be given a set of quoters. For example, letting " " be
  white space and adding "'" to the quoters, tokenizing

   "my_file = 'my documents with space in.txt'"

  would give:

  Token number 0 is "my_file"
  Token number 1 is "="
  Token number 2 is "'my documents with space in.txt'"

  If wanted, the quoting characters can be removed
  using the strip_quote_marks options when running
  the parser on the buffer. The last token
  in the example above would then be:

  Token number 2 is "my documents with space in.txt"

  To use one of the quoter characters in a string,
  place a "\" in front of it. Building on our previous
  example, let the string be "my_file = 'my \'doc.txt'"
  Tokenzing this with strip_quote_marks set to true
  would give:

  Token number 0 is "my_file"
  Token number 1 is "="
  Token number 2 is "my 'doc.txt"

  Note that the "\" in front of"'" has been removed.
  If strip_quote_marks is set to false, the result is:


  Token number 0 is "my_file"
  Token number 1 is "="
  Token number 2 is "'my \'doc.txt'"

*/


basic_parser_type * basic_parser_alloc(
  const char * whitespace,       /** Set to NULL if not interessting.         */
  const char * quoters,          /** Set to NULL if not interessting.         */
  const char * specials,         /** Set to NULL if not interessting.         */
  const char * delete_set,
  const char * comment_start,    /** Set to NULL if not interessting.         */
  const char * comment_end);     /** Set to NULL if not interessting.         */


void       basic_parser_set_splitters( basic_parser_type * parser , const char * splitters );
void       basic_parser_set_quoters( basic_parser_type * parser , const char * quoters );
void       basic_parser_set_specials( basic_parser_type * parser , const char * specials );
void       basic_parser_set_delete_set( basic_parser_type * parser , const char * delete_set );
void       basic_parser_set_comment_start( basic_parser_type * parser , const char * comment_start );
void       basic_parser_set_comment_end( basic_parser_type * parser , const char * comment_end );


void basic_parser_free(
  basic_parser_type * parser);


stringlist_type * basic_parser_tokenize_buffer(
  const basic_parser_type * parser,
  const char              * buffer,
  bool                      strip_quote_marks);


stringlist_type * basic_parser_tokenize_file(
  const basic_parser_type * parser,
  const char              * filename,
  bool                      strip_quote_marks);


/* Pollution by Joakim: */

void   basic_parser_strip_buffer(const basic_parser_type * parser , char ** __buffer);
bool   basic_parser_fseek_string(const basic_parser_type * parser , FILE * stream , const char * string , bool skip_string , bool case_sensitive);
char * basic_parser_fread_alloc_file_content(const char * filename , const char * quote_set , const char * delete_set , const char * comment_start , const char * comment_end);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif

