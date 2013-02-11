/**
   This file implements a simpler version of the util_abort() function
   which does not present a backtrace.
*/








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

  if (getenv("UTIL_ABORT") != NULL) {
    fprintf(stderr , "Aborting ... \n");
    abort();
  } else {
    fprintf(stderr , "Exiting ... \n");
    exit(1);
  }
    // Would have preferred abort() here - but that comes in conflict with the SIGABRT signal.
}

