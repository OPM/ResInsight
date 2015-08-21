#include <ert/enkf/summary_key_matcher.h>

#include <stdlib.h>
#include <stdbool.h>

#include <ert/util/util.h>
#include <ert/util/hash.h>
#include <ert/util/stringlist.h>
#include <ert/util/type_macros.h>

#include <ert/enkf/enkf_types.h>



#define SUMMARY_KEY_MATCHER_TYPE_ID 700672137

struct summary_key_matcher_struct {
  UTIL_TYPE_ID_DECLARATION;
  hash_type        * key_set;
};


UTIL_IS_INSTANCE_FUNCTION( summary_key_matcher , SUMMARY_KEY_MATCHER_TYPE_ID )


summary_key_matcher_type * summary_key_matcher_alloc() {
  summary_key_matcher_type * matcher = util_malloc(sizeof * matcher);
  UTIL_TYPE_ID_INIT( matcher , SUMMARY_KEY_MATCHER_TYPE_ID);
  matcher->key_set = hash_alloc();
  return matcher;
}

void summary_key_matcher_free(summary_key_matcher_type * matcher) {
    hash_free(matcher->key_set);
    free(matcher);
}

int summary_key_matcher_get_size(const summary_key_matcher_type * matcher) {
  return hash_get_size( matcher->key_set );
}

void summary_key_matcher_add_summary_key(summary_key_matcher_type * matcher, const char * summary_key) {
    if(!hash_has_key(matcher->key_set, summary_key)) {
        hash_insert_int(matcher->key_set, summary_key, !util_string_has_wildcard(summary_key));
    }
}

bool summary_key_matcher_match_summary_key(const summary_key_matcher_type * matcher, const char * summary_key) {
    stringlist_type * keys = hash_alloc_stringlist(matcher->key_set);
    bool has_key = false;

    for (int i = 0; i < stringlist_get_size(keys); i++) {
        const char * pattern = stringlist_iget(keys, i);
        if(util_fnmatch(pattern, summary_key) == 0) {
            has_key = true;
            break;
        }
    }

    stringlist_free(keys);

    return has_key;
}

stringlist_type * summary_key_matcher_get_keys(const summary_key_matcher_type * matcher) {
    return hash_alloc_stringlist(matcher->key_set);
}

bool summary_key_matcher_summary_key_is_required(const summary_key_matcher_type * matcher, const char * summary_key) {
    bool is_required = false;

    if(!util_string_has_wildcard(summary_key) && hash_has_key(matcher->key_set, summary_key)) {
        is_required = (bool) hash_get_int(matcher->key_set, summary_key);
    }

    return is_required;
}