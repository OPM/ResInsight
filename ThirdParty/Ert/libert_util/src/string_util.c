/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'string_util.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ctype.h>

#include <ert/util/util.h>
#include <ert/util/parser.h>
#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/string_util.h>
#include <ert/util/type_vector_functions.h>

/*****************************************************************/

/*
   This functions parses an input string 'range_string' of the type:

     "0,1,8, 10 - 20 , 15,17-21"

   I.e. integers separated by "," and "-". The integer values are
   parsed out.
*/

//#include <stringlist.h>
//#include <tokenizer.h>
//static int * util_sscanf_active_range__NEW(const char * range_string , int max_value , bool * active , int * _list_length) {
//  tokenizer_type * tokenizer = tokenizer_alloc( NULL  , /* No ordinary split characters. */
//                                                NULL  , /* No quoters. */
//                                                ",-"  , /* Special split on ',' and '-' */
//                                                " \t" , /* Removing ' ' and '\t' */
//                                                NULL  , /* No comment */
//                                                NULL  );
//  stringlist_type * tokens;
//  tokens = tokenize_buffer( tokenizer , range_string , true);
//
//  stringlist_free( tokens );
//  tokenizer_free( tokenizer );
//}



static bool valid_characters( const char * range_string ) {
  bool valid = false;
  if (range_string) {
    int offset = 0;
    valid = true;
    while (true) {
      char c = range_string[offset];
      if (isspace(c) || isdigit(c) || c == ',' || c == '-')
        offset++;
      else
        valid = false;
      if (offset == strlen( range_string ) || !valid)
        break;
    }
  }
  return valid;
}



static int_vector_type * string_util_sscanf_alloc_active_list(const char * range_string ) {
  int_vector_type *active_list = NULL;
  bool valid = valid_characters( range_string );

  if (valid)
  {
    basic_parser_type * parser = basic_parser_alloc( ","   , /* No ordinary split characters. */
                                         NULL  , /* No quoters. */
                                         NULL  , /* No special split */
                                         " \t" , /* Removing ' ' and '\t' */
                                         NULL  , /* No comment */
                                         NULL  );
    stringlist_type * tokens;
    int item;
    active_list = int_vector_alloc(0,0);
    tokens = basic_parser_tokenize_buffer( parser , range_string , true);

    for (item = 0; item < stringlist_get_size( tokens ); item++) {
      const char * string_item = stringlist_iget( tokens , item );
      char * pos_ptr = (char *) string_item;
      int value1 , value2;

      value1 = strtol( string_item , &pos_ptr , 10);
      if (*pos_ptr == '\0')
        // The pos_ptr points to the end of the string, i.e. this was a single digit.
        value2 = value1;
      else {
        // OK - this is a range; skip spaces and the range dash '-'
        while (isspace(*pos_ptr) || *pos_ptr == '-')
          pos_ptr++;
        util_sscanf_int( pos_ptr , &value2);
      }

      {
        int value;
        for (value = value1; value <= value2; value++)
          int_vector_append( active_list , value );
      }
    }


    stringlist_free( tokens );
    basic_parser_free( parser );
  }

  return active_list;
}




/*****************************************************************/




bool string_util_update_active_list( const char * range_string , int_vector_type * active_list ) {
  int_vector_sort( active_list );
  {
    bool_vector_type * mask = int_vector_alloc_mask( active_list );
    bool valid = false;

    if (string_util_update_active_mask( range_string , mask )) {
      int_vector_reset( active_list );
      {
        int i;
        for (i=0; i < bool_vector_size(mask); i++) {
          bool active = bool_vector_iget( mask , i );
          if (active)
            int_vector_append( active_list , i );
        }
      }
      valid = true;
    }
    bool_vector_free( mask );
    return valid;
  }
}


bool string_util_init_active_list( const char * range_string , int_vector_type * active_list ) {
  int_vector_reset( active_list );
  return string_util_update_active_list( range_string , active_list );
}


int_vector_type *  string_util_alloc_active_list( const char * range_string ) {
  int_vector_type * active_list = int_vector_alloc( 0 , 0 );
  string_util_init_active_list( range_string , active_list );
  return active_list;
}

/*****************************************************************/

/*
  This is the only function which actually invokes the low level
  string parsing in util_sscanf_alloc_active_list().
*/

bool string_util_update_active_mask( const char * range_string , bool_vector_type * active_mask) {
  int i;
  int_vector_type * sscanf_active = string_util_sscanf_alloc_active_list( range_string );
  if (sscanf_active) {
    for (i=0; i < int_vector_size( sscanf_active ); i++)
      bool_vector_iset( active_mask , int_vector_iget(sscanf_active , i) , true );

    int_vector_free( sscanf_active );
    return true;
  } else
    return false;
}


bool string_util_init_active_mask( const char * range_string , bool_vector_type * active_mask ) {
  bool_vector_reset( active_mask );
  return string_util_update_active_mask( range_string , active_mask );
}


bool_vector_type * string_util_alloc_active_mask( const char * range_string ) {
  bool_vector_type * mask  = bool_vector_alloc(0 , false );
  string_util_init_active_mask( range_string , mask );
  return mask;
}


/*****************************************************************/


bool string_util_update_value_list( const char * range_string , int_vector_type * value_list) {
  int_vector_type * new_values = string_util_sscanf_alloc_active_list( range_string );
  if (new_values) {
    int_vector_append_vector( value_list , new_values);
    int_vector_free( new_values );
    return true;
  } else
    return false;
}



bool string_util_init_value_list( const char * range_string , int_vector_type * value_list ) {
  int_vector_reset( value_list );
  return string_util_update_value_list( range_string , value_list );
}


int_vector_type * string_util_alloc_value_list(const char * range_string) {
  int_vector_type * value_list = int_vector_alloc(0,0);
  string_util_init_value_list( range_string , value_list);
  return value_list;
}
