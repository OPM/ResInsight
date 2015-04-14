/**
    A small function used to redirect a file descriptior,
    only used as a helper utility for util_fork_exec().
*/

static void __util_redirect(int src_fd , const char * target_file , int open_flags) {
  int new_fd = open(target_file , open_flags , 0644);
  dup2(new_fd , src_fd);
  close(new_fd);
}

/**
   This function does the following:

    1. Fork current process.
    2. if (run_path != NULL) chdir(run_path)
    3. The child execs() to run executable.
    4. Parent can wait (blocking = true) on the child to complete executable.

   If the executable is an absolute path it will run the command with
   execv(), otherwise it will use execvp() which will (try) to look up
   the executable with the PATH variable.

   argc / argv are the number of arguments and their value to the
   external executable. Observe that prior to calling execv the argv
   list is prepended with the name of the executable (convention), and
   a NULL pointer is appended (requirement by execv).

   If stdout_file != NULL stdout is redirected to this file.  Same
   with stdin_file and stderr_file.

   If target_file != NULL, the parent will check that the target_file
   has been created before returning; and abort if not. In this case
   you *MUST* have blocking == true, otherwise it will abort on
   internal error.


   The return value from the function is the pid of the child process;
   this is (obviously ?) only interesting if the blocking argument is
   'false'.

   Example:
   --------
   util_fork_exec("/local/gnu/bin/ls" , 1 , (const char *[1]) {"-l"} ,
   true , NULL , NULL , NULL , "listing" , NULL);


   This program will run the command 'ls', with the argument '-l'. The
   main process will block, i.e. wait until the 'ls' process is
   complete, and the results of the 'ls' operation will be stored in
   the file "listing". If the 'ls' should want to print something on
   stderr, it will go there, as stderr is not redirected.

*/


pid_t util_fork_exec(const char * executable , int argc , const char ** argv ,
                     bool blocking , const char * target_file , const char  * run_path ,
                     const char * stdin_file , const char * stdout_file , const char * stderr_file) {
  const char  ** __argv = NULL;
  pid_t child_pid;

  if (target_file != NULL && blocking == false)
    util_abort("%s: When giving a target_file != NULL - you must use the blocking semantics. \n",__func__);

  child_pid = fork();
  if (child_pid == -1) {
    fprintf(stderr,"Error: %s(%d) \n",strerror(errno) , errno);
    util_abort("%s: fork() failed when trying to run external command:%s \n",__func__ , executable);
  }

  if (child_pid == 0) {
    /* This is the child */
    int iarg;

    nice(19);    /* Remote process is run with nice(19). */
    if (run_path != NULL) {
      if (util_chdir(run_path) != 0)
        util_abort("%s: failed to change to directory:%s  %s \n",__func__ , run_path , strerror(errno));
    }

    if (stdout_file != NULL)
      __util_redirect(1 , stdout_file , O_WRONLY | O_TRUNC | O_CREAT);

    if (stderr_file != NULL)
      __util_redirect(2 , stderr_file , O_WRONLY | O_TRUNC | O_CREAT);

    if (stdin_file  != NULL)
      __util_redirect(0 , stdin_file  , O_RDONLY);

    __argv        = util_malloc((argc + 2) * sizeof * __argv );
    __argv[0]     = executable;
    for (iarg = 0; iarg < argc; iarg++)
      __argv[iarg+1] = argv[iarg];
    __argv[argc + 1] = NULL;

    /*
       If executable is an absolute path, it is invoked directly,
       otherwise PATH is used to locate the executable.
    */
    execvp( executable , (char **) __argv);
    /*
       Exec should *NOT* return - if this code is executed
       the exec??? function has indeed returned, and this is
       an error.
    */
    util_abort("%s: failed to execute external command: \'%s\': %s \n",__func__ , executable , strerror(errno));

  }  else  {
    /* Parent */

    if (blocking) {
      waitpid(child_pid , NULL , 0);

      if (target_file != NULL)
        if (!util_file_exists(target_file))
          util_abort("%s: %s failed to produce target_file:%s aborting \n",__func__ , executable , target_file);
    }
  }

  util_safe_free( __argv );
  return child_pid;
}


/*****************************************************************/


/**
   This function will use the external program /usr/sbin/lsof to
   determine which users currently have 'filename' open. The return
   value will be a (uid_t *) pointer of active users. The number of
   active users is returned by reference.

   * In the current implementation a user can occur several times if
     the user has the file open in several processes.

   * If a NFS mounted file is opened on a remote machine it will not
     appear in this listing. I.e. to check that an executable file can
     be safely modified you must iterate through the relevant
     computers.
*/


uid_t * util_alloc_file_users( const char * filename , int * __num_users) {
  const char * lsof_executable = "/usr/sbin/lsof";
  int     buffer_size = 8;
  int     num_users   = 0;
  uid_t * users       = util_malloc( sizeof * users * buffer_size );
  char * tmp_file     = util_alloc_tmp_file("/tmp" , "lsof" , false);
  util_fork_exec(lsof_executable , 2 , (const char *[2]) {"-F" , filename }, true , NULL , NULL , NULL , tmp_file , NULL);
  {
    FILE * stream = util_fopen(tmp_file , "r");
    while ( true ) {
      int pid , uid;
      char dummy_char;
      if (fscanf( stream , "%c%d %c%d" , &dummy_char , &pid , &dummy_char , &uid) == 4) {
        if (buffer_size == num_users) {
          buffer_size *= 2;
          users        = util_realloc( users , sizeof * users * buffer_size );
        }
        users[ num_users ] = uid;
        num_users++;
      } else
        break; /* have reached the end of file - seems like we will not find the file descriptor we are looking for. */
    }
    fclose( stream );
    remove( tmp_file );
  }
  free( tmp_file );
  users = util_realloc( users , sizeof * users * num_users );
  *__num_users = num_users;
  return users;
}



/**
   This function uses the external program lsof to (try) to associate
   an open FILE * instance with a filename in the filesystem.

   If it succeds in finding the filename the function will allocate
   storage and return a (char *) pointer with the filename. If the
   filename can not be found, the function will return NULL.

   This function is quite heavyweight (invoking an external program
   +++), and also quite fragile, it should therefor not be used in
   routine FILE -> name lookups, rather in situations where a FILE *
   operation has failed extraordinary, and we want to provide as much
   information as possible before going down in flames.
*/

char * util_alloc_filename_from_stream( FILE * input_stream ) {
  char * filename = NULL;
  const char * lsof_executable = "/usr/sbin/lsof";
  int   fd     = fileno( input_stream );

  if (util_is_executable( lsof_executable ) && (fd != -1)) {
    char  * fd_string = util_alloc_sprintf("f%d" , fd);
    char    line_fd[32];
    char    line_file[4096];
    char * pid_string = util_alloc_sprintf("%d" , getpid());
    char * tmp_file   = util_alloc_tmp_file("/tmp" , "lsof" , false);

    /*
       The lsof executable is run as:

       bash% lsof -p pid -Ffn

    */
    util_fork_exec(lsof_executable , 3 , (const char *[3]) {"-p" , pid_string , "-Ffn"}, true , NULL , NULL , NULL , tmp_file , NULL);
    {
      FILE * stream = util_fopen(tmp_file , "r");
      fscanf( stream , "%s" , line_fd);  /* Skipping the first pxxxx marker */
      while ( true ) {
        if (fscanf( stream , "%s %s" , line_fd , line_file) == 2) {
          if (util_string_equal( line_fd , fd_string )) {
            /* We have found the file descriptor we are looking for. */
            filename = util_alloc_string_copy( &line_file[1] );
            break;
          }
        } else
          break; /* have reached the end of file - seems like we will not find the file descriptor we are looking for. */
      }
      fclose( stream );
    }
    remove( tmp_file );
    free( tmp_file );
  }
  return filename;
}


/**
   The ping program must(?) be setuid root, so implementing a simple
   version based on sockets() proved to be nontrivial.

   The PING_CMD is passed as -D from the build system.
*/

#ifdef PING_CMD
bool util_ping(const char *hostname) {
  pid_t ping_pid = util_fork_exec(PING_CMD , 4 , (const char *[4]) {"-c" , "3" , "-q", hostname} , false , NULL , NULL , NULL , "/dev/null" , "/dev/null");
  int wait_status;
  pid_t wait_pid = waitpid(ping_pid , &wait_status , 0);

  if (wait_pid == -1)
    return false;
  else {
    if (WIFEXITED( wait_status )) {
      int ping_status = WEXITSTATUS( wait_status );
      if (ping_status == 0)
        return true;
      else
        return false;
    } else
      return false;
  }
}
#endif




