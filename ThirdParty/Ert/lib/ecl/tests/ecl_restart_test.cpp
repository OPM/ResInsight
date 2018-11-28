/*
   Copyright (C) 2012  Statoil ASA, Norway.

   The file 'ecl_restart_test.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/util.h>

#include <ert/ecl/ecl_file.hpp>


bool test_get( ecl_file_type * rst_file , int day , int month , int year , int expected_index) {
  time_t sim_time = ecl_util_make_date(day,month,year);
  int seqnum_index = ecl_file_get_restart_index( rst_file , sim_time );
  if (seqnum_index == expected_index)
    return true;
  else {
    printf("ERROR:  Date: %02d/%02d/%4d   Got:%d  Expected:%d \n",day , month , year , seqnum_index , expected_index);
    return false;
  }
}



int main(int argc , char ** argv) {
  bool OK = true;
  const char * unrst_file = argv[1];

  ecl_file_type * rst_file = ecl_file_open( unrst_file , 0);

  OK = OK && test_get( rst_file , 1 , 1 , 1998 , -1 );
  OK = OK && test_get( rst_file , 17 , 9 , 2003 , -1 );
  OK = OK && test_get( rst_file , 1 , 1 , 2008 , -1 );

  OK = OK && test_get( rst_file , 1 , 1 , 2000 , 0 );
  OK = OK && test_get( rst_file , 1 , 10 , 2000 , 10 );
  OK = OK && test_get( rst_file , 1 , 3 , 2003 , 40 );
  OK = OK && test_get( rst_file , 31 , 12 , 2004 , 62 );


  if (OK)
    exit(0);
  else
    exit(1);
}
