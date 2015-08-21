/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_file.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <time.h>

#include <ert/util/hash.h>
#include <ert/util/util.h>
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_rsthead.h>
#include <ert/ecl/ecl_file_kw.h>


/**
   This file implements functionality to load an ECLIPSE file in
   ecl_kw format. The implementation works by first searching through
   the complete file to create an index over all the keywords present
   in the file. The actual keyword data is not loaded before they are
   explicitly requested.

   The ecl_file_type is the middle layer of abstraction in the libecl
   hierarchy (see the file overview.txt in this directory); it works
   with a collection of ecl_kw instances and has various query
   functions, however it does not utilize knowledge of the
   structure/content of the files in the way e.g. ecl_grid.c does[1].

   The main datatype here is the ecl_file type, but in addition each
   ecl_kw instance is wrapped in an ecl_file_kw (implemented in
   ecl_file_kw.c) structure and all the indexing is implemented with
   the file_map type. The file_map type is not used outside this file.

   When the file is opened an index of all the keywords is created and
   stored in the field global_map, and the field active_map is set to
   point to global_map, i.e. all query/get operations on the ecl_file
   will be based on the complete index:

   In many cases (in particular for unified restart files) it is quite
   painful to work with this large and unvieldy index, and it is
   convenient to create a sub index based on a subset of the
   keywords. The creation of these sub indices is based on identifying
   a keyword from name and occurence number, and then including all
   keywords up to the next occurence of the same keyword:

      SEQHDR            ---\
      MINISTEP  0          |
      PARAMS    .....      |
      MINISTEP  1          |   Block 0
      PARAMS    .....      |
      MINISTEP  2          |
      PARAMS    .....      |
      SEQHDR            ---+
      MINISTEP  3          |
      PARAMS    .....      |
      MINISTEP  4          |   Block 1
      PARAMS    .....      |
      MINISTEP  5          |
      SEQHDR            ---+
      MINISTEP  6          |   Block 2
      PARAMS    ....       |
      SEQHDR            ---+
      MINISTEP  7          |
      PARAMS    ....       |   Block 3
      MINISTEP  8          |
      PARAMS    ....       |

   For the unified summary file depicted here e.g. the call

      ecl_file_get_blockmap( ecl_file , "SEQHDR" , 2 )

   Will create a sub-index consisting of the (three) keywords in what
   is called 'Block 2' in the figure above. In particular for restart
   files this abstraction is very convenient, because an extra layer
   of functionality is required to get from natural time coordinates
   (i.e. simulation time or report step) to the occurence number (see
   ecl_rstfile for more details).

   To select a subindex as the active index you use the
   ecl_file_select_block() function, or alternatively you can use
   ecl_file_open_block() to directly select the relevant block
   immediately after the open() statement. Observe that when using a
   sub index thorugh ecl_file_select_block() function the global_map
   will still be present in the ecl_file instance, and subsequent
   calls to create a new sub index will also use the global_map index
   - i.e. the indexing is not recursive, a sub index is always created
   based on the global_map, and not on the currently active map.


   [1]: This is not entirely true - in the file ecl_rstfile.c; which
        is included from this file are several specialized function
        for working with restart files. However the restart files are
        still treated as collections of ecl_kw instances, and not
        internalized as in e.g. ecl_sum.
*/



#define ECL_FILE_ID 776107



typedef struct file_map_struct file_map_type;
struct file_map_struct {
  vector_type       * kw_list;      /* This is a vector of ecl_file_kw instances corresponding to the content of the file. */
  hash_type         * kw_index;     /* A hash table with integer vectors of indices - see comment below. */
  stringlist_type   * distinct_kw;  /* A stringlist of the keywords occuring in the file - each string occurs ONLY ONCE. */
  fortio_type       * fortio;       /* The same fortio instance pointer as in the ecl_file styructure. */
  bool                owner;        /* Is this map the owner of the ecl_file_kw instances; only true for the global_map. */
  inv_map_type     *  inv_map;       /* Shared reference owned by the ecl_file structure. */
  int               * flags;
};


struct ecl_file_struct {
  UTIL_TYPE_ID_DECLARATION;
  fortio_type       * fortio;       /* The source of all the keywords - must be retained
                                       open for reading for the entire lifetime of the
                                       ecl_file object. */
  file_map_type * global_map;       /* The index of all the ecl_kw instances in the file. */
  file_map_type * active_map;       /* The currently active index. */
  vector_type   * map_list;         /* Storage container for the map instances. */
  bool            read_only;
  int             flags;
  vector_type   * map_stack;
  inv_map_type  * inv_map;
};


/*
  This illustrates the indexing. The ecl_file instance contains in
  total 7 ecl_kw instances, the global index [0...6] is the internal
  way to access the various keywords. The kw_index is a hash table
  with entries 'SEQHDR', 'MINISTEP' and 'PARAMS'. Each entry in the
  hash table is an integer vector which again contains the internal
  index of the various occurences:

   ------------------
   SEQHDR            \
   MINISTEP  0        |
   PARAMS    .....    |
   MINISTEP  1        |
   PARAMS    .....    |
   MINISTEP  2        |
   PARAMS    .....   /
   ------------------

   kw_index    = {"SEQHDR": [0], "MINISTEP": [1,3,5], "PARAMS": [2,4,6]}    <== This is hash table.
   kw_list     = [SEQHDR , MINISTEP , PARAMS , MINISTEP , PARAMS , MINISTEP , PARAMS]
   distinct_kw = [SEQHDR , MINISTEP , PARAMS]

*/


/*****************************************************************/
/* Here comes the functions related to the index file_map. These
   functions are all of them static.
*/


static bool FILE_FLAGS_SET( int state_flags , int query_flags) {
  if ((state_flags & query_flags) == query_flags)
    return true;
  else
    return false;
}



static file_map_type * file_map_alloc( fortio_type * fortio , int * flags , inv_map_type * inv_map , bool owner ) {
  file_map_type * file_map     = util_malloc( sizeof * file_map );
  file_map->kw_list            = vector_alloc_new();
  file_map->kw_index           = hash_alloc();
  file_map->distinct_kw        = stringlist_alloc_new();
  file_map->owner              = owner;
  file_map->fortio             = fortio;
  file_map->inv_map            = inv_map;
  file_map->flags          = flags;
  return file_map;
}

static int file_map_get_global_index( const file_map_type * file_map , const char * kw , int ith) {
  const int_vector_type * index_vector = hash_get(file_map->kw_index , kw);
  int global_index = int_vector_iget( index_vector , ith);
  return global_index;
}



/**
   This function iterates over the kw_list vector and builds the
   internal index fields 'kw_index' and 'distinct_kw'. This function
   must be called every time the content of the kw_list vector is
   modified (otherwise the ecl_file instance will be in an
   inconsistent state).
*/


static void file_map_make_index( file_map_type * file_map ) {
  stringlist_clear( file_map->distinct_kw );
  hash_clear( file_map->kw_index );
  {
    int i;
    for (i=0; i < vector_get_size( file_map->kw_list ); i++) {
      const ecl_file_kw_type * file_kw = vector_iget_const( file_map->kw_list , i);
      const char             * header  = ecl_file_kw_get_header( file_kw );
      if ( !hash_has_key( file_map->kw_index , header )) {
        int_vector_type * index_vector = int_vector_alloc( 0 , -1 );
        hash_insert_hash_owned_ref( file_map->kw_index , header , index_vector , int_vector_free__);
        stringlist_append_copy( file_map->distinct_kw , header);
      }

      {
        int_vector_type * index_vector = hash_get( file_map->kw_index , header);
        int_vector_append( index_vector , i);
      }
    }
  }
}

static bool file_map_has_kw( const file_map_type * file_map, const char * kw) {
  return hash_has_key( file_map->kw_index , kw );
}


static ecl_file_kw_type * file_map_iget_file_kw( const file_map_type * file_map , int global_index) {
  ecl_file_kw_type * file_kw = vector_iget( file_map->kw_list , global_index);
  return file_kw;
}

static ecl_file_kw_type * file_map_iget_named_file_kw( const file_map_type * file_map , const char * kw, int ith) {
  int global_index = file_map_get_global_index( file_map , kw , ith);
  ecl_file_kw_type * file_kw = file_map_iget_file_kw( file_map , global_index );
  return file_kw;
}



static ecl_kw_type * file_map_iget_kw( const file_map_type * file_map , int index) {
  ecl_file_kw_type * file_kw = file_map_iget_file_kw( file_map , index );
  ecl_kw_type * ecl_kw = ecl_file_kw_get_kw_ptr( file_kw , file_map->fortio , file_map->inv_map);
  if (!ecl_kw) {
    if (fortio_assert_stream_open( file_map->fortio )) {

      ecl_kw = ecl_file_kw_get_kw( file_kw , file_map->fortio , file_map->inv_map);

      if (FILE_FLAGS_SET( file_map->flags[0] , ECL_FILE_CLOSE_STREAM))
        fortio_fclose_stream( file_map->fortio );
    }
  }
  return ecl_kw;
}

static void file_map_index_fload_kw(const file_map_type * file_map, const char* kw, int index, const int_vector_type * index_map, char* buffer) {
    ecl_file_kw_type * file_kw = file_map_iget_named_file_kw( file_map , kw , index);

    if (fortio_assert_stream_open( file_map->fortio )) {
        offset_type offset = ecl_file_kw_get_offset(file_kw);
        ecl_type_enum ecl_type = ecl_file_kw_get_type(file_kw);
        int element_count = ecl_file_kw_get_size(file_kw);

        ecl_kw_fread_indexed_data(file_map->fortio, offset + ECL_KW_HEADER_FORTIO_SIZE, ecl_type, element_count, index_map, buffer);
    }
}


static int file_map_find_kw_value( const file_map_type * file_map , const char * kw , const void * value) {
  int global_index = -1;
  if ( file_map_has_kw( file_map , kw)) {
    const int_vector_type * index_list = hash_get( file_map->kw_index , kw );
    int index = 0;
    while (index < int_vector_size( index_list )) {
      const ecl_kw_type * ecl_kw = file_map_iget_kw( file_map , int_vector_iget( index_list , index ));
      if (ecl_kw_data_equal( ecl_kw , value )) {
        global_index = int_vector_iget( index_list , index );
        break;
      }
      index++;
    }
  }
  return global_index;
}

static const char * file_map_iget_distinct_kw( const file_map_type * file_map , int index) {
  return stringlist_iget( file_map->distinct_kw , index);
}

static int file_map_get_num_distinct_kw( const file_map_type * file_map ) {
  return stringlist_get_size( file_map->distinct_kw );
}

static int file_map_get_size( const file_map_type * file_map ) {
  return vector_get_size( file_map->kw_list );
}


static ecl_type_enum file_map_iget_type( const file_map_type * file_map , int index) {
  ecl_file_kw_type * file_kw = file_map_iget_file_kw( file_map , index );
  return ecl_file_kw_get_type( file_kw );
}

static int file_map_iget_size( const file_map_type * file_map , int index) {
  ecl_file_kw_type * file_kw = file_map_iget_file_kw( file_map , index );
  return ecl_file_kw_get_size( file_kw );
}

static const char * file_map_iget_header( const file_map_type * file_map , int index) {
  ecl_file_kw_type * file_kw = file_map_iget_file_kw( file_map , index );
  return ecl_file_kw_get_header( file_kw );
}


static ecl_kw_type * file_map_iget_named_kw( const file_map_type * file_map , const char * kw, int ith) {
  ecl_file_kw_type * file_kw = file_map_iget_named_file_kw( file_map , kw , ith);
  ecl_kw_type * ecl_kw = ecl_file_kw_get_kw_ptr( file_kw , file_map->fortio , file_map->inv_map );
  if (!ecl_kw) {
    if (fortio_assert_stream_open( file_map->fortio )) {

      ecl_kw = ecl_file_kw_get_kw( file_kw , file_map->fortio , file_map->inv_map);

      if (FILE_FLAGS_SET( file_map->flags[0] , ECL_FILE_CLOSE_STREAM))
        fortio_fclose_stream( file_map->fortio );
    }
  }
  return ecl_kw;
}

static ecl_type_enum file_map_iget_named_type( const file_map_type * file_map , const char * kw , int ith) {
  ecl_file_kw_type * file_kw = file_map_iget_named_file_kw( file_map , kw, ith);
  return ecl_file_kw_get_type( file_kw );
}

static int file_map_iget_named_size( const file_map_type * file_map , const char * kw , int ith) {
  ecl_file_kw_type * file_kw = file_map_iget_named_file_kw( file_map , kw , ith );
  return ecl_file_kw_get_size( file_kw );
}


static void file_map_replace_kw( file_map_type * file_map , ecl_kw_type * old_kw , ecl_kw_type * new_kw , bool insert_copy) {
  int index = 0;
  while (index < vector_get_size( file_map->kw_list )) {
    ecl_file_kw_type * ikw = vector_iget( file_map->kw_list , index );
    if (ecl_file_kw_ptr_eq( ikw , old_kw)) {
      /*
         Found it; observe that the vector_iset() function will
         automatically invoke the destructor on the old_kw.
      */
      ecl_kw_type * insert_kw = new_kw;

      if (insert_copy)
        insert_kw = ecl_kw_alloc_copy( new_kw );
      ecl_file_kw_replace_kw( ikw , file_map->fortio , insert_kw );

      file_map_make_index( file_map );
      return;
    }
    index++;
  }
  util_abort("%s: could not find ecl_kw ptr: %p \n",__func__ , old_kw);
}


static bool file_map_load_all( file_map_type * file_map ) {
  bool loadOK = false;

  if (fortio_assert_stream_open( file_map->fortio )) {
    int index;
    for (index = 0; index < vector_get_size( file_map->kw_list); index++) {
      ecl_file_kw_type * ikw = vector_iget( file_map->kw_list , index );
      ecl_file_kw_get_kw( ikw , file_map->fortio , file_map->inv_map);
    }
    loadOK = true;
  }

  if (FILE_FLAGS_SET( file_map->flags[0] , ECL_FILE_CLOSE_STREAM))
    fortio_fclose_stream( file_map->fortio );

  return loadOK;
}


/*****************************************************************/



static void file_map_add_kw( file_map_type * file_map , ecl_file_kw_type * file_kw) {
  if (file_map->owner)
    vector_append_owned_ref( file_map->kw_list , file_kw , ecl_file_kw_free__ );
  else
    vector_append_ref( file_map->kw_list , file_kw);
}

static void file_map_free( file_map_type * file_map ) {
  hash_free( file_map->kw_index );
  stringlist_free( file_map->distinct_kw );
  vector_free( file_map->kw_list );
  free( file_map );
}

static void file_map_free__( void * arg ) {
  file_map_type * file_map = ( file_map_type * ) arg;
  file_map_free( file_map );
}


static int file_map_get_num_named_kw(const file_map_type * file_map , const char * kw) {
  if (hash_has_key(file_map->kw_index , kw)) {
    const int_vector_type * index_vector = hash_get(file_map->kw_index , kw);
    return int_vector_size( index_vector );
  } else
    return 0;
}

static void file_map_fwrite( const file_map_type * file_map , fortio_type * target , int offset) {
  int index;
  for (index = offset; index < vector_get_size( file_map->kw_list ); index++) {
    ecl_kw_type * ecl_kw = file_map_iget_kw( file_map , index );
    ecl_kw_fwrite( ecl_kw , target );
  }
}




static int file_map_iget_occurence( const file_map_type * file_map , int global_index) {
  const ecl_file_kw_type * file_kw = vector_iget_const( file_map->kw_list , global_index);
  const char * header              = ecl_file_kw_get_header( file_kw );
  const int_vector_type * index_vector = hash_get( file_map->kw_index , header );
  const int * index_data = int_vector_get_const_ptr( index_vector );

  int occurence = -1;
  {
    /* Manual reverse lookup. */
    int i;
    for (i=0; i < int_vector_size( index_vector ); i++)
      if (index_data[i] == global_index)
        occurence = i;
  }
  if (occurence < 0)
    util_abort("%s: internal error ... \n" , __func__);

  return occurence;
}

static void file_map_fprintf_kw_list(const file_map_type * file_map , FILE * stream) {
  int i;
  for (i=0; i < vector_get_size( file_map->kw_list ); i++) {
    const ecl_file_kw_type * file_kw = vector_iget_const( file_map->kw_list , i );
    fprintf(stream , "%-8s %7d:%s\n",
            ecl_file_kw_get_header( file_kw ) ,
            ecl_file_kw_get_size( file_kw ) ,
            ecl_util_get_type_name( ecl_file_kw_get_type( file_kw )));
  }
}

/**
   Will return NULL if the block which is asked for is not present.
*/
static file_map_type * file_map_alloc_blockmap(const file_map_type * file_map , const char * header, int occurence) {
  if (file_map_get_num_named_kw( file_map , header ) > occurence) {
    file_map_type * block_map = file_map_alloc( file_map->fortio , file_map->flags , file_map->inv_map , false);
    if (file_map_has_kw( file_map , header )) {
      int kw_index = file_map_get_global_index( file_map , header , occurence );
      ecl_file_kw_type * file_kw = vector_iget( file_map->kw_list , kw_index );

      while (true) {
        file_map_add_kw( block_map , file_kw );

        kw_index++;
        if (kw_index == vector_get_size( file_map->kw_list ))
          break;
        else {
          file_kw = vector_iget(file_map->kw_list , kw_index);
          if (strcmp( header , ecl_file_kw_get_header( file_kw )) == 0)
            break;
        }
      }
    }
    file_map_make_index( block_map );
    return block_map;
  } else
    return NULL;
}


/*****************************************************************/
/* Here comes the implementation of the ecl_file proper 'class'. */


UTIL_SAFE_CAST_FUNCTION( ecl_file , ECL_FILE_ID)
UTIL_IS_INSTANCE_FUNCTION( ecl_file , ECL_FILE_ID)



ecl_file_type * ecl_file_alloc_empty( int flags ) {
  ecl_file_type * ecl_file = util_malloc( sizeof * ecl_file );
  UTIL_TYPE_ID_INIT(ecl_file , ECL_FILE_ID);
  ecl_file->map_list  = vector_alloc_new();
  ecl_file->map_stack = vector_alloc_new();
  ecl_file->inv_map   = inv_map_alloc( );
  ecl_file->flags     = flags;
  return ecl_file;
}


/*****************************************************************/
/* fwrite functions */

void ecl_file_fwrite_fortio(const ecl_file_type * ecl_file , fortio_type * target, int offset) {
  file_map_fwrite( ecl_file->active_map , target , offset );
}



/*
   Observe : if the filename is a standard filename which can be used
   to infer formatted/unformatted automagically the fmt_file variable
   is NOT consulted.
*/

void ecl_file_fwrite(const ecl_file_type * ecl_file , const char * filename, bool fmt_file) {
  bool __fmt_file;
  ecl_file_enum file_type;

  file_type = ecl_util_get_file_type( filename , &__fmt_file , NULL);
  if (file_type == ECL_OTHER_FILE)
    __fmt_file = fmt_file;

  {
    fortio_type * target = fortio_open_writer( filename , __fmt_file , ECL_ENDIAN_FLIP);
    ecl_file_fwrite_fortio( ecl_file , target , 0);
    fortio_fclose( target );
  }
}









/*****************************************************************/
/**
   Here comes several functions for querying the ecl_file instance, and
   getting pointers to the ecl_kw content of the ecl_file. For getting
   ecl_kw instances there are two principally different access methods:

   * ecl_file_iget_named_kw(): This function will take a keyword
   (char *) and an integer as input. The integer corresponds to the
   ith occurence of the keyword in the file.

   * ecl_file_iget_kw(): This function just takes an integer index as
   input, and returns the corresponding ecl_kw instance - without
   considering which keyword it is.

   -------

   In addition the functions ecl_file_get_num_distinct_kw() and
   ecl_file_iget_distinct_kw() will return the number of distinct
   keywords, and distinct keyword keyword nr i (as a const char *).


   Possible usage pattern:

   ....
   for (ikw = 0; ikw < ecl_file_get_num_distinct_kw(ecl_file); ikw++) {
   const char * kw = ecl_file_iget_distinct_kw(ecl_file , ikw);

   printf("The file contains: %d occurences of \'%s\' \n",ecl_file_get_num_named_kw( ecl_file , kw) , kw);
   }
   ....

   For the summary file showed in the top this code will produce:

   The file contains 1 occurences of 'SEQHDR'
   The file contains 3 occurences of 'MINISTEP'
   The file contains 3 occurences of 'PARAMS'

*/









/**
   This function will iterate through the ecl_file instance and search
   for the ecl_kw instance @old_kw - the search is based on pointer
   equality, i.e. the actual ecl_kw instance, and not on content
   equality.

   When @old_kw is found that keyword instance will be discarded with
   ecl_kw_free() and the new keyword @new_kw will be inserted. If
   @old_kw can not be found the function will fail hard - to verify
   that @new_kw is indeed in the ecl_file instance you should use
   ecl_file_has_kw_ptr() first.

   The ecl_file function typically gives out references to the
   internal ecl_kw instances via the ecl_file_iget_kw() function. Use
   of ecl_file_replace_kw() might lead to invalidating ecl_kw
   instances held by the calling scope:


   ....
   ecl_file_type * restart_file   = ecl_file_fread_alloc( "ECLIPSE.UNRST" );
   ecl_kw_type * initial_pressure = ecl_file_iget_named_kw( ecl_file , "PRESSURE" , 0);
   ecl_kw_type * faked_pressure   = ecl_kw_alloc_copy( initial_pressure );

   ecl_kw_scale( faked_pressure , 1.25 );
   ecl_file_replace_kw( restart_file , initial_pressure , faked_pressure , false );  <--- This call will invalidate the inital_pressure reference
   ....
   ....
   // This will fail horribly:
   printf("The initial pressure in cell(0) was:%g \n",ecl_kw_iget_double( initial_pressure , 0 ));
   /|\
   |
   +---------> Using initial_pressure => Crash and burn!

   The ecl_file structure takes ownership of all the keywords, and
   will also take ownership of the newly instered @new_kw instance; if
   the boolean @insert_copy is set to true the function will insert a
   copy of @new_kw, leaving the original reference untouched.
*/



void ecl_file_replace_kw( ecl_file_type * ecl_file , ecl_kw_type * old_kw , ecl_kw_type * new_kw , bool insert_copy) {
  file_map_replace_kw( ecl_file->active_map , old_kw , new_kw , insert_copy );
}





ecl_kw_type * ecl_file_icopy_named_kw( const ecl_file_type * ecl_file , const char * kw, int ith) {
  return ecl_kw_alloc_copy( ecl_file_iget_named_kw( ecl_file , kw , ith ));
}


/*
  Will return the number of times a particular keyword occurs in a
  ecl_file instance. Will return 0 if the keyword can not be found.
*/

int ecl_file_get_num_named_kw(const ecl_file_type * ecl_file , const char * kw) {
  return file_map_get_num_named_kw( ecl_file->active_map , kw);
}






/**
   This function does the following:

   1. Takes an input index which goes in to the global kw_list vector.
   2. Looks up the corresponding keyword.
   3. Return the number of this particular keyword instance, among
   the other instance with the same header.

   With the example above we get:

   ecl_file_iget_occurence(ecl_file , 2) -> 0; Global index 2 will
   look up the first occurence of PARAMS.

   ecl_file_iget_occurence(ecl_file , 5) -> 2; Global index 5 will
   look up th third occurence of MINISTEP.

   The enkf layer uses this funny functionality.
*/


int ecl_file_iget_occurence( const ecl_file_type * ecl_file , int index) {
  return file_map_iget_occurence( ecl_file->active_map , index );
}


/**
    Returns the total number of ecl_kw instances in the ecl_file
    instance.
*/
int ecl_file_get_size( const ecl_file_type * ecl_file ){
  return file_map_get_size( ecl_file->active_map );
}


/**
   Returns true if the ecl_file instance has at-least one occurence of
   ecl_kw 'kw'.
*/
bool ecl_file_has_kw( const ecl_file_type * ecl_file , const char * kw) {
  return file_map_has_kw( ecl_file->active_map , kw );
}


int ecl_file_get_num_distinct_kw(const ecl_file_type * ecl_file) {
  return file_map_get_num_distinct_kw( ecl_file->active_map );
}


const char * ecl_file_iget_distinct_kw(const ecl_file_type * ecl_file, int index) {
  return file_map_iget_distinct_kw( ecl_file->active_map , index );
}


const char * ecl_file_get_src_file( const ecl_file_type * ecl_file ) {
  return fortio_filename_ref( ecl_file->fortio );
}


void ecl_file_fprintf_kw_list( const ecl_file_type * ecl_file , FILE * stream ) {
  file_map_fprintf_kw_list( ecl_file->active_map , stream );
}

#ifdef HAVE_FORK
const char * ecl_file_enum_iget( int index , int * value) {
  return util_enum_iget( index , ECL_FILE_ENUM_SIZE , (const util_enum_element_type []) { ECL_FILE_ENUM_DEFS } , value);
}
#endif

/*****************************************************************/

ecl_file_kw_type * ecl_file_iget_file_kw( const ecl_file_type * file , int global_index) {
  return file_map_iget_file_kw( file->active_map , global_index);
}

ecl_file_kw_type * ecl_file_iget_named_file_kw( const ecl_file_type * file , const char * kw, int ith) {
  return file_map_iget_named_file_kw( file->active_map , kw, ith);
}

/* ---- */

ecl_kw_type * ecl_file_iget_kw( const ecl_file_type * file , int global_index) {
  return file_map_iget_kw( file->active_map , global_index);
}

ecl_type_enum ecl_file_iget_type( const ecl_file_type * file , int global_index) {
  return file_map_iget_type( file->active_map , global_index);
}

int ecl_file_iget_size( const ecl_file_type * file , int global_index) {
  return file_map_iget_size( file->active_map , global_index);
}

const char * ecl_file_iget_header( const ecl_file_type * file , int global_index) {
  return file_map_iget_header( file->active_map , global_index);
}

/* ---------- */

/*
   This function will return the ith occurence of 'kw' in
   ecl_file. Will abort hard if the request can not be satisifed - use
   query functions if you can not take that.
*/

ecl_kw_type * ecl_file_iget_named_kw( const ecl_file_type * file , const char * kw, int ith) {
  return file_map_iget_named_kw( file->active_map , kw , ith);
}

void ecl_file_indexed_read(const ecl_file_type * file , const char * kw, int index, const int_vector_type * index_map, char* buffer) {
    file_map_index_fload_kw(file->active_map, kw, index, index_map, buffer);
}

ecl_type_enum ecl_file_iget_named_type( const ecl_file_type * file , const char * kw , int ith) {
  return file_map_iget_named_type( file->active_map , kw , ith );
}

int ecl_file_iget_named_size( const ecl_file_type * file , const char * kw , int ith) {
  return file_map_iget_named_size( file->active_map , kw , ith );
}



/*****************************************************************/

static void ecl_file_add_map( ecl_file_type * ecl_file , file_map_type * file_map) {
  vector_append_owned_ref(ecl_file->map_list , file_map , file_map_free__ );
}


static file_map_type * ecl_file_get_blockmap( ecl_file_type * ecl_file , const char * kw , int occurence , bool use_global) {
  file_map_type * blockmap;
  if (use_global)
    blockmap = file_map_alloc_blockmap( ecl_file->global_map , kw , occurence );
  else
    blockmap = file_map_alloc_blockmap( ecl_file->active_map , kw , occurence );

  if (blockmap != NULL)
    ecl_file_add_map( ecl_file , blockmap );
  return blockmap;
}


/*****************************************************************/
/*
  Different functions to open and close a file.
*/

/**
   The ecl_file_scan() function will scan through the whole file and
   build up an index of all the kewyords. The map created from this
   scan will be stored under the 'global_map' field; and all
   subsequent lookup operations will ultimately be based on the global
   map.
*/

static bool ecl_file_scan( ecl_file_type * ecl_file ) {
  bool scan_ok = false;
  fortio_fseek( ecl_file->fortio , 0 , SEEK_SET );
  {
    ecl_kw_type * work_kw = ecl_kw_alloc_new("WORK-KW" , 0 , ECL_INT_TYPE , NULL);

    while (true) {
      if (fortio_read_at_eof(ecl_file->fortio)) {
        scan_ok = true;
        break;
      }

      {
        offset_type current_offset = fortio_ftell( ecl_file->fortio );
        if (ecl_kw_fread_header( work_kw , ecl_file->fortio)) {
          ecl_file_kw_type * file_kw = ecl_file_kw_alloc( work_kw , current_offset);
          if (ecl_file_kw_fskip_data( file_kw , ecl_file->fortio ))
            file_map_add_kw( ecl_file->global_map , file_kw );
          else
            break;
        } else
          break;
      }
    }

    ecl_kw_free( work_kw );
  }
  if (scan_ok)
    file_map_make_index( ecl_file->global_map );

  return scan_ok;
}


void ecl_file_select_global( ecl_file_type * ecl_file ) {
  ecl_file->active_map = ecl_file->global_map;
}


/**
   The fundamental open file function; all alternative open()
   functions start by calling this one. This function will read
   through the complete file, extract all the keyword headers and
   create the map/index stored in the global_map field of the ecl_file
   structure. No keyword data will be loaded from the file.

   The ecl_file instance will retain an open fortio reference to the
   file until ecl_file_close() is called.
*/


ecl_file_type * ecl_file_open( const char * filename , int flags) {
  fortio_type * fortio;
  bool          fmt_file;

  ecl_util_fmt_file( filename , &fmt_file);
  //flags |= ECL_FILE_CLOSE_STREAM;   // DEBUG DEBUG DEBUG

  if (FILE_FLAGS_SET(flags , ECL_FILE_WRITABLE))
    fortio = fortio_open_readwrite( filename , fmt_file , ECL_ENDIAN_FLIP);
  else
    fortio = fortio_open_reader( filename , fmt_file , ECL_ENDIAN_FLIP);

  if (fortio) {
    ecl_file_type * ecl_file = ecl_file_alloc_empty( flags );
    ecl_file->fortio = fortio;
    ecl_file->global_map = file_map_alloc( ecl_file->fortio , &ecl_file->flags , ecl_file->inv_map , true );

    ecl_file_add_map( ecl_file , ecl_file->global_map );
    if (ecl_file_scan( ecl_file )) {
      ecl_file_select_global( ecl_file );

      if (FILE_FLAGS_SET( ecl_file->flags , ECL_FILE_CLOSE_STREAM))
        fortio_fclose_stream( ecl_file->fortio );

      return ecl_file;
    } else {
      ecl_file_close( ecl_file );
      return NULL;
    }
  } else
    return NULL;
}





int ecl_file_get_flags( const ecl_file_type * ecl_file ) {
  return ecl_file->flags;
}

void ecl_file_set_flags( ecl_file_type * ecl_file, int flags ) {
  ecl_file->flags = flags;
}

bool ecl_file_flags_set( const ecl_file_type * ecl_file , int flags) {
  return FILE_FLAGS_SET( ecl_file->flags , flags );
}

bool ecl_file_writable( const ecl_file_type * ecl_file ) {
  return FILE_FLAGS_SET( ecl_file->flags , ECL_FILE_WRITABLE );
}


void ecl_file_push_block( ecl_file_type * ecl_file ) {
  vector_append_ref( ecl_file->map_stack , ecl_file->active_map );
}

void ecl_file_pop_block( ecl_file_type * ecl_file ) {
  ecl_file->active_map = vector_pop_back( ecl_file->map_stack );
}


bool ecl_file_subselect_block( ecl_file_type * ecl_file , const char * kw , int occurence) {
  file_map_type * blockmap = ecl_file_get_blockmap( ecl_file , kw , occurence , false);
  if (blockmap != NULL) {
    ecl_file->active_map = blockmap;
    return true;
  } else
    return false;
}


bool ecl_file_select_block( ecl_file_type * ecl_file , const char * kw , int occurence ) {
  file_map_type * blockmap = ecl_file_get_blockmap( ecl_file , kw , occurence , true);
  if (blockmap != NULL) {
    ecl_file->active_map = blockmap;
    return true;
  } else
    return false;
}


/**
   The ecl_file_close() function will close the fortio instance and
   free all the data created by the ecl_file instance; this includes
   the ecl_kw instances which have been loaded on demand.
*/

void ecl_file_close(ecl_file_type * ecl_file) {
  if (ecl_file->fortio != NULL)
    fortio_fclose( ecl_file->fortio  );

  inv_map_free( ecl_file->inv_map );
  vector_free( ecl_file->map_list  );
  vector_free( ecl_file->map_stack );
  free( ecl_file );
}


void ecl_file_close_fortio_stream(ecl_file_type * ecl_file) {
    if (ecl_file->fortio != NULL) {
        fortio_fclose_stream(ecl_file->fortio);
    }
}


/**
   This function will detach the current ecl_file instance from the
   underlying fortio instance. The ecl_file instance can be used
   further to access the ecl_kw instances which have been loaded
   already, but if you try on-demand loading of a keyword you will get
   crash-and-burn. To ensure that all keywords are in memory you can
   call ecl_file_load_all() prior to the detach call.
*/


void ecl_file_fortio_detach( ecl_file_type * ecl_file ) {
  fortio_fclose( ecl_file->fortio );
  ecl_file->fortio = NULL;
}


bool ecl_file_load_all( ecl_file_type * ecl_file ) {
  return file_map_load_all( ecl_file->active_map );
}


void ecl_file_free__(void * arg) {
  ecl_file_close( ecl_file_safe_cast( arg ) );
}


/****************************************************************************/
/* Here we include two files with functions specialized to work with
   restart and summary files. Observe that the files ecl_rstfile.c and
   ecl_smryfile are compiled as part of the same compilation unit as
   ecl_file.c
*/

#include "ecl_rstfile.c"
#include "ecl_smryfile.c"

/*****************************************************************/
/* Two small lookup functions which consider the INTEHEAD keyword,
   work equally well for both restart and INIT files. */

/*
  The input @file must be either an INIT file or a restart file. Will
  fail hard if an INTEHEAD kw can not be found - or if the INTEHEAD
  keyword is not sufficiently large.

  The eclipse files can distinguish between ECLIPSE300 ( value == 300)
  and ECLIPSE300-Thermal option (value == 500). This function will
  return ECLIPSE300 in both those cases.
*/

ecl_version_enum ecl_file_get_ecl_version( const ecl_file_type * file ) {
  ecl_kw_type * intehead_kw = ecl_file_iget_named_kw( file , INTEHEAD_KW , 0 );
  int int_value = ecl_kw_iget_int( intehead_kw , INTEHEAD_IPROG_INDEX );

  if (int_value == INTEHEAD_ECLIPSE100_VALUE)
    return ECLIPSE100;
  else if ((int_value == INTEHEAD_ECLIPSE300_VALUE) || (int_value == INTEHEAD_ECLIPSE300THERMAL_VALUE))
    return ECLIPSE300;
  else {
    util_abort("%s: ECLIPSE version value:%d not recognized \n",__func__ , int_value );
    return -1;
  }
}

/*
  1: Oil
  2: Water
  3: Oil + water
  4: Gas
  5: Gas + Oil
  6: Gas + water
  7: Gas + Water + Oil
*/

int ecl_file_get_phases( const ecl_file_type * init_file ) {
  ecl_kw_type * intehead_kw = ecl_file_iget_named_kw( init_file , INTEHEAD_KW , 0 );
  int phases = ecl_kw_iget_int( intehead_kw , INTEHEAD_PHASE_INDEX );
  return phases;
}


/*
bool ecl_file_writable( const ecl_file_type * ecl_file ) {
  return fortio_writable( ecl_file->fortio );
}
*/

/**
   Checks if the ecl_file contains ecl_kw; this check is based on
   pointer equality - i.e. we check if the ecl_file contains exactly
   this keyword - not an arbitrary equivalent keyword.

   This function can be called as a safeguard before calling
   ecl_file_save_kw().
*/

bool ecl_file_has_kw_ptr( const ecl_file_type * ecl_file , const ecl_kw_type * ecl_kw) {
  ecl_file_kw_type * file_kw = inv_map_get_file_kw( ecl_file->inv_map , ecl_kw );
  if (file_kw == NULL)
    return false;
  else
    return true;
}


/*
  Will save the content of @ecl_kw to the on-disk file wrapped by the
  ecl_file instance. This function is quite strict:

  1. The actual keyword which should be updated is identified using
     pointer comparison; i.e. the ecl_kw argument must be the actual
     return value from an earlier ecl_file_get_kw() operation. To
     check this you can call ecl_file_has_kw_ptr().

  2. The header data of the ecl_kw must be unmodified; this is checked
     by the ecl_file_kw_inplace_fwrite() function and crash-and-burn
     will ensue if this is not satisfied.

  3. The ecl_file must have been opened with one of the _writable()
     open functions.
*/

bool ecl_file_save_kw( const ecl_file_type * ecl_file , const ecl_kw_type * ecl_kw) {
  ecl_file_kw_type * file_kw = inv_map_get_file_kw( ecl_file->inv_map , ecl_kw );  // We just verify that the input ecl_kw points to an ecl_kw
  if (file_kw != NULL) {                                                           // we manage; from then on we use the reference contained in
    if (fortio_assert_stream_open( ecl_file->fortio )) {                           // the corresponding ecl_file_kw instance.

      ecl_file_kw_inplace_fwrite( file_kw , ecl_file->fortio );

      if (FILE_FLAGS_SET( ecl_file->flags , ECL_FILE_CLOSE_STREAM))
        fortio_fclose_stream( ecl_file->fortio );

      return true;
    } else
      return false;
  } else {
    util_abort("%s: keyword pointer:%p not found in ecl_file instance. \n",__func__ , ecl_kw);
    return false;
  }
}

/* Small function to support Python based enum introspection. */

#ifdef HAVE_FORK

const char * ecl_util_file_flags_enum_iget( int index , int * value) {
  return util_enum_iget( index , ECL_FILE_FLAGS_ENUM_SIZE , (const util_enum_element_type []) { ECL_FILE_FLAGS_ENUM_DEFS }, value);
}

#endif


