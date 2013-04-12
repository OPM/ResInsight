/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'hash.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#define  _GNU_SOURCE   /* Must define this to get access to pthread_rwlock_t */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <ert/util/hash.h>
#include <ert/util/hash_sll.h>
#include <ert/util/hash_node.h>
#include <ert/util/node_data.h>
#include <ert/util/util.h>
#include <ert/util/stringlist.h>

#ifdef HAVE_PTHREAD
#include <pthread.h>
typedef pthread_rwlock_t lock_type;
#else
typedef int lock_type; 
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_DEFAULT_SIZE 16
#define HASH_TYPE_ID      771065

/**
   This is **THE** hash function - which actually does the hashing.
*/

static uint32_t hash_index(const char *key, size_t len) {
  uint32_t hash = 0;
  size_t i;

  for (i=0; i < len; i++) {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >>  6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}


struct hash_struct {
  UTIL_TYPE_ID_DECLARATION;
  uint32_t          size;            /* This is the size of the internal table **NOT**NOT** the number of elements in the table. */
  uint32_t          elements;        /* The number of elements in the hash table. */
  double            resize_fill;
  hash_sll_type   **table;
  hashf_type       *hashf;

  lock_type         rwlock;
};



typedef struct hash_sort_node {
  char     *key;
  int       cmp_value;
} hash_sort_type;


/*****************************************************************/
/*                          locking                              */
/*****************************************************************/
#ifdef HAVE_PTHREAD
static void __hash_deadlock_abort(hash_type * hash) {
  util_abort("%s: A deadlock condition has been detected in the hash routine - and the program will abort.\n", __func__);
}


static void __hash_rdlock(hash_type * hash) {
  int lock_error = pthread_rwlock_tryrdlock( &hash->rwlock );
  if (lock_error != 0) {
    /* We did not get the lock - let us check why: */
    if (lock_error == EDEADLK)
      /* A deadlock is detected - we just abort. */
      __hash_deadlock_abort(hash);
    else 
      /* We ignore all other error conditions than DEADLOCK and just try again. */
      pthread_rwlock_rdlock( &hash->rwlock );
  }
  /* Ok - when we are here - we are guranteed to have the lock. */
}


static void __hash_wrlock(hash_type * hash) {
  int lock_error = pthread_rwlock_trywrlock( &hash->rwlock );
  if (lock_error != 0) {
    /* We did not get the lock - let us check why: */
    if (lock_error == EDEADLK)
      /* A deadlock is detected - we just abort. */
      __hash_deadlock_abort(hash);
    else 
      /* We ignore all other error conditions than DEADLOCK and just try again. */
      pthread_rwlock_wrlock( &hash->rwlock );
  }
  /* Ok - when we are here - we are guranteed to have the lock. */
}


static void __hash_unlock( hash_type * hash) {
  pthread_rwlock_unlock( &hash->rwlock );
}


static void LOCK_DESTROY( lock_type * rwlock ) {
  pthread_rwlock_destroy( rwlock );
}

static void LOCK_INIT( lock_type * rwlock ) {
  pthread_rwlock_init( rwlock , NULL);
}

#else

static void __hash_rdlock(hash_type * hash) {}
static void __hash_wrlock(hash_type * hash) {}
static void __hash_unlock(hash_type * hash) {}
static void LOCK_DESTROY(lock_type * rwlock) {}
static void LOCK_INIT(lock_type * rwlock) {}

#endif

/*****************************************************************/
/*                    Low level access functions                 */
/*****************************************************************/


static void * __hash_get_node_unlocked(const hash_type *__hash , const char *key, bool abort_on_error) {
  hash_type * hash = (hash_type *) __hash;  /* The net effect is no change - but .... ?? */
  hash_node_type * node = NULL;
  {
    const uint32_t global_index = hash->hashf(key , strlen(key));
    const uint32_t table_index  = (global_index % hash->size);
    
    node = hash_sll_get(hash->table[table_index] , global_index , key);
    if (node == NULL && abort_on_error) 
      util_abort("%s: tried to get from key:%s which does not exist - aborting \n",__func__ , key);
    
  }
  return node;
}


/*
  This function looks up a hash_node from the hash. This is the common
  low-level function to get content from the hash. The function takes
  read-lock which is held during execution.

  Would strongly preferred that the hash_type * was const - but that is
  difficult due to locking requirements.
*/

static void * __hash_get_node(hash_type *hash , const char *key, bool abort_on_error) {
  hash_node_type * node;
  __hash_rdlock( hash );
  node = __hash_get_node_unlocked(hash , key , abort_on_error);
  __hash_unlock( hash );
  return node;
}


static node_data_type * hash_get_node_data(hash_type *hash , const char *key) {
  hash_node_type * node = __hash_get_node(hash , key , true);
  return hash_node_get_data(node);
}




/**
   This function resizes the hash table when it has become to full.
   The table only grows - this function is called from
   __hash_insert_node(). 

   If you know in advance (roughly) how large the hash table will be
   it can be advantageous to call hash_resize() manually, to avoid
   repeated internal calls to hash_resize().
*/

void hash_resize(hash_type *hash, int new_size) {
  hash_sll_type ** new_table = hash_sll_alloc_table( new_size );
  hash_node_type * node;
  int i;
  
  for (i=0; i < hash->size; i++) {
    node = hash_sll_get_head(hash->table[i]);
    while (node != NULL) {
      uint32_t new_table_index  = hash_node_set_table_index(node , new_size);
      hash_node_type *next_node = hash_node_get_next(node);
      hash_sll_add_node(new_table[new_table_index] , node);
      node = next_node;
    }
  }
  
  /* 
     Only freeing the table structure, *NOT* calling the node_free()
     functions, which happens when hash_sll_free() is called.
  */
  
  {
    for (i=0; i < hash->size; i++) 
      free( hash->table[i] );
    free( hash->table );
  }
  
  hash->size     = new_size;
  hash->table    = new_table;
}


/**
   This is the low-level function for inserting a hash node. This
   function takes a write-lock which is held during the execution of
   the function.
*/
   
static void __hash_insert_node(hash_type *hash , hash_node_type *node) {
  __hash_wrlock( hash );
  {
    uint32_t table_index = hash_node_get_table_index(node);
    {
      /*
        If a node with the same key already exists in the table
        it is removed.
      */
      hash_node_type *existing_node = __hash_get_node_unlocked(hash , hash_node_get_key(node) , false);
      if (existing_node != NULL) {
        hash_sll_del_node(hash->table[table_index] , existing_node);
        hash->elements--;
      } 
    }
    
    hash_sll_add_node(hash->table[table_index] , node);  
    hash->elements++;
    if ((1.0 * hash->elements / hash->size) > hash->resize_fill) 
      hash_resize(hash , hash->size * 2);
  }
  __hash_unlock( hash );
}



/**
   This function deletes a node from the hash_table. Observe that this
   function does *NOT* do any locking - it is the repsonsibility of
   the calling functions: hash_del() and hash_clear() to take the
   necessary write lock.
*/


static void hash_del_unlocked__(hash_type *hash , const char *key) {
  const uint32_t global_index = hash->hashf(key , strlen(key));
  const uint32_t table_index  = (global_index % hash->size);
  hash_node_type *node        = hash_sll_get(hash->table[table_index] , global_index , key);
  
  if (node == NULL) 
    util_abort("%s: hash does not contain key:%s - aborting \n",__func__ , key);
  else
    hash_sll_del_node(hash->table[table_index] , node);
  
  hash->elements--;
}



/**
   This functions takes a hash_node and finds the "next" hash node by
   traversing the internal hash structure. Should NOT be confused with
   the other functions providing iterations to user-space.
*/

static hash_node_type * hash_internal_iter_next(const hash_type *hash , const hash_node_type * node) {
  hash_node_type *next_node = hash_node_get_next(node);
  if (next_node == NULL) {
    const uint32_t table_index = hash_node_get_table_index(node);
    if (table_index < hash->size) {
      uint32_t i = table_index + 1;
      while (i < hash->size && hash_sll_empty(hash->table[i]))
        i++;

      if (i < hash->size) 
        next_node = hash_sll_get_head(hash->table[i]);
    }
  }
  return next_node;
}



/**
   This is the low level function which traverses a hash table and
   allocates a char ** list of keys. 

   It takes a read-lock which is held during the execution of the
   function. The locking guarantees that the list of keys is valid
   when this function is exited, but the the hash table can be
   subsequently updated.

   If the hash table is empty NULL is returned.
*/

static char ** hash_alloc_keylist__(hash_type *hash , bool lock) {
  char **keylist;
  if (lock) __hash_rdlock( hash );
  {
    if (hash->elements > 0) {
      int i = 0;
      hash_node_type *node = NULL;
      keylist = calloc(hash->elements , sizeof *keylist);
      {
        uint32_t i = 0;
        while (i < hash->size && hash_sll_empty(hash->table[i]))
          i++;
        
        if (i < hash->size) 
          node = hash_sll_get_head(hash->table[i]);
      }
      
      while (node != NULL) {
        const char *key = hash_node_get_key(node); 
        keylist[i] = util_alloc_string_copy(key);
        node = hash_internal_iter_next(hash , node);
        i++;
      }
    } else keylist = NULL;
  }
  if (lock) __hash_unlock( hash );
  return keylist;
}






/*****************************************************************/
/** 
   The fundamental functions above relate the hash_node
   structure. Here comes a list of functions for inserting managed
   copies of various types.
*/

void hash_insert_string(hash_type * hash , const char * key , const char * value) {
  node_data_type * node_data = node_data_alloc_string( value );
  hash_node_type * hash_node = hash_node_alloc_new(key , node_data , hash->hashf , hash->size);
  __hash_insert_node(hash , hash_node);
}


char * hash_get_string(hash_type * hash , const char * key) {
  node_data_type * node_data = hash_get_node_data(hash , key);
  return node_data_get_string( node_data );
}


void hash_insert_int(hash_type * hash , const char * key , int value) {
  node_data_type * node_data = node_data_alloc_int( value );
  hash_node_type * hash_node = hash_node_alloc_new(key , node_data , hash->hashf , hash->size);
  __hash_insert_node(hash , hash_node);
}


int hash_get_int(hash_type * hash , const char * key) {
  node_data_type * node_data = hash_get_node_data(hash , key);
  return node_data_get_int( node_data );
}

/**
   Small wrapper around hash_get_int() / hash_insert_int() which
   implements a zero based counter.

      hash_inc_counter()

   Will increment the integer value stored in the node_data instance,
   and return the updated value. If the key is not present in the hash
   it will be inserted as an integer with value 0, and 0 will be
   returned.
*/

int hash_inc_counter(hash_type * hash , const char * counter_key) {
  if (hash_has_key( hash , counter_key)) {
    node_data_type * node_data = hash_get_node_data(hash , counter_key);
    return node_data_fetch_and_inc_int( node_data );
  } else {
    hash_insert_int(hash , counter_key , 0);
    return 0;
  }
}

/**
   Will return 0 if the key is not in the hash.
*/

int hash_get_counter(hash_type * hash , const char * key) {
  if (hash_has_key( hash , key ))
    return hash_get_int( hash , key );
  else
    return 0;
}


void hash_insert_double(hash_type * hash , const char * key , double value) {
  node_data_type * node_data = node_data_alloc_double( value );
  hash_node_type * hash_node = hash_node_alloc_new(key , node_data , hash->hashf , hash->size);
  __hash_insert_node(hash , hash_node);
}

double hash_get_double(hash_type * hash , const char * key) {
  node_data_type * node_data = hash_get_node_data(hash , key);
  return node_data_get_double( node_data );
}


/*****************************************************************/

void hash_del(hash_type *hash , const char *key) {
  __hash_wrlock( hash );
  hash_del_unlocked__(hash , key);
  __hash_unlock( hash );
}

/**
   This function will delete the key if it exists in the hash, but it
   will NOT fail if the key is not already in the hash table.
*/
   
void hash_safe_del(hash_type * hash , const char * key) {
  __hash_wrlock( hash );
  if (__hash_get_node_unlocked(hash , key , false))
    hash_del_unlocked__(hash , key);
  __hash_unlock( hash );
}


void hash_clear(hash_type *hash) {
  __hash_wrlock( hash );
  {
    int old_size = hash_get_size(hash);
    if (old_size > 0) {
      char **keyList = hash_alloc_keylist__( hash , false);
      int i;
      for (i=0; i < old_size; i++) {
        hash_del_unlocked__(hash , keyList[i]);
        free(keyList[i]);
      }
      free(keyList);
    }
  }
  __hash_unlock( hash );
}


void * hash_get(const hash_type *hash , const char *key) {
  hash_node_type * hash_node = __hash_get_node(hash , key , true);
  node_data_type * data_node = hash_node_get_data( hash_node );
  return node_data_get_ptr( data_node ); 
}


/**
   This function will return NULL if the hash does not
   contain 'key'.
*/
void * hash_safe_get( const hash_type * hash , const char * key ) {
  hash_node_type * hash_node = __hash_get_node(hash , key , false);
  if (hash_node != NULL) {
    node_data_type * data_node = hash_node_get_data( hash_node );
    return node_data_get_ptr( data_node ); 
  } else
    return NULL;
}


/**
   This function will:

    1. Return an object from the hash table.
    2. Remove it from the table.

   Observe that if the object has been installed with a destructor,
   the object will be destroyed by the hash_del() operation, and the
   return value is complete gibberish - i.e. this function can NOT be
   used on hash-owned references.  
*/

void * hash_pop( hash_type * hash , const char * key) {
  void * value = hash_get( hash , key);
  hash_del( hash , key );
  return value;
}




/******************************************************************/
/*                     Allocators                                 */
/******************************************************************/


static hash_type * __hash_alloc(int size, double resize_fill , hashf_type *hashf) {
  hash_type* hash;
  hash = util_malloc(sizeof *hash );
  UTIL_TYPE_ID_INIT(hash , HASH_TYPE_ID);
  hash->size      = size;
  hash->hashf     = hashf;
  hash->table     = hash_sll_alloc_table(hash->size);
  hash->elements  = 0;
  hash->resize_fill  = resize_fill;
  LOCK_INIT( &hash->rwlock );
  
  return hash;
}


hash_type * hash_alloc() {
  return __hash_alloc(HASH_DEFAULT_SIZE , 0.50 , hash_index);
}

// Purely a helper in the process of removing the internal locking
// in the hash implementation.
hash_type * hash_alloc_unlocked() {  
  return __hash_alloc(HASH_DEFAULT_SIZE , 0.50 , hash_index);
}


UTIL_SAFE_CAST_FUNCTION( hash , HASH_TYPE_ID)
UTIL_IS_INSTANCE_FUNCTION(hash , HASH_TYPE_ID)

void hash_free(hash_type *hash) {
  int i;
  for (i=0; i < hash->size; i++) 
    hash_sll_free(hash->table[i]);
  free(hash->table);
  LOCK_DESTROY( &hash->rwlock );
  free(hash);
}


void hash_free__(void * void_hash) {
  hash_free(hash_safe_cast( void_hash ));
}


char ** hash_alloc_keylist(hash_type *hash) {
  return hash_alloc_keylist__(hash , true);
}

stringlist_type * hash_alloc_stringlist(hash_type * hash) {
  stringlist_type * stringlist = stringlist_alloc_new();
  char ** keylist = hash_alloc_keylist__(hash , true);
  int i;
  for (i = 0; i < hash_get_size( hash ); i++)
    stringlist_append_owned_ref( stringlist , keylist[i] );
  
  free( keylist );
  return stringlist;
}



/*****************************************************************/
/** 
    The standard functions for inserting an entry in the hash table:

    hash_insert_copy(): The hash table uses copyc() to make a copy of
         value, which is inserted in the table. This means that the
         calling scope is free to do whatever it wants with value; and
         is also responsible for freeing it. The hash_table takes
         responsibility for freeing it's own copy.

    hash_insert_hash_owned_ref(): The hash table takes ownership of
         'value', in the sense that the hash table will call the
         'destructor' del() on value when the node is deleted.

    hash_insert_ref(): The hash table ONLY contains a pointer to
         value; the calling scope retains full ownership to
         value. When the hash node is deleted, the hash implementation
         will just drop the reference on the floor.
*/


void hash_insert_copy(hash_type *hash , const char *key , const void *value , copyc_ftype *copyc , free_ftype *del) {
  hash_node_type *hash_node;
  if (copyc == NULL || del == NULL) 
    util_abort("%s: must provide copy constructer and delete operator for insert copy - aborting \n",__func__);
  {
    node_data_type * data_node = node_data_alloc_ptr( value , copyc , del );
    hash_node                  = hash_node_alloc_new(key , data_node , hash->hashf , hash->size);
    __hash_insert_node(hash , hash_node);
  }
}


/**
  This function will insert a reference "value" with key "key"; when
  the key is deleted - either by an explicit call to hash_del(), or
  when the complete hash table is free'd with hash_free(), the
  destructur 'del' is called with 'value' as argument.

  It is importand to realize that when elements are inserted into a
  hash table with this function the calling scope gives up
  responsibility of freeing the memory pointed to by value.
*/

void hash_insert_hash_owned_ref(hash_type *hash , const char *key , const void *value , free_ftype *del) {
  hash_node_type *hash_node;
  if (del == NULL) 
    util_abort("%s: must provide delete operator for insert hash_owned_ref - aborting \n",__func__);
  {
    node_data_type * data_node = node_data_alloc_ptr( value , NULL , del );
    hash_node                  = hash_node_alloc_new(key , data_node , hash->hashf , hash->size);
    __hash_insert_node(hash , hash_node);
  }
}


void hash_insert_ref(hash_type *hash , const char *key , const void *value) {
  hash_node_type *hash_node;
  {
    node_data_type * data_node = node_data_alloc_ptr( value , NULL , NULL);
    hash_node                  = hash_node_alloc_new(key , data_node , hash->hashf , hash->size);
    __hash_insert_node(hash , hash_node);
  }
}



bool hash_has_key(const hash_type *hash , const char *key) {
  if (__hash_get_node(hash , key , false) == NULL)
    return false;
  else
    return true;
}


int hash_get_size(const hash_type *hash) { 
  return hash->elements; 
}



/******************************************************************/
/** 
   Here comes a list of functions for allocating keylists which have
   been sorted in various ways.
*/
   

static hash_sort_type * hash_alloc_sort_list(const hash_type *hash ,
                                             const char **keylist) { 

  int i; hash_sort_type * sort_list = calloc(hash_get_size(hash) , sizeof * sort_list); 
  for (i=0; i < hash_get_size(hash); i++) 
    sort_list[i].key = util_alloc_string_copy(keylist[i]);
  
  return sort_list;
}

static void hash_free_sort_list(const hash_type *hash , hash_sort_type *sort_list) {
  int i;
  for (i=0; i < hash_get_size(hash); i++) 
    free(sort_list[i].key);
  free(sort_list);
}


static int hash_sortlist_cmp(const void *_p1 , const void  *_p2) {
  const hash_sort_type *p1 = (const hash_sort_type *) _p1;
  const hash_sort_type *p2 = (const hash_sort_type *) _p2;

  if (p1->cmp_value == p2->cmp_value)
    return 0;
  else if (p1->cmp_value < p2->cmp_value)
    return -1;
  else
    return 1;
}


static char ** __hash_alloc_ordered_keylist(hash_type *hash , int ( hash_get_cmp_value) (const void * )) {    
  int i;
  char **sorted_keylist;
  char **tmp_keylist   = hash_alloc_keylist(hash);
  hash_sort_type * sort_list = hash_alloc_sort_list(hash , (const char **) tmp_keylist);

  for (i = 0; i < hash_get_size(hash); i++) 
    sort_list[i].cmp_value = hash_get_cmp_value( hash_get(hash , sort_list[i].key) );
  
  qsort(sort_list , hash_get_size(hash) , sizeof *sort_list , &hash_sortlist_cmp);
  sorted_keylist = calloc(hash_get_size(hash) , sizeof *sorted_keylist);
  for (i = 0; i < hash_get_size(hash); i++) {
    sorted_keylist[i] = util_alloc_string_copy(sort_list[i].key);
    free(tmp_keylist[i]);
  }
  free(tmp_keylist);
  hash_free_sort_list(hash , sort_list);
  return sorted_keylist;
}


char ** hash_alloc_sorted_keylist(hash_type *hash , int ( hash_get_cmp_value) (const void *)) {
  char ** key_list;

  key_list = __hash_alloc_ordered_keylist(hash , hash_get_cmp_value);

  return key_list;
}


static int key_cmp(const void *_s1 , const void *_s2) {
  const char * s1 = *((const char **) _s1);
  const char * s2 = *((const char **) _s2);
  
  return strcmp(s1 , s2);
}


static char ** __hash_alloc_key_sorted_list(hash_type *hash, int (*cmp) (const void * , const void *)) { 
  char **keylist = hash_alloc_keylist(hash);
  
  qsort(keylist , hash_get_size(hash) , sizeof *keylist , cmp);
  return keylist;
}



char ** hash_alloc_key_sorted_list(hash_type * hash, int (*cmp) (const void *, const void *))
{
  char ** key_list;

  key_list =__hash_alloc_key_sorted_list(hash , cmp);

  return key_list;
}



bool hash_key_list_compare(hash_type * hash1, hash_type * hash2)
{
  bool has_equal_keylist;
  int i,size1, size2;
  char **keylist1, **keylist2; 

  size1 = hash_get_size(hash1);
  size2 = hash_get_size(hash2);

  if(size1 != size2) return false;

  keylist1 = hash_alloc_key_sorted_list(hash1, &key_cmp);
  keylist2 = hash_alloc_key_sorted_list(hash2, &key_cmp);

  has_equal_keylist = true;
  for(i=0; i<size1; i++)
  {
    if(strcmp(keylist1[i],keylist2[i]) != 0)
    {
      has_equal_keylist = false;
      break;
    }
  }

  for(i=0; i<size1; i++) {
    free( keylist1[i] );
    free( keylist2[i] );
  }
  free( keylist1 );
  free( keylist2 );
  return has_equal_keylist;
  
}




/*****************************************************************/
/**
   This function will take a list of strings of type: 

     ["OPT1:Value1" , "MIN:0.0001" , "MAX:1.00" , "FILE:XX"]

   and build a hash table where the element in front of ':' is used as
   key, and the element behind the ':' is used as value. The value is
   internalized as a (char *) pointer with no type conversion.  In the
   calling scope the values should be extracted with hash_get().

   Strings which can not be interpreted as KEY:VALUE are simply
   ignored.
*/


hash_type * hash_alloc_from_options(const stringlist_type * options) {
  int num_options = stringlist_get_size( options );
  hash_type * opt_hash = hash_alloc();
  int iopt;

  for (iopt = 0; iopt < num_options; iopt++) {
    char * option;
    char * value;
    
    util_binary_split_string( stringlist_iget(options , iopt) , ":" , true , &option , &value);
    if ((option != NULL) && (value != NULL)) 
      hash_insert_hash_owned_ref( opt_hash , option , util_alloc_string_copy(value) , free);
    // Warning: could not interpret string as KEY:VALUE - ignored
      
    
    util_safe_free(option);
    util_safe_free(value);
  }
  
  return opt_hash;
}


 bool hash_add_option( hash_type * hash, const char * key_value) {
   bool addOK = false;
   {
     char * value;
     char * key;

     util_binary_split_string( key_value , ":" , true , &key , &value);
     if (value != NULL) {
       hash_insert_hash_owned_ref( hash , key , value , free );
       addOK = true;
     }
     
     util_safe_free( key );
   }
   return addOK;
}



/*****************************************************************/


/**
  This is a **VERY** simple iteration object.

  Do **NOT** use with multi-threading.
*/
struct hash_iter_struct {
  const hash_type  * hash;            /* The hash we are iterating over. */
  char            ** keylist;         /* The keys in the hash table - at the moment of hash_iter_alloc(). */ 
  int                num_keys;        
  int                current_key_num; /* This integer retains the state. */ 
};


void hash_iter_restart( hash_iter_type * iter ) {
  iter->current_key_num = 0;
}




hash_iter_type * hash_iter_alloc(const hash_type * hash) {
  hash_iter_type * iter = util_malloc(sizeof * iter ); 

  iter->hash            = hash;
  iter->num_keys        = hash_get_size(hash);
  iter->keylist         = hash_alloc_keylist( (hash_type *) hash);
  hash_iter_restart( iter );
  return iter;
}



void hash_iter_free(hash_iter_type * iter) {
  util_free_stringlist(iter->keylist, iter->num_keys);
  free(iter);
}



bool hash_iter_is_complete(const hash_iter_type * iter) {
  if(iter->current_key_num == iter->num_keys)
    return true;
  else
    return false;
}



/**
  Get the next key.

  Returns NULL if the iteration has ended.
*/
const char * hash_iter_get_next_key(hash_iter_type * iter) {
  const char * key;

  if(iter->current_key_num == iter->num_keys)
    return NULL;

  key = iter->keylist[iter->current_key_num];
  iter->current_key_num++;

  if(!hash_has_key(iter->hash, key))
    util_abort("%s: Programming error. Using hash_iter with multi-threading??\n", __func__);

  return key;
}



void * hash_iter_get_next_value(hash_iter_type * iter) {
  const char * key = hash_iter_get_next_key(iter);

  if(key != NULL)
    return hash_get(iter->hash, key);
  else
    return NULL;
}


/*****************************************************************/

/**
   This function will iterate through the hash table, and call the
   function func on each of the elements in the table inplace:

       ....
       value = hash_get( hash , key );
       func( value );                      <-- The call is inplace - with no arguments, 
       ....                                    and no return value. The content of 'value'
                                               can obviously change, but 'value' itself 
                                               must still be a valid reference!
*/


void hash_apply( hash_type * hash , hash_apply_ftype * func) {
  hash_iter_type * iter = hash_iter_alloc( hash );
  while (!hash_iter_is_complete( iter )) {
    const char * key = hash_iter_get_next_key( iter );
    void * value     = hash_get( hash , key );
    
    func( value );
  }
  hash_iter_free( iter );
}



#undef HASH_GET_SCALAR
#undef HASH_INSERT_SCALAR
#undef HASH_INSERT_ARRAY
#undef HASH_GET_ARRAY_PTR
#undef HASH_NODE_AS
#undef HASH_DEFAULT_SIZE

#ifdef __cplusplus
}
#endif
