/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_file_kw.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/size_t_vector.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_kw.h>
#include <ert/ecl/ecl_file_kw.h>
#include <ert/ecl/fortio.h>

/*
  This file implements the datatype ecl_file_kw which is used to hold
  header-information about an ecl_kw instance on file. When a
  ecl_file_kw instance is created it is initialized with the header
  information (name, size, type) for an ecl_kw instance and the offset
  in a file actually containing the keyword.

  If and when the keyword is actually queried for at a later stage the
  ecl_file_kw_get_kw() method will seek to the keyword position in an
  open fortio instance and call ecl_kw_fread_alloc() to instantiate
  the keyword itself.

  The ecl_file_kw datatype is mainly used by the ecl_file datatype;
  whose index tables consists of ecl_file_kw instances.
*/


#define ECL_FILE_KW_TYPE_ID 646107

struct inv_map_struct {
  size_t_vector_type * file_kw_ptr;
  size_t_vector_type * ecl_kw_ptr;
  bool                 sorted;
};

struct ecl_file_kw_struct {
  UTIL_TYPE_ID_DECLARATION;
  offset_type      file_offset;
  ecl_data_type    data_type;
  int              kw_size;
  char           * header;
  ecl_kw_type    * kw;
};



/*****************************************************************/

inv_map_type * inv_map_alloc() {
  inv_map_type * map = util_malloc( sizeof * map );
  map->file_kw_ptr = size_t_vector_alloc( 0 , 0 );
  map->ecl_kw_ptr  = size_t_vector_alloc( 0 , 0 );
  map->sorted = false;
  return map;
}

void inv_map_free( inv_map_type * map ) {
  size_t_vector_free( map->file_kw_ptr );
  size_t_vector_free( map->ecl_kw_ptr );
  free( map );
}


static void inv_map_assert_sort( inv_map_type * map ) {
  if (!map->sorted) {
    perm_vector_type * perm = size_t_vector_alloc_sort_perm( map->ecl_kw_ptr );

    size_t_vector_permute( map->ecl_kw_ptr , perm );
    size_t_vector_permute( map->file_kw_ptr , perm );
    map->sorted = true;

    free( perm );
  }
}

static void inv_map_drop_kw( inv_map_type * map , const ecl_kw_type * ecl_kw) {
  inv_map_assert_sort( map );
  {
    int index = size_t_vector_index_sorted( map->ecl_kw_ptr , (size_t) ecl_kw );
    if (index == -1)
      util_abort("%s: trying to drop non-existent kw \n",__func__);

    size_t_vector_idel( map->ecl_kw_ptr  , index );
    size_t_vector_idel( map->file_kw_ptr , index );
    map->sorted = false;
  }
}


static void inv_map_add_kw( inv_map_type * map , const ecl_file_kw_type * file_kw , const ecl_kw_type * ecl_kw) {
  size_t_vector_append( map->file_kw_ptr , (size_t) file_kw );
  size_t_vector_append( map->ecl_kw_ptr  , (size_t) ecl_kw );
  map->sorted = false;
}


ecl_file_kw_type * inv_map_get_file_kw( inv_map_type * inv_map , const ecl_kw_type * ecl_kw ) {
  inv_map_assert_sort( inv_map );
  {
    int index = size_t_vector_index_sorted( inv_map->ecl_kw_ptr , (size_t) ecl_kw );
    if (index == -1)
      /* ecl_kw ptr not found. */
      return NULL;
    else
      return (ecl_file_kw_type * ) size_t_vector_iget( inv_map->file_kw_ptr , index );
  }
}


/*****************************************************************/

static UTIL_SAFE_CAST_FUNCTION( ecl_file_kw , ECL_FILE_KW_TYPE_ID )
UTIL_IS_INSTANCE_FUNCTION( ecl_file_kw , ECL_FILE_KW_TYPE_ID )



static ecl_file_kw_type * ecl_file_kw_alloc__( const char * header , ecl_data_type data_type , int size , offset_type offset) {
  ecl_file_kw_type * file_kw = util_malloc( sizeof * file_kw );
  UTIL_TYPE_ID_INIT( file_kw , ECL_FILE_KW_TYPE_ID );

  file_kw->header = util_alloc_string_copy( header );
  memcpy(&file_kw->data_type, &data_type, sizeof data_type);
  file_kw->kw_size = size;
  file_kw->file_offset = offset;
  file_kw->kw = NULL;

  return file_kw;
}

/**
   Create a new ecl_file_kw instance based on header information from
   the input keyword. Typically only the header has been loaded from
   the keyword.

   Observe that it is the users responsability that the @offset
   argument in ecl_file_kw_alloc() comes from the same fortio instance
   as used when calling ecl_file_kw_get_kw() to actually instatiate
   the ecl_kw. This is automatically assured when using ecl_file to
   access the ecl_file_kw instances.
*/

ecl_file_kw_type * ecl_file_kw_alloc( const ecl_kw_type * ecl_kw , offset_type offset ) {
  return ecl_file_kw_alloc__( ecl_kw_get_header( ecl_kw ) , ecl_kw_get_data_type( ecl_kw ) , ecl_kw_get_size( ecl_kw ) , offset );
}


/**
    Does NOT copy the kw pointer which must be reloaded.
*/
ecl_file_kw_type * ecl_file_kw_alloc_copy( const ecl_file_kw_type * src ) {
  return ecl_file_kw_alloc__( src->header , ecl_file_kw_get_data_type(src) , src->kw_size , src->file_offset );
}




void ecl_file_kw_free( ecl_file_kw_type * file_kw ) {
  if (file_kw->kw != NULL) {
    ecl_kw_free( file_kw->kw );
    file_kw->kw = NULL;
  }
  free( file_kw->header );
  free( file_kw );
}


void ecl_file_kw_free__( void * arg ) {
  ecl_file_kw_type * file_kw = ecl_file_kw_safe_cast( arg );
  ecl_file_kw_free( file_kw );
}



static void ecl_file_kw_assert_kw( const ecl_file_kw_type * file_kw ) {
  if(!ecl_type_is_equal(
              ecl_file_kw_get_data_type(file_kw),
              ecl_kw_get_data_type(file_kw->kw)
              ))
    util_abort("%s: type mismatch between header and file.\n",__func__);

  if (file_kw->kw_size != ecl_kw_get_size( file_kw->kw ))
    util_abort("%s: size mismatch between header and file.\n",__func__);

  if (strcmp( file_kw->header , ecl_kw_get_header( file_kw->kw )) != 0 )
    util_abort("%s: name mismatch between header and file.\n",__func__);
}


static void ecl_file_kw_drop_kw( ecl_file_kw_type * file_kw , inv_map_type * inv_map ) {
  if (file_kw->kw != NULL) {
    inv_map_drop_kw( inv_map , file_kw->kw );
    ecl_kw_free( file_kw->kw );
    file_kw->kw = NULL;
  }
}


static void ecl_file_kw_load_kw( ecl_file_kw_type * file_kw , fortio_type * fortio , inv_map_type * inv_map) {
  if (fortio == NULL)
    util_abort("%s: trying to load a keyword after the backing file has been detached.\n",__func__);

  if (file_kw->kw != NULL)
    ecl_file_kw_drop_kw( file_kw , inv_map );

  {
    fortio_fseek( fortio , file_kw->file_offset , SEEK_SET );
    file_kw->kw = ecl_kw_fread_alloc( fortio );
    ecl_file_kw_assert_kw( file_kw );
    inv_map_add_kw( inv_map , file_kw , file_kw->kw );
  }
}

ecl_kw_type * ecl_file_kw_get_kw_ptr( ecl_file_kw_type * file_kw , fortio_type * fortio , inv_map_type * inv_map ) {
  return file_kw->kw;
}

/*
  Will return the ecl_kw instance of this file_kw; if it is not
  currently loaded the method will instantiate the ecl_kw instance
  from the @fortio input handle.

  After loading the keyword it will be kept in memory, so a possible
  subsequent lookup will be served from memory.

  The ecl_file layer maintains a pointer mapping between the
  ecl_kw_type pointers and their ecl_file_kw_type containers; this
  mapping needs the new_load return value from the
  ecl_file_kw_get_kw() function.
*/


ecl_kw_type * ecl_file_kw_get_kw( ecl_file_kw_type * file_kw , fortio_type * fortio , inv_map_type * inv_map ) {
  if (file_kw->kw == NULL)
    ecl_file_kw_load_kw( file_kw , fortio , inv_map);

  return file_kw->kw;
}


bool ecl_file_kw_ptr_eq( const ecl_file_kw_type * file_kw , const ecl_kw_type * ecl_kw) {
  if (file_kw->kw == ecl_kw)
    return true;
  else
    return false;
}



void ecl_file_kw_replace_kw( ecl_file_kw_type * file_kw , fortio_type * target , ecl_kw_type * new_kw ) {
  if (!ecl_type_is_equal(
              ecl_file_kw_get_data_type(file_kw),
              ecl_kw_get_data_type(new_kw)
              ))
    util_abort("%s: sorry type mismatch between in-file keyword and new keyword \n",__func__);
  if((file_kw->kw_size == ecl_kw_get_size( new_kw )))
    util_abort("%s: sorry size mismatch between in-file keyword and new keyword \n",__func__);

  if (file_kw->kw != NULL)
    ecl_kw_free( file_kw->kw );

  file_kw->kw = new_kw;
  fortio_fseek( target , file_kw->file_offset , SEEK_SET );
  ecl_kw_fwrite( file_kw->kw , target );
}



const char * ecl_file_kw_get_header( const ecl_file_kw_type * file_kw ) {
  return file_kw->header;
}

int ecl_file_kw_get_size( const ecl_file_kw_type * file_kw ) {
  return file_kw->kw_size;
}

ecl_data_type ecl_file_kw_get_data_type(const ecl_file_kw_type * file_kw) {
  return file_kw->data_type;
}

offset_type ecl_file_kw_get_offset(const ecl_file_kw_type * file_kw) {
    return file_kw->file_offset;
}

bool ecl_file_kw_fskip_data( const ecl_file_kw_type * file_kw , fortio_type * fortio) {
  return ecl_kw_fskip_data__( ecl_file_kw_get_data_type(file_kw) , file_kw->kw_size , fortio );
}


/**
   This function will replace the file content of the keyword pointed
   to by @file_kw, with the new content given by @ecl_kw. The new
   @ecl_kw keyword must have identical header to the one already
   present in the file.
*/

void ecl_file_kw_inplace_fwrite( ecl_file_kw_type * file_kw , fortio_type * fortio) {
  ecl_file_kw_assert_kw( file_kw );
  fortio_fseek( fortio , file_kw->file_offset , SEEK_SET );
  ecl_kw_fskip_header( fortio );
  ecl_kw_fwrite_data( file_kw->kw , fortio );
}



