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

#include <ert/util/test_util.h>
#include <ert/util/stringlist.h>
#include <ert/util/util.h>

#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_rsthead.h>
#include <ert/ecl/ecl_kw_magic.h>

#include <ert/ecl_well/well_conn.h>
#include <ert/ecl_well/well_conn_collection.h>
#include <ert/ecl_well/well_const.h>


void well_conn_test_CF( const ecl_kw_type * iwel_kw , const ecl_kw_type * icon_kw , const ecl_kw_type * scon_kw, const ecl_kw_type * xcon_kw , const const ecl_rsthead_type * rst_head , int iwell , int iconn, double CF) {
  well_conn_type * conn = well_conn_alloc_from_kw( icon_kw , scon_kw, xcon_kw , rst_head , iwell , iconn );
  test_assert_double_equal( CF , well_conn_get_connection_factor(conn));
  well_conn_free( conn );
}





int main(int argc , char ** argv) {
  const char * Xfile = argv[1];
  ecl_file_type * rst_file = ecl_file_open( Xfile , 0 );
  ecl_rsthead_type * rst_head = ecl_rsthead_alloc( ecl_file_get_global_view( rst_file ) , ecl_util_filename_report_nr(Xfile));
  const ecl_kw_type * iwel_kw = ecl_file_iget_named_kw( rst_file , IWEL_KW , 0 );
  const ecl_kw_type * icon_kw = ecl_file_iget_named_kw( rst_file , ICON_KW , 0 );
  const ecl_kw_type * scon_kw = ecl_file_iget_named_kw( rst_file , SCON_KW , 0 );
  const ecl_kw_type * xcon_kw = 0;

  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw, rst_head , 0 , 0 , 32.948 );
  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw , rst_head , 0 , 1 , 46.825 );
  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw , rst_head , 0 , 2 , 51.867 );

  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw , rst_head , 1 , 0 ,  1.168 );
  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw , rst_head , 1 , 1 , 15.071 );
  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw , rst_head , 1 , 2 ,  6.242 );

  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw , rst_head , 2 , 0 , 27.412 );
  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw , rst_head , 2 , 1 , 55.195 );
  well_conn_test_CF(iwel_kw , icon_kw , scon_kw, xcon_kw , rst_head , 2 , 2 , 18.032 );

  ecl_file_close( rst_file );
  ecl_rsthead_free( rst_head );
  exit( 0 );
}

