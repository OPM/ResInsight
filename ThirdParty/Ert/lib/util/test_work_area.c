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

#include <ert/util/ert_api_config.h>

#ifdef ERT_HAVE_GETUID
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <paths.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/test_work_area.h>
#include <ert/util/type_macros.h>
#include <ert/util/rng.h>

#include <ert/util/ert_api_config.h>
#ifdef ERT_HAVE_OPENDIR
#include <sys/types.h>
#include <dirent.h>
#endif

/*
  This file implements a small work area implementation to be used for
  tests which write to disk. The implementation works by creating a
  work area in /tmp and then call chdir() change the current working
  directory.

  An important aspect of this implementation is that test output from
  different users should not come in conflict with e.g. permission
  problems, to achieve this the directories created will be per user.

  When creating the work area you pass in a boolean flag whether you
  want the area to be stored when the destructor is called. After the
  the work_area is destroyed the cwd is changed back to the value it
  had before the area was created.

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
  -- with input flag @store set to true the are on disk will not be
  -- cleared. After the test_work_area_free( ) function has been
  -- called the original cwd will be restored.

  test_work_area_free( work_area );
*/

#define DEFAULT_STORE  false

#define TEST_PATH_FMT  "%s/test/%s/%08d"         /* username/test/test_name/random-integer */
#define FULL_PATH_FMT  "%s/%s"                   /* prefix/test-path */

#define TEST_WORK_AREA_TYPE_ID 1107355

struct test_work_area_struct {
  UTIL_TYPE_ID_DECLARATION;
  bool        store;
  char      * cwd;
  char      * original_cwd;
  bool        change_dir;

  /*
    There have been issues where a test like this:

     1. Create new file in test area.
     2. Open file for reading.

    Fail randomly at step 2 with "File not found", although the file
    is clearly there when checking afterwards. Inspired by this
    article: https:/lwn.net/Articles/457667/ we try to call fsync() on
    the directory file descriptor.
  */
#ifdef ERT_HAVE_OPENDIR
  DIR       * dir_stream;
#endif
  int         dir_fd;
};




static test_work_area_type * test_work_area_alloc__(const char * prefix , const char * test_path, bool change_dir) {
  test_work_area_type * work_area = NULL;

  if (util_is_directory( prefix )) {
    char * test_cwd = util_alloc_sprintf(FULL_PATH_FMT , prefix , test_path );
    util_make_path( test_cwd );
    if (true) {
      work_area = util_malloc( sizeof * work_area );

      UTIL_TYPE_ID_INIT( work_area , TEST_WORK_AREA_TYPE_ID );
      work_area->original_cwd = util_alloc_cwd();
      work_area->cwd = test_cwd;
      work_area->change_dir = change_dir;
      if (change_dir )
        if(util_chdir( work_area->cwd ) != 0)
            util_abort("%s: Failed to move into temporary directory: %s", __func__, test_cwd);

      test_work_area_set_store( work_area , DEFAULT_STORE);

#ifdef ERT_HAVE_OPENDIR
      work_area->dir_stream = opendir( work_area->cwd );
      if (work_area->dir_stream)
        work_area->dir_fd = dirfd( work_area->dir_stream );
      else
        work_area->dir_fd = -1;
#else
      work_area->dir_fd = -1
#endif
    } else
      free( test_cwd );
  }
  return work_area;
}


test_work_area_type * test_work_area_alloc_relative(const char * prefix , const char * test_path) {
  return test_work_area_alloc__(prefix , test_path , true );
}


test_work_area_type * temp_area_alloc_relative(const char * prefix , const char * test_path) {
  return test_work_area_alloc__(prefix , test_path , false );
}


UTIL_IS_INSTANCE_FUNCTION( test_work_area , TEST_WORK_AREA_TYPE_ID)

static test_work_area_type * test_work_area_alloc_with_prefix(const char * prefix , const char * test_name, bool change_dir) {
  if (test_name) {
    rng_type * rng = rng_alloc(MZRAN , INIT_DEV_URANDOM );
#ifdef ERT_HAVE_GETUID
    uid_t uid = getuid();
    struct passwd * pw = getpwuid( uid );
    char * user_name = util_alloc_string_copy( pw->pw_name );
#else
    char * user_name =  util_alloc_sprintf("ert-test-%08d" , rng_get_int(rng , 100000000));
#endif
    char * test_path = util_alloc_sprintf( TEST_PATH_FMT , user_name , test_name , rng_get_int( rng , 100000000 ));
    test_work_area_type * work_area = test_work_area_alloc__( prefix , test_path, change_dir);
    free( test_path );
    rng_free( rng );
    free( user_name );
    return work_area;
  } else
    return NULL;
}


static char * test_work_area_alloc_prefix( ) {
#ifdef HAVE_WINDOWS_GET_TEMP_PATH

  char tmp_path[MAX_PATH];
  GetTempPath( MAX_PATH , tmp_path );
  return util_alloc_string_copy( tmp_path );

#else

  const char * prefix_path = getenv("TMPDIR");

  #ifdef P_tmpdir
  if (!prefix_path)
    prefix_path = P_tmpdir;
  #endif

  if (!prefix_path)
    prefix_path = _PATH_TMP;

  return util_alloc_realpath(prefix_path);

#endif
}


test_work_area_type * test_work_area_alloc(const char * test_name) {
  test_work_area_type * work_area;
  {
    char * tmp_prefix = test_work_area_alloc_prefix( );
    work_area = test_work_area_alloc_with_prefix( tmp_prefix , test_name , true);
    free( tmp_prefix );
  }
  return work_area;
}


test_work_area_type * temp_area_alloc(const char * path) {
  test_work_area_type * work_area;
  {
    char * tmp_prefix = test_work_area_alloc_prefix( );
    work_area = test_work_area_alloc_with_prefix( tmp_prefix , path , false );
    free( tmp_prefix );
  }
  return work_area;
}



void test_work_area_set_store( test_work_area_type * work_area , bool store) {
  work_area->store = store;
}


void test_work_area_sync( test_work_area_type * work_area) {
#ifdef ERT_HAVE_OPENDIR
  if (work_area->dir_fd >= 0)
    fsync( work_area->dir_fd );
#endif
}


void test_work_area_free(test_work_area_type * work_area) {
  if (!work_area->store)
    util_clear_directory( work_area->cwd , true , true );

  if (work_area->change_dir)
    util_chdir( work_area->original_cwd );

#ifdef ERT_HAVE_OPENDIR
  if (work_area->dir_stream)
    closedir( work_area->dir_stream );
#endif

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

char * test_work_area_alloc_input_path( const test_work_area_type * work_area , const char * input_path ) {
  if (util_is_abs_path( input_path ))
    return util_alloc_string_copy( input_path );
  else {
    if (work_area->change_dir)
      return util_alloc_filename( work_area->original_cwd , input_path , NULL);
    else
      return util_alloc_string_copy( input_path );
  }
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
    char * src_file = test_work_area_alloc_input_path( work_area , input_src_file );
    char * src_path = NULL;

    util_alloc_file_components( input_src_file , &src_path , NULL , NULL);
    if (!util_entry_exists( src_path ))
      util_make_path( src_path );

    if (util_file_exists( src_file )) {
      char * target_file   = util_alloc_filename( work_area->cwd , input_src_file , NULL );
      util_copy_file( src_file , target_file );
      free( target_file );
    }
    free( src_file );
    free( src_path );
  }
}


void test_work_area_copy_directory( test_work_area_type * work_area , const char * input_directory) {
  char * src_directory = test_work_area_alloc_input_path( work_area , input_directory );
  util_copy_directory(src_directory , work_area->cwd );
  free( src_directory );
  test_work_area_sync( work_area );
}


void test_work_area_copy_directory_content( test_work_area_type * work_area , const char * input_directory) {
  char * src_directory = test_work_area_alloc_input_path( work_area , input_directory );
  util_copy_directory_content(src_directory , work_area->cwd );
  free( src_directory );
  test_work_area_sync( work_area );
}



void test_work_area_copy_file( test_work_area_type * work_area , const char * input_file) {
  if (input_file) {
    char * src_file = test_work_area_alloc_input_path( work_area , input_file );

    if (util_file_exists( src_file )) {
      char * target_name = util_split_alloc_filename( input_file );
      char * target_file = util_alloc_filename( work_area->cwd , target_name , NULL );
      util_copy_file( src_file , target_file );
      free( target_file );
      free( target_name );
    }
    free( src_file );
    test_work_area_sync( work_area );
  }
}



static bool test_work_area_copy_parent__( test_work_area_type * work_area , const char * input_path, bool copy_content) {
  char * full_path;

  if (util_is_abs_path( input_path ))
    full_path = util_alloc_string_copy( input_path );
  else
    full_path = util_alloc_filename( work_area->original_cwd , input_path , NULL);

  if (util_entry_exists( full_path)) {
    char * parent_path = NULL;

    parent_path = util_alloc_parent_path( full_path );

    if (copy_content)
      test_work_area_copy_directory_content( work_area , parent_path );
    else
      test_work_area_copy_directory( work_area , parent_path );

    free( full_path );
    free( parent_path );
    test_work_area_sync( work_area );
    return true;
  } else {
    free( full_path );
    return false;
  }
}


bool test_work_area_copy_parent_directory( test_work_area_type * work_area , const char * input_path) {
  return test_work_area_copy_parent__( work_area , input_path , false );
}


bool test_work_area_copy_parent_content( test_work_area_type * work_area , const char * input_path) {
  return test_work_area_copy_parent__( work_area , input_path , true );
}
