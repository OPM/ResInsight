#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <ert/util/util.h>

#include <dirent.h>
#include <sys/types.h>

bool util_copy_file__(const char * src_file , const char * target_file, size_t buffer_size , void * buffer , bool abort_on_error);

static void util_copy_directory__(const char * src_path , const char * target_path , int buffer_size , void * buffer ) {
  if (!util_is_directory(src_path))
    util_abort("%s: %s is not a directory \n",__func__ , src_path);
  
  util_make_path(target_path);
  {
    DIR * dirH = opendir( src_path );
    if (dirH == NULL) 
      util_abort("%s: failed to open directory:%s / %s \n",__func__ , src_path , strerror(errno));

    {
      struct dirent * dp;
      do {
        dp = readdir(dirH);
        if (dp != NULL) {
          if (dp->d_name[0] != '.') {
            char * full_src_path    = util_alloc_filename(src_path , dp->d_name , NULL);
            char * full_target_path = util_alloc_filename(target_path , dp->d_name , NULL);
            if (util_is_file( full_src_path )) {
              util_copy_file__( full_src_path , full_target_path , buffer_size , buffer , true);
            } else {
              if (util_is_directory( full_src_path ) && !util_is_link( full_src_path))
                util_copy_directory__( full_src_path , full_target_path , buffer_size , buffer);
            }

            free( full_src_path );
            free( full_target_path );
          }
        }
      } while (dp != NULL);
    }
    closedir( dirH );
  }
}

/*  Does not handle symlinks. */
void util_copy_directory_content(const char * src_path , const char * target_path) {
  int buffer_size = 16 * 1024 * 1024; /* 16 MB */
  void * buffer   = util_malloc( buffer_size );
  
  util_copy_directory__( src_path , target_path , buffer_size , buffer);
  free( buffer );
}

/** 
    Equivalent to shell command cp -r src_path target_path
*/

/*  Does not handle symlinks. */


void util_copy_directory(const char * src_path , const char * __target_path) {
  int     num_components;
  char ** path_parts;
  char  * path_tail;
  char  * target_path;

  util_path_split(src_path , &num_components , &path_parts);
  path_tail   = path_parts[num_components - 1];
  target_path = util_alloc_filename(__target_path , path_tail , NULL);

  util_copy_directory_content(src_path , target_path );
  
  free(target_path);
  util_free_stringlist( path_parts , num_components );
}


/**
   This function will start at 'root_path' and then recursively go
   through all file/subdirectore located below root_path. For each
   file/directory in the tree it will call the user-supplied funtions
   'file_callback' and 'dir_callback'. 

   The arguments to file_callback will be: 

     file_callback(root_path, file , file_callback_arg):

   For the dir_callback function the the depth in the filesystem will
   also be supplied as an argument:

      dir_callback(root_path , directory , depth , dir_callback_arg)

   The dir_callback / file_callback arguments can be NULL. Observe
   that IFF supplied the dir_callback function can be used to stop the
   recursion, if the dir_callback returns false for a particular
   directory the function will not descend into that directory. (If
   dir_callback == NULL it will descend to the bottom irrespectively).


   Example
   -------
   Root
   Root/File1
   Root/File2
   Root/dir
   Root/dir/fileXX
   Root/dir/dir2

   The call:
   util_walk_directory("Root" , file_callback , file_arg , dir_callback , dir_arg);
      
   Will result in the following calls to the callbacks:

      file_callback("Root"     , "File1"  , file_arg); 
      file_callback("Root"     , "File2"  , file_arg); 
      file_callback("Root/dir" , "fileXX" , arg); 

      dir_callback("Root"     , "dir"  , 1 , dir_arg);
      dir_callback("Root/dir" , "dir2" , 2 , dir_arg);

   Symlinks are ignored when descending into subdirectories. The tree
   is walked in a 'width-first' mode (i.e. all the callbacks in a
   directory are evaluated before the function descends further down
   in the tree).

   If we encounter permission denied when opening a directory a
   message is printed on stderr, the directory is ignored and the
   function returns.
*/


static void util_walk_directory__(const char               * root_path , 
                                  bool                       depth_first , 
                                  int                        current_depth  ,
                                  walk_file_callback_ftype * file_callback , 
                                  void                     * file_callback_arg , 
                                  walk_dir_callback_ftype  * dir_callback , 
                                  void                     * dir_callback_arg);


static DIR * util_walk_opendir__( const char * root_path ) {

  DIR * dirH = opendir( root_path );
  if (dirH == NULL) {
    if (errno == EACCES) 
      fprintf(stderr,"** Warning could not open directory:%s - permission denied - IGNORED.\n" , root_path);
    else
      util_abort("%s: failed to open directory:%s / %s \n",__func__ , root_path , strerror(errno));
  }
  return dirH;
}



/**
   This function will evaluate all file_callbacks for all the files in
   the root_path directory.
*/

static void util_walk_file_callbacks__(const char               * root_path , 
                                       int                        current_depth  ,
                                       walk_file_callback_ftype * file_callback , 
                                       void                     * file_callback_arg) {
  

  DIR * dirH = util_walk_opendir__( root_path );
  if (dirH != NULL) {
    struct dirent * dp;
    do {
      dp = readdir(dirH);
      if (dp != NULL) {
        if (dp->d_name[0] != '.') {
          char * full_path    = util_alloc_filename(root_path , dp->d_name , NULL);
          
          if (util_is_file( full_path ) && file_callback != NULL) 
            file_callback( root_path , dp->d_name , file_callback_arg);
          
          free(full_path);
        }
      }
    } while (dp != NULL);
    closedir( dirH );
  }
}



/**
   This function will evaluate the dir_callback for all the (sub)
   directories in root_path, and afterwards descend recursively into
   the subdireectories. It will descend into the subdirectory
   immediately after completing the callback, i.e. it will not do all
   the callbacks first.

   Observe that if the dir_callback function returns false, this sub
   directory will not be descended into. (If dir_callback == NULL that
   amounts to return true.)
*/
  


static void util_walk_descend__(const char               * root_path , 
                                bool                       depth_first ,   
                                int                        current_depth  ,
                                walk_file_callback_ftype * file_callback , 
                                void                     * file_callback_arg , 
                                walk_dir_callback_ftype  * dir_callback , 
                                void                     * dir_callback_arg) {
  
  DIR * dirH = util_walk_opendir__( root_path );
  if (dirH != NULL) {
    struct dirent * dp;
    do {
      dp = readdir(dirH);
      if (dp != NULL) {
        if (dp->d_name[0] != '.') {
          char * full_path    = util_alloc_filename(root_path , dp->d_name , NULL);
          
          if ((util_is_directory( full_path ) && (!util_is_link(full_path)))) {
            bool descend = true;
            if (dir_callback != NULL)
              descend = dir_callback( root_path , dp->d_name , current_depth , dir_callback_arg);
            
            if (descend && util_file_exists(full_path)) /* The callback might have removed it. */
              util_walk_directory__( full_path , depth_first , current_depth + 1 , file_callback, file_callback_arg , dir_callback , dir_callback_arg );
          }
          free( full_path );
        }
      }
    } while (dp != NULL);
    closedir( dirH );
  }
}
 
 

static void util_walk_directory__(const char               * root_path , 
                                  bool                       depth_first , 
                                  int                        current_depth  ,
                                  walk_file_callback_ftype * file_callback , 
                                  void                     * file_callback_arg , 
                                  walk_dir_callback_ftype  * dir_callback , 
                                  void                     * dir_callback_arg) {
  
  if (depth_first) {
    util_walk_descend__( root_path , depth_first , current_depth , file_callback , file_callback_arg , dir_callback , dir_callback_arg );
    util_walk_file_callbacks__( root_path , current_depth , file_callback , file_callback_arg );
  } else {
    util_walk_file_callbacks__( root_path , current_depth , file_callback , file_callback_arg );
    util_walk_descend__( root_path , depth_first , current_depth , file_callback , file_callback_arg , dir_callback , dir_callback_arg);
  }
}



void util_walk_directory(const char               * root_path , 
                         walk_file_callback_ftype * file_callback , 
                         void                     * file_callback_arg , 
                         walk_dir_callback_ftype  * dir_callback , 
                         void                     * dir_callback_arg) {

  bool depth_first = false;

  util_walk_directory__( root_path , depth_first , 0 , file_callback , file_callback_arg , dir_callback , dir_callback_arg);

}

/* End of recursive walk_directory implementation.               */
/*****************************************************************/

