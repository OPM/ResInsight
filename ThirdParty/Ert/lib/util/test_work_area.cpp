/*
   Copyright (C) 2013  Equinor ASA, Norway.

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

#include <ert/util/ert_api_config.hpp>

#ifdef ERT_HAVE_GETUID
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <paths.h>
#endif

#include <ert/util/ert_api_config.hpp>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>

#include "detail/util/path.hpp"

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



namespace ecl {
namespace util {

static bool test_work_area_copy_parent__( const TestArea * work_area , const std::string& input_path, bool copy_content) {
  char * full_path;

  if (util_is_abs_path( input_path.c_str() ))
    full_path = util_alloc_string_copy( input_path.c_str() );
  else
    full_path = util_alloc_filename( work_area->original_cwd().c_str( ) , input_path.c_str() , NULL);

  if (util_entry_exists( full_path)) {
    char * parent_path = util_alloc_parent_path( full_path );

    if (copy_content)
      work_area->copy_directory_content(std::string(parent_path));
    else
      work_area->copy_directory(std::string(parent_path));

    free( full_path );
    free( parent_path );
    return true;
  } else {
    free( full_path );
    return false;
  }
}


TestArea::TestArea(const std::string& test_name, bool store_area) :
    store(store_area)
{
    char * prefix = test_work_area_alloc_prefix();
    unsigned int random_int;
    util_fread_dev_urandom( sizeof random_int, (char *) &random_int);
    random_int = random_int % 100000000;

#ifdef ERT_HAVE_GETUID
    uid_t uid = getuid();
    struct passwd * pw = getpwuid( uid );
    char * user_name = util_alloc_string_copy( pw->pw_name );
#else
    char * user_name =  util_alloc_sprintf("ert-test-%08u" , random_int);
#endif

    char * test_path = util_alloc_sprintf( TEST_PATH_FMT , user_name , test_name.c_str() , random_int);
    char * test_cwd = util_alloc_sprintf(FULL_PATH_FMT , prefix , test_path );
    util_make_path( test_cwd );

    {
        char * cwd_tmp = util_alloc_cwd();
        this->org_cwd = cwd_tmp;
        free(cwd_tmp);
    }
    this->cwd = test_cwd;
    if (util_chdir( this->cwd.c_str() ) != 0)
        util_abort("%s: Failed to move into temporary directory: %s", __func__, this->cwd.c_str());

    free( test_path );
    free( user_name );
    free( test_cwd );
    free( prefix );
}

TestArea::~TestArea() {
    if (!this->store)
        util_clear_directory( this->cwd.c_str() , true , true );

    util_chdir( this->org_cwd.c_str() );
}

const std::string& TestArea::test_cwd() const {
    return this->cwd;
}

const std::string& TestArea::original_cwd() const {
    return this->org_cwd;
}

std::string TestArea::original_path(const std::string& input_path) const {
  if (util_is_abs_path( input_path.c_str() ))
    return std::string(input_path);
  else {
    char * fname = util_alloc_filename( this->original_cwd().c_str(), input_path.c_str() , NULL);

    std::string return_string = std::string(fname);
    free(fname);

    return return_string;
  }
}


void TestArea::copy_file(const std::string& input_src_file) const {
  std::string src_file = this->original_path(input_src_file);

  if (util_file_exists( src_file.c_str() )) {
    char * target_name = util_split_alloc_filename( input_src_file.c_str() );
    char * target_file = util_alloc_filename( this->test_cwd().c_str() , target_name , NULL );
    util_copy_file( src_file.c_str(), target_file );
    free( target_file );
    free( target_name );
  }
}

void TestArea::copy_directory(const std::string input_directory) const {
  std::string src_directory = this->original_path(input_directory);
  util_copy_directory(src_directory.c_str() , this->test_cwd().c_str() );
}

void TestArea::copy_directory_content(const std::string input_directory) const {
  std::string src_directory = this->original_path(input_directory);
  util_copy_directory_content(src_directory.c_str() , this->test_cwd().c_str() );
}

bool TestArea::copy_parent(const std::string input_path) const {
  return test_work_area_copy_parent__(this, input_path, false);
}

bool TestArea::copy_parent_content(const std::string input_path) const {
  return test_work_area_copy_parent__(this, input_path, true);
}

}
}

/*****************************************************************/
/* C API */

test_work_area_type * test_work_area_alloc__(const char * test_name, bool store_area) {
  if (test_name)
    return new ecl::util::TestArea(test_name, store_area);
  else
    return NULL;
}


test_work_area_type * test_work_area_alloc(const char * test_name) {
  return test_work_area_alloc__(test_name, false);
}




void test_work_area_free(test_work_area_type * work_area) {
  delete work_area;
}


const char * test_work_area_get_cwd( const test_work_area_type * work_area ) {
  return work_area->test_cwd().c_str();
}


const char * test_work_area_get_original_cwd( const test_work_area_type * work_area ) {
  return work_area->original_cwd().c_str();
}


char * test_work_area_alloc_input_path( const test_work_area_type * work_area , const char * input_path ) {
  std::string relocated_input_path = work_area->original_path(std::string(input_path));
  return util_alloc_string_copy(relocated_input_path.c_str());
}




/**
   The point of this function is that the test code should be able to
   access the file @input_file independent of the fact that it has
   changed path. If @input_file is an absolute path the function will
   do nothing, if @input_file is a relative path the function will
   copy @input_file from the location relative to the original cwd to
   the corresponding location relative to the test cwd.
*/
void test_work_area_install_file( const test_work_area_type * work_area , const char * input_src_file ) {
  if (util_is_abs_path( input_src_file))
    return;
  else {
    std::string src_file = work_area->original_path(input_src_file);
    std::string src_path = ecl::util::path::dirname(input_src_file);

    if (!util_entry_exists( src_path.c_str() ))
      util_make_path( src_path.c_str() );

    if (util_file_exists( src_file.c_str() )) {
      char * target_file   = util_alloc_filename( work_area->test_cwd().c_str(), input_src_file, NULL );
      util_copy_file( src_file.c_str() , target_file );
      free( target_file );
    }
  }
}


void test_work_area_copy_file( const test_work_area_type * work_area , const char * input_file) {
  if (input_file)
    work_area->copy_file(std::string(input_file));
}


void test_work_area_copy_directory( const test_work_area_type * work_area , const char * input_directory) {
  work_area->copy_directory(std::string(input_directory));
}


void test_work_area_copy_directory_content( const test_work_area_type * work_area , const char * input_directory) {
  work_area->copy_directory_content(std::string(input_directory));
}


bool test_work_area_copy_parent_directory( const test_work_area_type * work_area , const char * input_path) {
  return work_area->copy_parent(std::string(input_path));
}


bool test_work_area_copy_parent_content( const test_work_area_type * work_area , const char * input_path) {
  return work_area->copy_parent_content(std::string(input_path));
}
