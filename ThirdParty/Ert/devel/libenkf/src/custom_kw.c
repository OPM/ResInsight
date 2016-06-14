#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include <ert/util/util.h>
#include <ert/util/matrix.h>
#include <ert/util/bool_vector.h>
#include <ert/util/rng.h>
#include <ert/util/stringlist.h>

#include <ert/ecl/fortio.h>
#include <ert/ecl/ecl_sum.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_util.h>

#include <ert/enkf/enkf_serialize.h>
#include <ert/enkf/enkf_types.h>
#include <ert/enkf/enkf_macros.h>
#include <ert/enkf/enkf_util.h>
#include <ert/enkf/custom_kw_config.h>
#include <ert/enkf/custom_kw.h>
#include <ert/enkf/enkf_fs.h>


struct custom_kw_struct {
    int __type_id;
    custom_kw_config_type * config;
    /* Thin config object - mainly contains filename for remote load */
    stringlist_type * data;                 /* Actual storage - will be casted to double or float on use. */
};


custom_kw_type * custom_kw_alloc(const custom_kw_config_type * config) {
    custom_kw_type * custom_kw = util_malloc(sizeof * custom_kw);
    custom_kw->config = (custom_kw_config_type *) config;
    custom_kw->data = stringlist_alloc_new();

    stringlist_type * keys = custom_kw_config_get_keys(custom_kw->config);
    for(int index = 0; index < stringlist_get_size(keys); index++) {
        const char * key = stringlist_iget(keys, index);
        if(custom_kw_config_key_is_double(custom_kw->config, key)) {
            custom_kw_set_double(custom_kw, key, 0.0);
        } else {
            custom_kw_set_string(custom_kw, key, "");
        }
    }
    
    custom_kw->__type_id = CUSTOM_KW;
    return custom_kw;
}

void custom_kw_free(custom_kw_type * custom_kw) {
    stringlist_free(custom_kw->data);
    free(custom_kw);
}

const stringlist_type * custom_kw_get_data(const custom_kw_type * custom_kw) {
    return custom_kw->data;
}

bool custom_kw_key_is_null(const custom_kw_type * custom_kw, char * key) {
    int index = custom_kw_config_index_of_key(custom_kw->config, key);
    return stringlist_iget(custom_kw->data, index) == NULL;
}


void custom_kw_set_double(custom_kw_type * custom_kw, const char * key, double value) {
    char value_as_string[128];
    sprintf(value_as_string, "%26.100f", value);
    custom_kw_set_string(custom_kw, key, value_as_string);
}


void custom_kw_set_string(custom_kw_type * custom_kw, const char * key, const char * value) {
    int index = custom_kw_config_index_of_key(custom_kw->config, key);
    stringlist_iset_copy(custom_kw->data, index, value);
}


double custom_kw_iget_as_double(const custom_kw_type * custom_kw, int index) {
    double value;

    util_sscanf_double(stringlist_iget(custom_kw->data, index), & value);
    return value;
}

const char * custom_kw_iget_as_string(const custom_kw_type * custom_kw, int index) {
    return stringlist_iget(custom_kw->data, index);
}

custom_kw_config_type * custom_kw_get_config(const custom_kw_type * custom_kw) {
  return custom_kw->config;
}

bool custom_kw_fload(custom_kw_type * custom_kw, const char * filename) {
  return custom_kw_config_parse_result_file(custom_kw->config, filename, custom_kw->data);
}

bool custom_kw_forward_load(custom_kw_type * custom_kw, const char * ecl_file, const forward_load_context_type * load_context) {
  return custom_kw_fload(custom_kw, ecl_file);
}

bool custom_kw_write_to_buffer(const custom_kw_type * custom_kw, buffer_type * buffer, int report_step) {
  stringlist_buffer_fwrite(custom_kw->data, buffer);
  return true;
}

void custom_kw_read_from_buffer(const custom_kw_type * custom_kw, buffer_type * buffer, enkf_fs_type * fs, int report_step) {
    stringlist_buffer_fread(custom_kw->data, buffer);
}

void custom_kw_ecl_write(const custom_kw_type * custom_kw, const char * run_path, const char * base_file, void * filestream) {
    //printf("CustomKW ecl_write\n");
}

void custom_kw_serialize(const custom_kw_type * custom_kw, node_id_type node_id, const active_list_type * active_list, matrix_type * A, int row_offset, int column) {
    printf("CustomKW serialize\n");
}

void custom_kw_deserialize(custom_kw_type * custom_kw, node_id_type node_id, const active_list_type * active_list, const matrix_type * A, int row_offset, int column) {
    printf("CustomKW deserialize\n");
}

/******************************************************************/
/* Anonumously generated functions used by the enkf_node object   */
/******************************************************************/
UTIL_SAFE_CAST_FUNCTION_CONST(custom_kw, CUSTOM_KW)

UTIL_SAFE_CAST_FUNCTION(custom_kw, CUSTOM_KW)

VOID_ALLOC(custom_kw)

VOID_FREE(custom_kw)

VOID_FORWARD_LOAD(custom_kw)

VOID_FLOAD(custom_kw)

VOID_ECL_WRITE(custom_kw)

VOID_READ_FROM_BUFFER(custom_kw);

VOID_WRITE_TO_BUFFER(custom_kw);

VOID_SERIALIZE(custom_kw)

VOID_DESERIALIZE(custom_kw)
