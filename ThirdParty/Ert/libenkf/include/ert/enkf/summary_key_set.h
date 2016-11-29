#ifndef ERT_SUMMARY_KEY_SET_H
#define ERT_SUMMARY_KEY_SET_H

#ifdef __cplusplus 
extern "C" {
#endif 

#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_types.h>

  typedef struct summary_key_set_struct summary_key_set_type;

  summary_key_set_type   * summary_key_set_alloc();
  summary_key_set_type   * summary_key_set_alloc_from_file(const char * filename, bool read_only);
  void                     summary_key_set_free(summary_key_set_type * set);
  int                      summary_key_set_get_size(summary_key_set_type * set);
  bool                     summary_key_set_add_summary_key(summary_key_set_type * set, const char * summary_key);
  bool                     summary_key_set_has_summary_key(summary_key_set_type * set, const char * summary_key);
  stringlist_type *        summary_key_set_alloc_keys(summary_key_set_type * set);
  bool                     summary_key_set_is_read_only(const summary_key_set_type * set);

  void                     summary_key_set_fwrite(summary_key_set_type * set, const char * filename);
  bool                     summary_key_set_fread(summary_key_set_type * set, const char * filename);



  UTIL_IS_INSTANCE_HEADER( summary_key_set );


#ifdef __cplusplus 
}
#endif
#endif
