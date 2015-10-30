/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'job_queue_test.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>


/*
  This is a small test job used by the job_queue_stress_test. The job
  does the following:

    0. Chadir to runpath
    1. Create the file @runfile.
    2. Wait with usleep( @usleep_time ).
    3. Remove the @runfile.
    4. Create new file @OK_file
    5. exit.
*/

int main(int argc, char ** argv) {
  const char * runpath = argv[1];
  const char * runfile = argv[2];
  const char * OK_file = argv[3];

  int usleep_time;

  util_chdir( runpath );
  util_sscanf_int( argv[4] , &usleep_time );
  {
    FILE * stream = util_fopen( runfile , "w");
    fprintf(stream , "Running ... \n");
    fclose( stream );
  }
  usleep( usleep_time );
  util_unlink_existing(runfile);
  {
    FILE * stream = util_fopen( OK_file , "w");
    fprintf(stream , "OK ... \n");
    fclose( stream );
  }
}
