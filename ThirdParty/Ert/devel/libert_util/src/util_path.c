/*
   Copyright (C) 2011  Statoil ASA, Norway. 
    
   The file 'util_path.c' is part of ERT - Ensemble based Reservoir Tool. 
    
   ERT is free software: you can redistribute it and/or modify 
   it under the terms of the GNU General Public License as published by 
   the Free Software Foundation, either version 3 of the License, or 
   (at your option) any later version. 
    
   ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
   WARRANTY; without even the implied warranty of MERCHANTABILITY or 
   FITNESS FOR A PARTICULAR PURPOSE.   
    
   See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
   for more details. 
*/

#include <stdlib.h>
/**
  This little function checks if the supplied path is an abolute path,
  or a relative path. The check is extremely simple - if the first
  character equals "/" (on Unix) it is interpreted as an abolute path, 
  otherwise not.
*/


bool util_is_abs_path(const char * path) {
  if (path[0] == UTIL_PATH_SEP_CHAR)
    return true;
  else
    return false;
}

static int util_mkdir( const char * path ) {
#ifdef MKDIR_POSIX
  return mkdir( path , UTIL_DEFAULT_MKDIR_MODE );
#else
  return _mkdir( path );
#endif
}


void util_make_path(const char *_path) {
  char *active_path;
  char *path = (char *) _path;
  int current_pos = 0;
  
  if (!util_is_directory(path)) {
    int i = 0;
    active_path = util_calloc(strlen(path) + 1 , sizeof * active_path );
    do {
      int n = strcspn(path , UTIL_PATH_SEP_STRING);
      if (n < strlen(path))
        n += 1;
      path += n;
      i++;
      strncpy(active_path , _path , n + current_pos); 
      active_path[n+current_pos] = '\0';
      current_pos += n; 
      
      if (!util_is_directory(active_path)) {
        if (util_mkdir(active_path) != 0) { 
          bool fail = false;
          switch (errno) {
          case(EEXIST):
            if (util_is_directory(active_path))
              fail = false;
            break;
          default:
            fail = true;
            break;
          }
          if (fail)
            util_abort("%s: failed to make directory:%s - aborting\n: %s(%d) \n",__func__ , active_path , strerror(errno), errno);
        }
      }
      
    } while (strlen(active_path) < strlen(_path));
    free(active_path);
  }
}


/**
   This function will allocate a unique filename with a random part in
   it. If the the path corresponding to the first argument does not
   exist it is created.

   If the value include_pid is true, the pid of the calling process is
   included in the filename, the resulting filename will be:

      path/prefix-pid-RANDOM

   if include_pid is false the resulting file will be:

      path/prefix-RANDOM

   Observe that IFF the prefix contains any path separator character
   they are translated to "_".
*/



char * util_alloc_tmp_file(const char * path, const char * prefix , bool include_pid ) {
  // Should be reimplemented to use mkstemp() 
  const int pid_digits    = 6;
  const int pid_max       = 1000000;
  const int random_digits = 6;
  const int random_max    = 1000000;

#ifdef HAVE_PID_T  
  pid_t  pid            = getpid() % pid_max;
#else
  int    pid            = 0;
#endif

  char * file           = util_calloc(strlen(path) + 1 + strlen(prefix) + 1 + pid_digits + 1 + random_digits + 1 , sizeof * file );
  char * tmp_prefix     = util_alloc_string_copy( prefix );
  
  if (!util_is_directory(path))
    util_make_path(path);
  util_string_tr( tmp_prefix ,  UTIL_PATH_SEP_CHAR , '_');  /* removing path seps. */
  
  do {
    long int rand_int = rand() % random_max;
    if (include_pid)
      sprintf(file , "%s%c%s-%d-%ld" , path , UTIL_PATH_SEP_CHAR , tmp_prefix , pid , rand_int);
    else
      sprintf(file , "%s%c%s-%ld" , path , UTIL_PATH_SEP_CHAR , tmp_prefix , rand_int);
  } while (util_file_exists(file));   

  free( tmp_prefix );
  return file;
}

/**
   This file allocates a filename consisting of a leading path, a
   basename and an extension. Both the path and the extension can be
   NULL, but not the basename. 
   
   Observe that this function does pure string manipulation; there is
   no input check on whether path exists, if baseneme contains "."
   (or even a '/') and so on.
*/

char * util_alloc_filename(const char * path , const char * basename , const char * extension) {
  char * file;
  int    length = strlen(basename) + 1; 
  
  if (path != NULL) 
    length += strlen(path) + 1;

  if (extension != NULL)
    length += strlen(extension) + 1;

  file = util_calloc(length , sizeof * file );

  if (path == NULL) {
    if (extension == NULL)
      memcpy(file , basename , strlen(basename) + 1);
    else
      sprintf(file , "%s.%s" , basename , extension);
  } else {
    if (extension == NULL)
      sprintf(file , "%s%c%s" , path , UTIL_PATH_SEP_CHAR , basename);
    else
      sprintf(file , "%s%c%s.%s" , path , UTIL_PATH_SEP_CHAR , basename , extension);
  }

  return file;
}


char * util_realloc_filename(char * filename , const char * path , const char * basename , const char * extension) {
  util_safe_free(filename);
  return util_alloc_filename( path , basename , extension );
}




#ifdef HAVE_PROC
bool util_proc_alive(pid_t pid) {
  char proc_path[16];
  sprintf(proc_path , "/proc/%d" , pid);
  return util_is_directory(proc_path);
}
#endif

int util_proc_mem_free(void) {
  FILE *stream = util_fopen("/proc/meminfo" , "r");
  int mem;
  util_fskip_lines(stream , 1);
  util_fskip_token(stream);
  util_fscanf_int(stream , &mem);
  fclose(stream);
  return mem;
}









void util_path_split(const char *line , int *_tokens, char ***_token_list) {
  util_split_string( line , UTIL_PATH_SEP_STRING , _tokens , _token_list);
}
