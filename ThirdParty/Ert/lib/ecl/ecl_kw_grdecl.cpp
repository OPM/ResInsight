/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_kw_grdecl.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <string.h>
#include <ctype.h>

#include <ert/util/util.hpp>

#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_type.hpp>
#include <ert/ecl/ecl_util.hpp>

#define MAX_GRDECL_HEADER_SIZE 512

/*
  This file is devoted to different routines for reading and writing
  GRDECL formatted files. These files are very weakly formatted; and
  generally a pain in the ass to work with. Things to consider
  include:

   1. The files have no proper header; only the name of the keyword.

      a) If you have a corresponding grid file it might be possible to
         infer the correct size from this, otherwise you must just
         load all the data you find.

      b) There is not type information; presented with a bunch of
         formatted numbers it is in general impossible to determine
         whether the underlying datatype should be integer, float or
         double. Therefore all the file-reading routines here expect an
         ecl_data_type as input.

   2. The files can have comment sections; even in the data block.

   3. The * notation can be used to print repeated values in a compact
      form, i.e.  1000*0.25 to get 1000 conecutive 0.25 values.

  The typical ECLIPSE keywords found in the datafile contain a mixture
  of numeric and character data; the current code assumes that all the
  data of a keyword is of the same underlying type, and do NOT support
  such keywords.
*/


/*
  Will seek from the current position to the next keyword. If a valid
  next keyword is found the function will position the file reader at
  the beginning of the string header and return true, otherwise the
  file position will be left untouched and the function will return
  false.

  An eligible kw string should be first non whitespace string on a
  line; i.e. the function will start by checking if there are any
  non-whitespace characters on the current line. In that case the
  current line is skipped. Lines starting with the "--" comment marker
  are ignored.
*/


bool ecl_kw_grdecl_fseek_next_kw( FILE * stream ) {
  long start_pos = util_ftell( stream );
  long current_pos;
  char next_kw[MAX_GRDECL_HEADER_SIZE];

  /*
    Determine if the current position of the file pointer is at the
    beginning of the line; if not skip the rest of the line; this is
    applies even though the tokens leading up this are not comments.
  */
  {
    while (true) {
      char c;
      if (util_ftell(stream) == 0)
        /*
          We are at the very beginning of the file. Can just jump out of
          the loop.
        */
        break;

      util_fseek( stream , -1 , SEEK_CUR );
      c = fgetc( stream );
      if (c == '\n') {
        /*
           We have walked backwards reaching the start of the line. We
           have not reached any !isspace() characters on the way and
           can go back to start_pos and read from there.
        */
        util_fseek( stream , start_pos , SEEK_SET );
        break;
      }

      if (!isspace( c )) {
        /*
           We hit a non-whitespace character; this means that start_pos
           was not at the start of the line. We skip the rest of this
           line, and then start reading on the next line.
        */
        util_fskip_lines( stream , 1 );
        break;
      }
      util_fseek( stream , -2 , SEEK_CUR );
    }
  }


  while (true) {
    current_pos = util_ftell( stream );
    if (fscanf(stream , "%s" , next_kw) == 1) {
      if ((next_kw[0] == next_kw[1]) && (next_kw[0] == ECL_COMMENT_CHAR))
        // This is a comment line - skip it.
        util_fskip_lines( stream , 1 );
      else {
        // This is a valid keyword i.e. a non-commented out string; return true.
        util_fseek( stream , current_pos , SEEK_SET );
        return true;
      }
    } else {
      // EOF reached - return False.
      util_fseek( stream , start_pos , SEEK_SET );
      return false;
    }
  }
}


/**
   Will use the ecl_kw_grdecl_fseek_next_header() to seek out the next
   header string, and read and return that string. If no more headers
   are found the function will return NULL. The storage allocated by
   this function must be free'd by the calling scope.
*/

char * ecl_kw_grdecl_alloc_next_header( FILE * stream ) {
  if (ecl_kw_grdecl_fseek_next_kw( stream )) {
    char next_kw[MAX_GRDECL_HEADER_SIZE];
    fscanf( stream , "%s" , next_kw);
    return util_alloc_string_copy( next_kw );
  } else
    return NULL;
}


/**
  This function will search through a GRDECL file to look for the
  'kw'; input variables and return vales are similar to
  ecl_kw_fseek_kw(). Observe that the GRDECL files are extremely
  weakly structured, it is therefore veeeery easy to fool this function
  with a malformed GRDECL file.

  In particular the comparison is case sensitive; that is probably not
  the case with ECLIPSE proper?

  If the kw is not found the file pointer is repositioned.
*/

static bool ecl_kw_grdecl_fseek_kw__(const char * kw , FILE * stream) {
  long init_pos = util_ftell( stream );
  while (true) {
    if (ecl_kw_grdecl_fseek_next_kw( stream )) {
      char next_kw[256];
      fscanf( stream , "%s" , next_kw);
      if (strcmp( kw , next_kw ) == 0) {
        offset_type offset = (offset_type) strlen(next_kw);
        util_fseek( stream , -offset , SEEK_CUR);
        return true;
      }
    } else {
      util_fseek( stream , init_pos , SEEK_SET);
      return false;
    }
  }
}






bool ecl_kw_grdecl_fseek_kw(const char * kw , bool rewind , FILE * stream) {
  if (ecl_kw_grdecl_fseek_kw__(kw , stream))
    return true;       /* OK - we found the kw between current file pos and EOF. */
  else if (rewind) {
    long int init_pos = util_ftell(stream);

    util_fseek(stream , 0L , SEEK_SET);
    if (ecl_kw_grdecl_fseek_kw__( kw , stream )) /* Try again from the beginning of the file. */
      return true;
    else
      util_fseek(stream , init_pos , SEEK_SET);       /* Could not find it - reposition to initial position. */
  }

  /* OK: If we are here - that means that we failed to find the kw. */
  return false;
}



/**
   Observe that this function does not preserve the '*' structure
   which (might) have been used in the input.
*/


static void iset_range( char * data , int data_offset , int sizeof_ctype , void * value_ptr , int multiplier) {
  int index;
  for ( index =0; index < multiplier; index++)
    memcpy( &data[ (index + data_offset) * sizeof_ctype ] , value_ptr , sizeof_ctype);
}


/**
   The @strict flag is used to indicate whether the loader will accept
   character strings embedded into a numerical grdecl keyword; this
   should of course in general not be allowed and @strict should be
   set to true. However the SPECGRID keyword used when specifying a
   grid is often given as:

     SPECGRID
         10 10 100 100 F /

   Whatever that 'F' is - it is discarded when the SPECGRID header is
   written to a GRID/EGRID file. For this reason we have the
   possibility of setting @strict to false; in which case the 'F' or
   other characters in the numerical input will be ignored.

   If @strict is set to true the function will bomb when meeting a
   non-numeric character like the 'F' above.

   ----------------------------------------------------------------

   The function supports multiplier keywords like:

   PERMX
      10000*0.15  0.16 0.17 0.18 0.19 10000*0.20
   /

   Observe that no-spaces-are-allowed-around-the-*
*/

static char * fscanf_alloc_grdecl_data( const char * header , bool strict , ecl_data_type data_type , int * kw_size , FILE * stream ) {
  char newline        = '\n';
  bool atEOF          = false;
  int init_size       = 32;
  int buffer_size     = 64;
  int data_index      = 0;
  int sizeof_ctype    = ecl_type_get_sizeof_ctype( data_type );
  int data_size       = init_size;
  char * buffer       = (char*)util_calloc( (buffer_size + 1) , sizeof * buffer      );
  char * data         = (char*)util_calloc( sizeof_ctype * data_size , sizeof * data );

  while (true) {
    if (fscanf(stream , "%32s" , buffer) == 1) {
      if (strcmp(buffer , ECL_COMMENT_STRING) == 0) {
        // We have read a comment marker - just read up to the end of line.
        char c;
        while (true) {
          c = fgetc( stream );
          if (c == newline)
            break;
          if (c == EOF) {
            atEOF = true;
            break;
          }
        }
      } else if (strcmp(buffer , ECL_DATA_TERMINATION) == 0)
        break;
      else {
        // We have read a valid input string; scan numerical input values from it.
        // The multiplier algorithm will fail hard if there are spaces on either side
        // of the '*'.

        int multiplier;
        void * value_ptr = NULL;
        bool   char_input = false;

        if (ecl_type_is_int(data_type)) {
          int value;

          if (sscanf(buffer , "%d*%d" , &multiplier , &value) == 2)
            {}
          else if (sscanf( buffer , "%d" , &value) == 1)
            multiplier = 1;
          else {
            char_input = true;
            if (strict)
              util_abort("%s: Malformed content:\"%s\" when reading keyword:%s \n",__func__ , buffer , header);
          }

          value_ptr = &value;
        } else if (ecl_type_is_float(data_type)) {
          float value;

          if (sscanf(buffer , "%d*%g" , &multiplier , &value) == 2)
            {}
          else if (sscanf( buffer , "%g" , &value) == 1)
            multiplier = 1;
          else {
            char_input = true;
            if (strict)
              util_abort("%s: Malformed content:\"%s\" when reading keyword:%s \n",__func__ , buffer , header);
          }

          value_ptr = &value;
        } else if (ecl_type_is_double(data_type)) {
          double value;

          if (sscanf(buffer , "%d*%lg" , &multiplier , &value) == 2)
            {}
          else if (sscanf( buffer , "%lg" , &value) == 1)
            multiplier = 1;
          else {
            char_input = true;
            if (strict)
              util_abort("%s: Malformed content:\"%s\" when reading keyword:%s \n",__func__ , buffer , header);
          }

          value_ptr = &value;
        } else
          util_abort("%s: sorry type:%s not supported \n",__func__ , ecl_type_alloc_name(data_type));

        /*
          Removing this warning on user request:
          if (char_input)
          fprintf(stderr,"Warning: character string: \'%s\' ignored when reading keyword:%s \n",buffer , header);
        */
        if (!char_input) {
          size_t min_size = data_index + multiplier;
          if (min_size >= data_size) {
            if (min_size <= ECL_KW_MAX_SIZE) {
              size_t byte_size = sizeof_ctype * sizeof * data;

              data_size  = util_size_t_min( ECL_KW_MAX_SIZE , 2*(data_index + multiplier));
              byte_size *= data_size;

              data = (char*)util_realloc( data , byte_size );
            } else {
              /*
                We are asking for more elements than can possible be adressed in
                an integer. Return NULL - and data size == 0; let calling scope
                try to handle it.
              */
              data_index = 0;
              break;
            }
          }

          iset_range( data , data_index , sizeof_ctype , value_ptr , multiplier );
          data_index += multiplier;
        }

      }
      if (atEOF)
        break;
    } else
      break;
  }
  free( buffer );
  *kw_size = data_index;
  data = (char*)util_realloc( data , sizeof_ctype * data_index * sizeof * data );
  return data;
}

/*
   This function will load a keyword from a grdecl file, and return
   it. If input argument @kw is NULL it will just try loading from the
   current position, otherwise it will start with seeking to find @kw
   first.

   Observe that the grdecl files are very weakly structured, so the
   loading of ecl_kw instances from a grdecl file can go wrong in many
   ways; if the loading fails the function returns NULL.

   The main loop is extremely simple - it is just repeated calls to
   fscanf() to read one-number-at-atime; when that reading fails that
   is interpreted as the end of the keyword.

   Currently ONLY integer and float types are supported in ecl_type -
   any other types will lead to a hard failure.

   The ecl_kw class has a quite deeply wired assumption that the
   header is a string of length 8 (I hope/think that is an ECLIPSE
   limitation). The class cannot read/write kw with headers longer than 8 bytes. 
   ecl_kw_grdecl is a workaround allowing for reading/writing kw with long
   headers. 

   -----------------------------------------------------------------

    header: If the @header argument is != NULL the function will start
      by seeking through the file to find the header string; if the
      header can not be found the function will return NULL - but not
      fail any more than that.

      If @kw == NULL on input the function will just fscanf() the
      first available string and use that as header for the
      keyword. This can lead to failure in a zillion different ways;
      it is highly recommended to supply a valid string for the
      @header argument.

    strict: see the documentation of the strict flag in the
      fscanf_alloc_grdecl_data() function. Most of the exported
      functions have hardwired strict = true.


    size: If the @size is set to <= 0 the function will just load all
      data it can find until a terminating '/' is found. If a size
      argument is given the function will check that there is
      agreement between the size input argument and the number of
      elements found on the file.


    ecl_type: The files have no embedded type information and the type
      must be supplied by the calling scope. Currently only the
      ECL_FLOAT_TYPE and ECL_INT_TYPE types are supported.

   -----------------------------------------------------------------

   This function is static; there are several exported varieties with
   different sets of default values. These files are tricky to load -
   if there is something wrong it can be difficult to detect.
*/

static ecl_kw_type * __ecl_kw_fscanf_alloc_grdecl__(FILE * stream , const char * header , bool strict , int size , ecl_data_type data_type) {
  if (header && strlen(header) >MAX_GRDECL_HEADER_SIZE)
    util_abort("%s cannot read KW of more than %d bytes. strlen(header) == %d\n", __func__, MAX_GRDECL_HEADER_SIZE, strlen(header) );

  if (!ecl_type_is_numeric(data_type))
    util_abort("%s: sorry only types FLOAT, INT and DOUBLE supported\n",__func__);

  if (header != NULL)
    if (!ecl_kw_grdecl_fseek_kw( header , true , stream ))
      return NULL;  /* Could not find it. */

  {
    char file_header[MAX_GRDECL_HEADER_SIZE];
    if (fscanf(stream , "%s" , file_header) == 1) {
      int kw_size;
      char * data = fscanf_alloc_grdecl_data( file_header , strict , data_type , &kw_size , stream );

      // Verify size
      if (size > 0)
        if (size != kw_size) {
          util_safe_free( data );
          util_abort("%s: size mismatch when loading:%s. File:%d elements. Requested:%d elements \n",
                     __func__ , file_header , kw_size , size);
        }

      {
        ecl_kw_type * ecl_kw = ecl_kw_alloc_new( file_header , kw_size , data_type , NULL );
        ecl_kw_set_data_ptr( ecl_kw , data );
        return ecl_kw;
      }

    } else
      /** No header read - probably at EOF */
      return NULL;
  }
}
/*****************************************************************/
/*
   Here comes the exported functions for loading a grdecl formatted
   keyword. All of these function invoke the fundamental
   ecl_kw_fscanf_alloc_grdecl__() function, but the set of input
   parameters varies. The function varieties with a trailing '__'
   accepts a @strict flag from calling scope; in general you should
   use strict == true.
*/


/**
   This function assumes that the file pointer has already been
   positioned at the beginning of a keyword header, and will just
   start reading a header string right away; if the file pointer is
   incorrectly positioned this will most probably blow up big time.
*/

/*****************************************************************/

ecl_kw_type * ecl_kw_fscanf_alloc_grdecl_data__(FILE * stream , bool strict , int size ,  ecl_data_type data_type) {
  return __ecl_kw_fscanf_alloc_grdecl__( stream , NULL , strict , size , data_type );
}


ecl_kw_type * ecl_kw_fscanf_alloc_grdecl_data(FILE * stream , int size , ecl_data_type data_type) {
 bool strict = true;
 return ecl_kw_fscanf_alloc_grdecl_data__( stream , strict , size , data_type );
}

/*****************************************************************/

/*
   This function will seek through the file and position the file
   pointer at the beginning of @kw before starting to load (this
   includes rewinding the file pointer). If @kw can not be found the
   function will return NULL.

   As size is not supplied the function will keep loading data until
   the whole keyword is loaded, and then return.
*/

ecl_kw_type * ecl_kw_fscanf_alloc_grdecl_dynamic__( FILE * stream , const char * kw , bool strict , ecl_data_type data_type) {
  return __ecl_kw_fscanf_alloc_grdecl__( stream , kw , strict , 0 , data_type );
}

ecl_kw_type * ecl_kw_fscanf_alloc_grdecl_dynamic( FILE * stream , const char * kw , ecl_data_type data_type) {
  bool strict = true;
  return ecl_kw_fscanf_alloc_grdecl_dynamic__( stream , kw , strict , data_type );
}

/*****************************************************************/

/*
   This function will seek through the file and position the file
   pointer at the beginning of @kw before starting to load (this
   includes rewinding the file pointer). If @kw can not be found the
   function will return NULL.

   When the data has been loaded the function will compare actual size
   with the supplied size argument and verify equality; if they differ
   it will crash hard. If you are uncertain of the size use the
   ecl_kw_fscanf_alloc_grdecl_dynamic() function instead; or supply
   size == 0.
*/

ecl_kw_type * ecl_kw_fscanf_alloc_grdecl__( FILE * stream , const char * kw , bool strict , int size , ecl_data_type data_type) {
  return __ecl_kw_fscanf_alloc_grdecl__( stream , kw , strict , size , data_type );
}


ecl_kw_type * ecl_kw_fscanf_alloc_grdecl( FILE * stream , const char * kw , int size , ecl_data_type data_type) {
  bool strict = true;
  return ecl_kw_fscanf_alloc_grdecl__( stream , kw , strict , size , data_type );
}

/*****************************************************************/

/*
   This function will read and allocate the next keyword in the
   file. This function does not take either kw or the size of the kw
   as input, and has virtually zero possibilities to check what it is
   doing. The possibilities of failure are fucking endless, and the
   function should only be used when you are goddamn certain that the
   input file is well formatted.
*/

ecl_kw_type * ecl_kw_fscanf_alloc_current_grdecl__( FILE * stream , bool strict , ecl_data_type data_type) {
  return __ecl_kw_fscanf_alloc_grdecl__( stream , NULL , strict , 0 , data_type );
}


ecl_kw_type * ecl_kw_fscanf_alloc_current_grdecl( FILE * stream , ecl_data_type data_type) {
  bool strict = true;
  return ecl_kw_fscanf_alloc_current_grdecl__( stream , strict ,  data_type );
}



/*****************************************************************/



/*
  This method allows to write with a different header,
  i.e. PORO_XXXX. This header is even allowed to break the 8 character
  length limit; i.e. loading it back naively will fail.
*/

void ecl_kw_fprintf_grdecl__(const ecl_kw_type * ecl_kw , const char * special_header , FILE * stream) {
  if (special_header)
    fprintf(stream,"%s\n" , special_header);
  else
    fprintf(stream,"%s\n" , ecl_kw_get_header(ecl_kw));

  {
    fortio_type * fortio = fortio_alloc_FILE_wrapper(NULL , false , true , true , stream);   /* Endian flip should *NOT* be used */
    ecl_kw_fwrite_data(ecl_kw , fortio);
    fortio_free_FILE_wrapper( fortio );
  }
  fprintf(stream,"/\n");
}


void ecl_kw_fprintf_grdecl(const ecl_kw_type * ecl_kw , FILE * stream) {
  ecl_kw_fprintf_grdecl__(ecl_kw , NULL , stream );
}
