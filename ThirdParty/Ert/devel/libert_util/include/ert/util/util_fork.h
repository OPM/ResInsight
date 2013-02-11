/**
   Headers for the functions in util_fork.c - i.e. the functions which
   are based on the fork() system call.  

   This file will be included directly from the util.h header file.
*/

pid_t    util_fork_exec(const char *  , int , const char ** , bool , const char * , const char *  , const char * , const char *  , const char * );
uid_t  * util_alloc_file_users( const char * filename , int * __num_users);
char   * util_alloc_filename_from_stream( FILE * input_stream );

