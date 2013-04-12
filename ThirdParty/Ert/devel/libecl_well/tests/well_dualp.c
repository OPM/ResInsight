/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'well_dualp.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/ecl/ecl_file.h>
#include <ert/ecl/ecl_util.h>
#include <ert/ecl/ecl_kw_magic.h>
#include <ert/ecl/ecl_rsthead.h>

#include <ert/ecl_well/well_conn.h>


void test_rstfile( const char * filename , bool fracture_connection) {
  ecl_file_type * rst_file = ecl_file_open( filename );
  const ecl_kw_type * iwel_kw     = ecl_file_iget_named_kw( rst_file , IWEL_KW , 0);
  ecl_rsthead_type * header       = ecl_rsthead_alloc( rst_file );
  
  well_conn_type * wellhead = well_conn_alloc_wellhead( iwel_kw , header , 0 );
  
  if (fracture_connection) {
    test_assert_true( well_conn_fracture_connection( wellhead ));
    test_assert_false( well_conn_matrix_connection( wellhead ));
  } else {
    test_assert_false( well_conn_fracture_connection( wellhead ));
    test_assert_true( well_conn_matrix_connection( wellhead ));
  }
  
  test_assert_true( well_conn_get_k( wellhead ) < header->nz );
  
  ecl_rsthead_free( header );
  well_conn_free( wellhead );
  ecl_file_close( rst_file );
}


int main(int argc , char ** argv) {
  const char * matrix_rstfile   = argv[1];
  const char * fracture_rstfile = argv[2];

  test_rstfile( fracture_rstfile , true);
  test_rstfile( matrix_rstfile , false );
  
  exit(0);
}
