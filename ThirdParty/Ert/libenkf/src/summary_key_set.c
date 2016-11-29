/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   The file 'state_map.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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
#include <pthread.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/summary_key_set.h>


#define SUMMARY_KEY_SET_TYPE_ID 700672133

struct summary_key_set_struct {
  UTIL_TYPE_ID_DECLARATION;
  hash_type        * key_set;
  pthread_rwlock_t   rw_lock;
  bool               read_only;
};


UTIL_IS_INSTANCE_FUNCTION( summary_key_set , SUMMARY_KEY_SET_TYPE_ID )


summary_key_set_type * summary_key_set_alloc() {
  summary_key_set_type * set = util_malloc(sizeof * set);
  UTIL_TYPE_ID_INIT( set , SUMMARY_KEY_SET_TYPE_ID);
  set->key_set = hash_alloc();
  pthread_rwlock_init( &set->rw_lock , NULL);
  set->read_only = false;
  return set;
}

summary_key_set_type * summary_key_set_alloc_from_file(const char * filename, bool read_only) {
    summary_key_set_type * set = summary_key_set_alloc();
    summary_key_set_fread(set, filename);
    set->read_only = read_only;
    return set;
}

void summary_key_set_free(summary_key_set_type * set) {
    hash_free(set->key_set);
    free(set);
}

int summary_key_set_get_size(summary_key_set_type * set) {
  int size;
  pthread_rwlock_rdlock( &set->rw_lock );
  {
    size = hash_get_size( set->key_set );
  }
  pthread_rwlock_unlock( &set->rw_lock );
  return size;
}


bool summary_key_set_add_summary_key(summary_key_set_type * set, const char * summary_key) {
    bool writable_and_non_existent = true;

    pthread_rwlock_wrlock( &set->rw_lock);
    {

        if(hash_has_key(set->key_set, summary_key)) {
            writable_and_non_existent = false;
        }

        if(set->read_only) {
            writable_and_non_existent = false;
        }

        if(writable_and_non_existent) {
            hash_insert_int(set->key_set, summary_key, 1);
        }
    }
    pthread_rwlock_unlock( &set->rw_lock );

    return writable_and_non_existent;
}

bool summary_key_set_has_summary_key(summary_key_set_type * set, const char * summary_key) {
    bool has_key = false;

    pthread_rwlock_rdlock( &set->rw_lock );
    {
        has_key = hash_has_key(set->key_set, summary_key);
    }
    pthread_rwlock_unlock( &set->rw_lock );

    return has_key;
}

stringlist_type * summary_key_set_alloc_keys(summary_key_set_type * set) {
    stringlist_type * keys;

    pthread_rwlock_rdlock( &set->rw_lock );
    {
        keys = hash_alloc_stringlist(set->key_set);
    }
    pthread_rwlock_unlock( &set->rw_lock );

    return keys;
}


bool summary_key_set_is_read_only(const summary_key_set_type * set) {
    return set->read_only;
}

void summary_key_set_fwrite(summary_key_set_type * set, const char * filename) {
    pthread_rwlock_rdlock( &set->rw_lock );
    {
        FILE * stream = util_mkdir_fopen(filename , "w");
        if (stream) {
            stringlist_type * keys = hash_alloc_stringlist(set->key_set);
            stringlist_fwrite(keys, stream);
            stringlist_free(keys);
            fclose( stream );
        } else {
            util_abort("%s: failed to open: %s for writing \n", __func__, filename);
        }
    }
    pthread_rwlock_unlock( &set->rw_lock );
}

bool summary_key_set_fread(summary_key_set_type * set, const char * filename) {
    bool file_exists = false;
    pthread_rwlock_wrlock( &set->rw_lock );
    {
        hash_clear(set->key_set);

        if (util_file_exists(filename)) {
            FILE * stream = util_fopen(filename, "r");
            if (stream) {
                stringlist_type * key_set = stringlist_fread_alloc(stream);

                for (int i = 0; i < stringlist_get_size(key_set); i++) {
                    hash_insert_int(set->key_set, stringlist_iget(key_set, i), 1);
                }
                stringlist_free(key_set);
                fclose( stream );
            } else {
                util_abort("%s: failed to open: %s for reading \n",__func__ , filename );
            }
            file_exists = true;
        }
    }
    pthread_rwlock_unlock( &set->rw_lock );
    return file_exists;
}
