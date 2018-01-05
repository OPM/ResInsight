#ifndef ERT_TEMPLATE_TYPE_H
#define ERT_TEMPLATE_TYPE_H

#include <ert/util/ert_api_config.h>

#include <ert/util/subst_list.h>

#ifdef ERT_HAVE_REGEXP
#include <regex.h>
#endif //ERT_HAVE_REGEXP

#define TEMPLATE_TYPE_ID 7781045

struct template_struct {
  UTIL_TYPE_ID_DECLARATION;
  char            * template_file;           /* The template file - if internalize_template == false this filename can contain keys which will be replaced at instantiation time. */
  char            * template_buffer;         /* The content of the template buffer; only has valid content if internalize_template == true. */
  bool              internalize_template;    /* Should the template be loadad and internalized at template_alloc(). */
  subst_list_type * arg_list;                /* Key-value mapping established at alloc time. */
  char            * arg_string;              /* A string representation of the arguments - ONLY used for a _get_ function. */ 
  #ifdef ERT_HAVE_REGEXP
  regex_t start_regexp;
  regex_t end_regexp;
  #endif
};

#ifdef ERT_HAVE_REGEXP
typedef struct loop_struct loop_type;
void template_init_loop_regexp( struct template_struct* );
int template_eval_loop( const struct template_struct* , buffer_type * buffer , int global_offset , struct loop_struct * );
void template_eval_loops( const struct template_struct* template , buffer_type * buffer );
#endif //ERT_HAVE_REGEXP

#endif //ERT_TEMPLATE_TYPE_H
