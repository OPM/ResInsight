#define  _GNU_SOURCE   /* Must define this to get access to pthread_rwlock_t */

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_types.h>
#include <ert/enkf/custom_kw_config_set.h>


#define CUSTOM_KW_CONFIG_SET_TYPE_ID 701622133

struct custom_kw_config_set_struct {
    UTIL_TYPE_ID_DECLARATION;
    hash_type * config_set;
    pthread_rwlock_t rw_lock;
};


UTIL_IS_INSTANCE_FUNCTION(custom_kw_config_set, CUSTOM_KW_CONFIG_SET_TYPE_ID)


custom_kw_config_set_type * custom_kw_config_set_alloc() {
    custom_kw_config_set_type * set = util_malloc(sizeof * set);
    UTIL_TYPE_ID_INIT(set, CUSTOM_KW_CONFIG_SET_TYPE_ID);
    set->config_set = hash_alloc();
    pthread_rwlock_init(& set->rw_lock, NULL);
    return set;
}

custom_kw_config_set_type * custom_kw_config_set_alloc_from_file(const char * filename) {
    custom_kw_config_set_type * set = custom_kw_config_set_alloc();
    custom_kw_config_set_fread(set, filename);
    return set;
}

void custom_kw_config_set_free(custom_kw_config_set_type * set) {
    hash_free(set->config_set);
    free(set);
}

void custom_kw_config_set_add_config(custom_kw_config_set_type * set, const custom_kw_config_type * config) {
    pthread_rwlock_wrlock(& set->rw_lock);
    {
        const char * name = custom_kw_config_get_name(config);

        if (!hash_has_key(set->config_set, name)) {
            stringlist_type * stringlist = stringlist_alloc_new();
            hash_insert_hash_owned_ref(set->config_set, name, stringlist, stringlist_free__);
        }

        stringlist_type * formatted_keys = (stringlist_type *) hash_get(set->config_set, name);
        custom_kw_config_serialize(config, formatted_keys);

    }
    pthread_rwlock_unlock(& set->rw_lock);
}

void custom_kw_config_set_update_config(custom_kw_config_set_type * set, custom_kw_config_type * config) {
    pthread_rwlock_rdlock(& set->rw_lock);
    {
        const char * name = custom_kw_config_get_name(config);

        if(!hash_has_key(set->config_set, name)) {
            printf("[%s] Warning: The key:'%s' is not part of this set. Ignored!\n", __func__, name);
        } else {
            stringlist_type * formatted_keys = (stringlist_type *) hash_get(set->config_set, name);
            custom_kw_config_deserialize(config, formatted_keys);
        }
    }
    pthread_rwlock_unlock(& set->rw_lock);
}


void custom_kw_config_set_reset(custom_kw_config_set_type * set) {
    pthread_rwlock_wrlock(& set->rw_lock);
    {
        hash_clear(set->config_set);
    }
    pthread_rwlock_unlock(& set->rw_lock);
}

stringlist_type * custom_kw_config_set_get_keys_alloc(custom_kw_config_set_type * set) {
    return hash_alloc_stringlist(set->config_set);
}

void custom_kw_config_set_fwrite(custom_kw_config_set_type * set, const char * filename) {
    pthread_rwlock_rdlock(& set->rw_lock);
    {
        FILE * stream = util_mkdir_fopen(filename, "w");
        if (stream) {
            stringlist_type * keys = hash_alloc_stringlist(set->config_set);
            stringlist_fwrite(keys, stream);

            for (int i = 0; i < stringlist_get_size(keys); i++) {
                const char * key = stringlist_iget(keys, i);
                stringlist_type * formatted_keys = (stringlist_type *) hash_get(set->config_set, key);
                stringlist_fwrite(formatted_keys, stream);
            }

            stringlist_free(keys);
            fclose(stream);
        } else {
            util_abort("%s: failed to open: %s for writing \n", __func__, filename);
        }
    }
    pthread_rwlock_unlock(& set->rw_lock);
}


bool custom_kw_config_set_fread(custom_kw_config_set_type * set, const char * filename) {
    bool file_exists = false;

    pthread_rwlock_wrlock(& set->rw_lock);
    {
        hash_clear(set->config_set);

        if (util_file_exists(filename)) {
            FILE * stream = util_fopen(filename, "r");
            if (stream) {
                stringlist_type * key_set = stringlist_fread_alloc(stream);

                for (int i = 0; i < stringlist_get_size(key_set); i++) {
                    const char * key = stringlist_iget(key_set, i);
                    stringlist_type * config_keys = stringlist_fread_alloc(stream);
                    hash_insert_hash_owned_ref(set->config_set, key, config_keys, stringlist_free__);
                }
                stringlist_free(key_set);
                fclose(stream);
            } else {
                util_abort("%s: failed to open: %s for reading \n", __func__, filename);
            }
            file_exists = true;
        }
    }
    pthread_rwlock_unlock(& set->rw_lock);
    return file_exists;
}
