/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ert_util_file_readable.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ert/util/util.h>
#include <ert/util/test_util.hpp>


void assert_equal( bool equal ) {
  if (!equal)
    exit(1);
}




int main(int argc , char ** argv) {
  test_assert_true( util_file_readable( argv[0] ));
  {
      char * path;
      util_alloc_file_components( argv[0] , &path , NULL , NULL);
      test_assert_false( util_file_readable( path ));
      free( path );
  }
  {
    const char * file = "/tmp/test_file.txt";
    mode_t mode = 0;
    FILE * stream = util_fopen(file , "w");
    fclose( stream );

    chmod(file , mode);
    test_assert_false( util_file_readable( file));
    unlink( file );
  }
  exit(0);
}
