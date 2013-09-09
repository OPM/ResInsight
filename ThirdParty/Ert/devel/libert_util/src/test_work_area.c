/*
   Copyright (C) 2013  Statoil ASA, Norway. 
   
   The file 'test_work_area.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>


#include <ert/util/test_work_area.h>
#include <ert/util/util.h>
#include <ert/util/rng.h>

/*
  This file implements a small work area implementation to be used for
  tests which write to disk. The implementation works by creating a
  work area in /tmp and then call chdir() change the current working
  directory.

  An important aspect of this implementation is that test output from
  different users should not come in conflict with e.g. permission
  problems, to achieve this the directories created will be per user.

  When creating the work area you pass in a boolean flag whether you
  want the area to be retained when the destructor is called. After
  the the work_area is destroyed the cwd is changed back to the value
  it had before the area was created.

  The functions test_work_area_install_file(),
  test_work_area_copy_directory() and
  test_work_area_copy_directory_content() can be used to populate the
  work area with files and directories needed for the test
  execution. These functions have some intelligence when it comes to
  interpreting relative paths; a relative path input argument is
  interpreted relative to the original cwd.

  Basic usage example:
  --------------------


  -- Create directory /tmp/$USER/ert-test/my/funn/test and call 
  -- chdir() to the newly created directory. 
  test_work_area_type * work_area = test_work_area_alloc("my/funny/test" , true);    

  -- Make files available from the test directory. 
  test_work_area_install_file(work_area , "/home/user/build/test-data/file1");
  test_work_area_install_file(work_area , "relative/path/file2");

  -- Recursively copy directory and directory content into test area:
  test_work_area_copy_directory(work_area , "/home/user/build/test-data/case1");  
  
  ...
  -- Do normal test operations
  ...

  -- Destroy test_work_area structure; since the work_area is created
  -- with input flag @retain set to true the are on disk will not be
  -- cleared. After the test_work_area_free( ) function has been
  -- called the original cwd will be restored.

  test_work_area_free( work_area );
*/
  

#define PATH_FMT       "/tmp/%s/ert-test/%s/%08d"     /* /tmp/username/ert-test/test_name/random-integer */

struct test_work_area_struct {
  bool        retain;
  char      * cwd;
  char      * original_cwd;
};


static test_work_area_type * test_work_area_alloc__(const char * path , bool retain) {
  util_make_path( path );
  if (true) {
    test_work_area_type * work_area = util_malloc( sizeof * work_area );
    work_area->retain = retain;
    work_area->cwd = util_alloc_string_copy( path );
    work_area->original_cwd = util_alloc_cwd();
    chdir( work_area->cwd );  
    
    return work_area;
  }
}



test_work_area_type * test_work_area_alloc(const char * test_name, bool retain) {
  if (test_name) {
    uid_t uid = getuid();
    struct passwd * pw = getpwuid( uid );
    rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
    char * path = util_alloc_sprintf( PATH_FMT , pw->pw_name , test_name , rng_get_int( rng , 100000000 ));
    test_work_area_type * work_area = test_work_area_alloc__( path , retain );
    free( path );
    rng_free( rng );
    return work_area;
  } else 
    return NULL;
}



void test_work_area_free(test_work_area_type * work_area) { 
  if (!work_area->retain)
    util_clear_directory( work_area->cwd , true , true );

  chdir( work_area->original_cwd );
  free( work_area->original_cwd );
  free( work_area->cwd );
  free( work_area );
}


const char * test_work_area_get_cwd( const test_work_area_type * work_area ) {
  return work_area->cwd;
}


const char * test_work_area_get_original_cwd( const test_work_area_type * work_area ) {
  return work_area->original_cwd;
}


/**
   The point of this function is that the test code should be able to
   access the file @input_file independent of the fact that it has
   changed path. If @input_file is an absolute path the function will
   do nothing, if @input_file is a realtive path the function will
   copy @input_file from the location relative to the original cwd to
   the corresponding location relative to the test cwd.
*/

void test_work_area_install_file( test_work_area_type * work_area , const char * input_src_file ) {
  if (util_is_abs_path( input_src_file ))
    return;
  else {
    char * src_file = util_alloc_filename( work_area->original_cwd , input_src_file , NULL );
    char * src_path;
    
    util_alloc_file_components( input_src_file , &src_path , NULL , NULL);
    if (!util_entry_exists( src_path ))
      util_make_path( src_path );

    if (util_file_exists( src_file )) {
      char * target_file   = util_alloc_filename( work_area->cwd , input_src_file , NULL );
      util_copy_file( src_file , target_file );
      free( target_file );
    }
    free( src_file );
  }
}


void test_work_area_copy_directory( test_work_area_type * work_area , const char * input_directory) {
  char * src_directory;

  if (util_is_abs_path( input_directory ))
    src_directory = util_alloc_string_copy( input_directory );
  else
    src_directory = util_alloc_filename( work_area->original_cwd , input_directory , NULL);

  util_copy_directory(src_directory , work_area->cwd );
  free( src_directory );
}


void test_work_area_copy_directory_content( test_work_area_type * work_area , const char * input_directory) {
  char * src_directory;

  if (util_is_abs_path( input_directory ))
    src_directory = util_alloc_string_copy( input_directory );
  else
    src_directory = util_alloc_filename( work_area->original_cwd , input_directory , NULL);

  util_copy_directory_content(src_directory , work_area->cwd );
  free( src_directory );
}



void test_work_area_copy_file( test_work_area_type * work_area , const char * input_file) {
  if (input_file) {
    char * src_file;

    if (util_is_abs_path( input_file )) 
      src_file = util_alloc_string_copy( input_file );
    else
      src_file = util_alloc_filename( work_area->original_cwd , input_file , NULL);
    
    if (util_file_exists( src_file )) {
      char * target_file = util_split_alloc_filename( input_file );
      util_copy_file( src_file , target_file );
    }
    free( src_file );
  }
}


