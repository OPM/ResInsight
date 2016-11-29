#ifndef ERT_CUSTOM_KW_H
#define ERT_CUSTOM_KW_H
#ifdef __cplusplus
extern "C" {
#endif

#include <ert/util/util.h>
#include <ert/util/bool_vector.h>
#include <ert/util/double_vector.h>
#include <ert/util/buffer.h>

#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_file.h>

#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/custom_kw_config.h>
#include <ert/enkf/forward_load_context.h>

    typedef struct custom_kw_struct custom_kw_type;

    custom_kw_type         * custom_kw_alloc(const custom_kw_config_type * config);
    void                     custom_kw_free(custom_kw_type * custom_kw);
    custom_kw_config_type  * custom_kw_get_config(const custom_kw_type * custom_kw);
    const stringlist_type  * custom_kw_get_data(const custom_kw_type * custom_kw);
    bool                     custom_kw_key_is_null(const custom_kw_type * custom_kw, char * key);
    double                   custom_kw_iget_as_double(const custom_kw_type * custom_kw, int index);
    const char             * custom_kw_iget_as_string(const custom_kw_type * custom_kw, int index);
    void                     custom_kw_set_string(custom_kw_type * custom_kw, const char * key, const char * value);
    void                     custom_kw_set_double(custom_kw_type * custom_kw, const char * key, double value);

    bool custom_kw_fload(custom_kw_type * custom_kw, const char * filename);
    bool custom_kw_forward_load(custom_kw_type * custom_kw, const char * ecl_file, const forward_load_context_type * load_context);

    bool custom_kw_write_to_buffer(const custom_kw_type * custom_kw, buffer_type * buffer, int report_step);
    void custom_kw_read_from_buffer(const custom_kw_type * custom_kw, buffer_type * buffer, enkf_fs_type * fs, int report_step);
    void custom_kw_ecl_write(const custom_kw_type * custom_kw, const char * run_path, const char * base_file, void * filestream);
    void custom_kw_serialize(const custom_kw_type * custom_kw, node_id_type node_id, const active_list_type * active_list, matrix_type * A, int row_offset, int column);
    void custom_kw_deserialize(custom_kw_type * custom_kw, node_id_type node_id, const active_list_type * active_list, const matrix_type * A, int row_offset , int column);


    UTIL_SAFE_CAST_HEADER(custom_kw);
    UTIL_SAFE_CAST_HEADER_CONST(custom_kw);
    VOID_USER_GET_HEADER(custom_kw);
    VOID_ALLOC_HEADER(custom_kw);
    VOID_FREE_HEADER(custom_kw);
    VOID_FORWARD_LOAD_HEADER(custom_kw)
    VOID_FLOAD_HEADER(custom_kw)
    VOID_ECL_WRITE_HEADER(custom_kw);
    VOID_READ_FROM_BUFFER_HEADER(custom_kw);
    VOID_WRITE_TO_BUFFER_HEADER(custom_kw);
    VOID_SERIALIZE_HEADER(custom_kw)
    VOID_DESERIALIZE_HEADER(custom_kw)
#ifdef __cplusplus
}
#endif
#endif
