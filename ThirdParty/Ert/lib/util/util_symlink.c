#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "ert/util/build_config.h"
#include <ert/util/ert_api_config.h>
#include <ert/util/util.h>

#ifndef ERT_HAVE_SYMLINK

bool util_is_link(const char * path) {
  return false;
}

char * util_alloc_link_target(const char * link) {
  return util_alloc_string_copy( link );
}

#else

#include <dirent.h>
#include <unistd.h>

void util_make_slink(const char *target , const char * link) {
  if (util_file_exists(link)) {
    if (util_is_link(link)) {
      if (!util_same_file(target , link)) 
        util_abort("%s: %s already exists - not pointing to: %s - aborting \n",__func__ , link , target);
    } else 
      util_abort("%s: %s already exists - is not a link - aborting \n",__func__ , link);
  } else {
    if (symlink(target , link) != 0) 
      util_abort("%s: linking %s -> %s failed - aborting: %s \n",__func__ , link , target , strerror(errno));
  }
}


/**
  This function returns true if path is a symbolic link.
*/

bool util_is_link(const char * path) {
  stat_type stat_buffer;
  if (lstat(path , &stat_buffer) == 0)
    return S_ISLNK(stat_buffer.st_mode);
  else if (errno == ENOENT)
    /*Path does not exist at all. */
    return false;
  else {
    util_abort("%s: stat(%s) failed: %s \n",__func__ , path , strerror(errno));
    return false;
  }
}


/**
   A wrapper around readlink() which will:

     1. Allocate a sufficiently large buffer - reallocating if necessary.
     2. Append a terminating \0 at the end of the return string.

   If the input argument is not a symbolic link the function will just
   return a string copy of the input.
*/

char * util_alloc_link_target(const char * link) {
  if (util_is_link( link )) {
    bool retry = true;
    int target_length;
    int buffer_size = 256;
    char * target   = NULL;
    do {
      target        = util_realloc(target , buffer_size );
      target_length = readlink(link , target , buffer_size);
      
      if (target_length == -1) 
        util_abort("%s: readlink(%s,...) failed with error:%s - aborting\n",__func__ , link , strerror(errno));
      
      if (target_length < (buffer_size - 1))   /* Must leave room for the trailing \0 */
        retry = false;
      else
        buffer_size *= 2;
      
    } while (retry);
    target[target_length] = '\0';
    target = util_realloc( target , strlen( target ) + 1 );   /* Shrink down to accurate size. */
    return target;
  } else
    return util_alloc_string_copy( link );
}

#ifdef ERT_HAVE_READLINKAT

/*
  The manual page says that the readlinkat() function should be in the
  unistd.h header file, but not on RedHat5. On RedHat6 it is.

*/
extern ssize_t readlinkat (int __fd, __const char *__restrict __path, char *__restrict __buf, size_t __len);

char * util_alloc_atlink_target(const char * path , const char * link) {
  if (util_is_abs_path( link ))
    return util_alloc_link_target( link );
  else {
    char * target   = NULL;
    DIR * dir;
    int   path_fd;

    dir = opendir( path );
    if (dir == NULL)
      return NULL;
    
    path_fd = dirfd( dir );
    if (path_fd == -1)
      return NULL;
    
    {
      bool retry = true;
      int target_length;
      int buffer_size = 256;
      do {
        target        = util_realloc(target , buffer_size );
        target_length = readlinkat( path_fd , link , target , buffer_size);
        
        if (target_length == -1) 
          util_abort("%s: readlinkat(%s,...) failed with error:%s - aborting\n",__func__ , link , strerror(errno));
        
        if (target_length < (buffer_size - 1))   /* Must leave room for the trailing \0 */
          retry = false;
        else
          buffer_size *= 2;
        
      } while (retry);
      target[target_length] = '\0';
      target = util_realloc( target , strlen( target ) + 1 );   /* Shrink down to accurate size. */
    } 
    closedir( dir );
    return target;
  }
}
#endif

#endif // ERT_HAVE_SYMLINK
