/* 
 * File:   ert_util_abort_gnu_tests.c
 * Author: kflik
 *
 * Created on August 16, 2013, 9:41 AM
 */

#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/wait.h>

#include <ert/util/util.h>
#include <ert/util/test_util.h>

/*
 * 
 */
int main(int argc, char** argv) {

  char * filename = util_alloc_dump_filename();
  unsetenv( "ERT_SHOW_BACKTRACE");
  if (util_file_exists(filename)) {
    remove(filename);
  }
  
  test_assert_false(util_file_exists(filename));

  pid_t child_pid = fork();

  if (child_pid == 0) {
    util_abort("I was terminated with the util_abort function");
  } else {
    waitpid(child_pid, NULL, 0);
    test_assert_true(util_file_exists(filename));
    free(filename);
  }

  return (EXIT_SUCCESS);
}

