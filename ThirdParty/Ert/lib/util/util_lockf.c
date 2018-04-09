#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <ert/util/util.h>

#include <fcntl.h>
#include <unistd.h>

/** 
    This function will TRY to aquire an exclusive lock to the file
    filename. If the file does not exist it will be created. The mode
    will be changed to 'mode' (irrespective of whether it exists
    already or not).

    Observe that before the lockf() call we *MUST* succeed in opening
    the file, that means that if we do not have the necessary rights
    to open the file (with modes O_WRONLY + O_CREATE), the function
    will fail hard before even reaching the lockf system call.

    If the lock is aquired the function will return true, otherwise it
    will return false. The lock is only active as long as the lockfile
    is open, we therefore have to keep track of the relevant file
    descriptor; it is passed back to the calling scope through a
    reference. Observe that if the locking fails we close the file
    immediately, and return -1 in the file descriptor argument.

    When the calling scope is no longer interested in locking the
    resource it should close the file descriptor.

    ** Observe that with this locking scheme the existence of a lockfile
    ** is not really interesting. 

    Observe that the lockf() system call, which this function is based
    on, will always succeed in the same process. I.e. this function
    can NOT be used to protect against concurrent update from two
    threads in the same process.
*/
    

bool util_try_lockf(const char * lockfile , mode_t mode , int * __fd) {
  int status;
  int lock_fd;
  lock_fd = open(lockfile , O_WRONLY + O_CREAT , mode); 
  if (lock_fd == -1) 
    util_abort("%s: failed to open lockfile:%s %d/%s\n",__func__ , lockfile,errno , strerror(errno));

  fchmod(lock_fd , mode);
  status = lockf(lock_fd , F_TLOCK , 0);
  if (status == 0) {
    /* We got the lock for exclusive access - all is hunkadory.*/
    *__fd = lock_fd;
    return true;
  } else {
    if (errno == EACCES || errno == EAGAIN) {
      close(lock_fd);
      *__fd = -1;

      return false;
    } else {
      util_abort("%s: lockf() system call failed:%d/%s \n",__func__ , errno , strerror(errno));
      return false; /* Compiler shut up. */
    }
  }
}


/* 
   Opens a file, and locks it for exclusive acces. fclose() will
   release all locks.
*/

FILE * util_fopen_lockf(const char * filename, const char * mode) {
  int flags = 0; /* Compiler shut up */
  int fd;
  int lock_status;

  flags = O_RDWR;  /* Observe that the open call must have write option to be able to place a lock - even though we only attempt to read from the file. */
  if (strcmp(mode , "w") == 0)
    flags += O_CREAT;
  
  fd = open(filename , flags, S_IRUSR|S_IWUSR);
  if (fd == -1) 
    util_abort("%s: failed to open:%s with flags:%d \n",__func__ , filename , flags);
  
  lock_status = lockf(fd , F_LOCK , 0);
  if (lock_status != 0)
    util_abort("%s: failed to lock file: %s %s(%d) \n",__func__ , filename , strerror(errno) , errno);
  
  return fdopen(fd , mode);
}


