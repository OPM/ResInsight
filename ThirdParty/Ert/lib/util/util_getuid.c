#include <stdlib.h>
#include <string.h>

#include <ert/util/util.h>

#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>

uid_t util_get_entry_uid( const char * file ) {
  stat_type buffer;
  util_stat( file , &buffer);
  return buffer.st_uid;
}

mode_t util_getmode( const char * file ) {
  stat_type buffer;
  util_stat( file , &buffer);
  return buffer.st_mode;
}



bool util_chmod_if_owner( const char * filename , mode_t new_mode) {
  stat_type buffer;
  uid_t  exec_uid = getuid(); 
  util_stat( filename , &buffer );
  
  if (exec_uid == buffer.st_uid) {  /* OKAY - the current running uid is also the owner of the file. */
    mode_t current_mode = buffer.st_mode & ( S_IRWXU + S_IRWXG + S_IRWXO );
    if (current_mode != new_mode) {
      chmod( filename , new_mode); /* No error check ... */
      return true;
    }
  }
  
  return false; /* No update performed. */
}





/*
  IFF the current uid is also the owner of the file the current
  function will add the permissions specified in the add_mode variable
  to the file.

  The function simulates the "chmod +???" behaviour of the shell
  command. If the mode of the file is changed the function will return
  true, otherwise it will return false.
*/



bool util_addmode_if_owner( const char * filename , mode_t add_mode) {
  stat_type buffer;
  util_stat( filename , &buffer );
  
  {
    mode_t current_mode = buffer.st_mode & ( S_IRWXU + S_IRWXG + S_IRWXO );
    mode_t target_mode  = (current_mode | add_mode);

    return util_chmod_if_owner( filename , target_mode );
  }
}



/**
   Implements shell chmod -??? behaviour.
*/
bool util_delmode_if_owner( const char * filename , mode_t del_mode) {
  stat_type buffer;
  util_stat( filename , &buffer );
  
  {
    mode_t current_mode = buffer.st_mode & ( S_IRWXU + S_IRWXG + S_IRWXO );
    mode_t target_mode  = (current_mode -  (current_mode & del_mode));
    
    return util_chmod_if_owner( filename , target_mode );
  }
}
  


/**
   Only removes the last component in path.
*/
void static util_clear_directory__( const char *path , bool strict_uid , bool unlink_root) {
  if (util_is_directory(path)) {
    DIR  *dirH = opendir( path );

    if (dirH != NULL) {
      const uid_t uid = getuid();
      struct dirent *dentry;
      
      while ( (dentry = readdir(dirH)) != NULL) {
        stat_type buffer;
        mode_t mode;
        const char * entry_name = dentry->d_name;
        if ((strcmp(entry_name , ".") != 0) && (strcmp(entry_name , "..") != 0)) {
          char * full_path = util_alloc_filename(path , entry_name , NULL);

          if (lstat(full_path , &buffer) == 0) {
            mode = buffer.st_mode;
          
            if (S_ISDIR(mode)) 
              /*
                Recursively descending into sub directory. 
              */
              util_clear_directory__(full_path , strict_uid , true);
            else if (S_ISLNK(mode)) 
              /*
                Symbolic links are unconditionally removed.
              */
              unlink(full_path);
            else if (S_ISREG(mode)) {
              /* 
                 It is a regular file - we remove it (if we own it!).
              */
              if ((!strict_uid) || (buffer.st_uid == uid)) {
                int unlink_return = unlink(full_path);
                if (unlink_return != 0) {
                  /* Unlink failed - we don't give a shit. */
                }
              } 
            }
          }
          free(full_path);
        }
      }
    }
    closedir(dirH);

    /* Finish with clearing the root directory */
    if (unlink_root) {
      int rmdir_return = rmdir(path);
      if (rmdir_return != 0) {
        /* Unlink failed - we don't give a shit. */
      }
    }
  }
}


/**
   This function will clear away all the contents (including
   subdirectories) in the directory @path.

   If the parameter @strict_uid is set to true, the function will only
   attempt to remove entries where the calling uid is also the owner
   of the entry. 

   If the parameter @unlink_root is true the directory @path will also
   be removed, otherwise it will be left as an empty directory.

   The function will just go about deleting as much as it can; errors
   are not signalled in any way!

   The function is in util_getuid() because uid_t and getuid() are so
   important elements of the function.  
*/


void util_clear_directory(const char * path , bool strict_uid , bool unlink_root) {
  util_clear_directory__( path , strict_uid , unlink_root );
}
