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

#ifdef HAVE_WINDOWS_MKDIR
#include <direct.h>
#endif

#include <stdlib.h>

/**
  This little function checks if the supplied path is an abolute path,
  or a relative path. On posix the check is extremely simple - if the
  first character equals "/" it is interpreted as an abolute path,
  otherwise not.
*/


bool util_is_abs_path(const char * path) {
#ifdef ERT_WINDOWS
  if ((path[0] == '/') || (path[0] == '\\'))
    return true;
  else 
    if ((isalpha(path[0]) && (path[1] == ':')))
      return true;

  return false;

#else

  if (path[0] == UTIL_PATH_SEP_CHAR)
    return true;
  else
    return false;

#endif
}

static int util_mkdir( const char * path ) {
#ifdef HAVE_POSIX_MKDIR
  return mkdir( path , UTIL_DEFAULT_MKDIR_MODE );
#endif

#ifdef HAVE_WINDOWS_MKDIR
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
  const int random_digits = 6;
  const int random_max    = 1000000;

#ifdef HAVE_PID_T
  const int pid_max     = 1000000;
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




char * util_split_alloc_dirname( const char * input_path ) {
  char * path;
  util_alloc_file_components( input_path , &path , NULL , NULL);
  return path;
}


char * util_split_alloc_filename( const char * input_path ) {
  char * filename = NULL;
  {
    char * basename;
    char * extension;

    util_alloc_file_components( input_path , NULL , &basename , &extension);

    if (basename)
      filename = util_alloc_filename( NULL , basename , extension );

    util_safe_free( basename );
    util_safe_free( extension );
  }

  return filename;
}



void util_path_split(const char *line , int *_tokens, char ***_token_list) {
  util_split_string( line , UTIL_PATH_SEP_STRING , _tokens , _token_list);
}

/**
   Observe that
*/

char * util_alloc_parent_path( const char * path) {
  int     path_ncomp;
  char ** path_component_list;
  char *  parent_path = NULL;

  if (path) {
    bool is_abs = util_is_abs_path( path );
    char * work_path;

    if (strstr(path , "..")) {
      if (is_abs)
        work_path = util_alloc_realpath__( path );
      else {
        char * abs_path = util_alloc_realpath__( path );
        char * cwd = util_alloc_cwd();
        work_path = util_alloc_rel_path( cwd , abs_path );
        free( abs_path );
        free( cwd );
      }
    } else
      work_path = util_alloc_string_copy( path );

    util_path_split( work_path , &path_ncomp , &path_component_list );
    if (path_ncomp > 0) {
      int current_length = 4;
      int ip;

      parent_path = util_realloc( parent_path , current_length * sizeof * parent_path);
      parent_path[0] = '\0';

      for (ip=0; ip < path_ncomp - 1; ip++) {
        const char * ipath = path_component_list[ip];
        int min_length = strlen(parent_path) + strlen(ipath) + 1;

        if (min_length >= current_length) {
          current_length = 2 * min_length;
          parent_path = util_realloc( parent_path , current_length * sizeof * parent_path);
        }

        if (is_abs || (ip > 0))
          strcat( parent_path , UTIL_PATH_SEP_STRING );
        strcat( parent_path , ipath );
      }
    }
    util_free_stringlist( path_component_list , path_ncomp );
    free( work_path );
  }
  return parent_path;
}
