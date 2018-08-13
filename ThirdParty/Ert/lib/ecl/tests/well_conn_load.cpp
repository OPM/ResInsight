/*
   Copyright (C) 2013  Statoil ASA, Norway.

   The file 'well_conn_load.c' is part of ERT - Ensemble based Reservoir Tool.

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
#include <ert/util/stringlist.hpp>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.hpp>
#include <ert/ecl/ecl_file.hpp>
#include <ert/ecl/ecl_rsthead.hpp>
#include <ert/ecl/ecl_kw_magic.hpp>

#include <ert/ecl_well/well_conn.hpp>
#include <ert/ecl_well/well_conn_collection.hpp>
#include <ert/ecl_well/well_const.hpp>



int main(int argc , char ** argv) {
  const char * Xfile = argv[1];
  bool MSW;
  ecl_file_type * rst_file = ecl_file_open( Xfile , 0 );
  ecl_rsthead_type * rst_head = ecl_rsthead_alloc( ecl_file_get_global_view(rst_file) , ecl_util_filename_report_nr( Xfile ));

  test_install_SIGNALS();
  test_assert_true( util_sscanf_bool( argv[2] , &MSW ));
  test_assert_not_NULL( rst_file );
  test_assert_not_NULL( rst_head );

  {
    int iwell;
    const ecl_kw_type * iwel_kw = ecl_file_iget_named_kw( rst_file , IWEL_KW , 0 );
    const ecl_kw_type * icon_kw = ecl_file_iget_named_kw( rst_file , ICON_KW , 0 );
    ecl_kw_type * scon_kw = NULL;
    ecl_kw_type * xcon_kw = NULL;

    bool caseMSW = false;

    for (iwell = 0; iwell < rst_head->nwells; iwell++) {
      const int iwel_offset = rst_head->niwelz * iwell;
      int num_connections   = ecl_kw_iget_int( iwel_kw , iwel_offset + IWEL_CONNECTIONS_INDEX );
      int iconn;
      well_conn_collection_type * wellcc = well_conn_collection_alloc( );
      well_conn_collection_type * wellcc_ref = well_conn_collection_alloc();

      for (iconn = 0; iconn < num_connections; iconn++) {
        well_conn_type * conn = well_conn_alloc_from_kw( icon_kw , scon_kw , xcon_kw, rst_head , iwell , iconn );

        test_assert_true( well_conn_is_instance( conn ));
        test_assert_not_NULL( conn );
        if (!MSW)
          test_assert_bool_equal( well_conn_MSW( conn ) , MSW);
        else
          caseMSW |= well_conn_MSW( conn );

        well_conn_collection_add( wellcc , conn );
        well_conn_collection_add_ref( wellcc_ref , conn );
        test_assert_int_equal( iconn + 1 , well_conn_collection_get_size( wellcc ));
        test_assert_ptr_equal( well_conn_collection_iget_const( wellcc , iconn) , conn);
        test_assert_ptr_equal( well_conn_collection_iget_const( wellcc_ref , iconn) , conn);
      }
      well_conn_collection_free( wellcc_ref );
      {

        int i;
        for (i=0; i < well_conn_collection_get_size( wellcc ); i++)
          test_assert_true( well_conn_is_instance( well_conn_collection_iget_const( wellcc , i )));

      }
      {
        well_conn_collection_type * wellcc2 = well_conn_collection_alloc();
        int i;

        test_assert_int_equal( well_conn_collection_get_size( wellcc ) ,
                               well_conn_collection_load_from_kw( wellcc2 , iwel_kw , icon_kw , scon_kw, xcon_kw, iwell , rst_head));

        for (i=0; i < well_conn_collection_get_size( wellcc2 ); i++) {
          test_assert_true( well_conn_is_instance( well_conn_collection_iget_const( wellcc2 , i )));
          test_assert_true( well_conn_equal( well_conn_collection_iget_const( wellcc2 , i ) , well_conn_collection_iget_const( wellcc , i )));
        }
        well_conn_collection_free( wellcc2 );
      }
      well_conn_collection_free( wellcc );
    }
    test_assert_bool_equal( caseMSW , MSW);
  }


  exit( 0 );
}
