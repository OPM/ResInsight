/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'ecl_kw_grdecl.c' is part of ERT - Ensemble based Reservoir Tool.

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

#include <ert/util/test_util.hpp>
#include <ert/util/util.h>
#include <ert/util/test_work_area.hpp>

#include <ert/ecl/ecl_kw.hpp>

int main(int argc , char ** argv) {
  int i;
  ecl_kw_type * ecl_kw = ecl_kw_alloc("HEAD" , 10  , ECL_INT);

  for (i=0; i < 10; i++)
    ecl_kw_iset_int(ecl_kw , i , i );

  {
    test_work_area_type * work_area = test_work_area_alloc("ecl_kw_grdecl");
    FILE * stream = util_fopen( "FILE.grdecl" , "w");

    ecl_kw_fprintf_grdecl(ecl_kw , stream );
    fclose(stream);

    stream = util_fopen( "FILE.grdecl" , "r");
    {
      ecl_kw_type * ecl_kw2 = ecl_kw_fscanf_alloc_grdecl( stream , "HEAD" , 10 , ECL_INT);

      test_assert_not_NULL( ecl_kw2 );
      test_assert_true( ecl_kw_equal( ecl_kw , ecl_kw2));
      ecl_kw_free( ecl_kw2 );
    }
    fclose( stream );

    stream = util_fopen( "FILE.grdecl" , "w");
    ecl_kw_fprintf_grdecl__(ecl_kw , "HEAD1234" , stream );
    fclose( stream );

    stream = util_fopen( "FILE.grdecl" , "r");
    {
      ecl_kw_type * ecl_kw2 = ecl_kw_fscanf_alloc_grdecl( stream , "HEAD" , 10 , ECL_INT);

      test_assert_NULL( ecl_kw2 );
      ecl_kw2 = ecl_kw_fscanf_alloc_grdecl( stream , "HEAD1234" , 10 , ECL_INT);
      test_assert_not_NULL( ecl_kw2 );

      test_assert_string_equal( ecl_kw_get_header( ecl_kw2 ) , "HEAD1234" );
      test_assert_true( ecl_kw_content_equal( ecl_kw , ecl_kw2 ));
      ecl_kw_free( ecl_kw2 );
    }
    fclose( stream );
    test_work_area_free( work_area );
  }
  ecl_kw_free( ecl_kw );

  exit(0);
}
