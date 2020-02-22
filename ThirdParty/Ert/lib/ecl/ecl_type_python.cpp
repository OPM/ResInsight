#include <ert/ecl/ecl_type.hpp>
#include <ert/ecl/ecl_kw.hpp>
#include <ert/ecl/ecl_kw_grdecl.hpp>

/**
 *
 * Functions only to be used by the *PYTHON* prototype for EclDataType
 *
 */

#ifdef __cplusplus
extern "C" {

ecl_data_type * ecl_type_alloc_copy_python(const ecl_data_type * src_type) {
    ecl_data_type * data_type = (ecl_data_type*)util_malloc(sizeof * src_type);
    memcpy(data_type, src_type, sizeof * data_type);
    return data_type;
}

ecl_data_type * ecl_type_alloc_python(const ecl_type_enum type, const size_t element_size) {
    ecl_data_type src_type = ecl_type_create(type, element_size);
    return ecl_type_alloc_copy_python(&src_type);
}

ecl_data_type * ecl_type_alloc_from_type_python(const ecl_type_enum type) {
    ecl_data_type src_type = ecl_type_create_from_type(type);
    return ecl_type_alloc_copy_python(&src_type);
}

ecl_data_type * ecl_type_alloc_from_name_python(const char * name) {
    ecl_data_type src_type = ecl_type_create_from_name(name);
    return ecl_type_alloc_copy_python(&src_type);
}

void ecl_type_free_python(ecl_data_type * data_type) {
    free(data_type);
}

ecl_type_enum ecl_type_get_type_python(const ecl_data_type * ecl_type) {
    return ecl_type_get_type(*ecl_type);
}

const char * ecl_type_alloc_name_python(const ecl_data_type * ecl_type) {
    return ecl_type_alloc_name(*ecl_type);
}


int ecl_type_get_sizeof_iotype_python(const ecl_data_type * ecl_type) {
  return ecl_type_get_sizeof_iotype(*ecl_type);
}


int ecl_type_get_sizeof_ctype_python(const ecl_data_type * ecl_type) {
    return ecl_type_get_sizeof_ctype(*ecl_type);
}

bool ecl_type_is_numeric_python(const ecl_data_type * ecl_type) {
    return ecl_type_is_numeric(*ecl_type);
}

bool ecl_type_is_equal_python(const ecl_data_type * ecl_type1,
                       const ecl_data_type * ecl_type2) {
    return ecl_type_is_equal(*ecl_type1, *ecl_type2);
}

bool ecl_type_is_char_python(const ecl_data_type * ecl_type) {
    return ecl_type_is_char(*ecl_type);
}

bool ecl_type_is_int_python(const ecl_data_type * ecl_type) {
    return ecl_type_is_int(*ecl_type);
}

bool ecl_type_is_float_python(const ecl_data_type * ecl_type) {
    return ecl_type_is_float(*ecl_type);
}

bool ecl_type_is_double_python(const ecl_data_type * ecl_type) {
    return ecl_type_is_double(*ecl_type);
}

bool ecl_type_is_mess_python(const ecl_data_type * ecl_type) {
    return ecl_type_is_mess(*ecl_type);
}

bool ecl_type_is_bool_python(const ecl_data_type * ecl_type) {
    return ecl_type_is_bool(*ecl_type);
}

bool ecl_type_is_string_python(const ecl_data_type * ecl_type) {
    return ecl_type_is_string(*ecl_type);
}

/**
 *
 * Functions for the EclKw prototype
 *
 */
ecl_kw_type * ecl_kw_fscanf_alloc_grdecl_dynamic_python( FILE * stream , const char * kw , bool strict , const ecl_data_type * data_type) {
  return ecl_kw_fscanf_alloc_grdecl_dynamic__( stream , kw , strict, *data_type );
}

ecl_kw_type * ecl_kw_alloc_python( const char * header , int size , const ecl_data_type * data_type ) {
  return ecl_kw_alloc(header, size, *data_type);
}

ecl_data_type * ecl_kw_get_data_type_python( const ecl_kw_type * ecl_kw ) {
  ecl_data_type data_type = ecl_kw_get_data_type(ecl_kw);
  return ecl_type_alloc_copy_python(&data_type);
}

void ecl_kw_fread_indexed_data_python(fortio_type * fortio, offset_type data_offset, const ecl_data_type * data_type, int element_count, const int_vector_type* index_map, char* buffer) {
  return ecl_kw_fread_indexed_data(fortio, data_offset, *data_type, element_count, index_map, buffer);
}


}
#endif
