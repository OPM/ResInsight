/*
   Copyright (C) 2012  Statoil ASA, Norway. 
    
   The file 'latex.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>

#include <util.h>
#include <latex.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LATEX_CMD
#define __LATEX_CMD LATEX_CMD
#else
#define __LATEX_CMD "/usr/bin/pdflatex"
#endif


#define TARGET_EXTENSION "pdf"
#define RUNPATH_FMT      "/tmp/latex-XXXXXX"
#define DEFAULT_TIMEOUT  60

static const char* delete_extensions[] = { "log", 
                                           "aux",
                                           "nav",
                                           "out",
                                           "snm",
                                           "toc" };

struct latex_struct {
  char * latex_cmd;
  int    timeout;  // Seconds to wait for the compilation to complete.
  bool   in_place;
  
  char * run_path;
  char * src_file;
  char * result_file;

  char * basename;
  char * target_extension;
  char * target_file;
  char * target_path;
};



void latex_set_command( latex_type * latex , const char * latex_cmd) {
  latex->latex_cmd = util_realloc_string_copy( latex->latex_cmd , latex_cmd );
}

static void latex_set_target_extension( latex_type * latex , const char * target_extension) {
  latex->target_extension = util_realloc_string_copy( latex->target_extension , target_extension );
}

static void latex_set_target_path( latex_type * latex , const char * target_path) {
  latex->target_path = util_realloc_string_copy( latex->target_path , target_path );
}


void latex_set_target_file( latex_type * latex , const char * target_file ) {
  latex->target_file = util_realloc_string_copy( latex->target_file , target_file );
  {
    char * full_path = util_alloc_abs_path( target_file );
    free( latex->target_path );
    util_safe_free( latex->target_extension );

    util_alloc_file_components( full_path , &latex->target_path , NULL , &latex->target_extension );
    free( full_path );
  }
}




latex_type * latex_alloc( const char * input_file , bool in_place ) {
  char * input_path;
  char * input_extension;
  latex_type * latex = util_malloc( sizeof * latex );
  
  latex->in_place = in_place;
  util_alloc_file_components( input_file , &input_path , &latex->basename , &input_extension );
  if (input_path == NULL)
    input_path = util_alloc_cwd( );
  else {
    char * tmp = input_path;
    input_path = util_alloc_abs_path( tmp );
    free( tmp );
  }
  
  if (!util_string_equal( input_extension , LATEX_EXTENSION ))
    util_abort("%s: sorry - the input file must have extension .tex \n",__func__);
  
  if (in_place) 
    latex->run_path = util_alloc_abs_path( input_path );
  else {
    latex->run_path = util_alloc_string_copy( RUNPATH_FMT );
    if (mkdtemp( latex->run_path ) == NULL)
      util_abort("%s: failed to create temporary directory with template:%s\n",__func__ , RUNPATH_FMT);
    {
      char * src_file = util_alloc_filename( latex->run_path , latex->basename , LATEX_EXTENSION );
      util_copy_file( input_file , src_file );
      free( src_file );
    }
    latex_link_directory_content( latex , input_path );
  }
  latex->src_file    = util_alloc_filename( NULL , latex->basename , LATEX_EXTENSION );
  /*
    At this stage the following path variables should be correctly set:

    latex->basename : The basename of the file we are compiling.

    latex->src_file : The name of the file we are compiling; with a .tex extension, 
                      but without any leading path part.

    latex->run_path : The directory where the compilation will be performed.
  */                  

  /*
    Setting various default values.
  */
  latex->target_file      = NULL;
  latex->latex_cmd        = NULL;
  latex->target_extension = NULL;
  latex->target_path      = NULL;
  latex->result_file      = NULL;

  latex_set_target_extension( latex , TARGET_EXTENSION );
  latex_set_target_path( latex , input_path );
  latex_set_command( latex , __LATEX_CMD );
  latex_set_timeout( latex , DEFAULT_TIMEOUT );

  free( input_extension );
  free( input_path );
  return latex;
}


/**
   Whether the run was successfull or not is only checked by looking
   at the mtime of the result file. If this is greater than the time
   of the compilation start the compilation is deemed a success.
*/

static bool latex_inspect_run( latex_type * latex , time_t compile_start ) {
  bool OK = false;
  
  {
    char * result_file = util_alloc_filename( latex->run_path , latex->basename , latex->target_extension );
    if (util_file_exists( result_file )) {
      if (!util_file_older( result_file , compile_start ))
        OK = true;
    }     
    free( result_file );
  }
  
  return OK;
}

static void latex_copy_target( latex_type * latex ) {
  if (!util_same_file(latex->target_file , latex->result_file)) 
    util_copy_file( latex->result_file , latex->target_file );
}


static void latex_cleanup( latex_type * latex ) {
  if (latex->in_place) {
    int num_extensions = sizeof( delete_extensions ) / sizeof( delete_extensions[0] );
    for (int iext = 0; iext < num_extensions; iext++) {
      char * filename = util_alloc_filename( latex->run_path , latex->basename , delete_extensions[iext]);
      unlink(filename);
      free( filename );
    }
  } else 
    util_clear_directory(latex->run_path , true , true );
}



static void latex_ensure_target_file( latex_type * latex) {
  if (latex->target_file == NULL) 
    latex->target_file = util_alloc_filename( latex->target_path , latex->basename , latex->target_extension);
}

static bool latex_compile__( latex_type * latex , bool ignore_errors) {
  bool    normal_exit = true;
  char ** argv;
  int     argc;
  char  * stderr_file = util_alloc_filename( latex->run_path , "latex" , "stderr");
  char  * stdout_file = util_alloc_filename( latex->run_path , "latex" , "stdout");
  int     usleep_time = 500000;  /* 1/2 second. */
    
  argc = 2;
  argv = util_malloc( argc * sizeof * argv );
  if (ignore_errors)
    argv[0] = "-interaction=nonstopmode";
  else
    argv[0] = "-halt-on-error";
  argv[0] = "-halt-on-error";//latex->src_file;

  argv[1] = latex->src_file;
  {
    pid_t  child_pid  = util_fork_exec( latex->latex_cmd , argc  , (const char **) argv , false , NULL , latex->run_path , NULL , stdout_file , stderr_file );
    double total_wait = 0;
    int status;

    while (true) {
      if (waitpid(child_pid , &status , WNOHANG) == 0) {
        util_usleep( usleep_time );
        total_wait += usleep_time / 1000000.0;
        
        if (total_wait > latex->timeout) {
          // Exit due to excessive time usage.
          normal_exit = false;
          kill( child_pid , SIGKILL );
        }
      } else 
        // The child has exited - succesfull or not?
        break;
    }
  }
  
  free( stderr_file );
  free( stdout_file );
  free( argv );
  
  return normal_exit;
}

/**
   Unfortunately it is very difficult to determine whether a latex
   compilation has been successfull or not.
*/

bool latex_compile( latex_type * latex , bool ignore_errors , bool with_xref) {
  int num_compile = 1;
  time_t compile_start;
  
  time( &compile_start );
  latex_ensure_target_file( latex );
  latex->result_file = util_alloc_filename( latex->run_path , latex->basename , latex->target_extension );

  /* If we are compiling out of place we might have created a symlink
     from the compile path to an old version of the target file.  */
  if (util_is_link(latex->result_file))
    unlink( latex->result_file );

  if (with_xref)
    num_compile = 2;

  {
    bool normal_exit = true;
    int compile_num = 0;
    while ((normal_exit) && (compile_num < num_compile)) {
      normal_exit = latex_compile__( latex , ignore_errors );
      compile_num++;
    }
  
    /* 
       OK - the compilation is finished; currently we do not have a clue
       whether it was a success or not.
    */
    
    {
      bool success = false;
      if (normal_exit)
        success = latex_inspect_run( latex , compile_start );
      
      if (success) {
        latex_copy_target( latex );
        latex_cleanup( latex );
      }
  
      return success;
    }
  }
}


int latex_get_timeout( const latex_type * latex ) {
  return latex->timeout;
}

void latex_set_timeout( latex_type * latex , int timeout) {
  latex->timeout = timeout;
}


void latex_free( latex_type * latex ) {

  free( latex->latex_cmd );
  free( latex->run_path );
  free( latex->src_file );
  free( latex->basename );
  free( latex->target_extension );
  free( latex->target_path );
  
  util_safe_free( latex->target_file );
  util_safe_free( latex->result_file );
  
  free( latex );
}


const char * latex_get_runpath( const latex_type * latex ) {
  return latex->run_path;
}

const char * latex_get_target_file( const latex_type * latex ) {
  return latex->target_file;
}


void latex_link_path( const latex_type * latex , const char * path) {
  char * target_name;
  {
    char * tmp_copy = util_alloc_string_copy( path );
    target_name = util_alloc_string_copy( basename( tmp_copy ));
    free( tmp_copy );
  }
  {
    char * link   = util_alloc_filename( latex->run_path , target_name , NULL);
    char * target = util_alloc_abs_path( path );
    
    if (!util_entry_exists( link )) 
      util_make_slink( target , link );
    
    free( link );
    free( target );
  }
  util_safe_free( target_name );
}


void latex_link_directory_content(const latex_type * latex , const char * path) {
  if (util_is_directory( path )) {
    DIR * dirH = opendir( path );
    struct dirent * dentry;
    
    while ( true ) {
      dentry = readdir( dirH );
      if (dentry != NULL) {
        if ((strcmp(dentry->d_name , ".") != 0) && (strcmp(dentry->d_name , "..") != 0)) {
          char * full_path = util_alloc_filename( path , dentry->d_name , NULL );
          
          latex_link_path( latex , full_path );
          
          free( full_path );
        }
      } else
        break;
    }
    closedir( dirH );
  }
}


#ifdef __cplusplus
}
#endif


