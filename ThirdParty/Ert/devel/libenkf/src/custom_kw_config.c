#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <ert/util/util.h>
#include <ert/util/int_vector.h>
#include <ert/util/bool_vector.h>
#include <ert/util/string_util.h>
#include <ert/util/type_macros.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>

#include <ert/config/config_parser.h>
#include <ert/ecl/ecl_util.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_fs.h>
#include <ert/enkf/config_keys.h>
#include <ert/enkf/enkf_defaults.h>

#include <ert/enkf/custom_kw_config.h>


#define CUSTOM_KW_CONFIG_ID 90051933

struct custom_kw_config_struct {
    CONFIG_STD_FIELDS;
    char * name;
    char * result_file;
    char * output_file;

    hash_type * custom_keys;
    hash_type * custom_key_types; /* True if float */
    bool undefined;
    char * key_definition_file;

    pthread_rwlock_t rw_lock;
};


custom_kw_config_type * custom_kw_config_alloc_empty(const char * key, const char * result_file, const char * output_file) {
    custom_kw_config_type * custom_kw_config = util_malloc(sizeof * custom_kw_config);
    UTIL_TYPE_ID_INIT(custom_kw_config, CUSTOM_KW_CONFIG_ID);

    custom_kw_config->name = NULL;
    custom_kw_config->result_file = util_alloc_string_copy(result_file);
    custom_kw_config->output_file = util_alloc_string_copy(output_file);
    custom_kw_config->name = util_alloc_string_copy(key);
    custom_kw_config->undefined = true;
    custom_kw_config->key_definition_file = NULL;

    custom_kw_config->custom_keys = hash_alloc();
    custom_kw_config->custom_key_types = hash_alloc(); //types: 0 if string 1 if double
    pthread_rwlock_init(& custom_kw_config->rw_lock, NULL);

    return custom_kw_config;
}

void custom_kw_config_free(custom_kw_config_type * config) {
    util_safe_free(config->name);
    util_safe_free(config->result_file);
    util_safe_free(config->output_file);
    util_safe_free(config->key_definition_file);

    hash_free(config->custom_keys);
    hash_free(config->custom_key_types);

    pthread_rwlock_destroy(& config->rw_lock);

    free(config);
}

static void custom_kw_config_reset__(custom_kw_config_type * config) {
    config->undefined = true;
    hash_clear(config->custom_keys);
    hash_clear(config->custom_key_types);
    util_safe_free(config->key_definition_file);
    config->key_definition_file = NULL;
}

void custom_kw_config_serialize(const custom_kw_config_type * config, stringlist_type * config_set) {
    pthread_rwlock_t * rw_lock = (pthread_rwlock_t *)& config->rw_lock;
    pthread_rwlock_rdlock(rw_lock);
    {
        stringlist_clear(config_set);

        stringlist_type * configured_keys = custom_kw_config_get_keys(config);

        for (int i = 0; i < stringlist_get_size(configured_keys); i++) {
            const char * key = stringlist_iget(configured_keys, i);
            bool double_type = custom_kw_config_key_is_double(config, key);
            int index = custom_kw_config_index_of_key(config, key);
            char buffer[256];

            sprintf(buffer, "%s %d %d", key, index, double_type);
            stringlist_append_copy(config_set, buffer);
        }

        stringlist_free(configured_keys);

    }
    pthread_rwlock_unlock(rw_lock);
}

void custom_kw_config_deserialize(custom_kw_config_type * config, stringlist_type * config_set) {
    pthread_rwlock_wrlock(& config->rw_lock);
    {
        custom_kw_config_reset__(config);

        for (int i = 0; i < stringlist_get_size(config_set); i++) {
            const char * items = stringlist_iget(config_set, i);

            char key[128];
            int index;
            int is_double;

            int count = sscanf(items, "%s %d %d", key, &index, &is_double);
            
            if (count == 3) {
              hash_insert_int(config->custom_keys, key, index);
              hash_insert_int(config->custom_key_types, key, is_double);
            } else
              util_abort("%s: internal error - deserialize failed\n",__func__);
        }
        config->undefined = false;
        config->key_definition_file = util_alloc_string_copy("from storage"); //Todo: Handle this differently?
    }
    pthread_rwlock_unlock(& config->rw_lock);
}

int custom_kw_config_size(const custom_kw_config_type * config) {
    return hash_get_size(config->custom_keys);
}

const char * custom_kw_config_get_name(const custom_kw_config_type * config) {
    return config->name;
}

char * custom_kw_config_get_result_file(const custom_kw_config_type * config) {
    return config->result_file;
}

char * custom_kw_config_get_output_file(const custom_kw_config_type * config) {
    return config->output_file;
}

bool custom_kw_config_has_key(const custom_kw_config_type * config, const char * key) {
    return hash_has_key(config->custom_keys, key);
}

bool custom_kw_config_key_is_double(const custom_kw_config_type * config, const char * key) {
    return hash_get_int(config->custom_key_types, key) == 1;
}

int custom_kw_config_index_of_key(const custom_kw_config_type * config, const char * key) {
    return hash_get_int(config->custom_keys, key);
}

stringlist_type * custom_kw_config_get_keys(const custom_kw_config_type * config) {
    return hash_alloc_stringlist(config->custom_keys);
}

static bool custom_kw_config_setup__(custom_kw_config_type * config, const char * result_file) {
    FILE * stream = util_fopen__(result_file, "r");
    if (stream != NULL) {
        bool read_ok = true;
        config->key_definition_file = util_alloc_string_copy(result_file);

        int counter = 0;
        char key[128];
        char value[128];
        int read_count;
        while ((read_count = fscanf(stream, "%s %s", key, value)) != EOF) {
            if (read_count == 1) {
                printf("[%s] Warning: Key: '%s:%s' is missing value in file: '%s'\n", __func__, config->name, key, result_file);
                read_ok = false;
                break;
            }

            if (custom_kw_config_has_key(config, key)) {
                printf("[%s] Warning: Key: '%s:%s' already defined!\n", __func__, config->name, key);
            } else {
                hash_insert_int(config->custom_keys, key, counter++);
                hash_insert_int(config->custom_key_types, key, util_sscanf_double(value, NULL));
            }
        }

        fclose(stream);
        return read_ok;
    }
    return false;
}

static bool custom_kw_config_read_data__(const custom_kw_config_type * config, const char * result_file, stringlist_type * result) {
    FILE * stream = util_fopen__(result_file, "r");
    if (stream != NULL) {
        bool read_ok = true;

        stringlist_clear(result);
        stringlist_iset_ref(result, hash_get_size(config->custom_keys) - 1, NULL);
        hash_type * read_keys = hash_alloc();

        char key[128];
        char value[128];
        int read_count;
        while ((read_count = fscanf(stream, "%s %s", key, value)) != EOF) {
            if (read_count == 1) {
                printf("[%s] Warning: Key: '%s:%s' missing value in file: %s!\n", __func__, config->name, key, result_file);
                read_ok = false;
                break;
            }

            if (custom_kw_config_has_key(config, key)) {
                if (hash_has_key(read_keys, key)) {
                    printf("[%s] Warning:  Key: '%s:%s' has appeared multiple times. Only the last occurrence will be used!\n", __func__, config->name, key);
                }

                hash_insert_int(read_keys, key, 1);
                int index = custom_kw_config_index_of_key(config, key);
                stringlist_iset_copy(result, index, value);

            } else {
                printf("[%s] Warning: Key: '%s:%s' not in the available set. Ignored!\n", __func__, config->name, key);
            }
        }

        fclose(stream);

        if (read_ok) {
            read_ok = hash_key_list_compare(read_keys, config->custom_keys);
        }

        return read_ok;
    }
    return false;
}

bool custom_kw_config_parse_result_file(custom_kw_config_type * config, const char * result_file, stringlist_type * result) {
    bool read_ok = true;

    pthread_rwlock_wrlock(& config->rw_lock);
    if (config->undefined) {
        read_ok = custom_kw_config_setup__(config, result_file);
        if (read_ok) {
            config->undefined = false;
        }
    }
    pthread_rwlock_unlock(& config->rw_lock);

    if (read_ok) {
        read_ok = custom_kw_config_read_data__(config, result_file, result);
    }

    return read_ok;
}



/*****************************************************************/

UTIL_IS_INSTANCE_FUNCTION(custom_kw_config, CUSTOM_KW_CONFIG_ID)

UTIL_SAFE_CAST_FUNCTION(custom_kw_config, CUSTOM_KW_CONFIG_ID)

UTIL_SAFE_CAST_FUNCTION_CONST(custom_kw_config, CUSTOM_KW_CONFIG_ID)



/*****************************************************************/

VOID_FREE(custom_kw_config)
