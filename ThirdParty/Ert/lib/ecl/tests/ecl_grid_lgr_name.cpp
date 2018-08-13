/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_grid_lgr_name.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/test_util.hpp>

#include <ert/ecl/ecl_grid.hpp>

/*
  Name ..................: LG003017
  Grid nr ...............: 104

  Name ..................: LG006024
  Grid nr ...............: 2

  Name ..................: LG005025
  Grid nr ...............: 4

  Name ..................: LG011029
  Grid nr ...............: 82

  Name ..................: LG007021
  Grid nr ...............: 100

  Name ..................: LG003014
  Grid nr ...............: 110

  Name ..................: /private/joaho/ERT/git/ert/test-data/Statoil/ECLIPSE/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3.EGRID
*/


void test_name(const ecl_grid_type * grid , int lgr_nr , const char * name) {
  test_assert_string_equal( name , ecl_grid_get_lgr_name( grid , lgr_nr ));
  test_assert_int_equal( lgr_nr , ecl_grid_get_lgr_nr_from_name( grid , name ));
}


int main(int argc , char ** argv) {
  const char * grid_file = argv[1];
  ecl_grid_type * grid = ecl_grid_alloc( grid_file );

  test_name( grid , 104 , "LG003017");
  test_name( grid ,   2 , "LG006024");
  test_name( grid ,   4 , "LG005025");
  test_name( grid ,  82 , "LG011029");
  test_name( grid , 100 , "LG007021");
  test_name( grid , 110 , "LG003014");
  test_name( grid ,   0 , grid_file);


  ecl_grid_free( grid );
  exit(0);
}
