/**
   This file implements a simpler version of the util_abort() function
   which does not present a backtrace.
*/
#include <assert.h>







void util_abort(const char * fmt , ...) {
  fprintf(stderr,"\n-----------------------------------------------------------------\n");
  fprintf(stderr,"A fatal error has been detected and the program will abort.\n\n");
  fprintf(stderr, "Current executable : %s\n" , (__current_executable == NULL) ? "<Not set>" : __current_executable);
  fprintf(stderr, "Version info       : %s\n" , (__abort_program_message == NULL) ? "<Not set>" : __abort_program_message);
  fprintf(stderr, "\nError message: ");
  {  
    va_list ap;
    va_start(ap , fmt);
    vfprintf(stderr , fmt , ap);
    va_end(ap);
  } 
  fprintf(stderr,"-----------------------------------------------------------------\n");

  signal(SIGABRT , SIG_DFL);
  fprintf(stderr , "Aborting ... \n");
  assert(0);
  abort();
}

