/*
   Copyright (C) 2011  Statoil ASA, Norway.

   The file 'ecl_rft_file.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef HAVE_FNMATCH
#include <fnmatch.h>
#endif

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/vector.h>
#include <ert/util/int_vector.h>

#include <ert/ecl/ecl_rft_file.h>
#include <ert/ecl/ecl_rft_node.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_endian_flip.h>
#include <ert/ecl/ecl_kw_magic.h>


/**
   This data structure is for loading one eclipse RFT file. One RFT
   file can in general contain RFT information from:

     o Many different times
     o Many different wells

   All of this is just lumped together in one long vector, both in the
   file, and in this implementation. The data for one specific RFT
   (one well, one time) is internalized in the ecl_rft_node type.
*/



#define ECL_RFT_FILE_ID 6610632

struct ecl_rft_file_struct {
  UTIL_TYPE_ID_DECLARATION;
  char        * filename;
  vector_type * data;          /* This vector just contains all the rft nodes in one long vector. */
  hash_type   * well_index;    /* This indexes well names into the data vector - very similar to the scheme used in ecl_file. */
};



static ecl_rft_file_type * ecl_rft_file_alloc_empty(const char * filename) {
  ecl_rft_file_type * rft_vector = util_malloc(sizeof * rft_vector );
  UTIL_TYPE_ID_INIT( rft_vector , ECL_RFT_FILE_ID );
  rft_vector->data       = vector_alloc_new();
  rft_vector->filename   = util_alloc_string_copy(filename);
  rft_vector->well_index = hash_alloc();
  return rft_vector;
}

/**
   Generating the two functions:

   bool                ecl_rft_file_is_instance( void * );
   ecl_rft_file_type * ecl_rft_file_safe_cast( void * );
*/

UTIL_SAFE_CAST_FUNCTION( ecl_rft_file , ECL_RFT_FILE_ID );
UTIL_IS_INSTANCE_FUNCTION( ecl_rft_file , ECL_RFT_FILE_ID );


static void ecl_rft_file_add_node(ecl_rft_file_type * rft_vector , const ecl_rft_node_type * rft_node) {
  vector_append_owned_ref( rft_vector->data , rft_node , ecl_rft_node_free__);
}




ecl_rft_file_type * ecl_rft_file_alloc(const char * filename) {
  ecl_rft_file_type * rft_vector = ecl_rft_file_alloc_empty( filename );
  ecl_file_type * ecl_file       = ecl_file_open( filename , 0);
  int global_index = 0;
  int block_nr = 0;

  while (true) {
    ecl_file_view_type * rft_view = ecl_file_alloc_global_blockview(ecl_file, TIME_KW, block_nr);

    if (rft_view) {
      ecl_rft_node_type * rft_node = ecl_rft_node_alloc( rft_view );
      if (rft_node != NULL) {
        const char * well_name = ecl_rft_node_get_well_name( rft_node );
        ecl_rft_file_add_node(rft_vector , rft_node);
        if (!hash_has_key( rft_vector->well_index , well_name))
          hash_insert_hash_owned_ref( rft_vector->well_index , well_name , int_vector_alloc( 0 , 0 ) , int_vector_free__);
        {
          int_vector_type * index_list = hash_get( rft_vector->well_index , well_name );
          int_vector_append(index_list , global_index);
        }
        global_index++;
      }
    } else
      break;

    block_nr++;
    ecl_file_view_free( rft_view );
  }
  ecl_file_close( ecl_file );
  return rft_vector;
}


/**
   Will look for .RFT / .FRFT files very similar to the
   ecl_grid_load_case(). Will return NULL if no RFT file can be found,
   and the name of RFT file if it is found. New storage is allocated
   for the new name.

*/

char * ecl_rft_file_alloc_case_filename(const char * case_input ) {
  ecl_file_enum    file_type;
  bool             fmt_file;
  file_type = ecl_util_get_file_type( case_input , &fmt_file ,  NULL);
  if (file_type == ECL_RFT_FILE)
    return util_alloc_string_copy (case_input );
  else {
    char * return_file = NULL;
    char * path;
    char * basename;
    util_alloc_file_components( case_input , &path , &basename , NULL);
    if ((file_type == ECL_OTHER_FILE) || (file_type == ECL_DATA_FILE)) {      /* Impossible to infer formatted/unformatted from the case_input */
      char * RFT_file  = ecl_util_alloc_filename( path , basename , ECL_RFT_FILE , false , -1 );
      char * FRFT_file = ecl_util_alloc_filename( path , basename , ECL_RFT_FILE , true  , -1 );

      if (util_file_exists( RFT_file ))
        return_file = util_alloc_string_copy( RFT_file );
      else if (util_file_exists( FRFT_file ))
        return_file = util_alloc_string_copy( FRFT_file );

      free( RFT_file );
      free( FRFT_file );
    } else {
      char * RFT_file  = ecl_util_alloc_filename( path , basename , ECL_RFT_FILE , fmt_file , -1 );

      if (util_file_exists( RFT_file ))
        return_file = util_alloc_string_copy( RFT_file );

      free( RFT_file );
    }
    return return_file;
  }
}



ecl_rft_file_type * ecl_rft_file_alloc_case( const char * case_input ) {
  ecl_rft_file_type * ecl_rft_file = NULL;
  char * file_name = ecl_rft_file_alloc_case_filename( case_input );

  if (file_name != NULL) {
    ecl_rft_file = ecl_rft_file_alloc( file_name );
    free( file_name );
  }
  return ecl_rft_file;
}



bool ecl_rft_file_case_has_rft( const char * case_input ) {
  bool has_rft = false;
  char * file_name = ecl_rft_file_alloc_case_filename( case_input );

  if (file_name != NULL) {
    has_rft = true;
    free( file_name );
  }

  return has_rft;
}



void ecl_rft_file_free(ecl_rft_file_type * rft_vector) {
  vector_free(rft_vector->data);
  hash_free( rft_vector->well_index );
  free(rft_vector->filename);
  free(rft_vector);
}


void ecl_rft_file_free__(void * arg) {
  ecl_rft_file_free( ecl_rft_file_safe_cast( arg ));
}


/**
    Will return the number of RFT nodes in the file. If @well != NULL
    only wells matching @well be included. The @well variable can
    contain '*', so the function call

          ecl_rft_file_get_size__( rft_file , "OP*" , -1)

    will count the number of rft instances with a well name matching
    well "OP*".

    If recording_time >= only rft_nodes with recording time ==
    @recording_time are included.
*/


int ecl_rft_file_get_size__( const ecl_rft_file_type * rft_file, const char * well_pattern , time_t recording_time) {
  if ((well_pattern == NULL) && (recording_time < 0))
    return vector_get_size( rft_file->data );
  else {
    int match_count = 0;
    int i;
    for ( i=0; i < vector_get_size( rft_file->data ); i++) {
      const ecl_rft_node_type * rft = vector_iget_const( rft_file->data , i);

      if (well_pattern) {
        if (util_fnmatch( well_pattern , ecl_rft_node_get_well_name( rft )) != 0)
          continue;
      }

      /*OK - we either do not care about the well, or alternatively the well matches. */
      if (recording_time >= 0) {
        if (recording_time != ecl_rft_node_get_date( rft ))
          continue;
      }
      match_count++;
    }
    return match_count;
  }
}


/**
   Returns the total number of rft nodes in the file, not caring if
   the same well occurse many times and so on.
*/

int ecl_rft_file_get_size( const ecl_rft_file_type * rft_file) {
  return ecl_rft_file_get_size__( rft_file , NULL , -1 );
}




const char * ecl_rft_file_get_filename( const ecl_rft_file_type * rft_file ) {
  return rft_file->filename;
}


/**
   Return rft_node number 'i' in the rft_file - not caring when this
   particular RFT is from, or which well it is.

   If you ask for an index which is beyond the size of the vector it will
   go up in flames - use ecl_file_get_size() first if you can not
   handle that.
*/

ecl_rft_node_type * ecl_rft_file_iget_node( const ecl_rft_file_type * rft_file , int index) {
  return vector_iget( rft_file->data , index );
}



/**
   This function will return ecl_rft_node nr index - for well
   'well'. I.e. for an RFT file which looks like this:

   RFT - Well P1: 01/01/2000
   RFT - Well P2: 01/01/2000
   RFT - WEll P1: 01/01/2001
   RFT - Well P2: 01/01/2001   <--
   RFT - Well P1: 01/01/2002
   RFT - Well P2: 01/01/2002

   The function call:

      ecl_rft_iget_well_rft(rft_file , "P2" , 1)

   will return the rft node indicated by the arrow (i.e. the second
   occurence of well "P2" in the file.)

   If the rft_file does not have the well, or that occurence, the
   function will go down in flames with util_abort(). Use
   ecl_rft_file_has_well() and ecl_rft_file_get_well_occurences()
   first if you can not take util_abort().
*/



ecl_rft_node_type * ecl_rft_file_iget_well_rft( const ecl_rft_file_type * rft_file , const char * well, int index) {
  const int_vector_type * index_vector = hash_get(rft_file->well_index , well);
  return ecl_rft_file_iget_node( rft_file , int_vector_iget(index_vector , index));
}


static int ecl_rft_file_get_node_index_time_rft( const ecl_rft_file_type * rft_file , const char * well , time_t recording_time) {
  int global_index = -1;
  if (hash_has_key( rft_file->well_index , well)) {
    const int_vector_type * index_vector = hash_get(rft_file->well_index , well);
    int well_index = 0;
    while (true) {
      if (well_index == int_vector_size( index_vector ))
        break;

      {
        const ecl_rft_node_type * node = ecl_rft_file_iget_node( rft_file , int_vector_iget( index_vector , well_index ));
        if (ecl_rft_node_get_date( node ) == recording_time) {
          global_index = int_vector_iget( index_vector , well_index );
          break;
        }
      }

      well_index++;
    }
  }
  return global_index;
}


/**
   Returns an rft_node for well 'well' and time 'recording_time'. If
   the rft can not be found, either due to "wrong" well name, or "wrong"
   time; the function will return NULL.
*/


ecl_rft_node_type * ecl_rft_file_get_well_time_rft( const ecl_rft_file_type * rft_file , const char * well , time_t recording_time) {
  int index = ecl_rft_file_get_node_index_time_rft(rft_file, well, recording_time);
  if (index !=-1) {
    return ecl_rft_file_iget_node(rft_file, index);
  } else{
    return NULL;
  }
}




bool ecl_rft_file_has_well( const ecl_rft_file_type * rft_file , const char * well) {
  return hash_has_key(rft_file->well_index , well);
}


/**
   Returns the number of occurences of 'well' in rft_file.
*/

int ecl_rft_file_get_well_occurences( const ecl_rft_file_type * rft_file , const char * well) {
  const int_vector_type * index_vector = hash_get(rft_file->well_index , well);
  return int_vector_size( index_vector );
}


/**
   Returns the number of distinct wells in RFT file.
*/
int ecl_rft_file_get_num_wells( const ecl_rft_file_type * rft_file ) {
  return hash_get_size( rft_file->well_index );
}



stringlist_type * ecl_rft_file_alloc_well_list(const ecl_rft_file_type * rft_file ) {
  return hash_alloc_stringlist( rft_file->well_index );
}



void ecl_rft_file_update(const char * rft_file_name, ecl_rft_node_type ** nodes,int num_nodes, ert_ecl_unit_enum unit_set){
    ecl_rft_file_type * rft_file;

    if(util_file_exists(rft_file_name)){
      int node_index;
      rft_file = ecl_rft_file_alloc( rft_file_name );
      for(node_index = 0; node_index < num_nodes; node_index++) {
        ecl_rft_node_type * new_node = nodes[node_index];
        int storage_index = ecl_rft_file_get_node_index_time_rft(rft_file, ecl_rft_node_get_well_name(new_node), ecl_rft_node_get_date(new_node));
        if (storage_index == -1) {
          ecl_rft_file_add_node(rft_file, new_node);
        } else {
          vector_iset_owned_ref(rft_file->data, storage_index, new_node,ecl_rft_node_free__);
        }
      }
    }else{
      int node_index;
      rft_file = ecl_rft_file_alloc_empty( rft_file_name );
      for(node_index = 0; node_index < num_nodes; node_index++) {
        ecl_rft_file_add_node(rft_file, nodes[node_index]);
      }
    }

    {
      bool fmt_file = false;
      fortio_type * fortio = fortio_open_writer( rft_file_name , fmt_file , ECL_ENDIAN_FLIP );
      int node_index;

      /**
         The sorting here works directly on the internal node storage
         rft_file->data; that might in principle ruin the indexing of
         the ecl_file object - it is therefore absolutely essential
         that this ecl_rft_file object does not live beyond this
         function, and also that the ecl_rft_file api functions are
         avoided for the rest of this function.
      */

      vector_sort(rft_file->data,(vector_cmp_ftype *) ecl_rft_node_cmp);
      for(node_index=0; node_index < vector_get_size( rft_file->data ); node_index++) {
        const ecl_rft_node_type *new_node = vector_iget_const(rft_file->data, node_index);
        ecl_rft_node_fwrite(new_node, fortio, unit_set);
      }

      fortio_fclose( fortio );
    }
    ecl_rft_file_free(rft_file);
}





