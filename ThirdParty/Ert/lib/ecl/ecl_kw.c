/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_kw.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/buffer.h>
#include <ert/util/int_vector.h>

#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_type.h>


#define ECL_KW_TYPE_ID  6111098





struct ecl_kw_struct {
  UTIL_TYPE_ID_DECLARATION;
  int               size;
  ecl_data_type     data_type;
  char            * header8;              /* Header which is right padded with ' ' to become exactly 8 characters long. Should only be used internally.*/
  char            * header;               /* Header which is trimmed to no-space. */
  char            * data;                 /* The actual data vector. */
  bool              shared_data;          /* Whether this keyword has shared data or not. */
};


UTIL_IS_INSTANCE_FUNCTION(ecl_kw , ECL_KW_TYPE_ID )



/*****************************************************************/
/* For some peculiar reason the keyword data is written in blocks, all
   numeric data is in blocks of 1000 elements, and character data is
   in blocks of 105 elements.
*/

#define BLOCKSIZE_NUMERIC  1000
#define BLOCKSIZE_CHAR     105




/*****************************************************************/
/* When writing formatted data, the data comes in columns, with a
   certain number of elements in each row, i.e. four columns for float
   data:

   0.000   0.000   0.000   0.000
   0.000   0.000   0.000   0.000
   0.000   0.000   0.000   0.000
   ....

   These #define symbols define the number of columns for the
   different datatypes.
*/
#define COLUMNS_CHAR     7
#define COLUMNS_FLOAT    4
#define COLUMNS_DOUBLE   3
#define COLUMNS_INT      6
#define COLUMNS_MESSAGE  1
#define COLUMNS_BOOL    25


/*****************************************************************/
/* Format string used when writing a formatted header. */
#define WRITE_HEADER_FMT  " '%-8s' %11d '%-4s'\n"



/*****************************************************************/
/* Format string used when reading and writing formatted
   files. Observe the following about these format strings:

    1. The format string for reading double contains two '%'
       identifiers, that is because doubles are read by parsing a
       prefix and power separately.

    2. For both double and float the write format contains two '%'
       characters - that is because the values are split in a prefix
       and a power prior to writing - see the function
       __fprintf_scientific().

    3. The logical type involves converting back and forth between 'T'
       and 'F' and internal logical representation. The format strings
       are therefor for reading/writing a character.

*/

#define READ_FMT_CHAR     "%8c"
#define READ_FMT_FLOAT    "%gE"
#define READ_FMT_INT      "%d"
#define READ_FMT_MESS     "%8c"
#define READ_FMT_BOOL     "  %c"
#define READ_FMT_DOUBLE   "%lgD%d"


#define WRITE_FMT_CHAR    " '%-8s'"
#define WRITE_FMT_INT     " %11d"
#define WRITE_FMT_FLOAT   "  %11.8fE%+03d"
#define WRITE_FMT_DOUBLE  "  %17.14fD%+03d"
#define WRITE_FMT_MESS    "%s"
#define WRITE_FMT_BOOL    "  %c"


/*****************************************************************/
/* The boolean type is not a native type which can be uniquely
   identified between Fortran (ECLIPSE), C, formatted and unformatted
   files:

    o In the formatted ECLIPSE files the characters BOOL_TRUE_CHAR and
      BOOL_FALSE_CHAR are used to represent true and false values
      repsectively.

    o In the unformatted ECLIPSE files the boolean values are
      represented as integers with the values ECL_BOOL_TRUE_INT and
      ECL_BOOL_FALSE_INT respectively.

   Internally in an ecl_kw instance boolean values are represented as
   integers (NOT bool), with the representation given by ECL_BOOL_TRUE_INT
   and ECL_BOOL_FALSE_INT. This implies that read/write of unformatted
   data can go transparently without between ECLIPSE and the ecl_kw
   implementation, but exported set()/get() functions with bool must
   intercept the bool values and convert to the appropriate integer
   value.
*/


// For formatted files:
#define BOOL_TRUE_CHAR       'T'
#define BOOL_FALSE_CHAR      'F'


ecl_type_enum  ecl_kw_get_type(const ecl_kw_type *);
void ecl_kw_set_data_type(ecl_kw_type * ecl_kw, ecl_data_type data_type);

static char * alloc_read_fmt_string(const ecl_data_type ecl_type) {
  return util_alloc_sprintf(
          "%%%dc",
          ecl_type_get_sizeof_ctype_fortio(ecl_type)
          );
}

static char * alloc_read_fmt(const ecl_data_type data_type ) {
  switch(ecl_type_get_type(data_type)) {
  case(ECL_CHAR_TYPE):
    return util_alloc_string_copy(READ_FMT_CHAR);
  case(ECL_INT_TYPE):
    return util_alloc_string_copy(READ_FMT_INT);
  case(ECL_FLOAT_TYPE):
    return util_alloc_string_copy(READ_FMT_FLOAT);
  case(ECL_DOUBLE_TYPE):
    return util_alloc_string_copy(READ_FMT_DOUBLE);
  case(ECL_BOOL_TYPE):
    return util_alloc_string_copy(READ_FMT_BOOL);
  case(ECL_MESS_TYPE):
    return util_alloc_string_copy(READ_FMT_MESS);
  case(ECL_STRING_TYPE):
    return alloc_read_fmt_string(data_type);
  default:
    util_abort("%s: invalid ecl_type:%s \n",__func__ , ecl_type_alloc_name(data_type));
    return NULL;
  }
}

static char * alloc_write_fmt_string(const ecl_data_type ecl_type) {
  return util_alloc_sprintf(
          " '%%-%ds'",
          ecl_type_get_sizeof_ctype_fortio(ecl_type)
          );
}

static char * alloc_write_fmt(const ecl_data_type data_type) {
  switch(ecl_type_get_type(data_type)) {
  case(ECL_CHAR_TYPE):
    return util_alloc_string_copy(WRITE_FMT_CHAR);
  case(ECL_INT_TYPE):
    return util_alloc_string_copy(WRITE_FMT_INT);
  case(ECL_FLOAT_TYPE):
    return util_alloc_string_copy(WRITE_FMT_FLOAT);
  case(ECL_DOUBLE_TYPE):
    return util_alloc_string_copy(WRITE_FMT_DOUBLE);
  case(ECL_BOOL_TYPE):
    return util_alloc_string_copy(WRITE_FMT_BOOL);
  case(ECL_MESS_TYPE):
    return util_alloc_string_copy(WRITE_FMT_MESS);
  case(ECL_STRING_TYPE):
    return alloc_write_fmt_string(data_type);
  default:
    util_abort("%s: invalid ecl_type: %s\n",__func__ , ecl_type_alloc_name(data_type));
    return NULL;
  }
}

static int get_blocksize( ecl_data_type data_type ) {
  if (ecl_type_is_alpha(data_type))
    return BLOCKSIZE_CHAR;

  return BLOCKSIZE_NUMERIC;
}


static int get_columns(const ecl_data_type data_type) {
  switch(ecl_type_get_type(data_type)) {
  case(ECL_CHAR_TYPE):
    return COLUMNS_CHAR;
  case(ECL_INT_TYPE):
    return COLUMNS_INT;
  case(ECL_FLOAT_TYPE):
    return COLUMNS_FLOAT;
  case(ECL_DOUBLE_TYPE):
    return COLUMNS_DOUBLE;
  case(ECL_BOOL_TYPE):
    return COLUMNS_BOOL;
  case(ECL_MESS_TYPE):
    return COLUMNS_MESSAGE;
  case(ECL_STRING_TYPE):
    return COLUMNS_CHAR; // TODO: Is this correct?
  default:
    util_abort("%s: invalid ecl_type: %s\n",__func__ , ecl_type_alloc_name(data_type));
    return -1;
  }
}



/******************************************************************/

static void ecl_kw_assert_index(const ecl_kw_type *ecl_kw , int index, const char *caller) {
  if (index < 0 || index >= ecl_kw->size)
    util_abort("%s: Invalid index lookup. kw:%s input_index:%d   size:%d \n",caller , ecl_kw->header , index , ecl_kw->size);
}



static void ecl_kw_endian_convert_data(ecl_kw_type *ecl_kw) {
  if (ecl_type_is_numeric(ecl_kw->data_type) || ecl_type_is_bool(ecl_kw->data_type))
    util_endian_flip_vector(ecl_kw->data , ecl_kw_get_sizeof_ctype(ecl_kw) , ecl_kw->size);
}


const char * ecl_kw_get_header8(const ecl_kw_type *ecl_kw) {
  return ecl_kw->header8;
}

/*
   Return the header without the trailing spaces
*/
const char * ecl_kw_get_header(const ecl_kw_type * ecl_kw ) {
  return ecl_kw->header;
}

bool ecl_kw_name_equal( const ecl_kw_type * ecl_kw , const char * name) {
  return (strcmp( ecl_kw->header , name) == 0);
}



void ecl_kw_get_memcpy_data(const ecl_kw_type *ecl_kw , void *target) {
  memcpy(target , ecl_kw->data , ecl_kw->size * ecl_kw_get_sizeof_ctype(ecl_kw));
}

void ecl_kw_get_memcpy_int_data(const ecl_kw_type *ecl_kw , int * target) {
  if (ecl_type_is_int(ecl_kw->data_type))
    ecl_kw_get_memcpy_data( ecl_kw , target );
}

void ecl_kw_get_memcpy_float_data(const ecl_kw_type *ecl_kw , float *target) {
  if (ecl_type_is_float(ecl_kw->data_type))
    ecl_kw_get_memcpy_data( ecl_kw , target );
}

void ecl_kw_get_memcpy_double_data(const ecl_kw_type *ecl_kw , double *target) {
if (ecl_type_is_double(ecl_kw->data_type))
    ecl_kw_get_memcpy_data( ecl_kw , target );
}


/** Allocates a untyped buffer with exactly the same content as the ecl_kw instances data. */
void * ecl_kw_alloc_data_copy(const ecl_kw_type * ecl_kw) {
  void * buffer = util_alloc_copy( ecl_kw->data , ecl_kw->size * ecl_kw_get_sizeof_ctype(ecl_kw) );
  return buffer;
}


void ecl_kw_set_memcpy_data(ecl_kw_type *ecl_kw , const void *src) {
  if (src != NULL)
    memcpy(ecl_kw->data , src , ecl_kw->size * ecl_kw_get_sizeof_ctype(ecl_kw));
}


static bool ecl_kw_string_eq(const char *s1 , const char *s2) {
  const char space_char = ' ';
  const char *long_kw   = (strlen(s1) >= strlen(s2)) ? s1 : s2;
  const char *short_kw  = (strlen(s1) <  strlen(s2)) ? s1 : s2;
  const int  len1       = strlen(long_kw);
  const int  len2       = strlen(short_kw);
  int index;
  bool eq = true;
  if (len1 > ECL_STRING8_LENGTH)
    util_abort("%s : eclipse keyword:%s is too long - aborting \n",__func__ , long_kw);

  for (index = 0; index < len2; index++)
    eq = eq & (long_kw[index] == short_kw[index]);

  if (eq) {
    for (index = len2; index < len1; index++)
      eq = eq & (long_kw[index] == space_char);
  }

  return eq;
}



bool ecl_kw_ichar_eq(const ecl_kw_type *ecl_kw , int i , const char *value) {
  char s1[ECL_STRING8_LENGTH + 1];
  ecl_kw_iget(ecl_kw , i , s1);
  return ecl_kw_string_eq(s1 , value);
}


bool ecl_kw_size_and_type_equal( const ecl_kw_type *ecl_kw1 , const ecl_kw_type * ecl_kw2 ) {
  return (ecl_kw1->size == ecl_kw2->size &&
          ecl_type_is_equal(ecl_kw1->data_type, ecl_kw2->data_type));
}


bool ecl_kw_header_eq(const ecl_kw_type *ecl_kw1 , const ecl_kw_type * ecl_kw2) {
  bool  equal = true;

  if (strcmp(ecl_kw1->header8 , ecl_kw2->header8) != 0)
    equal  = false;
  else
    equal = ecl_kw_size_and_type_equal( ecl_kw1 , ecl_kw2 );

  return equal;
}

static bool ecl_kw_data_equal__( const ecl_kw_type * ecl_kw , const void * data , int cmp_elements) {
  int cmp = memcmp( ecl_kw->data , data , cmp_elements * ecl_kw_get_sizeof_ctype(ecl_kw));
  if (cmp == 0)
    return true;
  else
    return false;
}





/**
   Observe that the comparison is done with memcmp() -
   i.e. "reasonably good" numerical agreement is *not* enough.
*/

bool ecl_kw_data_equal( const ecl_kw_type * ecl_kw , const void * data) {
  return ecl_kw_data_equal__( ecl_kw , data , ecl_kw->size);
}


bool ecl_kw_content_equal( const ecl_kw_type * ecl_kw1 , const ecl_kw_type * ecl_kw2) {
  if (ecl_kw_size_and_type_equal( ecl_kw1 , ecl_kw2))
    return ecl_kw_data_equal__( ecl_kw1 , ecl_kw2->data , ecl_kw1->size);
  else
    return false;
}





/**
   This function compares two ecl_kw instances, and returns true if they are equal.
*/

bool ecl_kw_equal(const ecl_kw_type *ecl_kw1, const ecl_kw_type *ecl_kw2) {
  bool equal = ecl_kw_header_eq( ecl_kw1 , ecl_kw2 );
  if (equal)
    equal = ecl_kw_data_equal( ecl_kw1 , ecl_kw2->data );

  return equal;
}



#define ECL_KW_NUMERIC_CMP(ctype)                                                                                           \
  static bool ecl_kw_numeric_equal_ ## ctype( const ecl_kw_type * ecl_kw1 , const ecl_kw_type * ecl_kw2 , ctype abs_diff , ctype rel_diff) { \
  int index;                                                                                                                \
  bool equal = true;                                                                                                        \
  {                                                                                                                         \
     const ctype * data1 = (const ctype *) ecl_kw1->data;                                                                   \
     const ctype * data2 = (const ctype *) ecl_kw2->data;                                                                   \
     for (index = 0; index < ecl_kw1->size; index++) {                                                                      \
       equal = util_ ## ctype ## _approx_equal__( data1[index], data2[index] , rel_diff , abs_diff);                        \
       if (!equal)                                                                                                          \
           break;                                                                                                           \
     }                                                                                                                      \
  }                                                                                                                         \
  return equal;                                                                                                             \
}

ECL_KW_NUMERIC_CMP( float )
ECL_KW_NUMERIC_CMP( double )
#undef ECL_KW_NUMERIC_CMP


/**
   This function compares the data of two ecl_kw instances, and
   returns true if the relative numerical difference is less than
   @rel_diff. Does not consider consider the kw header.
*/

bool ecl_kw_numeric_equal(const ecl_kw_type *ecl_kw1, const ecl_kw_type *ecl_kw2 , double abs_diff , double rel_diff) {
  if(!ecl_kw_size_and_type_equal(ecl_kw1, ecl_kw2))
    return false;

  if (ecl_type_is_float(ecl_kw1->data_type))
    return ecl_kw_numeric_equal_float( ecl_kw1 , ecl_kw2 , abs_diff , rel_diff );
  else if (ecl_type_is_double(ecl_kw1->data_type))
    return ecl_kw_numeric_equal_double( ecl_kw1 , ecl_kw2 , abs_diff , rel_diff );
  else
    return ecl_kw_data_equal( ecl_kw1 , ecl_kw2->data );
}



bool ecl_kw_block_equal( const ecl_kw_type * ecl_kw1 , const ecl_kw_type * ecl_kw2 , int cmp_elements) {
  if (ecl_kw_header_eq( ecl_kw1 , ecl_kw2)) {
    if (cmp_elements == 0)
      cmp_elements = ecl_kw1->size;

    return ecl_kw_data_equal__( ecl_kw1 , ecl_kw2->data , cmp_elements );
  } else
    return false;
}



static void ecl_kw_set_shared_ref(ecl_kw_type * ecl_kw , void *data_ptr) {
  if (!ecl_kw->shared_data) {
    if (ecl_kw->data != NULL)
      util_abort("%s: can not change to shared for keyword with allocated storage - aborting \n",__func__);
  }
  ecl_kw->shared_data = true;
  ecl_kw->data = data_ptr;
}



static void ecl_kw_initialize(ecl_kw_type * ecl_kw , const char *header ,  int size , ecl_data_type data_type) {
  ecl_kw_set_data_type(ecl_kw, data_type);
  ecl_kw_set_header_name(ecl_kw , header);
  ecl_kw->size = size;
}

static size_t ecl_kw_fortio_data_size( const ecl_kw_type * ecl_kw) {
  const int blocksize  = get_blocksize( ecl_kw->data_type );
  const int num_blocks = ecl_kw->size / blocksize + (ecl_kw->size % blocksize == 0 ? 0 : 1);

  return num_blocks * (4 + 4) +                                           // Fortran fluff for each block
    ecl_kw->size * ecl_type_get_sizeof_ctype_fortio( ecl_kw->data_type );  // Actual data
}



/**
   Returns the number of bytes this ecl_kw instance would occupy in
   BINARY file; we add 2*4 to the header size to include the size of
   the fortran header and trailer combo.
*/

size_t ecl_kw_fortio_size( const ecl_kw_type * ecl_kw ) {
  size_t size = ECL_KW_HEADER_FORTIO_SIZE;
  size += ecl_kw_fortio_data_size(ecl_kw );
  return size;
}


/**
   The data is copied from the input argument to the ecl_kw; data can be NULL.
*/
ecl_kw_type * ecl_kw_alloc_new(const char * header ,  int size, ecl_data_type data_type , const void * data) {
  ecl_kw_type *ecl_kw;
  ecl_kw = ecl_kw_alloc_empty();
  ecl_kw_initialize(ecl_kw , header , size , data_type);
  if (data != NULL) {
    ecl_kw_alloc_data(ecl_kw);
    ecl_kw_set_memcpy_data(ecl_kw , data);
  }
  return ecl_kw;
}



ecl_kw_type * ecl_kw_alloc( const char * header , int size , ecl_data_type data_type ) {
  ecl_kw_type *ecl_kw;

  ecl_kw = ecl_kw_alloc_empty();
  ecl_kw_initialize(ecl_kw , header , size , data_type);
  ecl_kw_alloc_data(ecl_kw);

  return ecl_kw;
}



ecl_kw_type * ecl_kw_alloc_new_shared(const char * header ,  int size, ecl_data_type data_type , void * data) {
  ecl_kw_type *ecl_kw;
  ecl_kw = ecl_kw_alloc_empty();
  ecl_kw_initialize(ecl_kw , header , size , data_type);
  ecl_kw_set_shared_ref(ecl_kw , data);
  return ecl_kw;
}



ecl_kw_type * ecl_kw_alloc_empty() {
  ecl_kw_type *ecl_kw;

  ecl_kw                 = util_malloc(sizeof *ecl_kw );
  ecl_kw->header         = NULL;
  ecl_kw->header8        = NULL;
  ecl_kw->data           = NULL;
  ecl_kw->shared_data    = false;
  ecl_kw->size           = 0;

  UTIL_TYPE_ID_INIT(ecl_kw , ECL_KW_TYPE_ID);

  return ecl_kw;
}



void ecl_kw_free(ecl_kw_type *ecl_kw) {
  util_safe_free( ecl_kw->header );
  util_safe_free(ecl_kw->header8);
  ecl_kw_free_data(ecl_kw);
  free(ecl_kw);
}

void ecl_kw_free__(void *void_ecl_kw) {
  ecl_kw_free((ecl_kw_type *) void_ecl_kw);
}


void ecl_kw_memcpy_data( ecl_kw_type * target , const ecl_kw_type * src) {
  if (!ecl_kw_size_and_type_equal( target , src ))
    util_abort("%s: type/size mismatch \n",__func__);

  memcpy(target->data , src->data , target->size * ecl_kw_get_sizeof_ctype(target));
}



void ecl_kw_memcpy(ecl_kw_type *target, const ecl_kw_type *src) {
  target->size                = src->size;
  ecl_kw_set_data_type(target, src->data_type);

  ecl_kw_set_header_name( target , src->header );
  ecl_kw_alloc_data(target);
  ecl_kw_memcpy_data( target , src );
}


ecl_kw_type *ecl_kw_alloc_copy(const ecl_kw_type *src) {
  ecl_kw_type *new;
  new = ecl_kw_alloc_empty();
  ecl_kw_memcpy(new , src);
  return new;
}

/**
   This function will allocate a new copy of @src, where only the
   elements corresponding to the slice [index1:index2) is included.

   The input parameters @index1 and @index2 can to some extent be
   out-of-range:

       index1 = max( index1 , 0 );
       index2 = min( index2 , size );

   If index1 > index2 it will fail hard; the same applies if stride is
   <= 0.
*/



ecl_kw_type * ecl_kw_alloc_slice_copy( const ecl_kw_type * src, int index1, int index2, int stride) {
  if (index1 < 0) index1 = 0;
  if (index2 >  src->size) index2 = src->size;
  if (index1 >= src->size) util_abort("%s: index1=%d > size:%d \n",__func__ , index1 , src->size);
  if (stride <= 0)         util_abort("%s: stride:%d completely broken ...\n",__func__ , stride);
  {
    ecl_kw_type * new_kw = NULL;
    int src_index = index1;
    /* 1: Determine size of the sliced copy. */
    {
      int new_size = 0;
      while (src_index < index2) {
        new_size++;
        src_index += stride;
      }
      if (new_size > 0) {
        new_kw = ecl_kw_alloc_empty();
        ecl_kw_initialize(new_kw , src->header , new_size , src->data_type);
        ecl_kw_alloc_data(new_kw);

        /* 2: Copy over the elements. */
        src_index = index1;
        {
          int target_index = 0;
          const char * src_ptr = src->data;
          char * new_ptr = new_kw->data;
          int sizeof_ctype = ecl_kw_get_sizeof_ctype(new_kw);

          while ( src_index < index2 ) {
            memcpy( &new_ptr[ target_index * sizeof_ctype ] , &src_ptr[ src_index * sizeof_ctype ] , sizeof_ctype );
            src_index += stride;
            target_index += 1;
          }
        }
      }
    }
    return new_kw;
  }
}



void ecl_kw_resize( ecl_kw_type * ecl_kw, int new_size) {
  if (ecl_kw->shared_data)
    util_abort("%s: trying to allocate data for ecl_kw object which has been declared with shared storage - aborting \n",__func__);

  if (new_size != ecl_kw->size) {
    size_t old_byte_size = ecl_kw->size * ecl_kw_get_sizeof_ctype(ecl_kw);
    size_t new_byte_size = new_size * ecl_kw_get_sizeof_ctype(ecl_kw);

    ecl_kw->data = util_realloc(ecl_kw->data , new_byte_size );
    if (new_byte_size > old_byte_size) {
      size_t offset = old_byte_size;
      memset(&ecl_kw->data[offset] , 0 , new_byte_size - old_byte_size);
    }
    ecl_kw->size = new_size;
  }
}

/**
   Will allocate a copy of the src_kw. Will copy @count elements
   starting at @offset. If @count < 0 all remaining elements from
   @offset will be copied. If new_kw == NULL the new keyword will have
   the same header as the @src, otherwise the value @new_kw will be
   used.
*/


ecl_kw_type * ecl_kw_alloc_sub_copy( const ecl_kw_type * src, const char * new_kw , int offset , int count) {
  if (new_kw == NULL)
    new_kw = src->header;

  if (count < 0)
    count = src->size - offset;

  if ((offset < 0) || (offset >= src->size)) util_abort("%s: invalid offset - limits: [%d,%d) \n",__func__ , 0 , src->size);
  if ((count + offset) > src->size) util_abort("%s: invalid count value: %d \n",__func__ , count);

  {
    void * src_data = ecl_kw_iget_ptr( src , offset );
    return ecl_kw_alloc_new( new_kw , count , src->data_type , src_data );
  }
}



const void * ecl_kw_copyc__(const void * void_kw) {
  return ecl_kw_alloc_copy((const ecl_kw_type *) void_kw);
}

static void * ecl_kw_iget_ptr_static(const ecl_kw_type *ecl_kw , int i) {
  ecl_kw_assert_index(ecl_kw , i , __func__);
  return &ecl_kw->data[i * ecl_kw_get_sizeof_ctype(ecl_kw)];
}


static void ecl_kw_iget_static(const ecl_kw_type *ecl_kw , int i , void *iptr) {
  memcpy(iptr , ecl_kw_iget_ptr_static(ecl_kw , i) , ecl_kw_get_sizeof_ctype(ecl_kw));
}


static void ecl_kw_iset_static(ecl_kw_type *ecl_kw , int i , const void *iptr) {
  ecl_kw_assert_index(ecl_kw , i , __func__);
  memcpy(&ecl_kw->data[i * ecl_kw_get_sizeof_ctype(ecl_kw)] , iptr, ecl_kw_get_sizeof_ctype(ecl_kw));
}


void ecl_kw_iget(const ecl_kw_type *ecl_kw , int i , void *iptr) {
  ecl_kw_iget_static(ecl_kw , i , iptr);
}


/**
   Will return a double value for underlying data types of double,
   float and int.
*/
double ecl_kw_iget_as_double(const ecl_kw_type * ecl_kw , int index) {
  if (ecl_type_is_float(ecl_kw->data_type))
    return ecl_kw_iget_float( ecl_kw , index); /* Here the compiler will silently insert a float -> double conversion. */
  else if (ecl_type_is_double(ecl_kw->data_type))
    return ecl_kw_iget_double( ecl_kw, index);
  else if (ecl_type_is_int(ecl_kw->data_type))
    return ecl_kw_iget_int( ecl_kw, index); /*  */
  else {
    util_abort("%s: can not be converted to double - no data for you! \n",__func__);
    return -1;
  }
}

/**
   Will return a float value for underlying data types of double and float.
*/

float ecl_kw_iget_as_float(const ecl_kw_type * ecl_kw , int i) {
  if (ecl_type_is_float(ecl_kw->data_type))
    return ecl_kw_iget_float( ecl_kw , i);
  else if (ecl_type_is_double(ecl_kw->data_type))
    return (float) ecl_kw_iget_double( ecl_kw, i);
  else {
    util_abort("%s: can not be converted to float - no data for you! \n",__func__);
    return -1;
  }
}


#define ECL_KW_IGET_TYPED(ctype , ECL_TYPE)                                                                 \
ctype ecl_kw_iget_ ## ctype(const ecl_kw_type * ecl_kw, int i) {                                            \
  ctype value;                                                                                              \
  if (ecl_kw_get_type(ecl_kw) != ECL_TYPE)                                                                  \
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));        \
  ecl_kw_iget_static(ecl_kw , i , &value);                                                                  \
  return value;                                                                                             \
}                                                                                                           \

ECL_KW_IGET_TYPED(double , ECL_DOUBLE_TYPE);
ECL_KW_IGET_TYPED(float  , ECL_FLOAT_TYPE);
ECL_KW_IGET_TYPED(int    , ECL_INT_TYPE);
#undef ECL_KW_IGET_TYPED


bool ecl_kw_iget_bool( const ecl_kw_type * ecl_kw , int i) {
  int  int_value;
  if (ecl_kw_get_type(ecl_kw) != ECL_BOOL_TYPE)
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));
  ecl_kw_iget_static(ecl_kw , i , &int_value);
  if (int_value == ECL_BOOL_TRUE_INT)
    return true;
  else if (int_value == ECL_BOOL_FALSE_INT)
    return false;
  else {
    util_abort("%s: fuckup - wrong integer in BOOL type \n",__func__);
    return false;
  }
}

const char * ecl_kw_iget_char_ptr( const ecl_kw_type * ecl_kw , int i) {
  if (ecl_kw_get_type(ecl_kw) != ECL_CHAR_TYPE)
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));
  return ecl_kw_iget_ptr( ecl_kw , i );
}

const char * ecl_kw_iget_string_ptr( const ecl_kw_type * ecl_kw, int i) {
  if (ecl_kw_get_type(ecl_kw) != ECL_STRING_TYPE)
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));
  return ecl_kw_iget_ptr( ecl_kw , i );
}


/**
   This will set the elemnts of the ecl_kw data storage in index to
   the value of s8; if s8 is shorter than 8 characters the result will
   be padded, if s8 is longer than 8 characters the characters from 9
   and out will be ignored.
*/
void ecl_kw_iset_string8(ecl_kw_type * ecl_kw , int index , const char *s8) {
  char * ecl_string = (char *) ecl_kw_iget_ptr( ecl_kw , index );
  if (strlen( s8 ) >= ECL_STRING8_LENGTH) {
    /* The whole string goes in - possibly loosing content at the end. */
    int i;
    for (i=0; i < ECL_STRING8_LENGTH; i++)
      ecl_string[i] = s8[i];
  } else {
    /* The string is padded with trailing spaces. */
    int string_length = strlen( s8 );
    int i;

    for (i=0; i < string_length; i++)
      ecl_string[i] = s8[i];

    for (i=string_length; i < ECL_STRING8_LENGTH; i++)
      ecl_string[i] = ' ';

  }

  ecl_string[ ECL_STRING8_LENGTH ] = '\0';
}

/**
   This function will set the string @index in the ecl_kw string array
   to @s. IFF @s is longer than 8 characters, the first part will go
   in element @index, and then we will continue writing into the next
   elements. If the resulting index goes beyond the length of the
   keyword - WhamBang!

   You should know what you are doing when sending in a string of
   length greater than 8 - maybe the overwriting of consecutive
   elements is not what you want?
*/
void ecl_kw_iset_char_ptr( ecl_kw_type * ecl_kw , int index, const char * s) {
  int strings = strlen( s ) / ECL_STRING8_LENGTH;
  if ((strlen( s ) %  ECL_STRING8_LENGTH) != 0)
    strings++;
  {
    int sub_index;
    for (sub_index = 0; sub_index < strings; sub_index++)
      ecl_kw_iset_string8( ecl_kw , index + sub_index , &s[ sub_index * ECL_STRING8_LENGTH ]);
  }
}

/**
 This function will verify that the given string is of approperiate
 length (0 <= lenght <= data_type.element_size). If so, the elements
 of @s will be written to the @ecl_kw string array starting at
 @index. If the input string is shorter than the type length the
 string will be padded with trailing spaces.
 */
void ecl_kw_iset_string_ptr( ecl_kw_type * ecl_kw, int index, const char * s) {
  if(!ecl_type_is_alpha(ecl_kw_get_data_type(ecl_kw))) {
    char * type_name = ecl_type_alloc_name(ecl_kw_get_data_type(ecl_kw));
    util_abort("%s: Expected alphabetic data type (CHAR, CXXX or MESS), was %s\n", __func__, type_name);
  }

  size_t input_len = strlen(s);
  size_t type_len  = ecl_type_get_sizeof_ctype_fortio(ecl_kw_get_data_type(ecl_kw));

  if(input_len > type_len)
    util_abort("%s: String of length %d cannot hold input string of length %d\n", __func__, type_len, input_len);

  {
    char * ecl_string = (char *) ecl_kw_iget_ptr(ecl_kw, index);
    int i;

    for(i = 0; i < input_len; ++i)
      ecl_string[i] = s[i];

    for (i=input_len; i < type_len; ++i)
      ecl_string[i] = ' ';

    ecl_string[type_len] = '\0';
  }
}


/**
   This function will compare the string at position @index with the
   input @other string. The comparison will be done in a
   'space-tolerant', i.e. trailing spaces are ignored in the
   comparison. If the strings are considered equal true is returned.
*/

bool ecl_kw_icmp_string( const ecl_kw_type * ecl_kw , int index, const char * other_string) {
  const char * kw_string = ecl_kw_iget_char_ptr( ecl_kw , index );
  if (strlen(other_string)) {
    char * match = strstr( kw_string , other_string);
    if (match == kw_string)
      return true;
  }

  return false;
}



#define ECL_KW_ISET_TYPED(ctype , ECL_TYPE)                                                                 \
void ecl_kw_iset_ ## ctype(ecl_kw_type * ecl_kw, int i, ctype value) {                                      \
  if (ecl_kw_get_type(ecl_kw) != ECL_TYPE)                                                                  \
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));        \
  ecl_kw_iset_static(ecl_kw , i , &value);                                                                  \
}                                                                                                           \

ECL_KW_ISET_TYPED(double , ECL_DOUBLE_TYPE);
ECL_KW_ISET_TYPED(float  , ECL_FLOAT_TYPE);
ECL_KW_ISET_TYPED(int    , ECL_INT_TYPE);
#undef ECL_KW_ISET_TYPED


#define ECL_KW_SET_INDEXED(ctype , ECL_TYPE)                                                                   \
void ecl_kw_set_indexed_ ## ctype( ecl_kw_type * ecl_kw, const int_vector_type * index_list , ctype value) {   \
   if (ecl_kw_get_type(ecl_kw) != ECL_TYPE)                                                                    \
      util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));         \
   {                                                                                                           \
     ctype * data = (ctype *) ecl_kw->data;                                                                    \
     int size = int_vector_size( index_list );                                                                 \
     const int * index_ptr = int_vector_get_const_ptr( index_list );                                           \
     int i;                                                                                                    \
     for (i = 0; i < size; i++)                                                                                \
         data[index_ptr[i]] = value;                                                                           \
   }                                                                                                           \
}

ECL_KW_SET_INDEXED( double , ECL_DOUBLE_TYPE);
ECL_KW_SET_INDEXED( float  , ECL_FLOAT_TYPE);
ECL_KW_SET_INDEXED( int    , ECL_INT_TYPE);
#undef ECL_KW_SET_INDEXED


#define ECL_KW_SHIFT_INDEXED(ctype , ECL_TYPE)                                                                   \
void ecl_kw_shift_indexed_ ## ctype( ecl_kw_type * ecl_kw, const int_vector_type * index_list , ctype shift) {   \
   if (ecl_kw_get_type(ecl_kw) != ECL_TYPE)                                                                      \
      util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));           \
   {                                                                                                             \
     ctype * data = (ctype *) ecl_kw->data;                                                                      \
     int size = int_vector_size( index_list );                                                                   \
     const int * index_ptr = int_vector_get_const_ptr( index_list );                                             \
     int i;                                                                                                      \
     for (i = 0; i < size; i++)                                                                                  \
          data[index_ptr[i]] += shift;                                                                           \
   }                                                                                                             \
}


ECL_KW_SHIFT_INDEXED( double , ECL_DOUBLE_TYPE);
ECL_KW_SHIFT_INDEXED( float  , ECL_FLOAT_TYPE);
ECL_KW_SHIFT_INDEXED( int    , ECL_INT_TYPE);
#undef ECL_KW_SHIFT_INDEXED


#define ECL_KW_SCALE_INDEXED(ctype , ECL_TYPE)                                                                \
void ecl_kw_scale_indexed_ ## ctype( ecl_kw_type * ecl_kw, const int_vector_type * index_list , ctype scale) {  \
   if (ecl_kw_get_type(ecl_kw) != ECL_TYPE)                                                                   \
      util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));        \
   {                                                                                                          \
     ctype * data = (ctype *) ecl_kw->data;                                                                   \
     int size = int_vector_size( index_list );                                                                \
     const int * index_ptr = int_vector_get_const_ptr( index_list );                                          \
     int i;                                                                                                   \
     for (i = 0; i < size; i++)                                                                               \
          data[index_ptr[i]] *= scale;                                                                        \
   }                                                                                                          \
}

ECL_KW_SCALE_INDEXED( double , ECL_DOUBLE_TYPE);
ECL_KW_SCALE_INDEXED( float  , ECL_FLOAT_TYPE);
ECL_KW_SCALE_INDEXED( int    , ECL_INT_TYPE);
#undef ECL_KW_SCALE_INDEXED


void ecl_kw_iset_bool( ecl_kw_type * ecl_kw , int i , bool bool_value) {
  int  int_value;
  if (ecl_kw_get_type(ecl_kw) != ECL_BOOL_TYPE)
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));

  if (bool_value)
    int_value = ECL_BOOL_TRUE_INT;
  else
    int_value = ECL_BOOL_FALSE_INT;

  ecl_kw_iset_static(ecl_kw , i , &int_value);
}



/*****************************************************************/
/* Various ways to get pointers to the underlying data. */

#define ECL_KW_GET_TYPED_PTR(ctype , ECL_TYPE)                                                                      \
ctype * ecl_kw_get_ ## ctype ## _ptr(const ecl_kw_type * ecl_kw) {                                                  \
  if (ecl_kw_get_type(ecl_kw) != ECL_TYPE)                                                                          \
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));                \
  return (ctype *) ecl_kw->data;                                                                                    \
}

ECL_KW_GET_TYPED_PTR(double , ECL_DOUBLE_TYPE);
ECL_KW_GET_TYPED_PTR(float  , ECL_FLOAT_TYPE);
ECL_KW_GET_TYPED_PTR(int    , ECL_INT_TYPE);
#undef ECL_KW_GET_TYPED_PTR

void * ecl_kw_get_void_ptr(const ecl_kw_type * ecl_kw) {
  return ecl_kw->data;
}

/*****************************************************************/


void * ecl_kw_iget_ptr(const ecl_kw_type *ecl_kw , int i) {
  return ecl_kw_iget_ptr_static(ecl_kw , i);
}




void ecl_kw_iset(ecl_kw_type *ecl_kw , int i , const void *iptr) {
  ecl_kw_iset_static(ecl_kw , i , iptr);
}



static bool ecl_kw_qskip(FILE *stream) {
  const char sep       = '\'';
  const char space     = ' ';
  const char newline   = '\n';
  const char tab       = '\t';
  bool OK = true;
  char c;
  bool cont = true;
  while (cont) {
    c = fgetc(stream);
    if (c == EOF) {
      cont = false;
      OK   = false;
    } else {
      if (c == space || c == newline || c == tab)
        cont = true;
      else if (c == sep)
        cont = false;
    }
  }
  return OK;
}


static bool ecl_kw_fscanf_qstring(char *s , const char *fmt , int len, FILE *stream) {
  const char null_char = '\0';
  char last_sep;
  bool OK;
  OK = ecl_kw_qskip(stream);
  if (OK) {
    int read_count = 0;
    read_count += fscanf(stream , fmt , s);
    s[len] = null_char;
    read_count += fscanf(stream , "%c" , &last_sep);

    if (read_count != 2)
      util_abort("%s: reading \'xxxxxxxx\' formatted string failed \n",__func__);
  }
  return OK;
}



/*
  This rather painful parsing is because formatted eclipse double
  format : 0.ddddD+01 - difficult to parse the 'D';
*/
/** Should be: NESTED */

static double __fscanf_ECL_double( FILE * stream , const char * fmt) {
  int    read_count , power;
  double value , arg;
  read_count = fscanf( stream , fmt , &arg , &power);
  if (read_count == 2)
    value = arg * pow(10 , power );
  else {
    util_abort("%s: read failed \n",__func__);
    value = -1;
  }
  return value;
}

bool ecl_kw_fread_data(ecl_kw_type *ecl_kw, fortio_type *fortio) {
  const char null_char         = '\0';
  bool fmt_file                = fortio_fmt_file( fortio );
  if (ecl_kw->size > 0) {
    const int blocksize = get_blocksize( ecl_kw->data_type );
    if (fmt_file) {
      const int blocks      = ecl_kw->size / blocksize + (ecl_kw->size % blocksize == 0 ? 0 : 1);
      char * read_fmt       = alloc_read_fmt( ecl_kw->data_type );
      FILE * stream         = fortio_get_FILE(fortio);
      int    offset         = 0;
      int    index          = 0;
      int    ib,ir;
      for (ib = 0; ib < blocks; ib++) {
        int read_elm = util_int_min((ib + 1) * blocksize , ecl_kw->size) - ib * blocksize;
        for (ir = 0; ir < read_elm; ir++) {
          switch(ecl_kw_get_type(ecl_kw)) {
          case(ECL_CHAR_TYPE):
            ecl_kw_fscanf_qstring(&ecl_kw->data[offset] , read_fmt , 8, stream);
            break;
          case(ECL_STRING_TYPE):
            ecl_kw_fscanf_qstring(
                    &ecl_kw->data[offset],
                    read_fmt,
                    ecl_type_get_sizeof_ctype_fortio(ecl_kw_get_data_type(ecl_kw)),
                    stream
                    );
            break;
          case(ECL_INT_TYPE):
            {
              int iread = fscanf(stream , read_fmt , (int *) &ecl_kw->data[offset]);
              if (iread != 1)
                util_abort("%s: after reading %d values reading of keyword:%s from:%s failed - aborting \n",__func__ , offset / ecl_kw_get_sizeof_ctype(ecl_kw) , ecl_kw->header8 , fortio_filename_ref(fortio));
            }
            break;
          case(ECL_FLOAT_TYPE):
            {
              int iread = fscanf(stream , read_fmt , (float *) &ecl_kw->data[offset]);
              if (iread != 1) {
                util_abort("%s: after reading %d values reading of keyword:%s from:%s failed - aborting \n",__func__ ,
                           offset / ecl_kw_get_sizeof_ctype(ecl_kw) ,
                           ecl_kw->header8 ,
                           fortio_filename_ref(fortio));
              }
            }
            break;
          case(ECL_DOUBLE_TYPE):
            {
              double value = __fscanf_ECL_double( stream , read_fmt );
              ecl_kw_iset(ecl_kw , index , &value);
            }
            break;
          case(ECL_BOOL_TYPE):
            {
              char bool_char;
              if (fscanf(stream , read_fmt , &bool_char) == 1) {
                if (bool_char == BOOL_TRUE_CHAR)
                  ecl_kw_iset_bool(ecl_kw , index , true);
                else if (bool_char == BOOL_FALSE_CHAR)
                  ecl_kw_iset_bool(ecl_kw , index , false);
                else
                  util_abort("%s: Logical value: [%c] not recogniced - aborting \n", __func__ , bool_char);
              } else
                util_abort("%s: read failed - premature file end? \n",__func__ );
            }
            break;
          case(ECL_MESS_TYPE):
            ecl_kw_fscanf_qstring(&ecl_kw->data[offset] , read_fmt , 8 , stream);
            break;
          default:
            util_abort("%s: Internal error: internal eclipse_type: %d not recognized - aborting \n",__func__ , ecl_kw_get_type(ecl_kw));
          }
          offset += ecl_kw_get_sizeof_ctype(ecl_kw);
          index++;
        }
      }

      /* Skip the trailing newline */
      fortio_fseek( fortio , 1 , SEEK_CUR);
      free(read_fmt);
      return true;
    } else {
      bool read_ok = true;
      if (ecl_type_is_char(ecl_kw->data_type) || ecl_type_is_mess(ecl_kw->data_type) || ecl_type_is_string(ecl_kw->data_type)) {
        const int blocks = ecl_kw->size / blocksize + (ecl_kw->size % blocksize == 0 ? 0 : 1);
        int ib = 0;
        while (true) {
          /*
            Due to the necessary terminating \0 characters there is
            not a continous file/memory mapping.
          */
          int  read_elm = util_int_min((ib + 1) * blocksize , ecl_kw->size) - ib * blocksize;
          FILE * stream = fortio_get_FILE(fortio);
          int record_size = fortio_init_read(fortio);
          if (record_size >= 0) {
            int ir;
            const int sizeof_ctype        = ecl_type_get_sizeof_ctype(ecl_kw->data_type);
            const int sizeof_ctype_fortio = ecl_type_get_sizeof_ctype_fortio(ecl_kw->data_type);
            for (ir = 0; ir < read_elm; ir++) {
              util_fread( &ecl_kw->data[(ib * blocksize + ir) * sizeof_ctype] , 1 , sizeof_ctype_fortio , stream , __func__);
              ecl_kw->data[(ib * blocksize + ir) * sizeof_ctype + sizeof_ctype_fortio] = null_char;
            }
            read_ok = fortio_complete_read(fortio , record_size);
          } else
            read_ok = false;

          if (!read_ok)
            break;

          ib++;
          if (ib == blocks)
            break;
        }
      } else {
        /**
           This function handles the fuc***g blocks transparently at a
           low level.
        */
        read_ok = fortio_fread_buffer(fortio , ecl_kw->data , ecl_kw->size * ecl_kw_get_sizeof_ctype(ecl_kw));
        if (read_ok && ECL_ENDIAN_FLIP)
          ecl_kw_endian_convert_data(ecl_kw);
      }
      return read_ok;
    }
  } else
    /* The keyword has zero size - and reading data is trivially OK. */
    return true;
}


void ecl_kw_fread_indexed_data(fortio_type * fortio, offset_type data_offset, ecl_data_type data_type, int element_count, const int_vector_type* index_map, char* buffer) {
    const int block_size = get_blocksize(data_type);
    FILE *stream  = fortio_get_FILE( fortio );
    int index;
    int element_size = ecl_type_get_sizeof_ctype(data_type);

    if(ecl_type_is_char(data_type) || ecl_type_is_mess(data_type)) {
        element_size = ECL_STRING8_LENGTH;
    }


    for(index = 0; index < int_vector_size(index_map); index++) {
        int element_index = int_vector_iget(index_map, index);

        if(element_index < 0 || element_index >= element_count) {
            util_abort("%s: Element index is out of range 0 <= %d < %d\n", __func__, element_index, element_count);
        }
        fortio_data_fseek(fortio, data_offset, element_index, element_size, element_count, block_size);
        util_fread(&buffer[index * element_size], element_size, 1, stream, __func__);
    }

    if (ECL_ENDIAN_FLIP) {
        util_endian_flip_vector(buffer, element_size, int_vector_size(index_map));
    }
}

/**
   Allocates storage and reads data.
*/
bool ecl_kw_fread_realloc_data(ecl_kw_type *ecl_kw, fortio_type *fortio) {
  ecl_kw_alloc_data(ecl_kw);
  return ecl_kw_fread_data(ecl_kw , fortio);
}

/**
   Static method without a class instance.
*/

bool ecl_kw_fskip_data__( ecl_data_type data_type , const int element_count , fortio_type * fortio) {
  if (element_count <= 0)
    return true;

  bool fmt_file = fortio_fmt_file(fortio);
  if (fmt_file) {
    /* Formatted skipping actually involves reading the data - nice ??? */
    ecl_kw_type * tmp_kw = ecl_kw_alloc_empty( );
    ecl_kw_initialize( tmp_kw , "WORK" , element_count , data_type );
    ecl_kw_alloc_data(tmp_kw);
    ecl_kw_fread_data(tmp_kw , fortio);
    ecl_kw_free( tmp_kw );
  } else {
    const int blocksize = get_blocksize( data_type );
    const int block_count = element_count / blocksize + (element_count % blocksize != 0);
    int element_size = ecl_type_get_sizeof_ctype_fortio(data_type);

    if(!fortio_data_fskip(fortio, element_size, element_count, block_count))
      return false;
  }

  return true;
}


bool ecl_kw_fskip_data(ecl_kw_type *ecl_kw, fortio_type *fortio) {
  return ecl_kw_fskip_data__( ecl_kw_get_data_type(ecl_kw) , ecl_kw->size , fortio );
}


/**
   This function will skip the header part of an ecl_kw instance. The
   function will read the file content at the current position, it is
   therefor essential that the file pointer is positioned at the
   beginning of a keyword when this function is called; otherwise it
   will be complete crash and burn.
*/


void ecl_kw_fskip_header( fortio_type * fortio) {
  bool fmt_file = fortio_fmt_file( fortio );
  if (fmt_file) {
    ecl_kw_type * ecl_kw = ecl_kw_alloc_empty( );
    ecl_kw_fread_header( ecl_kw , fortio );
    ecl_kw_free( ecl_kw );
  } else
    fortio_fskip_record( fortio );
}


ecl_read_status_enum ecl_kw_fread_header(ecl_kw_type *ecl_kw , fortio_type * fortio) {
  const char null_char = '\0';
  FILE *stream  = fortio_get_FILE( fortio );
  bool fmt_file = fortio_fmt_file( fortio );
  char header[ECL_STRING8_LENGTH + 1];
  char ecl_type_str[ECL_TYPE_LENGTH + 1];
  int record_size;
  int size;

  if (fmt_file) {
    if(!ecl_kw_fscanf_qstring(header , "%8c" , 8 , stream))
      return ECL_KW_READ_FAIL;

    int read_count = fscanf(stream , "%d" , &size);
    if (read_count != 1)
      return ECL_KW_READ_FAIL;

    if (!ecl_kw_fscanf_qstring(ecl_type_str , "%4c" , 4 , stream))
      return ECL_KW_READ_FAIL;

    fgetc(stream);             /* Reading the trailing newline ... */
  }
  else {
    header[ECL_STRING8_LENGTH]    = null_char;
    ecl_type_str[ECL_TYPE_LENGTH] = null_char;
    record_size = fortio_init_read(fortio);

    if (record_size <= 0)
      return ECL_KW_READ_FAIL;

    char buffer[ECL_KW_HEADER_DATA_SIZE];
    size_t read_bytes = fread(buffer , 1 , ECL_KW_HEADER_DATA_SIZE , stream);

    if (read_bytes != ECL_KW_HEADER_DATA_SIZE)
      return ECL_KW_READ_FAIL;

    memcpy( header , &buffer[0] , ECL_STRING8_LENGTH);
    size = *( (int *) &buffer[ECL_STRING8_LENGTH] );
    memcpy( ecl_type_str , &buffer[ECL_STRING8_LENGTH + sizeof(size)] , ECL_TYPE_LENGTH);

    if(!fortio_complete_read(fortio , record_size))
      return ECL_KW_READ_FAIL;

    if (ECL_ENDIAN_FLIP)
      util_endian_flip_vector(&size , sizeof size , 1);
  }

  ecl_data_type data_type = ecl_type_create_from_name( ecl_type_str );
  ecl_kw_initialize( ecl_kw , header , size , data_type);

  return ECL_KW_READ_OK;
}


/**
   Will seek through the open fortio file and search for a keyword with
   header 'kw'. It will always start the search from the present
   position in the file, but if rewind is true it will rewind the
   fortio file if not finding 'kw' between current offset and EOF.

   If the kw is found the fortio pointer is positioned at the
   beginning of the keyword, and the function returns true. If the the
   'kw' is NOT found the file will be repositioned to the initial
   position, and the function will return false; unless abort_on_error
   == true in which case the function will abort if the 'kw' is not
   found.
*/


bool ecl_kw_fseek_kw(const char * kw , bool rewind , bool abort_on_error , fortio_type *fortio) {
  ecl_kw_type *tmp_kw = ecl_kw_alloc_empty();
  long int init_pos   = fortio_ftell( fortio );
  bool cont, kw_found;

  cont     = true;
  kw_found = false;
  while (cont) {
    long current_pos = fortio_ftell( fortio );
    if (ecl_kw_fread_header(tmp_kw , fortio) == ECL_KW_READ_OK) {
      if (ecl_kw_string_eq(ecl_kw_get_header8(tmp_kw) , kw)) {
        fortio_fseek( fortio , current_pos , SEEK_SET );
        kw_found = true;
        cont = false;
      } else
        ecl_kw_fskip_data(tmp_kw , fortio);
    } else {
      if (rewind) {
        fortio_rewind(fortio);
        rewind = false;
      } else
        cont = false;
    }
  }
  if (!kw_found) {
    if (abort_on_error)
      util_abort("%s: failed to locate keyword:%s in file:%s - aborting \n",__func__ , kw , fortio_filename_ref(fortio));

    fortio_fseek(fortio , init_pos , SEEK_SET);
  }

  ecl_kw_free(tmp_kw);
  return kw_found;
}


bool ecl_kw_ifseek_kw(const char * kw , fortio_type * fortio , int index) {
  int i = 0;
  do {
    ecl_kw_fseek_kw(kw , false , true , fortio);
    i++;
  } while (i <= index);
  return true;
}


bool ecl_kw_fseek_last_kw(const char * kw , bool abort_on_error , fortio_type *fortio) {
  long int init_pos = fortio_ftell( fortio );
  bool kw_found     = false;

  fortio_fseek(fortio , 0L , SEEK_SET);
  kw_found = ecl_kw_fseek_kw(kw ,  false , false , fortio);
  if (kw_found) {
    bool cont = true;
    do {
      long int current_pos = fortio_ftell( fortio );
      ecl_kw_fskip(fortio);
      cont = ecl_kw_fseek_kw(kw , false , false , fortio);
      if (!cont) fortio_fseek(fortio , current_pos , SEEK_SET);
    } while (cont);
  } else {
    if (abort_on_error)
      util_abort("%s: could not locate keyword:%s - aborting \n",__func__ , kw);
    else
      fortio_fseek(fortio , init_pos , SEEK_SET);
  }
  return kw_found;
}





void ecl_kw_set_data_ptr(ecl_kw_type * ecl_kw , void * data) {
  if (!ecl_kw->shared_data)
    util_safe_free( ecl_kw->data );
  ecl_kw->data = data;
}


/**
   This is where the storage buffer of the ecl_kw is allocated.
*/
void ecl_kw_alloc_data(ecl_kw_type *ecl_kw) {
  if (ecl_kw->shared_data)
    util_abort("%s: trying to allocate data for ecl_kw object which has been declared with shared storage - aborting \n",__func__);

  {
    size_t byte_size = ecl_kw->size * ecl_kw_get_sizeof_ctype(ecl_kw);
    ecl_kw->data = util_realloc(ecl_kw->data , byte_size );
    memset(ecl_kw->data , 0 , byte_size);
  }
}



void ecl_kw_free_data(ecl_kw_type *ecl_kw) {
  if (!ecl_kw->shared_data)
    util_safe_free(ecl_kw->data);

  ecl_kw->data = NULL;
}



void ecl_kw_set_header_name(ecl_kw_type * ecl_kw , const char * header) {
  ecl_kw->header8 = realloc(ecl_kw->header8 , ECL_STRING8_LENGTH + 1);
  if (strlen(header) <= 8) {
     sprintf(ecl_kw->header8 , "%-8s" , header);

     /* Internalizing a header without the trailing spaces as well. */
     util_safe_free( ecl_kw->header );
     ecl_kw->header = util_alloc_strip_copy( ecl_kw->header8 );
  }
  else {
     ecl_kw->header = util_alloc_copy(header, strlen( header ) + 1);
  }

}


void ecl_kw_set_data_type(ecl_kw_type * ecl_kw, ecl_data_type data_type) {
    memcpy(&ecl_kw->data_type, &data_type, sizeof data_type);
}


bool ecl_kw_fread_realloc(ecl_kw_type *ecl_kw , fortio_type *fortio) {
  if (ecl_kw_fread_header(ecl_kw , fortio) == ECL_KW_READ_OK)
    return ecl_kw_fread_realloc_data( ecl_kw , fortio );
  else
    return false;
}


void ecl_kw_fread(ecl_kw_type * ecl_kw , fortio_type * fortio) {
  int current_size = ecl_kw->size;
  if (ecl_kw_fread_header(ecl_kw , fortio) != ECL_KW_READ_OK)
    util_abort("%s: failed to read header for ecl_kw - aborting \n",__func__);

  if (ecl_kw->size == current_size)
    ecl_kw_fread_data(ecl_kw , fortio);
  else
    util_abort("%s: size mismatch - aborting \n",__func__);
}


ecl_kw_type *ecl_kw_fread_alloc(fortio_type *fortio) {
  bool OK;
  ecl_kw_type *ecl_kw = ecl_kw_alloc_empty();
  OK = ecl_kw_fread_realloc(ecl_kw , fortio);
  if (!OK) {
    free(ecl_kw);
    ecl_kw = NULL;
  }

  return ecl_kw;
}



void ecl_kw_fskip(fortio_type *fortio) {
  ecl_kw_type *tmp_kw;
  tmp_kw = ecl_kw_fread_alloc(fortio );
  ecl_kw_free(tmp_kw);
}




static void ecl_kw_fwrite_data_unformatted( ecl_kw_type * ecl_kw , fortio_type * fortio ) {
  if (ECL_ENDIAN_FLIP)
    ecl_kw_endian_convert_data(ecl_kw);

  {
    const int blocksize  = get_blocksize( ecl_kw->data_type );
    const int num_blocks = ecl_kw->size / blocksize + (ecl_kw->size % blocksize == 0 ? 0 : 1);
    int block_nr;

    for (block_nr = 0; block_nr < num_blocks; block_nr++) {
      int this_blocksize = util_int_min((block_nr + 1)*blocksize , ecl_kw->size) - block_nr*blocksize;
      if (ecl_type_is_char(ecl_kw->data_type) || ecl_type_is_mess(ecl_kw->data_type) || ecl_type_is_string(ecl_kw->data_type)) {
        /*
           Due to the terminating \0 characters there is not a
           continous file/memory mapping - the \0 characters arel
           skipped.
        */
        FILE *stream    = fortio_get_FILE(fortio);
        int word_size   = ecl_type_get_sizeof_ctype_fortio(ecl_kw->data_type);
        int record_size = this_blocksize * word_size;     /* The total size in bytes of the record written by the fortio layer. */
        int i;
        fortio_init_write(fortio , record_size );
        for (i = 0; i < this_blocksize; i++)
          fwrite(&ecl_kw->data[(block_nr * blocksize + i) * ecl_kw_get_sizeof_ctype(ecl_kw)] , 1 , word_size , stream);
        fortio_complete_write(fortio , record_size);
      } else {
        int   record_size = this_blocksize * ecl_kw_get_sizeof_ctype(ecl_kw);  /* The total size in bytes of the record written by the fortio layer. */
        fortio_fwrite_record(fortio , &ecl_kw->data[block_nr * blocksize * ecl_kw_get_sizeof_ctype(ecl_kw)] , record_size);
      }
    }
  }

  if (ECL_ENDIAN_FLIP)
    ecl_kw_endian_convert_data(ecl_kw);
}



/**
     The point of this awkward function is that I have not managed to
     use C fprintf() syntax to reproduce the ECLIPSE
     formatting. ECLIPSE expects the following formatting for float
     and double values:

        0.ddddddddE+03       (float)
        0.ddddddddddddddD+03 (double)

     The problem with printf have been:

        1. To force the radix part to start with 0.
        2. To use 'D' as the exponent start for double values.

     If you are more proficient with C fprintf() format strings than I
     am, the __fprintf_scientific() function should be removed, and
     the WRITE_FMT_DOUBLE and WRITE_FMT_FLOAT format specifiers
     updated accordingly.
  */

   static void __fprintf_scientific(FILE * stream, const char * fmt , double x) {
    double pow_x = ceil(log10(fabs(x)));
    double arg_x   = x / pow(10.0 , pow_x);
    if (x != 0.0) {
      if (fabs(arg_x) == 1.0) {
        arg_x *= 0.10;
        pow_x += 1;
      }
    } else {
      arg_x = 0.0;
      pow_x = 0.0;
    }
    fprintf(stream , fmt , arg_x , (int) pow_x);
  }


static void ecl_kw_fwrite_data_formatted( ecl_kw_type * ecl_kw , fortio_type * fortio ) {

  {

    FILE * stream           = fortio_get_FILE( fortio );
    const int blocksize     = get_blocksize( ecl_kw->data_type );
    const  int columns      = get_columns( ecl_kw->data_type );
    char * write_fmt        = alloc_write_fmt( ecl_kw->data_type );
    const int num_blocks    = ecl_kw->size / blocksize + (ecl_kw->size % blocksize == 0 ? 0 : 1);
    int block_nr;

    for (block_nr = 0; block_nr < num_blocks; block_nr++) {
      int this_blocksize = util_int_min((block_nr + 1)*blocksize , ecl_kw->size) - block_nr*blocksize;
      int num_lines      = this_blocksize / columns + ( this_blocksize % columns == 0 ? 0 : 1);
      int line_nr;
      for (line_nr = 0; line_nr < num_lines; line_nr++) {
        int num_columns = util_int_min( (line_nr + 1)*columns , this_blocksize) - columns * line_nr;
        int col_nr;
        for (col_nr =0; col_nr < num_columns; col_nr++) {
          int data_index  = block_nr * blocksize + line_nr * columns + col_nr;
          void * data_ptr = ecl_kw_iget_ptr_static( ecl_kw , data_index );
          switch (ecl_kw_get_type(ecl_kw)) {
          case(ECL_CHAR_TYPE):
            fprintf(stream , write_fmt , data_ptr);
            break;
          case(ECL_STRING_TYPE):
            fprintf(stream , write_fmt , data_ptr);
            break;
          case(ECL_INT_TYPE):
            {
              int int_value = ((int *) data_ptr)[0];
              fprintf(stream , write_fmt , int_value);
            }
            break;
          case(ECL_BOOL_TYPE):
            {
              bool bool_value = ((bool *) data_ptr)[0];
              if (bool_value)
                fprintf(stream , write_fmt , BOOL_TRUE_CHAR);
              else
                fprintf(stream , write_fmt , BOOL_FALSE_CHAR);
            }
            break;
          case(ECL_FLOAT_TYPE):
            {
              float float_value = ((float *) data_ptr)[0];
              __fprintf_scientific( stream , write_fmt , float_value );
            }
            break;
          case(ECL_DOUBLE_TYPE):
            {
              double double_value = ((double *) data_ptr)[0];
              __fprintf_scientific( stream , write_fmt , double_value );
            }
            break;
          case(ECL_MESS_TYPE):
            util_abort("%s: internal fuckup : message type keywords should NOT have data ??\n",__func__);
            break;
          }
        }
        fprintf(stream , "\n");
      }
    }

    free(write_fmt);
  }
}


void ecl_kw_fwrite_data(const ecl_kw_type *_ecl_kw , fortio_type *fortio) {
  ecl_kw_type *ecl_kw = (ecl_kw_type *) _ecl_kw;
  bool  fmt_file      = fortio_fmt_file( fortio );

  if (fmt_file)
    ecl_kw_fwrite_data_formatted( ecl_kw , fortio );
  else
    ecl_kw_fwrite_data_unformatted( ecl_kw ,fortio );
}



void ecl_kw_fwrite_header(const ecl_kw_type *ecl_kw , fortio_type *fortio) {
  FILE *stream  = fortio_get_FILE(fortio);
  bool fmt_file = fortio_fmt_file(fortio);
  char * type_name = ecl_type_alloc_name(ecl_kw->data_type);

  if (fmt_file)
    fprintf(stream , WRITE_HEADER_FMT , ecl_kw->header8 , ecl_kw->size , type_name);
  else {
    int size = ecl_kw->size;
    if (ECL_ENDIAN_FLIP)
      util_endian_flip_vector(&size , sizeof size , 1);

    fortio_init_write(fortio , ECL_KW_HEADER_DATA_SIZE );

    fwrite(ecl_kw->header8                            , sizeof(char)    , ECL_STRING8_LENGTH  , stream);
    fwrite(&size                                      , sizeof(int)     , 1                  , stream);
    fwrite(type_name , sizeof(char)    , ECL_TYPE_LENGTH    , stream);

    fortio_complete_write(fortio , ECL_KW_HEADER_DATA_SIZE);

  }

  free(type_name);
}


bool ecl_kw_fwrite(const ecl_kw_type *ecl_kw , fortio_type *fortio) {
  if (strlen(ecl_kw_get_header( ecl_kw)) > ECL_STRING8_LENGTH) {
     fortio_fwrite_error(fortio);
     return false;
  }
  ecl_kw_fwrite_header(ecl_kw ,  fortio);
  ecl_kw_fwrite_data(ecl_kw   ,  fortio);
  return true;
}





static void * ecl_kw_get_data_ref(const ecl_kw_type *ecl_kw) {
  return ecl_kw->data;
}

void * ecl_kw_get_ptr(const ecl_kw_type *ecl_kw) {
  return ecl_kw_get_data_ref( ecl_kw );
}


int ecl_kw_get_size(const ecl_kw_type * ecl_kw) {
  return ecl_kw->size;
}

ecl_type_enum ecl_kw_get_type(const ecl_kw_type * ecl_kw) {
  return ecl_type_get_type(ecl_kw->data_type);
}

ecl_data_type ecl_kw_get_data_type(const ecl_kw_type * ecl_kw) {
  return ecl_kw->data_type;
}

size_t ecl_kw_get_sizeof_ctype(const ecl_kw_type * ecl_kw) {
  return ecl_type_get_sizeof_ctype(ecl_kw->data_type);
}


/******************************************************************/


ecl_kw_type * ecl_kw_buffer_alloc(buffer_type * buffer) {
  const char * header    = buffer_fread_string( buffer );
  int size               = buffer_fread_int( buffer );
  ecl_type_enum ecl_type = buffer_fread_int( buffer );
  size_t element_size    = buffer_fread_int( buffer );

  ecl_data_type data_type = ecl_type_create(ecl_type, element_size);
  ecl_kw_type * ecl_kw = ecl_kw_alloc_empty();
  ecl_kw_initialize( ecl_kw , header , size , data_type );
  ecl_kw_alloc_data(ecl_kw);
  buffer_fread(buffer , ecl_kw->data , ecl_kw_get_sizeof_ctype(ecl_kw) , ecl_kw->size);
  return ecl_kw;
}


void ecl_kw_buffer_store(const ecl_kw_type * ecl_kw , buffer_type * buffer) {
  buffer_fwrite_string( buffer , ecl_kw->header8 );
  buffer_fwrite_int( buffer , ecl_kw->size );
  buffer_fwrite_int( buffer , ecl_type_get_type(ecl_kw->data_type) );
  buffer_fwrite_int( buffer , ecl_type_get_sizeof_ctype(ecl_kw->data_type));
  buffer_fwrite( buffer , ecl_kw->data , ecl_kw_get_sizeof_ctype(ecl_kw) , ecl_kw->size);
}




void ecl_kw_fwrite_param_fortio(fortio_type * fortio, const char * header ,  ecl_data_type data_type , int size, void * data) {
  ecl_kw_type   * ecl_kw = ecl_kw_alloc_new_shared(header , size , data_type , data);
  ecl_kw_fwrite(ecl_kw , fortio);
  ecl_kw_free(ecl_kw);
}



void ecl_kw_fwrite_param(const char * filename , bool fmt_file , const char * header ,  ecl_data_type data_type , int size, void * data) {
  fortio_type   * fortio = fortio_open_writer(filename , fmt_file , ECL_ENDIAN_FLIP);
  ecl_kw_fwrite_param_fortio(fortio , header , data_type , size , data);
  fortio_fclose(fortio);
}



void ecl_kw_get_data_as_double(const ecl_kw_type * ecl_kw , double * double_data) {

  if (ecl_type_is_double(ecl_kw->data_type))
    // Direct memcpy - no conversion
    ecl_kw_get_memcpy_data(ecl_kw , double_data);
  else {
    if (ecl_type_is_float(ecl_kw->data_type)) {
      const float * float_data = (const float *) ecl_kw->data;
      util_float_to_double(double_data , float_data  , ecl_kw->size);
    } else if (ecl_type_is_int(ecl_kw->data_type)) {
      const int * int_data = (const int *) ecl_kw->data;
      int i;
      for (i=0; i < ecl_kw->size; i++)
        double_data[i] = int_data[i];
    } else {
      fprintf(stderr,"%s: type can not be converted to double - aborting \n",__func__);
      ecl_kw_summarize(ecl_kw);
      util_abort("%s: Aborting \n",__func__);
    }
  }
}


void ecl_kw_get_data_as_float(const ecl_kw_type * ecl_kw , float * float_data) {

  if (ecl_type_is_float(ecl_kw->data_type))
    // Direct memcpy - no conversion
    ecl_kw_get_memcpy_data(ecl_kw , float_data);
  else {
    if (ecl_type_is_double(ecl_kw->data_type)) {
      const double * double_data = (const double *) ecl_kw->data;
      util_double_to_float(float_data , double_data  , ecl_kw->size);
    } else if (ecl_type_is_int(ecl_kw->data_type)) {
      const int * int_data = (const int *) ecl_kw->data;
      int i;
      for (i=0; i < ecl_kw->size; i++)
        float_data[i] = (float) int_data[i];
    } else {
      fprintf(stderr,"%s: type can not be converted to float - aborting \n",__func__);
      ecl_kw_summarize(ecl_kw);
      util_abort("%s: Aborting \n",__func__);
    }
  }
}




/**
   Will create a new keyword of the same type as src_kw, and size
   @target_size. The integer array mapping is a list sizeof(src_kw)
   elements, where each element is the new index, i.e.

       new_kw[ mapping[i] ]  = src_kw[i]

   For all inactive elements in new kw are set as follows:

   0          - For float / int / double
   False      - For logical
   ""         - For char
*/

ecl_kw_type * ecl_kw_alloc_scatter_copy( const ecl_kw_type * src_kw , int target_size , const int * mapping, void * def_value) {
  int default_int           = 0;
  double default_double     = 0;
  float default_float       = 0;
  int   default_bool        = ECL_BOOL_FALSE_INT;
  const char * default_char = "";
  ecl_kw_type * new_kw = ecl_kw_alloc( src_kw->header , target_size , src_kw->data_type );

  if (def_value != NULL)
    ecl_kw_scalar_set__( new_kw , def_value );
  else {
    /** Initialize with defaults .*/
    switch (ecl_kw_get_type(src_kw)) {
    case(ECL_INT_TYPE):
      ecl_kw_scalar_set__( new_kw , &default_int );
      break;
    case(ECL_FLOAT_TYPE):
      ecl_kw_scalar_set__( new_kw , &default_float );
      break;
    case(ECL_DOUBLE_TYPE):
      ecl_kw_scalar_set__( new_kw , &default_double );
      break;
    case(ECL_BOOL_TYPE):
      ecl_kw_scalar_set__( new_kw , &default_bool );
      break;
    case(ECL_CHAR_TYPE):
      ecl_kw_scalar_set__( new_kw , default_char );
      break;
    default:
      util_abort("%s: unsupported type:%d \n", __func__ , ecl_kw_get_type(src_kw));
    }
  }

  {
    int sizeof_ctype = ecl_type_get_sizeof_ctype( src_kw->data_type );
    int i;
        for( i =0; i < src_kw->size; i++) {
      int target_index = mapping[i];
      memcpy( &new_kw->data[ target_index * sizeof_ctype ] , &src_kw->data[ i * sizeof_ctype] , sizeof_ctype);
    }
  }

  return new_kw;
}



void ecl_kw_fread_double_param(const char * filename , bool fmt_file , double * double_data) {
  fortio_type   * fortio      = fortio_open_reader(filename , fmt_file , ECL_ENDIAN_FLIP);
  ecl_kw_type   * ecl_kw      = ecl_kw_fread_alloc(fortio);
  fortio_fclose(fortio);

  if (ecl_kw == NULL)
    util_abort("%s: fatal error: loading parameter from: %s failed - aborting \n",__func__ , filename);

  ecl_kw_get_data_as_double(ecl_kw , double_data);
  ecl_kw_free(ecl_kw);
}


void ecl_kw_summarize(const ecl_kw_type * ecl_kw) {
  char * type_name = ecl_type_alloc_name(ecl_kw->data_type);
  printf("%8s   %10d:%4s \n",ecl_kw_get_header8(ecl_kw),
         ecl_kw_get_size(ecl_kw),
         type_name);
  free(type_name);
}





/*****************************************************************/

#define ECL_KW_SCALAR_SET_TYPED( ctype , ECL_TYPE ) \
void ecl_kw_scalar_set_ ## ctype( ecl_kw_type * ecl_kw , ctype value){  \
 if (ecl_kw_get_type(ecl_kw) == ECL_TYPE) {                                    \
    ctype * data = ecl_kw_get_data_ref(ecl_kw);                         \
    int i;                                                              \
    for (i=0;i < ecl_kw->size; i++)                                     \
      data[i] = value;                                                  \
 } else util_abort("%s: wrong type\n",__func__);                        \
}

ECL_KW_SCALAR_SET_TYPED( int   , ECL_INT_TYPE )
ECL_KW_SCALAR_SET_TYPED( float , ECL_FLOAT_TYPE )
ECL_KW_SCALAR_SET_TYPED( double , ECL_DOUBLE_TYPE )
#undef ECL_KW_SCALAR_SET_TYPED

void ecl_kw_scalar_set_bool( ecl_kw_type * ecl_kw , bool bool_value) {
  if (ecl_kw_get_type(ecl_kw) != ECL_BOOL_TYPE)
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));
  {
    int * data = ecl_kw_get_data_ref(ecl_kw);
    int int_value;
    if (bool_value)
      int_value = ECL_BOOL_TRUE_INT;
    else
      int_value = ECL_BOOL_FALSE_INT;
    {
      int i;
      for ( i=0; i < ecl_kw->size; i++)
        data[i] = int_value;
    }
  }
}


void ecl_kw_scalar_set_float_or_double( ecl_kw_type * ecl_kw , double value ) {
  ecl_type_enum ecl_type = ecl_kw_get_type(ecl_kw);
  if (ecl_type == ECL_FLOAT_TYPE)
    ecl_kw_scalar_set_float( ecl_kw , (float) value);
  else if (ecl_type == ECL_DOUBLE_TYPE)
    ecl_kw_scalar_set_double( ecl_kw ,  value);
  else
    util_abort("%s: wrong type \n",__func__);
}

/*
  Untyped - low level alternative.
*/
void ecl_kw_scalar_set__(ecl_kw_type * ecl_kw , const void * value) {
  int sizeof_ctype = ecl_type_get_sizeof_ctype( ecl_kw->data_type );
  int i;
  for (i=0;i < ecl_kw->size; i++)
    memcpy( &ecl_kw->data[ i * sizeof_ctype ] , value , sizeof_ctype);
}

/*****************************************************************/




void ecl_kw_alloc_double_data(ecl_kw_type * ecl_kw , double * values) {
  ecl_kw_alloc_data(ecl_kw);
  memcpy(ecl_kw->data , values , ecl_kw->size * ecl_kw_get_sizeof_ctype(ecl_kw));
}

void ecl_kw_alloc_float_data(ecl_kw_type * ecl_kw , float * values) {
  ecl_kw_alloc_data(ecl_kw);
  memcpy(ecl_kw->data , values , ecl_kw->size * ecl_kw_get_sizeof_ctype(ecl_kw));
}

/*****************************************************************/
/* Macros for typed mathematical functions.                      */

#define ECL_KW_SCALE_TYPED( ctype , ECL_TYPE )                                                        \
void ecl_kw_scale_ ## ctype (ecl_kw_type * ecl_kw , ctype scale_factor) {                             \
  if (ecl_kw_get_type(ecl_kw) != ECL_TYPE)                                                            \
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));  \
  {                                                                                                   \
     ctype * data = ecl_kw_get_data_ref(ecl_kw);                                                      \
     int    size  = ecl_kw_get_size(ecl_kw);                                                          \
     int i;                                                                                           \
     for (i=0; i < size; i++)                                                                         \
        data[i] *= scale_factor;                                                                      \
  }                                                                                                   \
}

ECL_KW_SCALE_TYPED( int , ECL_INT_TYPE)
ECL_KW_SCALE_TYPED( float , ECL_FLOAT_TYPE)
ECL_KW_SCALE_TYPED( double , ECL_DOUBLE_TYPE )
#undef ECL_KW_SCALE_TYPED

void ecl_kw_scale_float_or_double( ecl_kw_type * ecl_kw , double scale_factor ) {
  ecl_type_enum ecl_type = ecl_kw_get_type(ecl_kw);
  if (ecl_type == ECL_FLOAT_TYPE)
    ecl_kw_scale_float( ecl_kw , (float) scale_factor);
  else if (ecl_type == ECL_DOUBLE_TYPE)
    ecl_kw_scale_double( ecl_kw ,  scale_factor);
  else
    util_abort("%s: wrong type \n",__func__);
}


#define ECL_KW_SHIFT_TYPED( ctype , ECL_TYPE )                                                        \
void ecl_kw_shift_ ## ctype (ecl_kw_type * ecl_kw , ctype shift_value) {                              \
  if (ecl_kw_get_type(ecl_kw) != ECL_TYPE)                                                            \
    util_abort("%s: Keyword: %s is wrong type - aborting \n",__func__ , ecl_kw_get_header8(ecl_kw));  \
  {                                                                                                   \
     ctype * data = ecl_kw_get_data_ref(ecl_kw);                                                      \
     int    size  = ecl_kw_get_size(ecl_kw);                                                          \
     int i;                                                                                           \
     for (i=0; i < size; i++)                                                                         \
        data[i] += shift_value;                                                                       \
  }                                                                                                   \
}

ECL_KW_SHIFT_TYPED( int , ECL_INT_TYPE)
ECL_KW_SHIFT_TYPED( float , ECL_FLOAT_TYPE)
ECL_KW_SHIFT_TYPED( double , ECL_DOUBLE_TYPE )
#undef ECL_KW_SHIFT_TYPED


void ecl_kw_shift_float_or_double( ecl_kw_type * ecl_kw , double shift_value ) {
  ecl_type_enum ecl_type = ecl_kw_get_type(ecl_kw);
  if (ecl_type == ECL_FLOAT_TYPE)
    ecl_kw_shift_float( ecl_kw , (float) shift_value );
  else if (ecl_type == ECL_DOUBLE_TYPE)
    ecl_kw_shift_double( ecl_kw ,  shift_value );
  else
    util_abort("%s: wrong type \n",__func__);
}

bool ecl_kw_size_and_numeric_type_equal( const ecl_kw_type * kw1, const ecl_kw_type * kw2) {
  return ecl_kw_size_and_type_equal(kw1, kw2) && ecl_type_is_numeric(kw1->data_type);
}



#define ECL_KW_ASSERT_TYPED_BINARY_OP( ctype , ECL_TYPE ) \
bool ecl_kw_assert_binary_ ## ctype( const ecl_kw_type * kw1 , const ecl_kw_type * kw2) { \
 if (!ecl_kw_size_and_numeric_type_equal( kw1 , kw2))                                                \
    return false;                                                                         \
 if (ecl_kw_get_type(kw1) != ECL_TYPE)                                                           \
    return false;   /* Type mismatch */                                                   \
 return true;                                                                             \
}

ECL_KW_ASSERT_TYPED_BINARY_OP( int , ECL_INT_TYPE )
ECL_KW_ASSERT_TYPED_BINARY_OP( float , ECL_FLOAT_TYPE )
ECL_KW_ASSERT_TYPED_BINARY_OP( double , ECL_DOUBLE_TYPE )
#undef ECL_KW_ASSERT_TYPED_BINARY_OP


void ecl_kw_copy_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * src_kw) {
  if (!ecl_kw_size_and_type_equal( target_kw , src_kw ))
    util_abort("%s: type/size  mismatch\n",__func__);
  {
    char * target_data = ecl_kw_get_data_ref( target_kw );
    const char * src_data = ecl_kw_get_data_ref( src_kw );
    int sizeof_ctype = ecl_type_get_sizeof_ctype(target_kw->data_type);
    int set_size     = int_vector_size( index_set );
    const int * index_data = int_vector_get_const_ptr( index_set );
    int i;
    for (i=0; i < set_size; i++) {
      int index = index_data[i];
      memcpy( &target_data[ index * sizeof_ctype ] , &src_data[ index * sizeof_ctype] , sizeof_ctype);
    }
  }
}



#define ECL_KW_TYPED_INPLACE_ADD_INDEXED( ctype ) \
static void ecl_kw_inplace_add_indexed_ ## ctype( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * add_kw) { \
 if (!ecl_kw_assert_binary_ ## ctype( target_kw , add_kw ))                                \
    util_abort("%s: type/size  mismatch\n",__func__);                                      \
 {                                                                                         \
    ctype * target_data = ecl_kw_get_data_ref( target_kw );                                \
    const ctype * add_data = ecl_kw_get_data_ref( add_kw );                                \
    int set_size     = int_vector_size( index_set );                                       \
    const int * index_data = int_vector_get_const_ptr( index_set );                        \
    int i;                                                                                 \
    for (i=0; i < set_size; i++) {                                                         \
      int index = index_data[i];                                                           \
      target_data[index] += add_data[index];                                               \
    }                                                                                      \
  }                                                                                        \
}


ECL_KW_TYPED_INPLACE_ADD_INDEXED( int )
ECL_KW_TYPED_INPLACE_ADD_INDEXED( double )
ECL_KW_TYPED_INPLACE_ADD_INDEXED( float )
#undef ECL_KW_TYPED_INPLACE_ADD

void ecl_kw_inplace_add_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * add_kw) {
  ecl_type_enum type = ecl_kw_get_type(target_kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_add_indexed_float( target_kw , index_set , add_kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_add_indexed_double( target_kw , index_set , add_kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_add_indexed_int( target_kw , index_set , add_kw );
    break;
  default:
    util_abort("%s: inplace add not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(target_kw) ));
  }
}




#define ECL_KW_TYPED_INPLACE_ADD( ctype ) \
static void ecl_kw_inplace_add_ ## ctype( ecl_kw_type * target_kw , const ecl_kw_type * add_kw) { \
 if (!ecl_kw_assert_binary_ ## ctype( target_kw , add_kw ))                                \
    util_abort("%s: type/size  mismatch\n",__func__);                                      \
 {                                                                                         \
    ctype * target_data = ecl_kw_get_data_ref( target_kw );                                \
    const ctype * add_data = ecl_kw_get_data_ref( add_kw );                                \
    int i;                                                                                 \
    for (i=0; i < target_kw->size; i++)                                                    \
      target_data[i] += add_data[i];                                                       \
 }                                                                                         \
}
ECL_KW_TYPED_INPLACE_ADD( int )
ECL_KW_TYPED_INPLACE_ADD( double )
ECL_KW_TYPED_INPLACE_ADD( float )

#undef ECL_KW_TYPED_INPLACE_ADD

void ecl_kw_inplace_add( ecl_kw_type * target_kw , const ecl_kw_type * add_kw) {
  ecl_type_enum type = ecl_kw_get_type(target_kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_add_float( target_kw , add_kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_add_double( target_kw , add_kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_add_int( target_kw , add_kw );
    break;
  default:
    util_abort("%s: inplace add not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(target_kw) ));
  }
}






#define ECL_KW_TYPED_INPLACE_SUB( ctype ) \
void ecl_kw_inplace_sub_ ## ctype( ecl_kw_type * target_kw , const ecl_kw_type * sub_kw) { \
 if (!ecl_kw_assert_binary_ ## ctype( target_kw , sub_kw ))                                \
    util_abort("%s: type/size  mismatch\n",__func__);                                      \
 {                                                                                         \
    ctype * target_data = ecl_kw_get_data_ref( target_kw );                                \
    const ctype * sub_data = ecl_kw_get_data_ref( sub_kw );                                \
    int i;                                                                                 \
    for (i=0; i < target_kw->size; i++)                                                    \
      target_data[i] -= sub_data[i];                                                       \
 }                                                                                         \
}
ECL_KW_TYPED_INPLACE_SUB( int )
ECL_KW_TYPED_INPLACE_SUB( double )
ECL_KW_TYPED_INPLACE_SUB( float )
#undef ECL_KW_TYPED_INPLACE_SUB

void ecl_kw_inplace_sub( ecl_kw_type * target_kw , const ecl_kw_type * sub_kw) {
  ecl_type_enum type = ecl_kw_get_type(target_kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_sub_float( target_kw , sub_kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_sub_double( target_kw , sub_kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_sub_int( target_kw , sub_kw );
    break;
  default:
    util_abort("%s: inplace sub not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(target_kw) ));
  }
}

#define ECL_KW_TYPED_INPLACE_SUB_INDEXED( ctype ) \
static void ecl_kw_inplace_sub_indexed_ ## ctype( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * sub_kw) { \
 if (!ecl_kw_assert_binary_ ## ctype( target_kw , sub_kw ))                                \
    util_abort("%s: type/size  mismatch\n",__func__);                                      \
 {                                                                                         \
    ctype * target_data = ecl_kw_get_data_ref( target_kw );                                \
    const ctype * sub_data = ecl_kw_get_data_ref( sub_kw );                                \
    int set_size     = int_vector_size( index_set );                                       \
    const int * index_data = int_vector_get_const_ptr( index_set );                        \
    int i;                                                                                 \
    for (i=0; i < set_size; i++) {                                                         \
      int index = index_data[i];                                                           \
      target_data[index] -= sub_data[index];                                               \
    }                                                                                      \
  }                                                                                        \
}


ECL_KW_TYPED_INPLACE_SUB_INDEXED( int )
ECL_KW_TYPED_INPLACE_SUB_INDEXED( double )
ECL_KW_TYPED_INPLACE_SUB_INDEXED( float )
#undef ECL_KW_TYPED_INPLACE_SUB

void ecl_kw_inplace_sub_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * sub_kw) {
  ecl_type_enum type = ecl_kw_get_type(target_kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_sub_indexed_float( target_kw , index_set , sub_kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_sub_indexed_double( target_kw , index_set , sub_kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_sub_indexed_int( target_kw , index_set , sub_kw );
    break;
  default:
    util_abort("%s: inplace sub not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(target_kw) ));
  }
}


/*****************************************************************/

#define ECL_KW_TYPED_INPLACE_ABS( ctype , abs_func)     \
void ecl_kw_inplace_abs_ ## ctype( ecl_kw_type * kw ) { \
  ctype * data = ecl_kw_get_data_ref( kw );             \
  int i;                                                \
  for (i=0; i < kw->size; i++)                          \
    data[i] = abs_func(data[i]);                        \
}

ECL_KW_TYPED_INPLACE_ABS( int , abs )
ECL_KW_TYPED_INPLACE_ABS( double , fabs )
ECL_KW_TYPED_INPLACE_ABS( float , fabsf )
#undef ECL_KW_TYPED_INPLACE_ABS



void ecl_kw_inplace_abs( ecl_kw_type * kw ) {
  ecl_type_enum type = ecl_kw_get_type(kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_abs_float( kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_abs_double( kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_abs_int( kw );
    break;
  default:
    util_abort("%s: inplace abs not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(kw) ));
  }
}


/*****************************************************************/

#define ECL_KW_TYPED_INPLACE_MUL( ctype ) \
void ecl_kw_inplace_mul_ ## ctype( ecl_kw_type * target_kw , const ecl_kw_type * mul_kw) { \
 if (!ecl_kw_assert_binary_ ## ctype( target_kw , mul_kw ))                                \
    util_abort("%s: type/size  mismatch\n",__func__);                                      \
 {                                                                                         \
    ctype * target_data = ecl_kw_get_data_ref( target_kw );                                \
    const ctype * mul_data = ecl_kw_get_data_ref( mul_kw );                                \
    int i;                                                                                 \
    for (i=0; i < target_kw->size; i++)                                                    \
      target_data[i] *= mul_data[i];                                                       \
 }                                                                                         \
}
ECL_KW_TYPED_INPLACE_MUL( int )
ECL_KW_TYPED_INPLACE_MUL( double )
ECL_KW_TYPED_INPLACE_MUL( float )
#undef ECL_KW_TYPED_INPLACE_MUL

void ecl_kw_inplace_mul( ecl_kw_type * target_kw , const ecl_kw_type * mul_kw) {
  ecl_type_enum type = ecl_kw_get_type(target_kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_mul_float( target_kw , mul_kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_mul_double( target_kw , mul_kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_mul_int( target_kw , mul_kw );
    break;
  default:
    util_abort("%s: inplace mul not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(target_kw) ));
  }
}

#define ECL_KW_TYPED_INPLACE_MUL_INDEXED( ctype ) \
static void ecl_kw_inplace_mul_indexed_ ## ctype( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * mul_kw) { \
 if (!ecl_kw_assert_binary_ ## ctype( target_kw , mul_kw ))                                \
    util_abort("%s: type/size  mismatch\n",__func__);                                      \
 {                                                                                         \
    ctype * target_data = ecl_kw_get_data_ref( target_kw );                                \
    const ctype * mul_data = ecl_kw_get_data_ref( mul_kw );                                \
    int set_size     = int_vector_size( index_set );                                       \
    const int * index_data = int_vector_get_const_ptr( index_set );                        \
    int i;                                                                                 \
    for (i=0; i < set_size; i++) {                                                         \
      int index = index_data[i];                                                           \
      target_data[index] *= mul_data[index];                                               \
    }                                                                                      \
  }                                                                                        \
}


ECL_KW_TYPED_INPLACE_MUL_INDEXED( int )
ECL_KW_TYPED_INPLACE_MUL_INDEXED( double )
ECL_KW_TYPED_INPLACE_MUL_INDEXED( float )
#undef ECL_KW_TYPED_INPLACE_MUL

void ecl_kw_inplace_mul_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * mul_kw) {
  ecl_type_enum type = ecl_kw_get_type(target_kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_mul_indexed_float( target_kw , index_set , mul_kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_mul_indexed_double( target_kw , index_set , mul_kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_mul_indexed_int( target_kw , index_set , mul_kw );
    break;
  default:
    util_abort("%s: inplace mul not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(target_kw) ));
  }
}



/*****************************************************************/

#define ECL_KW_TYPED_INPLACE_DIV( ctype ) \
void ecl_kw_inplace_div_ ## ctype( ecl_kw_type * target_kw , const ecl_kw_type * div_kw) { \
 if (!ecl_kw_assert_binary_ ## ctype( target_kw , div_kw ))                                \
    util_abort("%s: type/size  mismatch\n",__func__);                                      \
 {                                                                                         \
    ctype * target_data = ecl_kw_get_data_ref( target_kw );                                \
    const ctype * div_data = ecl_kw_get_data_ref( div_kw );                                \
    int i;                                                                                 \
    for (i=0; i < target_kw->size; i++)                                                    \
      target_data[i] /= div_data[i];                                                       \
 }                                                                                         \
}
ECL_KW_TYPED_INPLACE_DIV( int )
ECL_KW_TYPED_INPLACE_DIV( double )
ECL_KW_TYPED_INPLACE_DIV( float )
#undef ECL_KW_TYPED_INPLACE_DIV

void ecl_kw_inplace_div( ecl_kw_type * target_kw , const ecl_kw_type * div_kw) {
  ecl_type_enum type = ecl_kw_get_type(target_kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_div_float( target_kw , div_kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_div_double( target_kw , div_kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_div_int( target_kw , div_kw );
    break;
  default:
    util_abort("%s: inplace div not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(target_kw) ));
  }
}


#define ECL_KW_TYPED_INPLACE_DIV_INDEXED( ctype ) \
static void ecl_kw_inplace_div_indexed_ ## ctype( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * div_kw) { \
 if (!ecl_kw_assert_binary_ ## ctype( target_kw , div_kw ))                                \
    util_abort("%s: type/size  mismatch\n",__func__);                                      \
 {                                                                                         \
    ctype * target_data = ecl_kw_get_data_ref( target_kw );                                \
    const ctype * div_data = ecl_kw_get_data_ref( div_kw );                                \
    int set_size     = int_vector_size( index_set );                                       \
    const int * index_data = int_vector_get_const_ptr( index_set );                        \
    int i;                                                                                 \
    for (i=0; i < set_size; i++) {                                                         \
      int index = index_data[i];                                                           \
      target_data[index] *= div_data[index];                                               \
    }                                                                                      \
  }                                                                                        \
}


ECL_KW_TYPED_INPLACE_DIV_INDEXED( int )
ECL_KW_TYPED_INPLACE_DIV_INDEXED( double )
ECL_KW_TYPED_INPLACE_DIV_INDEXED( float )
#undef ECL_KW_TYPED_INPLACE_DIV

void ecl_kw_inplace_div_indexed( ecl_kw_type * target_kw , const int_vector_type * index_set , const ecl_kw_type * div_kw) {
  ecl_type_enum type = ecl_kw_get_type(target_kw);
  switch (type) {
  case(ECL_FLOAT_TYPE):
    ecl_kw_inplace_div_indexed_float( target_kw , index_set , div_kw );
    break;
  case(ECL_DOUBLE_TYPE):
    ecl_kw_inplace_div_indexed_double( target_kw , index_set , div_kw );
    break;
  case(ECL_INT_TYPE):
    ecl_kw_inplace_div_indexed_int( target_kw , index_set , div_kw );
    break;
  default:
    util_abort("%s: inplace div not implemented for type:%s \n",__func__ , ecl_type_alloc_name( ecl_kw_get_data_type(target_kw) ));
  }
}




/*****************************************************************/


void ecl_kw_inplace_inv(ecl_kw_type * my_kw) {
  int            size = ecl_kw_get_size(my_kw);
  ecl_type_enum type = ecl_kw_get_type(my_kw);
  {
    int i;
    void * my_data        = ecl_kw_get_data_ref(my_kw);

    switch (type) {
    case(ECL_DOUBLE_TYPE):
      {
        double *my_double        = (double *) my_data;
        for (i=0; i < size; i++)
          my_double[i] = 1.0/ my_double[i];
        break;
      }
    case(ECL_FLOAT_TYPE):
      {
        float *my_float        = (float *)       my_data;
        for (i=0; i < size; i++)
          my_float[i] = 1.0f / my_float[i];
        break;
      }
    default:
      util_abort("%s: can only be called on ECL_FLOAT_TYPE and ECL_DOUBLE_TYPE - aborting \n",__func__);
    }
  }
}





void ecl_kw_inplace_update_file(const ecl_kw_type * ecl_kw , const char * filename, int index) {
  if (util_file_exists(filename)) {
    bool fmt_file = util_fmt_bit8(filename);

    {
      fortio_type * fortio =  fortio_open_readwrite(filename , fmt_file , ECL_ENDIAN_FLIP);
      ecl_kw_ifseek_kw(ecl_kw_get_header8(ecl_kw) , fortio , index);
      {
        ecl_kw_type * file_kw  = ecl_kw_alloc_empty();
        long int   current_pos = fortio_ftell( fortio );
        ecl_kw_fread_header(file_kw , fortio);
        fortio_fseek( fortio , current_pos , SEEK_SET );

        if (!ecl_kw_size_and_type_equal(ecl_kw, file_kw))
          util_abort("%s: header mismatch when trying to update:%s in %s \n",__func__ , ecl_kw_get_header8(ecl_kw) , filename);
        ecl_kw_free(file_kw);
      }


      fortio_fflush(fortio);
      ecl_kw_fwrite(ecl_kw , fortio);
      fortio_fclose(fortio);
    }
  }
}


/******************************************************************/

bool ecl_kw_is_kw_file(fortio_type * fortio) {
  const long int init_pos = fortio_ftell( fortio );
  bool kw_file;

  {
    ecl_kw_type * ecl_kw = ecl_kw_alloc_empty();

    if (fortio_fmt_file( fortio ))
      kw_file = (ecl_kw_fread_header(ecl_kw , fortio) != ECL_KW_READ_FAIL);
    else {
      if (fortio_is_fortio_file(fortio))
        kw_file = (ecl_kw_fread_header(ecl_kw , fortio) != ECL_KW_READ_FAIL);
      else
        kw_file = false;
    }

    ecl_kw_free(ecl_kw);
  }

  fortio_fseek(fortio , init_pos , SEEK_SET);
  return kw_file;
}


#define KW_MAX_MIN(type)                                       \
{                                                              \
  type * data = ecl_kw_get_data_ref(ecl_kw);                   \
  type max = data[0];                                          \
  type min = data[0];                                          \
  int i;                                                       \
  for (i=1; i < ecl_kw_get_size(ecl_kw); i++)                  \
      util_update_ ## type ## _max_min(data[i] , &max , &min); \
  memcpy(_max , &max , ecl_kw_get_sizeof_ctype(ecl_kw));       \
  memcpy(_min , &min , ecl_kw_get_sizeof_ctype(ecl_kw));       \
}



void ecl_kw_max_min(const ecl_kw_type * ecl_kw , void * _max , void *_min) {
  switch (ecl_kw_get_type(ecl_kw)) {
  case(ECL_FLOAT_TYPE):
    KW_MAX_MIN(float);
    break;
  case(ECL_DOUBLE_TYPE):
    KW_MAX_MIN(double);
    break;
  case(ECL_INT_TYPE):
    KW_MAX_MIN(int);
    break;
  default:
    util_abort("%s: invalid type for element sum \n",__func__);
  }
}

#define ECL_KW_MAX_MIN( ctype )                                                       \
void ecl_kw_max_min_ ## ctype ( const ecl_kw_type * ecl_kw , ctype * _max , ctype * _min) { \
 KW_MAX_MIN( ctype );                                                                 \
}


#define ECL_KW_MAX( ctype )                                       \
  ctype ecl_kw_ ## ctype ## _max( const ecl_kw_type * ecl_kw ) {  \
  ctype max,min;                                                  \
  ecl_kw_max_min_ ## ctype( ecl_kw , &max , &min);                \
  return max;                                                     \
}

#define ECL_KW_MIN( ctype )                                       \
  ctype ecl_kw_ ## ctype ## _min( const ecl_kw_type * ecl_kw ) {  \
  ctype max,min;                                                  \
  ecl_kw_max_min_ ## ctype( ecl_kw , &max , &min);                \
  return min;                                                     \
}

ECL_KW_MAX_MIN( int )
ECL_KW_MAX_MIN( float )
ECL_KW_MAX_MIN( double )

ECL_KW_MAX( int )
ECL_KW_MAX( float )
ECL_KW_MAX( double )

ECL_KW_MIN( int )
ECL_KW_MIN( float )
ECL_KW_MIN( double )

#undef ECL_KW_MAX
#undef ECL_KW_MIN
#undef KW_MAX_MIN
#undef ECL_KW_MAX_MIN







#define KW_SUM(type)                                        \
{                                                           \
  type * data = ecl_kw_get_data_ref(ecl_kw);                \
  type sum = 0;                                             \
  int i;                                                    \
  for (i=0; i < ecl_kw_get_size(ecl_kw); i++)               \
     sum += data[i];                                        \
  memcpy(_sum , &sum , ecl_kw_get_sizeof_ctype(ecl_kw));    \
}



void ecl_kw_element_sum(const ecl_kw_type * ecl_kw , void * _sum) {
  switch (ecl_kw_get_type(ecl_kw)) {
  case(ECL_FLOAT_TYPE):
    KW_SUM(float);
    break;
  case(ECL_DOUBLE_TYPE):
    KW_SUM(double);
    break;
  case(ECL_INT_TYPE):
    KW_SUM(int);
    break;
  default:
    util_abort("%s: invalid type for element sum \n",__func__);
  }
}
#undef KW_SUM


double ecl_kw_element_sum_float( const ecl_kw_type * ecl_kw ) {
  float float_sum;
  double double_sum;
  void * sum_ptr = NULL;

  if (ecl_type_is_double(ecl_kw->data_type))
    sum_ptr = &double_sum;
  else if (ecl_type_is_float(ecl_kw->data_type))
    sum_ptr = &float_sum;
  else
    util_abort("%s: invalid type: \n",__func__);

  ecl_kw_element_sum( ecl_kw , sum_ptr );

  if (ecl_type_is_double(ecl_kw->data_type))
    return double_sum;
  else if (ecl_type_is_float(ecl_kw->data_type))
    return float_sum;
  else
    return 0;
}


int ecl_kw_element_sum_int( const ecl_kw_type * ecl_kw ) {
  int int_sum;
  ecl_kw_element_sum( ecl_kw , &int_sum);

  return int_sum;
}

/*****************************************************************/

#define ECL_KW_FPRINTF_DATA(ctype)                                                                        \
static void ecl_kw_fprintf_data_ ## ctype(const ecl_kw_type * ecl_kw , const char * fmt , FILE * stream)  \
{                                                                                                         \
  const ctype * data = (const ctype *) ecl_kw->data;                                                      \
  int i;                                                                                                  \
  for (i=0; i < ecl_kw->size; i++)                                                                        \
    fprintf(stream , fmt , data[i]);                                                                      \
}

ECL_KW_FPRINTF_DATA( int )
ECL_KW_FPRINTF_DATA( float )
ECL_KW_FPRINTF_DATA( double )
#undef ECL_KW_FPRINTF_DATA

static void ecl_kw_fprintf_data_bool( const ecl_kw_type * ecl_kw , const char * fmt , FILE * stream) {
  const int * data = (const int *) ecl_kw->data;
  int i;
  for (i=0; i < ecl_kw->size; i++) {
    if (data[i] == ECL_BOOL_TRUE_INT)
      fprintf(stream , fmt , 1);
    else
      fprintf(stream , fmt , 0);
  }
}


static void ecl_kw_fprintf_data_string( const ecl_kw_type * ecl_kw , const char * fmt , FILE * stream) {
  int i;
  for (i=0; i < ecl_kw->size; i++)
    fprintf(stream , fmt , &ecl_kw->data[ i * ecl_kw_get_sizeof_ctype(ecl_kw)]);
}


void ecl_kw_fprintf_data( const ecl_kw_type * ecl_kw , const char * fmt , FILE * stream) {
  if (ecl_type_is_double(ecl_kw->data_type))
    ecl_kw_fprintf_data_double( ecl_kw , fmt , stream );
  else if (ecl_type_is_float(ecl_kw->data_type))
    ecl_kw_fprintf_data_float( ecl_kw , fmt , stream );
  else if (ecl_type_is_int(ecl_kw->data_type))
    ecl_kw_fprintf_data_int( ecl_kw , fmt , stream );
  else if (ecl_type_is_bool(ecl_kw->data_type))
    ecl_kw_fprintf_data_bool( ecl_kw , fmt , stream );
  else if (ecl_type_is_char(ecl_kw->data_type) || ecl_type_is_string(ecl_kw->data_type))
    ecl_kw_fprintf_data_string( ecl_kw , fmt , stream );
}



static bool ecl_kw_elm_equal_numeric__( const ecl_kw_type * ecl_kw1 , const ecl_kw_type * ecl_kw2 , int offset, double abs_epsilon, double rel_epsilon) {
  double v1 = ecl_kw_iget_as_double( ecl_kw1 , offset );
  double v2 = ecl_kw_iget_as_double( ecl_kw2 , offset );
  return util_double_approx_equal__( v1, v2 , rel_epsilon , abs_epsilon );
}


static bool ecl_kw_elm_equal__( const ecl_kw_type * ecl_kw1 , const ecl_kw_type * ecl_kw2 , int offset) {
  size_t data_offset = ecl_kw_get_sizeof_ctype(ecl_kw1) * offset;
  int cmp = memcmp( &ecl_kw1->data[ data_offset ] , &ecl_kw2->data[ data_offset ] , ecl_kw_get_sizeof_ctype(ecl_kw1));
  if (cmp == 0)
    return true;
  else
    return false;
}


int ecl_kw_first_different( const ecl_kw_type * ecl_kw1 , const ecl_kw_type * ecl_kw2 , int offset, double abs_epsilon, double rel_epsilon) {
  if (!ecl_kw_size_and_type_equal( ecl_kw1 , ecl_kw2))
    util_abort("%s: sorry invalid comparison\n",__func__);


  if (offset >= ecl_kw_get_size( ecl_kw1 ))
    util_abort("%s: sorry - invalid offset value\n",__func__);

  {
    bool numeric_compare = false;

    if (((abs_epsilon > 0) || (rel_epsilon > 0)) && ((ecl_kw_get_type( ecl_kw1 ) == ECL_FLOAT_TYPE) || (ecl_kw_get_type( ecl_kw1 ) == ECL_DOUBLE_TYPE)))
      numeric_compare = true;
    {
      int index = offset;

      while (true) {
        bool equal = (numeric_compare) ? ecl_kw_elm_equal_numeric__( ecl_kw1 , ecl_kw2, index , abs_epsilon, rel_epsilon) : ecl_kw_elm_equal__( ecl_kw1 , ecl_kw2 , index );
        if (!equal)
          break;

        index++;
        if (index == ecl_kw_get_size( ecl_kw1 ))
          break;
      }

      return index;
    }
  }
}

#include "ecl_kw_functions.c"
