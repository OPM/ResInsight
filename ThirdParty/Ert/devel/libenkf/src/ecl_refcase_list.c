/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'ecl_refcase_list.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/vector.h>
#include <ert/util/util.h>
#include <ert/util/type_macros.h>
#include <ert/util/hash.h>

#include <ert/ecl/ecl_sum.h>

#include <ert/enkf/ecl_refcase_list.h>


/*
  This file implements a quite simple collection of ecl_sum instances,
  with the following twists:

   o The default case is special in several ways:

     - It is added with the ecl_refcase_list_set_default() function.

     - The default case is always the first case, i.e. iget(0) will
       return the default case.

     - When setting the default we verify that it can be loaded, and
       the thing fails if not. That is in contrast to the other cases
       where the actual summary loading is lazy and on demand.
       
   o The collection has an internal dictionary which ensures that the
     same case is not added twice.

   o When calling iget( ) the default case will come as index 0, and
     the remaining cases will come in util_strcmp_int() ordering.

   o Observe that the functions in this module return NULL quite
     liberally:

       - If you have added a case which does not exist you will get
         NULL when you (at a later stage) try to access the ecl_sum
         instance.  
  
       - If you ask for the default case before one has been set you
         will get NULL.
 */


#define SUM_PAIR_TYPE_ID 665109971

typedef struct sum_pair_struct {
  UTIL_TYPE_ID_DECLARATION;
  char         * case_name;     // This  should be (path)/basename - no extension
  ecl_sum_type * ecl_sum;
} sum_pair_type;



struct ecl_refcase_list_struct {
  sum_pair_type     * default_case;
  hash_type         * case_dict;
  vector_type       * case_list;      /* This is created, and sorted, on demand when we do indexed lookup of cases. */
  bool                sorted; 
};

/*****************************************************************/



static sum_pair_type * sum_pair_alloc( const char * case_name , bool strict_load) {
  ecl_sum_type * ecl_sum = NULL;
  char * path = NULL;
  char * basename = NULL;

  util_alloc_file_components( case_name , &path , &basename , NULL);
  if (basename != NULL) {
    char * use_case = util_alloc_filename( path , basename , NULL);
    if (strict_load) 
      ecl_sum = ecl_sum_fread_alloc_case( use_case , ":");

    util_safe_free( path );
    free( basename );
    if (strict_load && (ecl_sum == NULL)) {
      free( use_case );
      return NULL;
    }

    
    {
      sum_pair_type * pair = util_malloc( sizeof * pair );
      UTIL_TYPE_ID_INIT( pair , SUM_PAIR_TYPE_ID );
      pair->case_name = use_case;
      pair->ecl_sum   = ecl_sum;
      return pair;
    }
  } else {
    util_safe_free( path );
    util_safe_free( basename );
    return NULL;
  }
}


static UTIL_SAFE_CAST_FUNCTION( sum_pair , SUM_PAIR_TYPE_ID );
static UTIL_SAFE_CAST_FUNCTION_CONST( sum_pair , SUM_PAIR_TYPE_ID );
static UTIL_IS_INSTANCE_FUNCTION( sum_pair , SUM_PAIR_TYPE_ID);

const ecl_sum_type * sum_pair_get_ecl_sum( sum_pair_type * sum_pair ) {
  if (sum_pair->ecl_sum == NULL)
    sum_pair->ecl_sum = ecl_sum_fread_alloc_case( sum_pair->case_name , ":");
  return sum_pair->ecl_sum;
}


static void sum_pair_free( sum_pair_type * sum_pair ) {
  free( sum_pair->case_name );
  if (sum_pair->ecl_sum != NULL)
    ecl_sum_free( sum_pair->ecl_sum );
  free( sum_pair );
}


static void sum_pair_free__( void * arg ) {
  sum_pair_type * pair = sum_pair_safe_cast( arg );
  sum_pair_free( pair );
}




static int sum_pair_cmp( const void * arg1 , const void * arg2) {
  const sum_pair_type * pair1 = sum_pair_safe_cast_const( arg1 );
  const sum_pair_type * pair2 = sum_pair_safe_cast_const( arg2 );
  
  return util_strcmp_int( pair1->case_name , pair2->case_name );
}

/*****************************************************************/


ecl_refcase_list_type * ecl_refcase_list_alloc( ) {
  ecl_refcase_list_type * refcase_list = util_malloc( sizeof * refcase_list );
  refcase_list->default_case = NULL;
  refcase_list->case_list    = vector_alloc_new();
  refcase_list->case_dict    = hash_alloc();
  refcase_list->sorted       = false;
  return refcase_list;
}



void ecl_refcase_list_free( ecl_refcase_list_type * refcase_list ) {
  vector_free( refcase_list->case_list );
  hash_free( refcase_list->case_dict );
  free( refcase_list );
}



const ecl_sum_type * ecl_refcase_list_get_default( ecl_refcase_list_type * refcase_list ) {
  if (refcase_list->default_case)
    return sum_pair_get_ecl_sum( refcase_list->default_case );
  else
    return NULL;
}


bool ecl_refcase_list_has_default( ecl_refcase_list_type * refcase_list ) {
  if (refcase_list->default_case)
    return true;
  else
    return false;
}


static void ecl_refcase_list_del_default( ecl_refcase_list_type * refcase_list ) {
  sum_pair_type * default_pair = refcase_list->default_case;
  if (default_pair) {
    hash_del( refcase_list->case_dict , default_pair->case_name );
    refcase_list->default_case = NULL;
  }
}


/*
  If a valid refcase has been set already, and this fails, nothing happens.
*/

bool ecl_refcase_list_set_default( ecl_refcase_list_type * refcase_list , const char * default_case) {

  if (default_case) {
    sum_pair_type * default_pair = sum_pair_alloc( default_case , true );
    if (default_pair) {
      ecl_refcase_list_del_default( refcase_list );
      refcase_list->default_case = default_pair;
      
      hash_insert_hash_owned_ref( refcase_list->case_dict , default_pair->case_name , default_pair , sum_pair_free__);
      refcase_list->sorted = false;
      return true;
    } else
      return false;
  } else {
    ecl_refcase_list_del_default( refcase_list );
    return true;
  }
}




/**
   Will sort the list and remove all elements which can not be loaded.
*/

static void ecl_refcase_list_assert_clean( ecl_refcase_list_type * refcase_list ) {
  if (!refcase_list->sorted) {
    vector_free( refcase_list->case_list );
    refcase_list->case_list = vector_alloc_new();
    
    {
      stringlist_type * tmp_list = hash_alloc_stringlist( refcase_list->case_dict );
      sum_pair_type * default_case = refcase_list->default_case;
      int i;

      for (i =0; i < stringlist_get_size( tmp_list ); i++) {
        const char * casename = stringlist_iget( tmp_list , i );
        bool normal_case = true;

        if (default_case && util_string_equal( casename , default_case->case_name))
          normal_case = false;
        
        if (normal_case) {
          sum_pair_type * pair = hash_get( refcase_list->case_dict , casename);
          const ecl_sum_type * ecl_sum = sum_pair_get_ecl_sum( pair );
          
          if (ecl_sum)
            vector_append_ref( refcase_list->case_list , pair);
          else 
            hash_del( refcase_list->case_dict , casename );
          
        }
      }
      stringlist_free( tmp_list );
    }

    vector_sort( refcase_list->case_list , sum_pair_cmp );
    refcase_list->sorted = true;
  }
}


static sum_pair_type * ecl_refcase_list_get_pair( ecl_refcase_list_type * refcase_list , const char * case_name) {
  ecl_refcase_list_assert_clean( refcase_list );
  {
    if (hash_has_key( refcase_list->case_dict , case_name))
      return hash_get( refcase_list->case_dict , case_name );
    else
      return NULL;
  }
}


static sum_pair_type * ecl_refcase_list_iget_pair( ecl_refcase_list_type * refcase_list , int index) {
  ecl_refcase_list_assert_clean( refcase_list );
  {
    int index_offset = refcase_list->default_case ? 1 : 0;
    index -= index_offset;
    
    if (index < 0)
      return refcase_list->default_case;
    else
      return vector_safe_iget( refcase_list->case_list , index);
  }
}


int ecl_refcase_list_get_size(ecl_refcase_list_type * refcase_list ) {
  ecl_refcase_list_assert_clean( refcase_list );
  {
    int size = hash_get_size( refcase_list->case_dict );
    return size;
  }
}



const ecl_sum_type * ecl_refcase_list_iget_case( ecl_refcase_list_type * refcase_list , int index) {
  sum_pair_type * pair = ecl_refcase_list_iget_pair( refcase_list , index );
  if (pair)
    return sum_pair_get_ecl_sum( pair );
  else
    return NULL;
}


const ecl_sum_type * ecl_refcase_list_get_case( ecl_refcase_list_type * refcase_list , const char * case_name) {
  sum_pair_type * pair = ecl_refcase_list_get_pair( refcase_list , case_name );
  if (pair)
    return sum_pair_get_ecl_sum( pair );
  else
    return NULL;
}


bool ecl_refcase_list_has_case( ecl_refcase_list_type * refcase_list , const char * case_name) {
  const sum_pair_type * pair = ecl_refcase_list_get_pair( refcase_list , case_name );
  if (pair)
    return true;
  else
    return false;
}



const char * ecl_refcase_list_iget_pathcase( ecl_refcase_list_type * refcase_list , int index) {
  const ecl_sum_type * ecl_sum = ecl_refcase_list_iget_case( refcase_list , index );
  if (ecl_sum)
    return ecl_sum_get_case( ecl_sum );
  else
    return NULL;
}



int ecl_refcase_list_add_case( ecl_refcase_list_type * refcase_list , const char * case_name) {
  sum_pair_type * pair = sum_pair_alloc( case_name , false );
  if (pair) {
    if (hash_has_key( refcase_list->case_dict , pair->case_name)) {
      sum_pair_free( pair );
      return 0;
    } else {
      hash_insert_hash_owned_ref( refcase_list->case_dict , pair->case_name , pair , sum_pair_free__);
      refcase_list->sorted = false;
      return 1;
    }
  } else
    return 0;  
}

/*
  The glob_string pattern must (for all practical purposes) end in an
  explicit extension:

    - If it ends without an extension there will be zero mathces.  It
    - it ends with a '*' there will be a hell-of-a-lot of duplicate
      matches.

  If the input glob string does not have an explicit extension we add
  .*SMSPEC for the matching purpose.
*/


int ecl_refcase_list_add_matching( ecl_refcase_list_type * refcase_list , const char * __glob_string) {
  int count = 0;
  char * glob_string;

  {
    char * glob_ext;
    util_alloc_file_components( __glob_string , NULL , NULL , &glob_ext);
    if (glob_ext == NULL) 
      glob_string = util_alloc_filename(NULL , __glob_string , "*SMSPEC");
    else {
      glob_string = util_alloc_string_copy( __glob_string );
      free( glob_ext );
    }
  }
  
  {
    stringlist_type * case_list = stringlist_alloc_new();
    int i;
    stringlist_select_matching( case_list , glob_string );
    for (i=0; i < stringlist_get_size( case_list ); i++) {
      count += ecl_refcase_list_add_case( refcase_list , stringlist_iget( case_list , i ) );
    }
    stringlist_free( case_list );
  }

  free( glob_string );
  return count;
}

