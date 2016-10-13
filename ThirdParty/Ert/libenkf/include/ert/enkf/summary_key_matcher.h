#ifndef ERT_SUMMARY_KEY_MATCHER_H
#define ERT_SUMMARY_KEY_MATCHER_H

#ifdef __cplusplus 
extern "C" {
#endif 

#include <ert/util/type_macros.h>
#include <ert/util/stringlist.h>

#include <ert/enkf/enkf_types.h>

  typedef struct summary_key_matcher_struct summary_key_matcher_type;

  summary_key_matcher_type * summary_key_matcher_alloc();
  void                       summary_key_matcher_free(summary_key_matcher_type * matcher);
  int                        summary_key_matcher_get_size(const summary_key_matcher_type * matcher);
  void                       summary_key_matcher_add_summary_key(summary_key_matcher_type * matcher, const char * summary_key);
  bool                       summary_key_matcher_match_summary_key(const summary_key_matcher_type * matcher, const char * summary_key);
  bool                       summary_key_matcher_summary_key_is_required(const summary_key_matcher_type * matcher, const char * summary_key);
  stringlist_type *          summary_key_matcher_get_keys(const summary_key_matcher_type * matcher);

  UTIL_IS_INSTANCE_HEADER( summary_key_matcher );

#ifdef __cplusplus 
}
#endif
#endif
