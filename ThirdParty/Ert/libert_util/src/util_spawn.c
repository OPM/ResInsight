#include <spawn.h>
#include <pthread.h>
extern char **environ;


#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

static void __init_redirection(posix_spawn_file_actions_t * file_actions, const char *stdout_file, const char *stderr_file) {
  int status;
  status = posix_spawn_file_actions_init(file_actions);

  /* STDIN is unconditionally closed in the child process. */
  status += posix_spawn_file_actions_addclose(file_actions, STDIN_FILENO);

  /* The _addopen() call will first close the fd and then reopen it;
     if no file is specified for stdout/stderr redirect the child will
     send stdout & stderr to whereever the parent process was already
     sending it.
  */
  if (stdout_file)
    status += posix_spawn_file_actions_addopen(file_actions, STDOUT_FILENO, stdout_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

  if (stderr_file)
    status += posix_spawn_file_actions_addopen(file_actions, STDERR_FILENO, stderr_file, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

  if (status != 0)
    util_abort("%s: something failed while setting up redirect stdout:%s  stderr:%s \n",__func__ , stdout_file , stderr_file);

}

#undef STDIN_FILENO
#undef STDERR_FILENO
#undef STDOUT_FILENO



/*
  At least when Python versions newer than 2.7.9 are involved it seems
  to be necessary to protect the access to posix_spawn with a mutex.
*/
static pthread_mutex_t spawn_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
  The util_spawn function will start a new process running
  @executable. The pid of the new process will be
  returned. Alternatively the util_spawn_blocking() function will
  block until the newlye created process has completed.
*/
pid_t util_spawn(const char *executable, int argc, const char **argv, const char *stdout_file, const char *stderr_file) {
  pid_t pid;
  char **__argv = util_malloc((argc + 2) * sizeof *__argv);

  {
    int iarg;
    __argv[0] = (char *) executable;
    for (iarg = 0; iarg < argc; iarg++)
      __argv[iarg + 1] = (char *) argv[iarg];
    __argv[argc + 1] = NULL;
  }

  {
      posix_spawn_file_actions_t file_actions;
      __init_redirection(&file_actions , stdout_file , stderr_file);

      pthread_mutex_lock( &spawn_mutex );
      {
        int spawn_status;
        if (util_is_executable(executable)) { // the executable is in current directory or an absolute path
          spawn_status = posix_spawn(&pid, executable, &file_actions, NULL, __argv, environ);
        } else { // Try to find executable in path
          spawn_status = posix_spawnp(&pid, executable, &file_actions, NULL, __argv, environ);
        }

        if (spawn_status != 0)
          util_abort("%s: failed to spawn external command: \'%s\': %s \n", __func__, executable, strerror(spawn_status));
      }
      pthread_mutex_unlock( &spawn_mutex );
      posix_spawn_file_actions_destroy(&file_actions);
  }

  free(__argv);
  return pid;
}

/*
  Will spawn a new process and wait for its completion. The exit
  status of the new process is returned, observe that exit status 127
  typically means 'File not found' - i.e. the @executable could not be
  found.
*/
int util_spawn_blocking(const char *executable, int argc, const char **argv, const char *stdout_file, const char *stderr_file) {
  int status;
  pid_t pid = util_spawn( executable , argc  ,argv , stdout_file , stderr_file);
  waitpid(pid, &status, 0);
  return status;
}



/**
   The ping program must(?) be setuid root, so implementing a simple
   version based on sockets() proved to be nontrivial.

   The PING_CMD is passed as -D from the build system.
*/

#ifdef ERT_HAVE_PING
bool util_ping(const char *hostname) {
  int wait_status;
  wait_status = util_spawn_blocking(PING_CMD, 4, (const char *[4]) {"-c" , "3" , "-q", hostname}, "/dev/null" , "/dev/null");

  if (WIFEXITED( wait_status )) {
      int ping_status = WEXITSTATUS( wait_status );
      return ping_status == 0;
  } else {
      return false;
  }
}
#endif




