#ifndef ERT_CUSTOM_KW_CONFIG_H
#define ERT_CUSTOM_KW_CONFIG_H
#ifdef __cplusplus 
extern "C" {
#endif
#include <stdbool.h>

#include <ert/util/stringlist.h>
#include <ert/util/util.h>
#include <ert/util/bool_vector.h>
#include <ert/util/hash.h>

#include <ert/enkf/enkf_fs_type.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_macros.h>

    typedef struct custom_kw_config_struct custom_kw_config_type;

    custom_kw_config_type * custom_kw_config_alloc_empty(const char * key, const char * result_file, const char * output_file);
    custom_kw_config_type * custom_kw_config_alloc_with_definition(const char * key, const hash_type * definition);
    void                    custom_kw_config_free(custom_kw_config_type * config);
    const char *            custom_kw_config_get_name(const custom_kw_config_type * config);
    char *                  custom_kw_config_get_result_file(const custom_kw_config_type * config);
    char *                  custom_kw_config_get_output_file(const custom_kw_config_type * config);
    bool                    custom_kw_config_parse_result_file(custom_kw_config_type * config, const char * result_file, stringlist_type * result);
    void                    custom_kw_config_serialize(const custom_kw_config_type * config, stringlist_type * config_set);
    void                    custom_kw_config_deserialize(custom_kw_config_type * config, stringlist_type * config_set);
    bool                    custom_kw_config_has_key(const custom_kw_config_type * config, const char * key);
    bool                    custom_kw_config_key_is_double(const custom_kw_config_type * config, const char * key);
    int                     custom_kw_config_index_of_key(const custom_kw_config_type * config, const char * key);
    int                     custom_kw_config_size(const custom_kw_config_type * config);
    stringlist_type *       custom_kw_config_get_keys(const custom_kw_config_type * config);


    UTIL_IS_INSTANCE_HEADER(custom_kw_config);
    UTIL_SAFE_CAST_HEADER(custom_kw_config);
    UTIL_SAFE_CAST_HEADER_CONST(custom_kw_config);
    VOID_FREE_HEADER(custom_kw_config)
  
#ifdef __cplusplus
}
#endif
#endif
