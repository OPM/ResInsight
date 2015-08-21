/*
   Copyright (C) 2014  Statoil ASA, Norway.

   The file 'ext_joblist_test.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <stdbool.h>
#include <ert/util/test_util.h>
#include <ert/job_queue/ext_joblist.h>

void load_job_directory(ext_joblist_type * joblist , const char * path, const char * license_root_path) {
  bool user_mode = false;
  ext_joblist_add_jobs_in_directory(joblist  , path, license_root_path, user_mode, true );
  test_assert_true( ext_joblist_has_job(joblist, "SYMLINK"));
}

int main( int argc , char ** argv) {
    int status = 0;
    ext_joblist_type * joblist = ext_joblist_alloc();
    load_job_directory(joblist , argv[1], argv[2] );
    ext_joblist_free(joblist);
    exit( status );
}
