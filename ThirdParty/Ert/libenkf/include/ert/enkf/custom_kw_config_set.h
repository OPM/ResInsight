#ifndef ERT_CUSTOM_KW_CONFIG_SET_H
#define ERT_CUSTOM_KW_CONFIG_SET_H

#ifdef __cplusplus 
extern "C" {
#endif

#include <ert/enkf/custom_kw_config.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_types.h>

    typedef struct custom_kw_config_set_struct custom_kw_config_set_type;

    custom_kw_config_set_type   * custom_kw_config_set_alloc();
    custom_kw_config_set_type   * custom_kw_config_set_alloc_from_file(const char * filename);
    void                          custom_kw_config_set_free(custom_kw_config_set_type * set);
    void                          custom_kw_config_set_add_config(custom_kw_config_set_type * set, const custom_kw_config_type * config);
    void                          custom_kw_config_set_update_config(custom_kw_config_set_type * set, custom_kw_config_type * config);
    void                          custom_kw_config_set_reset(custom_kw_config_set_type * set);
    stringlist_type             * custom_kw_config_set_get_keys_alloc(custom_kw_config_set_type * set);
    void                          custom_kw_config_set_fwrite(custom_kw_config_set_type * set, const char * filename);
    bool                          custom_kw_config_set_fread(custom_kw_config_set_type * set, const char * filename);

  UTIL_IS_INSTANCE_HEADER(custom_kw_config_set);

#ifdef __cplusplus 
}
#endif
#endif
