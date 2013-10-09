/*
   Copyright (C) 2013  Statoil ASA, Norway. 
    
   The file 'plot_plplot.c' is part of ERT - Ensemble based Reservoir Tool. 
    
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

#include <ert/util/test_util.h>
#include <ert/util/arg_pack.h>

#include <ert/plot/plot.h>
#include <ert/plot/plplot_driver.h>



void test_create_PLPLOT_driver() {
  arg_pack_type * arg = arg_pack_alloc();
  arg_pack_append_ptr( arg , "FILE.png" );
  arg_pack_append_ptr( arg , "png");
  {
    plot_driver_type * plplot_driver = plplot_driver_alloc( arg );
  
    plplot_close_driver( plplot_driver );
  }
  arg_pack_free( arg );
}


void test_create_PLPLOT_driver_invalid_arg() {

  test_assert_NULL( plplot_driver_alloc( NULL ));
  test_assert_NULL( plplot_driver_alloc( "Invalid" ));
  
  {
    arg_pack_type * arg = arg_pack_alloc();
    test_assert_NULL( plplot_driver_alloc( arg ));

    arg_pack_append_int( arg , 1 );
    arg_pack_append_int( arg , 1 );
    test_assert_NULL( plplot_driver_alloc( arg ));
    arg_pack_free( arg );
  }
}





void test_create_PLPLOT_invalid_arg( ) {
  plot_type * plot = plot_alloc("PLPLOT" , NULL , false , false);
  
  test_assert_NULL( plot );
}


void test_create_PLPLOT( ) {
  plot_type * plot;
  arg_pack_type * arg = arg_pack_alloc();
  arg_pack_append_ptr( arg , "FILE.png" );
  arg_pack_append_ptr( arg , "png");
  
  plot = plot_alloc("PLPLOT" , arg , false , false);
  
  test_assert_true( plot_is_instance( plot  ));
  plot_free( plot );
  arg_pack_free( arg );
}



int main(int argc , char ** argv) {

  test_create_PLPLOT_driver();
  test_create_PLPLOT_driver_invalid_arg();
  
  test_create_PLPLOT();
  test_create_PLPLOT_invalid_arg();

  exit(0);
}
