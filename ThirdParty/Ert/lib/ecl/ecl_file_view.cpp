/*
   Copyright (C) 2016  Statoil ASA, Norway.

   The file 'ecl_file_view.c' is part of ERT - Ensemble based Reservoir Tool.

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


#include <vector>
#include <string>

#include <ert/util/vector.hpp>
#include <ert/util/hash.hpp>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>
#include <ert/ecl/ecl_file_kw.hpp>
#include <ert/ecl/ecl_file_view.hpp>
#include <ert/ecl/ecl_rsthead.hpp>
#include <ert/ecl/ecl_type.hpp>


struct ecl_file_view_struct {
  vector_type       * kw_list;      /* This is a vector of ecl_file_kw instances corresponding to the content of the file. */
  hash_type         * kw_index;     /* A hash table with integer vectors of indices - see comment below. */
  std::vector<std::string> distinct_kw;  /* A list of the keywords occuring in the file - each string occurs ONLY ONCE. */
  fortio_type       * fortio;       /* The same fortio instance pointer as in the ecl_file styructure. */
  bool                owner;        /* Is this map the owner of the ecl_file_kw instances; only true for the global_map. */
  inv_map_type      * inv_map;      /* Shared reference owned by the ecl_file structure. */
  vector_type       * child_list;
  int               * flags;
};

struct ecl_file_transaction_struct {
  const ecl_file_view_type * file_view;
  int * ref_count;
};


/*****************************************************************/
/* Here comes the functions related to the index ecl_file_view. These
   functions are all of them static.
*/

bool ecl_file_view_check_flags( int state_flags , int query_flags) {
  if ((state_flags & query_flags) == query_flags)
    return true;
  else
    return false;
}


bool ecl_file_view_flags_set( const ecl_file_view_type * file_view , int query_flags) {
  return ecl_file_view_check_flags( *file_view->flags , query_flags );
}


const char * ecl_file_view_get_src_file( const ecl_file_view_type * file_view ) {
  return fortio_filename_ref( file_view->fortio );
}


ecl_file_view_type * ecl_file_view_alloc( fortio_type * fortio , int * flags , inv_map_type * inv_map , bool owner ) {
  ecl_file_view_type * ecl_file_view  = new ecl_file_view_type();

  ecl_file_view->kw_list              = vector_alloc_new();
  ecl_file_view->kw_index             = hash_alloc();
  ecl_file_view->child_list           = vector_alloc_new();
  ecl_file_view->owner                = owner;
  ecl_file_view->fortio               = fortio;
  ecl_file_view->inv_map              = inv_map;
  ecl_file_view->flags                = flags;

  return ecl_file_view;
}

int ecl_file_view_get_global_index( const ecl_file_view_type * ecl_file_view , const char * kw , int ith) {
  const int_vector_type * index_vector = (const int_vector_type*)hash_get(ecl_file_view->kw_index , kw);
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


void ecl_file_view_make_index( ecl_file_view_type * ecl_file_view ) {
  ecl_file_view->distinct_kw.clear();
  hash_clear( ecl_file_view->kw_index );
  {
    int i;
    for (i=0; i < vector_get_size( ecl_file_view->kw_list ); i++) {
      const ecl_file_kw_type * file_kw = (const ecl_file_kw_type*)vector_iget_const( ecl_file_view->kw_list , i);
      const char             * header  = ecl_file_kw_get_header( file_kw );
      if ( !hash_has_key( ecl_file_view->kw_index , header )) {
        int_vector_type * index_vector = int_vector_alloc( 0 , -1 );
        hash_insert_hash_owned_ref( ecl_file_view->kw_index , header , index_vector , int_vector_free__);
        ecl_file_view->distinct_kw.push_back(header);
      }

      {
        int_vector_type * index_vector = (int_vector_type*)hash_get( ecl_file_view->kw_index , header);
        int_vector_append( index_vector , i);
      }
    }
  }
}

bool ecl_file_view_has_kw( const ecl_file_view_type * ecl_file_view, const char * kw) {
  return hash_has_key( ecl_file_view->kw_index , kw );
}


ecl_file_kw_type * ecl_file_view_iget_file_kw( const ecl_file_view_type * ecl_file_view , int global_index) {
  ecl_file_kw_type * file_kw = (ecl_file_kw_type*)vector_iget( ecl_file_view->kw_list , global_index);
  return file_kw;
}

ecl_file_kw_type * ecl_file_view_iget_named_file_kw( const ecl_file_view_type * ecl_file_view , const char * kw, int ith) {
  int global_index = ecl_file_view_get_global_index( ecl_file_view , kw , ith);
  ecl_file_kw_type * file_kw = ecl_file_view_iget_file_kw( ecl_file_view , global_index );
  return file_kw;
}

bool ecl_file_view_drop_flag( ecl_file_view_type * file_view , int flag)  {
  bool flag_set = ecl_file_view_flags_set( file_view , flag );
  if (flag_set)
    *file_view->flags -= flag;

  return flag_set;
}

void ecl_file_view_add_flag( ecl_file_view_type * file_view , int flag)  {
  *file_view->flags |= flag;
}

static ecl_kw_type * ecl_file_view_get_kw(const ecl_file_view_type * ecl_file_view, ecl_file_kw_type * file_kw) {
  ecl_kw_type * ecl_kw = ecl_file_kw_get_kw_ptr( file_kw );
  if (!ecl_kw) {
    if (fortio_assert_stream_open( ecl_file_view->fortio )) {

      ecl_kw = ecl_file_kw_get_kw( file_kw , ecl_file_view->fortio , ecl_file_view->inv_map);

      if (ecl_file_view_flags_set( ecl_file_view , ECL_FILE_CLOSE_STREAM))
        fortio_fclose_stream( ecl_file_view->fortio );
    }
  }
  return ecl_kw;
}

ecl_kw_type * ecl_file_view_iget_kw( const ecl_file_view_type * ecl_file_view , int index) {
  ecl_file_kw_type * file_kw = ecl_file_view_iget_file_kw( ecl_file_view , index );
  return ecl_file_view_get_kw(ecl_file_view, file_kw);
}

void ecl_file_view_index_fload_kw(const ecl_file_view_type * ecl_file_view, const char* kw, int index, const int_vector_type * index_map, char* io_buffer) {
    ecl_file_kw_type * file_kw = ecl_file_view_iget_named_file_kw( ecl_file_view , kw , index);

    if (fortio_assert_stream_open( ecl_file_view->fortio )) {
        offset_type offset = ecl_file_kw_get_offset(file_kw);
        ecl_data_type data_type = ecl_file_kw_get_data_type(file_kw);
        int element_count = ecl_file_kw_get_size(file_kw);

        ecl_kw_fread_indexed_data(ecl_file_view->fortio, offset + ECL_KW_HEADER_FORTIO_SIZE, data_type, element_count, index_map, io_buffer);
    }
}


int ecl_file_view_find_kw_value( const ecl_file_view_type * ecl_file_view , const char * kw , const void * value) {
  int global_index = -1;
  if ( ecl_file_view_has_kw( ecl_file_view , kw)) {
    const int_vector_type * index_list = (const int_vector_type*)hash_get( ecl_file_view->kw_index , kw );
    int index = 0;
    while (index < int_vector_size( index_list )) {
      const ecl_kw_type * ecl_kw = ecl_file_view_iget_kw( ecl_file_view , int_vector_iget( index_list , index ));
      if (ecl_kw_data_equal( ecl_kw , value )) {
        global_index = int_vector_iget( index_list , index );
        break;
      }
      index++;
    }
  }
  return global_index;
}

const char * ecl_file_view_iget_distinct_kw( const ecl_file_view_type * ecl_file_view , int index) {
  const std::string& string = ecl_file_view->distinct_kw[index];
  return string.c_str();
}

int ecl_file_view_get_num_distinct_kw( const ecl_file_view_type * ecl_file_view ) {
  return ecl_file_view->distinct_kw.size();
}

int ecl_file_view_get_size( const ecl_file_view_type * ecl_file_view ) {
  return vector_get_size( ecl_file_view->kw_list );
}


ecl_data_type ecl_file_view_iget_data_type( const ecl_file_view_type * ecl_file_view , int index) {
  ecl_file_kw_type * file_kw = ecl_file_view_iget_file_kw( ecl_file_view , index );
  return ecl_file_kw_get_data_type( file_kw );
}

int ecl_file_view_iget_size( const ecl_file_view_type * ecl_file_view , int index) {
  ecl_file_kw_type * file_kw = ecl_file_view_iget_file_kw( ecl_file_view , index );
  return ecl_file_kw_get_size( file_kw );
}

const char * ecl_file_view_iget_header( const ecl_file_view_type * ecl_file_view , int index) {
  ecl_file_kw_type * file_kw = ecl_file_view_iget_file_kw( ecl_file_view , index );
  return ecl_file_kw_get_header( file_kw );
}


ecl_kw_type * ecl_file_view_iget_named_kw( const ecl_file_view_type * ecl_file_view , const char * kw, int ith) {
  ecl_file_kw_type * file_kw = ecl_file_view_iget_named_file_kw( ecl_file_view , kw , ith);
  return ecl_file_view_get_kw(ecl_file_view, file_kw);
}

ecl_data_type ecl_file_view_iget_named_data_type( const ecl_file_view_type * ecl_file_view , const char * kw , int ith) {
  ecl_file_kw_type * file_kw = ecl_file_view_iget_named_file_kw( ecl_file_view , kw, ith);
  return ecl_file_kw_get_data_type( file_kw );
}

int ecl_file_view_iget_named_size( const ecl_file_view_type * ecl_file_view , const char * kw , int ith) {
  ecl_file_kw_type * file_kw = ecl_file_view_iget_named_file_kw( ecl_file_view , kw , ith );
  return ecl_file_kw_get_size( file_kw );
}


void ecl_file_view_replace_kw( ecl_file_view_type * ecl_file_view , ecl_kw_type * old_kw , ecl_kw_type * new_kw , bool insert_copy) {
  int index = 0;
  while (index < vector_get_size( ecl_file_view->kw_list )) {
    ecl_file_kw_type * ikw = (ecl_file_kw_type*)vector_iget( ecl_file_view->kw_list , index );
    if (ecl_file_kw_ptr_eq( ikw , old_kw)) {
      /*
         Found it; observe that the vector_iset() function will
         automatically invoke the destructor on the old_kw.
      */
      ecl_kw_type * insert_kw = new_kw;

      if (insert_copy)
        insert_kw = ecl_kw_alloc_copy( new_kw );
      ecl_file_kw_replace_kw( ikw , ecl_file_view->fortio , insert_kw );

      ecl_file_view_make_index( ecl_file_view );
      return;
    }
    index++;
  }
  util_abort("%s: could not find ecl_kw ptr: %p \n",__func__ , old_kw);
}


bool ecl_file_view_load_all( ecl_file_view_type * ecl_file_view ) {
  bool loadOK = false;

  if (fortio_assert_stream_open( ecl_file_view->fortio )) {
    int index;
    for (index = 0; index < vector_get_size( ecl_file_view->kw_list); index++) {
      ecl_file_kw_type * ikw = (ecl_file_kw_type*)vector_iget( ecl_file_view->kw_list , index );
      ecl_file_kw_get_kw( ikw , ecl_file_view->fortio , ecl_file_view->inv_map);
    }
    loadOK = true;
  }

  if (ecl_file_view_flags_set( ecl_file_view , ECL_FILE_CLOSE_STREAM))
    fortio_fclose_stream( ecl_file_view->fortio );

  return loadOK;
}


/*****************************************************************/



void ecl_file_view_add_kw( ecl_file_view_type * ecl_file_view , ecl_file_kw_type * file_kw) {
  if (ecl_file_view->owner)
    vector_append_owned_ref( ecl_file_view->kw_list , file_kw , ecl_file_kw_free__ );
  else
    vector_append_ref( ecl_file_view->kw_list , file_kw);
}

void ecl_file_view_free( ecl_file_view_type * ecl_file_view ) {
  vector_free( ecl_file_view->child_list );
  hash_free( ecl_file_view->kw_index );
  vector_free( ecl_file_view->kw_list );

  delete ecl_file_view;
}

void ecl_file_view_free__( void * arg ) {
  ecl_file_view_type * ecl_file_view = ( ecl_file_view_type * ) arg;
  ecl_file_view_free( ecl_file_view );
}


int ecl_file_view_get_num_named_kw(const ecl_file_view_type * ecl_file_view , const char * kw) {
  if (hash_has_key(ecl_file_view->kw_index , kw)) {
    const int_vector_type * index_vector = (const int_vector_type*)hash_get(ecl_file_view->kw_index , kw);
    return int_vector_size( index_vector );
  } else
    return 0;
}

void ecl_file_view_fwrite( const ecl_file_view_type * ecl_file_view , fortio_type * target , int offset) {
  int index;
  for (index = offset; index < vector_get_size( ecl_file_view->kw_list ); index++) {
    ecl_kw_type * ecl_kw = ecl_file_view_iget_kw( ecl_file_view , index );
    ecl_kw_fwrite( ecl_kw , target );
  }
}




int ecl_file_view_iget_occurence( const ecl_file_view_type * ecl_file_view , int global_index) {
  const ecl_file_kw_type * file_kw = (const ecl_file_kw_type*)vector_iget_const( ecl_file_view->kw_list , global_index);
  const char * header              = ecl_file_kw_get_header( file_kw );
  const int_vector_type * index_vector = (const int_vector_type*)hash_get( ecl_file_view->kw_index , header );
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

void ecl_file_view_fprintf_kw_list(const ecl_file_view_type * ecl_file_view , FILE * stream) {
  int i;
  for (i=0; i < vector_get_size( ecl_file_view->kw_list ); i++) {
    const ecl_file_kw_type * file_kw = (const ecl_file_kw_type*)vector_iget_const( ecl_file_view->kw_list , i );
    char * type_name = ecl_type_alloc_name(ecl_file_kw_get_data_type(file_kw));
    fprintf(stream , "%-8s %7d:%s\n",
            ecl_file_kw_get_header( file_kw ) ,
            ecl_file_kw_get_size( file_kw ) ,
            type_name);
    free(type_name);
  }
}


ecl_file_view_type * ecl_file_view_alloc_blockview2(const ecl_file_view_type * ecl_file_view , const char * start_kw, const char * end_kw, int occurence) {
  if ((start_kw != NULL) && ecl_file_view_get_num_named_kw( ecl_file_view , start_kw ) <= occurence)
    return NULL;


  ecl_file_view_type * block_map = ecl_file_view_alloc( ecl_file_view->fortio , ecl_file_view->flags , ecl_file_view->inv_map , false);
  int kw_index = 0;
  if (start_kw)
    kw_index = ecl_file_view_get_global_index( ecl_file_view , start_kw , occurence );

  {
    ecl_file_kw_type * file_kw = (ecl_file_kw_type*)vector_iget( ecl_file_view->kw_list , kw_index );
    while (true) {
      ecl_file_view_add_kw( block_map , file_kw );

      kw_index++;
      if (kw_index == vector_get_size( ecl_file_view->kw_list ))
        break;
      else {
        if (end_kw) {
          file_kw = (ecl_file_kw_type*)vector_iget(ecl_file_view->kw_list , kw_index);
          if (strcmp( end_kw , ecl_file_kw_get_header( file_kw )) == 0)
            break;
        }
      }
    }
  }
  ecl_file_view_make_index( block_map );
  return block_map;
}

/**
   Will return NULL if the block which is asked for is not present.
*/
ecl_file_view_type * ecl_file_view_alloc_blockview(const ecl_file_view_type * ecl_file_view , const char * header, int occurence) {
  return ecl_file_view_alloc_blockview2( ecl_file_view , header , header , occurence );
}


ecl_file_view_type * ecl_file_view_add_blockview(const ecl_file_view_type * file_view , const char * header, int occurence) {
  ecl_file_view_type * child  = ecl_file_view_alloc_blockview2(file_view, header, header, occurence);

  if (child)
    vector_append_owned_ref( file_view->child_list , child , ecl_file_view_free__ );

  return child;
}


ecl_file_view_type * ecl_file_view_add_blockview2(const ecl_file_view_type * ecl_file_view , const char * start_kw, const char * end_kw, int occurence) {
  ecl_file_view_type * child  = ecl_file_view_alloc_blockview2(ecl_file_view, start_kw , end_kw , occurence);

  if (child)
    vector_append_owned_ref( ecl_file_view->child_list , child , ecl_file_view_free__ );

  return child;
}



/*****************************************************************/
/*                   R E S T A R T   F I L E S                   */
/*****************************************************************/



/*
   There is no special datastructure for working with restart files,
   they are 100% stock ecl_file instances with the following limited
   structure:

   * They are organized in blocks; where each block starts with a
     SEQNUM keyword, which contains the report step.

   * Each block contains an INTEHEAD keyword, immediately after the
     SEQNUM keyword, which contains the true simulation date of of the
     block, and also some other data. Observe that also INIT files and
     GRID files contain an INTEHEAD keyword; and that for files with
     LGRs there is one INTEHEAD keyword for each LGR. This creates an
     extra level of mess.

   The natural time ordering when working with the file data is just
   the running index in the file; however from a user perspective the
   natural way to indicate time coordinate is through the report step
   or the true simulation time (i.e. 22.th of October 2009). This file
   is all about converting the natural input unit time and report_step
   to the internal indexing. This is achieved by consulting the value
   of the INTEHEAD and SEQNUM keywords respectively.
*/

/*
About the time-direction
========================

For the following discussion we will focus on the following simplified
unified restart file. The leading number is the global index of the
keyword, the value in [] corresponds to the relevant part of the
content of the keyword on the line, the labels A,B,C,D,E are used for
references in the text below.

 0 | SEQNUM   [0]           \  A
 1 | INTEHEAD [01.01.2005]  |
 2 | PRESSURE [... ]        |
 3 | SWAT     [...]         |
   | -----------------------+
 4 | SEQNUM   [5]           |  B
 5 | INTEHEAD [01.06.2005]  |
 6 | PRESSURE [... ]        |
 7 | SWAT     [...]         |
   |------------------------+
 8 | SEQNUM   [10]          |  C
 9 | INTEHEAD [01.12.2006]  |
10 | PRESSURE [...]         |
11 | SWAT     [...]         |
   |------------------------+
12 | SEQNUM   [20]          |  D
13 | INTEHEAD [01.12.2007]  |
14 | PRESSURE [...]         |
15 | SWAT     [...]         |
16 | OIL_DEN  [...]         |
   |------------------------+
17 | SEQNUM   [40]          |  E
18 | INTEHEAD [01.12.2009]  |
19 | PRESSURE [...]         |
20 | SWAT     [...]         /


This restart file has the following features:

 o It contains in total 16 keywords.

 o It contains 5 blocks of collected keywords corresponding to one
   time instant, each of these blocks is called a report_step,
   typcially coming from one DATES keyword in the ECLIPSE
   datafile. Observe that the file does not have the block structure
   visualized on this figure, the only thing separating the blocks in
   the file is the occurence of a SEQNUM keyword.

 o Only a few of the report steps are present, namely 0, 5, 10, 20 and
   40.

 o The different blocks are not equally long, the fourth block has an
   extra keyword OIL_DEN.

To adress these keywords and blocks using different time coordinates
we have introduced the following concepts:

 report_step: This corresponds to the value of the SEQNUM keword,
    i.e. to do queries based on the report_step we must load the
    seqnum kewyord and read the value.

        ecl_file_get_unrstmap_report_step( ecl_file , 0 ) => A
        ecl_file_get_unrstmap_report_step( ecl_file , 1 ) => NULL

        ecl_file_has_report_step( ecl_file , 5 ) => True
        ecl_file_has_report_step( ecl_file , 2 ) => False

 sim_time: This correpsonds to the true simulation time of the report
    step, the simulation time is stored as integers DAY, MONTH, YEAR
    in the INTEHEAD keyword; the function INTEHEAD_date() will extract
    the DAY, MONTH and YEAR values from an INTEHEAD keyword instance
    and convert to a time_t instance. The functions:

     ecl_file_get_unrstmap_sim_time() and ecl_file_has_has_sim_time()

    can be used to query for simulation times and get the
    corresponding block maps.

 index/global_index : This is typically the global running index of
    the keyword in the file; this is the unique address of the keyword
    which is used for the final lookup.

 occurence: The nth' time a particular keyword has occured in the
    file, i.e. the SEQNUM keyword in block C is the third occurence of
    SEQNUM. Instead of occurence xxxx_index is also used to indicate
    the occurence of keyword xxxx. The occurence number is the integer
    argument to the xxx_iget_named_kw() function, and also the final
    call to create blockmaps.

*/


bool ecl_file_view_has_report_step( const ecl_file_view_type * ecl_file_view , int report_step) {
  int global_index = ecl_file_view_find_kw_value( ecl_file_view , SEQNUM_KW , &report_step );
  if (global_index >= 0)
    return true;
  else
    return false;
}


time_t ecl_file_view_iget_restart_sim_date(const ecl_file_view_type * ecl_file_view , int seqnum_index) {
  time_t sim_time = -1;
  ecl_file_view_type * seqnum_map = ecl_file_view_alloc_blockview( ecl_file_view , SEQNUM_KW , seqnum_index);

  if (seqnum_map != NULL) {
    ecl_kw_type * intehead_kw = ecl_file_view_iget_named_kw( seqnum_map , INTEHEAD_KW , 0);
    sim_time = ecl_rsthead_date( intehead_kw );
    ecl_file_view_free( seqnum_map );
  }

  return sim_time;
}


double ecl_file_view_iget_restart_sim_days(const ecl_file_view_type * ecl_file_view , int seqnum_index) {
  double sim_days = 0;
  ecl_file_view_type * seqnum_map = ecl_file_view_alloc_blockview( ecl_file_view , SEQNUM_KW , seqnum_index);

  if (seqnum_map != NULL) {
    ecl_kw_type * doubhead_kw = ecl_file_view_iget_named_kw( seqnum_map , DOUBHEAD_KW , 0);
    sim_days = ecl_kw_iget_double( doubhead_kw , DOUBHEAD_DAYS_INDEX);
    ecl_file_view_free( seqnum_map );
  }

  return sim_days;
}




int ecl_file_view_find_sim_time(const ecl_file_view_type * ecl_file_view , time_t sim_time) {
  int seqnum_index = -1;
  if ( ecl_file_view_has_kw( ecl_file_view , INTEHEAD_KW)) {
    const int_vector_type * intehead_index_list = (const int_vector_type *)hash_get( ecl_file_view->kw_index , INTEHEAD_KW );
    int index = 0;
    while (index < int_vector_size( intehead_index_list )) {
      const ecl_kw_type * intehead_kw = ecl_file_view_iget_kw( ecl_file_view , int_vector_iget( intehead_index_list , index ));
      if (ecl_rsthead_date( intehead_kw ) == sim_time) {
        seqnum_index = index;
        break;
      }
      index++;
    }
  }
  return seqnum_index;
}


/**
   This function will scan through the ecl_file looking for INTEHEAD
   headers corresponding to sim_time. If sim_time is found the
   function will return the INTEHEAD occurence number, i.e. for a
   unified restart file like:

   INTEHEAD  /  01.01.2000
   ...
   PRESSURE
   SWAT
   ...
   INTEHEAD  /  01.03.2000
   ...
   PRESSURE
   SWAT
   ...
   INTEHEAD  /  01.05.2000
   ...
   PRESSURE
   SWAT
   ....

   The function call:

   ecl_file_get_restart_index( restart_file , (time_t) "01.03.2000")

   will return 1. Observe that this will in general NOT agree with the
   DATES step number.

   If the sim_time can not be found the function will return -1, that
   includes the case when the file in question is not a restart file
   at all, and no INTEHEAD headers can be found.

   Observe that the function requires on-the-second-equality; which is
   of course quite strict.

   Each report step only has one occurence of SEQNUM, but one INTEHEAD
   for each LGR; i.e. one should call iselect_rstblock() prior to
   calling this function.
*/


bool ecl_file_view_has_sim_time( const ecl_file_view_type * ecl_file_view , time_t sim_time) {
  int num_INTEHEAD = ecl_file_view_get_num_named_kw( ecl_file_view , INTEHEAD_KW );
  if (num_INTEHEAD == 0)
    return false;       /* We have no INTEHEAD headers - probably not a restart file at all. */
  else {
    int intehead_index = 0;
    while (true) {
      time_t itime = ecl_file_view_iget_restart_sim_date( ecl_file_view , intehead_index );

      if (itime == sim_time) /* Perfect hit. */
        return true;

      if (itime > sim_time)  /* We have gone past the target_time - i.e. we do not have it. */
        return false;

      intehead_index++;
      if (intehead_index == num_INTEHEAD)  /* We have iterated through the whole thing without finding sim_time. */
        return false;
    }
  }
}


bool ecl_file_view_has_sim_days( const ecl_file_view_type * ecl_file_view , double sim_days) {
  int num_DOUBHEAD = ecl_file_view_get_num_named_kw( ecl_file_view , DOUBHEAD_KW );
  if (num_DOUBHEAD == 0)
    return false;       /* We have no DOUBHEAD headers - probably not a restart file at all. */
  else {
    int doubhead_index = 0;
    while (true) {
      double file_sim_days  = ecl_file_view_iget_restart_sim_days( ecl_file_view , doubhead_index );

      if (util_double_approx_equal(sim_days, file_sim_days)) /* Perfect hit. */
        return true;

      if (file_sim_days > sim_days)  /* We have gone past the target_time - i.e. we do not have it. */
        return false;

      doubhead_index++;
      if (doubhead_index == num_DOUBHEAD)  /* We have iterated through the whole thing without finding sim_time. */
        return false;
    }
  }
}




int ecl_file_view_seqnum_index_from_sim_time( ecl_file_view_type * parent_map , time_t sim_time) {
  int num_seqnum = ecl_file_view_get_num_named_kw( parent_map , SEQNUM_KW );
  ecl_file_view_type * seqnum_map;

  for (int s_idx = 0; s_idx < num_seqnum; s_idx++) {
    seqnum_map = ecl_file_view_alloc_blockview( parent_map , SEQNUM_KW , s_idx );

    if (!seqnum_map)
      continue;

    bool sim = ecl_file_view_has_sim_time( seqnum_map , sim_time);
    ecl_file_view_free( seqnum_map );
    if (sim)
      return s_idx;
  }
  return -1;
}


int ecl_file_view_seqnum_index_from_sim_days( ecl_file_view_type * file_view , double sim_days) {
  int num_seqnum = ecl_file_view_get_num_named_kw( file_view , SEQNUM_KW );
  int seqnum_index = 0;
  ecl_file_view_type * seqnum_map;

  while (true) {
    seqnum_map = ecl_file_view_alloc_blockview( file_view , SEQNUM_KW , seqnum_index);

    if (seqnum_map != NULL) {
      if (ecl_file_view_has_sim_days( seqnum_map , sim_days)) {
        ecl_file_view_free( seqnum_map );
        return seqnum_index;
      } else {
        ecl_file_view_free( seqnum_map );
        seqnum_index++;
      }
    }

    if (num_seqnum == seqnum_index)
      return -1;
  }
}



/*
  Will mulitplex on the four input arguments.
*/
ecl_file_view_type * ecl_file_view_add_restart_view( ecl_file_view_type * file_view , int input_index, int report_step , time_t sim_time, double sim_days) {
  ecl_file_view_type * child = NULL;
  int seqnum_index = -1;

  if (input_index >= 0)
    seqnum_index = input_index;
  else if (report_step >= 0) {
    int global_index = ecl_file_view_find_kw_value( file_view , SEQNUM_KW , &report_step);
    if ( global_index >= 0)
      seqnum_index = ecl_file_view_iget_occurence( file_view , global_index );
  } else if (sim_time != -1)
    seqnum_index = ecl_file_view_seqnum_index_from_sim_time( file_view , sim_time );
  else if (sim_days >= 0)
    seqnum_index = ecl_file_view_seqnum_index_from_sim_days( file_view , sim_days );


  if (seqnum_index >= 0)
    child = ecl_file_view_add_blockview( file_view , SEQNUM_KW , seqnum_index );

  return child;
}



ecl_file_view_type * ecl_file_view_add_summary_view( ecl_file_view_type * file_view , int report_step ) {
  ecl_file_view_type * child = ecl_file_view_add_blockview( file_view , SEQHDR_KW , report_step );
  return child;
}


void ecl_file_view_fclose_stream( ecl_file_view_type * file_view ) {
  fortio_fclose_stream( file_view->fortio );
}

void ecl_file_view_write_index(const ecl_file_view_type * file_view, FILE * ostream) {
  int size = ecl_file_view_get_size(file_view);
  util_fwrite_int( size , ostream);

  ecl_file_kw_type * file_kw;
  for (int i = 0; i < size; i++) {
     file_kw = ecl_file_view_iget_file_kw( file_view, i );
     ecl_file_kw_fwrite( file_kw , ostream );
  }
}

ecl_file_view_type * ecl_file_view_fread_alloc( fortio_type * fortio , int * flags , inv_map_type * inv_map, FILE * istream ) {

  int index_size = util_fread_int(istream);
  ecl_file_kw_type ** file_kw_list = ecl_file_kw_fread_alloc_multiple( istream, index_size);
  if (file_kw_list) {
    ecl_file_view_type * file_view = ecl_file_view_alloc( fortio , flags , inv_map , true );
    for (int i=0; i < index_size; i++)
      ecl_file_view_add_kw(file_view , file_kw_list[i]);

    free(file_kw_list);
    ecl_file_view_make_index( file_view );
    return file_view;
  }
  else {
    fprintf(stderr, "%s: error reading ecl_file_type index file.\n", __func__);
    return NULL;
  }
}


ecl_file_transaction_type * ecl_file_view_start_transaction(ecl_file_view_type * file_view) {
  ecl_file_transaction_type * t = (ecl_file_transaction_type *)util_malloc(sizeof * t);
  int size = ecl_file_view_get_size(file_view);
  t->file_view = file_view;
  t->ref_count = (int*)util_malloc( size * sizeof * t->ref_count );
  for (int i = 0; i < size; i++) {
    ecl_file_kw_type * file_kw = ecl_file_view_iget_file_kw(file_view, i);
    ecl_file_kw_start_transaction(file_kw, &t->ref_count[i]);
  }

  return t;
}

void ecl_file_view_end_transaction( ecl_file_view_type * file_view, ecl_file_transaction_type * transaction) {
  if (transaction->file_view != file_view)
    util_abort("%s: internal error - file_view / transaction mismatch\n",__func__);

  const int * ref_count = transaction->ref_count;
  for (int i = 0; i < ecl_file_view_get_size(file_view); i++) {
    ecl_file_kw_type * file_kw = ecl_file_view_iget_file_kw(file_view, i);
    ecl_file_kw_end_transaction(file_kw, ref_count[i]);
  }
}



