#include <setjmp.h>

static jmp_buf jump_buffer;
static char  * intercept_function = NULL;


jmp_buf * util_abort_test_jump_buffer() {
  return &jump_buffer;
}

void util_abort_test_set_intercept_function(const char * function) {
  intercept_function = util_realloc_string_copy( intercept_function , function );
}


void util_abort_test_intercept( const char * function ) {
  if (intercept_function && (strcmp(function , intercept_function) == 0)) {
    longjmp(jump_buffer , 0 );
  }
}

